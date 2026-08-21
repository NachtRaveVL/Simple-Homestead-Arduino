/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Attachments
*/

#include "Terraduino.h"

SharedPtr<TerraObject> terraObjectByKey(uint32_t key)
{
    return getController() ? getController()->sharedObjectByKey(key) : SharedPtr<TerraObject>();
}

TerraMeasurement TerraSensorAttachment::getMeasurement(uint32_t now, bool poll)
{
    SharedPtr<TerraSensor> sensor = getObject();
    if (!sensor) return TerraMeasurement();
    if (poll) sensor->update(now);
    _measurement = sensor->getMeasurement();
    return _measurement;
}

TerraActuatorAttachment::TerraActuatorAttachment(TerraObject *parent)
    : TerraAttachment<TerraActuator>(parent), _actHandle(), _actSetup()
{ }

TerraActuatorAttachment::~TerraActuatorAttachment()
{
    disableActivation();
}

void TerraActuatorAttachment::setupActivation(float intensity, uint32_t duration)
{
    _actSetup = TerraActivation(intensity, duration);
    _actHandle.activation = _actSetup;
    if (_actHandle.actuator) { _actHandle.actuator->setNeedsUpdate(); }
}

void TerraActuatorAttachment::enableActivation()
{
    if (!_actHandle.actuator && _actSetup.isValid() && resolve()) {
        if (_actHandle.isDone()) { _actHandle.activation = _actSetup; }
        _actHandle = get();
    }
}
