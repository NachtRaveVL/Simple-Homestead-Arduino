/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Actuators
*/

#include "TerraActuators.h"
#include "TerraUtils.h"
#include "TerraCoreLogic.h"

TerraActuator::TerraActuator(Terra_ActuatorType actuatorType, uint32_t key, const TerraString &name)
    : TerraObject(Terra_ObjectType_Actuator, key, name), _actuatorType(actuatorType),
      _activation(), _driver(nullptr), _ownsDriver(false), _writeCallback(nullptr),
      _enableMode(Terra_EnableMode_Highest), _writeContext(nullptr) { }

TerraActuator::~TerraActuator() {
    if (_ownsDriver) delete _driver;
}

void TerraActuator::setDriver(TerraOutputDriver *driver, bool takeOwnership) {
    if (_driver != driver && _ownsDriver) delete _driver;
    _driver = driver;
    _ownsDriver = driver && takeOwnership;
    if (_driver) _driver->begin();
}

void TerraActuator::setWriteCallback(TerraWriteCallback callback, void *context) {
    _writeCallback = callback;
    _writeContext = context;
}

void TerraActuator::setEnabled(bool enabled) {
    if (!enabled) off();
    TerraObject::setEnabled(enabled);
}

void TerraActuator::setOutput(float intensity, uint32_t durationMs, uint32_t now) {
    if (!_enabled || _fault) return;
    intensity = terraClamp(intensity, 0.0f, 1.0f);
    if (intensity <= TERRA_EPSILON) {
        off();
        return;
    }
    _activation.activate(intensity, durationMs, now);
    if (_driver) _driver->write(_activation.getIntensity());
    else if (_writeCallback) _writeCallback(_writeContext, _activation.getIntensity());
}

void TerraActuator::setOutputRequests(const float *requests, uint8_t count, uint32_t durationMs, uint32_t now) {
    const float resolved = terraResolveActuatorRequests(requests, count, _enableMode);
    setOutput(terraClamp(resolved, 0.0f, 1.0f), durationMs, now);
}

void TerraActuator::off() {
    bool wasActive = _activation.isActive();
    _activation.deactivate();
    if (wasActive) {
        if (_driver) _driver->write(0.0f);
        else if (_writeCallback) _writeCallback(_writeContext, 0.0f);
    }
}

void TerraActuator::update(uint32_t now) {
    bool wasActive = _activation.isActive();
    _activation.update(now);
    if (wasActive && !_activation.isActive()) {
        if (_driver) _driver->write(0.0f);
        else if (_writeCallback) _writeCallback(_writeContext, 0.0f);
    }
}

void TerraPump::setOutput(float intensity, uint32_t durationMs, uint32_t now) {
    bool wasActive = isActive();
    TerraActuator::setOutput(intensity, durationMs, now);
    if (!wasActive && isActive()) _startedAt = now;
}

void TerraPump::update(uint32_t now) {
    TerraActuator::update(now);
    if (isActive() && _maxContinuousMs && terraElapsed(now, _startedAt, _maxContinuousMs)) {
        off();
        setFault(TerraString("maximum continuous runtime exceeded"));
    }
}

TerraSumpPump::TerraSumpPump(uint32_t key, const TerraString &name)
    : TerraPump(key, name), _startLevelPercent(TERRA_SUMP_START_LEVEL_PERCENT),
      _stopLevelPercent(TERRA_SUMP_STOP_LEVEL_PERCENT), _alarmLevelPercent(TERRA_SUMP_ALARM_LEVEL_PERCENT),
      _lastLevelPercent(0.0f), _levelValid(false), _highWaterAlarm(false)
{
    _actuatorType = Terra_ActuatorType_SumpPump;
}

bool TerraSumpPump::configureLevels(float startPercent, float stopPercent, float alarmPercent)
{
    if (stopPercent < 0.0f || stopPercent >= startPercent || startPercent >= alarmPercent || alarmPercent > 100.0f) return false;
    _startLevelPercent = startPercent;
    _stopLevelPercent = stopPercent;
    _alarmLevelPercent = alarmPercent;
    return true;
}

bool TerraSumpPump::updateLevel(float levelPercent, bool valid, uint32_t now)
{
    TerraPump::update(now);
    if (hasFault() && getFaultMessage() != TerraString("sump level invalid")) {
        off();
        return false;
    }

    if (!valid || isnan(levelPercent)) {
        _levelValid = false;
        _highWaterAlarm = false;
        off();
        setFault(TerraString("sump level invalid"));
        return false;
    }

    if (hasFault() && getFaultMessage() == TerraString("sump level invalid")) clearFault();
    _lastLevelPercent = terraClamp(levelPercent, 0.0f, 100.0f);
    _levelValid = true;
    _highWaterAlarm = _lastLevelPercent >= _alarmLevelPercent;

    if (isActive()) {
        if (_lastLevelPercent <= _stopLevelPercent) off();
    } else if (_lastLevelPercent >= _startLevelPercent) {
        setOutput(1.0f, 0, now);
    }
    return isActive();
}
