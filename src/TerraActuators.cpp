/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Actuators
*/

#include "Terraduino.h"
#include "TerraSensors.h"
#include "TerraUtils.h"

TerraActuator::TerraActuator(Terra_ActuatorType actuatorType, uint32_t key, const TerraString &name)
    : TerraObject(Terra_ObjectType_Actuator, key, name),
      _actuatorType(actuatorType), _driver(), _enableMode(Terra_EnableMode_Highest),
      _handles(), _directActivation(this), _output(0.0f), _needsUpdate(false)
{ ; }

TerraActuator::~TerraActuator()
{
    _directActivation.unset();
    for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS; ++index) {
        if (_handles[index]) _handles[index]->actuator = nullptr;
    }
    applyOutput(0.0f);
}

void TerraActuator::setDriver(const SharedPtr<TerraOutputDriver> &driver)
{
    _driver = driver;
    if (_driver) { _driver->begin(); }
    applyOutput(_output);
}

void TerraActuator::setEnabled(bool enabled)
{
    _enabled = enabled;
    setNeedsUpdate();
    resolveActivations();
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
    intensity = constrain(intensity, 0.0f, 1.0f);
    if (isFPEqual(intensity, _output)) { return; }
    _output = intensity;
    if (_driver) { _driver->write(_output); }
}

void TerraActuator::resolveActivations(uint32_t now)
{
    bool forced = false;
    uint8_t activeCount = 0;
    for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS && _handles[index]; ++index) {
        if (_handles[index]->isValid() && !_handles[index]->isDone()) {
            forced = forced || _handles[index]->isForced();
            ++activeCount;
        }
    }

    bool canEnable = activeCount && (forced || getCanEnable());
    if (!canEnable) {
        for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS && _handles[index]; ++index) {
            if (_handles[index]->checkTime) { _handles[index]->checkTime = 0; }
        }
        applyOutput(0.0f);
        _needsUpdate = false;
        return;
    }

    float drivingIntensity = 0.0f;
    uint8_t handleCount = 0;

    switch (_enableMode) {
        case Terra_EnableMode_Highest: {
            drivingIntensity = -__FLT_MAX__;
            for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS && _handles[index]; ++index) {
                if (_handles[index]->isValid() && !_handles[index]->isDone()) {
                    float handleIntensity = _handles[index]->getDriveIntensity();
                    if (handleIntensity > drivingIntensity) { drivingIntensity = handleIntensity; }
                    ++handleCount;
                }
            }
        } break;

        case Terra_EnableMode_Lowest: {
            drivingIntensity = __FLT_MAX__;
            for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS && _handles[index]; ++index) {
                if (_handles[index]->isValid() && !_handles[index]->isDone()) {
                    float handleIntensity = _handles[index]->getDriveIntensity();
                    if (handleIntensity < drivingIntensity) { drivingIntensity = handleIntensity; }
                    ++handleCount;
                }
            }
        } break;

        case Terra_EnableMode_Average: {
            for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS && _handles[index]; ++index) {
                if (_handles[index]->isValid() && !_handles[index]->isDone()) {
                    drivingIntensity += _handles[index]->getDriveIntensity();
                    ++handleCount;
                }
            }
            if (handleCount) { drivingIntensity /= handleCount; }
        } break;

        case Terra_EnableMode_Multiply: {
            bool started = false;
            for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS && _handles[index]; ++index) {
                if (_handles[index]->isValid() && !_handles[index]->isDone()) {
                    if (!started) {
                        drivingIntensity = _handles[index]->getDriveIntensity();
                        started = true;
                    } else {
                        drivingIntensity *= _handles[index]->getDriveIntensity();
                    }
                    ++handleCount;
                }
            }
        } break;

        case Terra_EnableMode_InOrder: {
            for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS && _handles[index]; ++index) {
                if (_handles[index]->isValid() && !_handles[index]->isDone()) {
                    drivingIntensity = _handles[index]->getDriveIntensity();
                    handleCount = 1;
                    break;
                }
            }
        } break;

        case Terra_EnableMode_RevOrder: {
            uint8_t count = 0;
            while (count < TERRA_MAX_ATTACHMENTS && _handles[count]) { ++count; }
            while (count) {
                TerraActivationHandle *handle = _handles[--count];
                if (handle->isValid() && !handle->isDone()) {
                    drivingIntensity = handle->getDriveIntensity();
                    handleCount = 1;
                    break;
                }
            }
        } break;

        case Terra_EnableMode_Undefined:
        case Terra_EnableMode_Count:
        default:
            break;
    }

    switch (_enableMode) {
        case Terra_EnableMode_InOrder: {
            bool selected = false;
            for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS && _handles[index]; ++index) {
                TerraActivationHandle *handle = _handles[index];
                if (!selected && handle->isValid() && !handle->isDone() &&
                    isFPEqual(handle->getDriveIntensity(), drivingIntensity)) {
                    selected = true;
                    if (!handle->checkTime) { handle->checkTime = now; }
                } else if (handle->checkTime) {
                    handle->checkTime = 0;
                }
            }
        } break;

        case Terra_EnableMode_RevOrder: {
            uint8_t count = 0;
            while (count < TERRA_MAX_ATTACHMENTS && _handles[count]) { ++count; }
            bool selected = false;
            while (count) {
                TerraActivationHandle *handle = _handles[--count];
                if (!selected && handle->isValid() && !handle->isDone() &&
                    isFPEqual(handle->getDriveIntensity(), drivingIntensity)) {
                    selected = true;
                    if (!handle->checkTime) { handle->checkTime = now; }
                } else if (handle->checkTime) {
                    handle->checkTime = 0;
                }
            }
        } break;

        default: {
            for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS && _handles[index]; ++index) {
                TerraActivationHandle *handle = _handles[index];
                if (handle->isValid() && !handle->isDone() && !handle->checkTime) {
                    handle->checkTime = now;
                }
            }
        } break;
    }

    applyOutput(handleCount ? drivingIntensity : 0.0f);
    _needsUpdate = false;
}

void TerraActuator::setOutput(float intensity, millis_t duration, uint32_t now)
{
    _directActivation.setup(intensity, duration);
    if (intensity > FLT_EPSILON) {
        _directActivation.enable();
        resolveActivations(now);
    } else {
        _directActivation.unset();
        resolveActivations(now);
    }
}

void TerraActuator::off()
{
    _directActivation.unset();
    resolveActivations();
}

void TerraActuator::update(uint32_t now)
{
    for (uint8_t index = 0; index < TERRA_MAX_ATTACHMENTS && _handles[index];) {
        TerraActivationHandle *handle = _handles[index];
        if (handle->isActive()) { handle->elapseTo(now); }
        if (handle->actuator != this || !handle->isValid() || handle->isDone()) {
            if (handle->actuator == this) { handle->actuator = nullptr; }
            removeActivationHandle(handle);
            continue;
        }
        ++index;
    }
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

    if (isActive() && _maxContinuousMs && _startedAt && (uint32_t)(now - _startedAt) >= _maxContinuousMs) {
        while (_handles[0]) { _handles[0]->unset(); }
        setFault(TerraString("maximum continuous runtime exceeded"));
        resolveActivations(now);
    }
}

TerraSumpPump::TerraSumpPump(uint32_t key, const TerraString &name)
    : TerraPump(key, name), _levelSensor(this), _startLevelPercent(TERRA_SUMP_START_LEVEL_PERCENT),
      _stopLevelPercent(TERRA_SUMP_STOP_LEVEL_PERCENT), _alarmLevelPercent(TERRA_SUMP_ALARM_LEVEL_PERCENT),
      _lastLevelPercent(0.0f), _levelValid(false), _highWaterAlarm(false)
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
            off();
            setFault(TerraString("sump level invalid"));
        } else {
            if (hasFault() && getFaultMessage() == TerraString("sump level invalid")) clearFault();
            _lastLevelPercent = constrain(level.value, 0.0f, 100.0f);
            _levelValid = true;
            _highWaterAlarm = _lastLevelPercent >= _alarmLevelPercent;

            if (isActive()) {
                if (_lastLevelPercent <= _stopLevelPercent) { off(); }
            } else if (_lastLevelPercent >= _startLevelPercent) {
                setOutput(1.0f, (millis_t)-1, now);
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
