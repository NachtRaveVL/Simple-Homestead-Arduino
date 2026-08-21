/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Activation
*/

#include "Terraduino.h"
#include "TerraActuators.h"
#include "TerraUtils.h"

TerraActivationHandle::TerraActivationHandle(TerraActuator *actuator, float intensity, uint32_t durationMs)
    : _actuator(nullptr), _activation(constrain(intensity, 0.0f, 1.0f), durationMs),
      _startedAt(0), _active(false)
{
    setActuator(actuator);
}

TerraActivationHandle::TerraActivationHandle(const TerraActivationHandle &other)
    : _actuator(nullptr), _activation(other._activation), _startedAt(0), _active(false)
{ }

TerraActivationHandle::~TerraActivationHandle()
{
    unset();
}

void TerraActivationHandle::setActuator(TerraActuator *actuator)
{
    if (_actuator == actuator) return;
    unset();
    _actuator = actuator;
}

void TerraActivationHandle::setup(float intensity, uint32_t durationMs)
{
    _activation._intensity = constrain(intensity, 0.0f, 1.0f);
    _activation._durationMs = durationMs;
    if (_actuator && _active) _actuator->resolveActivations();
}

void TerraActivationHandle::enable(uint32_t now)
{
    if (!_actuator || _activation.getIntensity() <= FLT_EPSILON) {
        unset();
        return;
    }

    if (!_active) {
        if (!_actuator->addActivationHandle(this)) return;
        _active = true;
    }
    _startedAt = now;
    _actuator->resolveActivations();
}

void TerraActivationHandle::unset()
{
    if (!_actuator || !_active) {
        _active = false;
        return;
    }

    TerraActuator *actuator = _actuator;
    _active = false;
    actuator->removeActivationHandle(this);
    actuator->resolveActivations();
}

void TerraActivationHandle::update(uint32_t now)
{
    if (_active && _activation.getDurationMs() &&
        (uint32_t)(now - _startedAt) >= _activation.getDurationMs()) {
        unset();
    }
}
