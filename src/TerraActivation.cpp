/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Activation
*/

#include "TerraActivation.h"
#include "TerraActuators.h"
#include "TerraUtils.h"

TerraActivationHandle::TerraActivationHandle(TerraActuator *actuatorIn, float intensity, uint32_t duration)
    : actuator(nullptr), activation(terraClamp(intensity, 0.0f, 1.0f), duration),
      checkTime(0), elapsed(0)
{
    operator=(actuatorIn);
}

TerraActivationHandle::TerraActivationHandle(const TerraActivationHandle &handle)
    : actuator(nullptr), activation(handle.activation), checkTime(0), elapsed(0)
{
    operator=(handle.actuator);
}

TerraActivationHandle::~TerraActivationHandle()
{
    if (actuator) { unset(); }
}

TerraActivationHandle &TerraActivationHandle::operator=(TerraActuator *actuatorIn)
{
    if (actuator != actuatorIn && isValid()) {
        if (actuator) { unset(); } else { checkTime = 0; }

        actuator = actuatorIn;

        if (actuator) {
            if (actuator->addActivationHandle(this)) {
                actuator->setNeedsUpdate();
            } else {
                actuator = nullptr;
            }
        }
    }
    return *this;
}

void TerraActivationHandle::unset()
{
    if (isActive()) { elapseTo(); }
    checkTime = 0;

    if (actuator) {
        TerraActuator *oldActuator = actuator;
        actuator = nullptr;
        oldActuator->removeActivationHandle(this);
        oldActuator->setNeedsUpdate();
    }
}

void TerraActivationHandle::elapseBy(uint32_t delta)
{
    if (delta && isValid() && isActive()) {
        if (!isUntimed()) {
            if (delta <= activation.duration) {
                activation.duration -= delta;
                checkTime += delta;
            } else {
                delta = activation.duration;
                activation.duration = 0;
                checkTime = 0;
                actuator->setNeedsUpdate();
            }
        }
        elapsed += delta;
    }
}
