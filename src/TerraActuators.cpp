/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Actuators
*/

#include "TerraActuators.h"
#include "TerraSensors.h"
#include "TerraUtils.h"

TerraActuator::TerraActuator(Terra_ActuatorType actuatorType, uint32_t key, const TerraString &name)
    : TerraObject(Terra_ObjectType_Actuator, key, name),
      _actuatorType(actuatorType), _driver(), _enableMode(Terra_EnableMode_Highest),
      _needsUpdate(false), _handles(), _output(0.0f)
{ ; }

TerraActuator::~TerraActuator()
{
    for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS; ++index) {
        if (_handles[index] && _handles[index]->actuator == this) {
            _handles[index]->actuator = nullptr;
            _handles[index]->checkTime = 0;
        }
    }
    applyOutput(0.0f);
}

void TerraActuator::setDriver(const SharedPtr<TerraOutputDriver> &driver)
{
    _driver = driver;
    if (_driver) { _driver->begin(); }
    setNeedsUpdate();
}

void TerraActuator::setEnabled(bool enabled)
{
    if (_enabled != enabled) {
        _enabled = enabled;
        setNeedsUpdate();
    }
}

bool TerraActuator::addActivationHandle(TerraActivationHandle *handle)
{
    if (!handle) return false;
    for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS; ++index) {
        if (_handles[index] == handle) return true;
        if (!_handles[index]) {
            _handles[index] = handle;
            setNeedsUpdate();
            return true;
        }
    }
    return false;
}

bool TerraActuator::removeActivationHandle(TerraActivationHandle *handle)
{
    for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS; ++index) {
        if (_handles[index] != handle) continue;
        for (uint8_t subIndex = index; subIndex + 1 < TERRA_MAX_ATTACHMENTS; ++subIndex) {
            _handles[subIndex] = _handles[subIndex + 1];
        }
        _handles[TERRA_MAX_ATTACHMENTS - 1] = nullptr;
        setNeedsUpdate();
        return true;
    }
    return false;
}

void TerraActuator::applyOutput(float intensity)
{
    intensity = (_enabled && !_fault) ? terraClamp(intensity, 0.0f, 1.0f) : 0.0f;
    if (isFPEqual(intensity, _output)) { return; }
    _output = intensity;
    if (_driver) { _driver->write(_output); }
}

void TerraActuator::resolveActivations(uint32_t now)
{
    if (!now) { now = 1; }
    float requests[TERRA_MAX_ATTACHMENTS];
    TerraActivationHandle *requestHandles[TERRA_MAX_ATTACHMENTS];
    uint8_t count = 0;

    for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS && _handles[index]; ) {
        TerraActivationHandle *handle = _handles[index];

        if (isActive() && handle->isActive()) { handle->elapseTo(now); }

        if (handle->actuator != this || !handle->isValid() || handle->isDone()) {
            if (handle->actuator == this) {
                handle->actuator = nullptr;
                handle->checkTime = 0;
            }
            removeActivationHandle(handle);
            continue;
        }

        requests[count] = handle->getDriveIntensity();
        requestHandles[count] = handle;
        ++count;
        ++index;
    }

    float resolved = 0.0f;
    int selectedIndex = -1;

    if (count) {
        resolved = requests[0];
        switch (_enableMode) {
            case Terra_EnableMode_Highest:
                for (uint8_t index = 1; index < count; ++index) {
                    if (requests[index] > resolved) { resolved = requests[index]; }
                }
                break;

            case Terra_EnableMode_Lowest:
                for (uint8_t index = 1; index < count; ++index) {
                    if (requests[index] < resolved) { resolved = requests[index]; }
                }
                break;

            case Terra_EnableMode_Average:
                resolved = 0.0f;
                for (uint8_t index = 0; index < count; ++index) { resolved += requests[index]; }
                resolved /= count;
                break;

            case Terra_EnableMode_Multiply:
                for (uint8_t index = 1; index < count; ++index) { resolved *= requests[index]; }
                break;

            case Terra_EnableMode_InOrder:
                selectedIndex = 0;
                break;

            case Terra_EnableMode_RevOrder:
                selectedIndex = count - 1;
                resolved = requests[selectedIndex];
                break;

            default:
                resolved = 0.0f;
                break;
        }
    }

    if (_enableMode == Terra_EnableMode_InOrder || _enableMode == Terra_EnableMode_RevOrder) {
        for (uint8_t index = 0; index < count; ++index) {
            requestHandles[index]->checkTime = (int)index == selectedIndex ? now : 0;
        }
    } else {
        for (uint8_t index = 0; index < count; ++index) {
            if (!requestHandles[index]->checkTime) { requestHandles[index]->checkTime = now; }
        }
    }

    applyOutput(resolved);
    _needsUpdate = false;
}

void TerraActuator::update(uint32_t now)
{
    TerraObject::update(now);
    resolveActivations(now);
}

TerraPump::TerraPump(uint32_t key, const TerraString &name)
    : TerraActuator(Terra_ActuatorType_Pump, key, name),
      _maxContinuousMs(0), _startedAt(0)
{ ; }

void TerraPump::update(uint32_t now)
{
    TerraActuator::update(now);
    if (isActive() && !_startedAt) { _startedAt = now; }
    if (!isActive()) { _startedAt = 0; }

    if (isActive() && _maxContinuousMs && _startedAt && terraElapsed(now, _startedAt, _maxContinuousMs)) {
        while (_handles[0]) { _handles[0]->unset(); }
        setFault(TerraString("maximum continuous runtime exceeded"));
        resolveActivations(now);
    }
}

TerraSumpPump::TerraSumpPump(uint32_t key, const TerraString &name)
    : TerraPump(key, name), _levelSensor(this), _startLevelPercent(TERRA_SUMP_START_LEVEL_PERCENT),
      _stopLevelPercent(TERRA_SUMP_STOP_LEVEL_PERCENT), _alarmLevelPercent(TERRA_SUMP_ALARM_LEVEL_PERCENT),
      _lastLevelPercent(0.0f), _levelValid(false), _highWaterAlarm(false), _levelActivation()
{
    _actuatorType = Terra_ActuatorType_SumpPump;
}

bool TerraSumpPump::configureLevels(float startPercent, float stopPercent, float alarmPercent)
{
    if (stopPercent < 0.0f || stopPercent >= startPercent ||
        startPercent >= alarmPercent || alarmPercent > 100.0f) return false;
    _startLevelPercent = startPercent;
    _stopLevelPercent = stopPercent;
    _alarmLevelPercent = alarmPercent;
    return true;
}

void TerraSumpPump::update(uint32_t now)
{
    if (_levelSensor.isSet()) {
        TerraMeasurement level = _levelSensor.getMeasurement(now, true);
        if (!level.valid || level.unit != Terra_Unit_Percent) {
            _levelValid = false;
            _highWaterAlarm = false;
            _levelActivation.unset();
            setFault(TerraString("sump level invalid"));
        } else {
            if (hasFault() && getFaultMessage() == TerraString("sump level invalid")) clearFault();
            _lastLevelPercent = terraClamp(level.value, 0.0f, 100.0f);
            _levelValid = true;
            _highWaterAlarm = _lastLevelPercent >= _alarmLevelPercent;

            if (_levelActivation.actuator) {
                if (_lastLevelPercent <= _stopLevelPercent) { _levelActivation.unset(); }
            } else if (_lastLevelPercent >= _startLevelPercent) {
                _levelActivation.activation = TerraActivation(1.0f, (uint32_t)-1);
                _levelActivation = this;
            }
        }
    }
    TerraPump::update(now);
}

void TerraSumpPump::unresolveAny(TerraObject *object)
{
    _levelSensor.unresolveAny(object);
    TerraPump::unresolveAny(object);
}
