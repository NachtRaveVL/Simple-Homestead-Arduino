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
