/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Attachment Points
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
    : TerraAttachment<TerraActuator>(parent), _activation()
{ }

TerraActuatorAttachment::~TerraActuatorAttachment()
{
    off();
}

void TerraActuatorAttachment::setOutput(float intensity, millis_t duration, uint32_t now)
{
    SharedPtr<TerraActuator> actuator = getObject();
    if (!actuator || intensity <= FLT_EPSILON) {
        off();
        return;
    }

    _activation.setActuator(actuator.get());
    _activation.setup(intensity, duration);
    _activation.enable();
    actuator->resolveActivations(now);
}

void TerraActuatorAttachment::off()
{
    TerraActuator *actuator = _activation.actuator;
    _activation.unset();
    if (actuator) { actuator->resolveActivations(); }
}
