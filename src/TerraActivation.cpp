/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Activation
*/

#include "Terraduino.h"
#include "TerraActuators.h"

TerraActivationHandle::TerraActivationHandle(TerraActuator *actuatorIn, float intensity,
                                             millis_t duration, bool force)
    : actuator(nullptr), activation(intensity, duration, force ? Terra_ActivationFlags_Forced : Terra_ActivationFlags_None),
      checkTime(0), elapsed(0)
{
    setActuator(actuatorIn);
}

TerraActivationHandle::TerraActivationHandle(const TerraActivationHandle &other)
    : actuator(nullptr), activation(other.activation), checkTime(0), elapsed(0)
{
    setActuator(other.actuator);
    if (actuator && isValid() && !isDone()) { enable(); }
}

TerraActivationHandle::~TerraActivationHandle()
{
    if (actuator) { unset(); }
}

void TerraActivationHandle::setActuator(TerraActuator *actuatorIn)
{
    if (actuator == actuatorIn) return;
    if (actuator) { unset(); } else { checkTime = 0; }
    actuator = actuatorIn;
}

void TerraActivationHandle::setup(float intensity, millis_t duration, bool force)
{
    activation = TerraActivation(intensity, duration, force ? Terra_ActivationFlags_Forced : Terra_ActivationFlags_None);
    elapsed = 0;
    if (actuator) { actuator->setNeedsUpdate(); }
}

void TerraActivationHandle::enable()
{
    if (!actuator || !isValid() || isDone()) {
        unset();
        return;
    }

    if (!actuator->addActivationHandle(this)) { actuator = nullptr; }
}

void TerraActivationHandle::unset()
{
    if (isActive()) { elapseTo(); }
    checkTime = 0;

    if (actuator) {
        TerraActuator *oldActuator = actuator;
        actuator = nullptr;
        oldActuator->removeActivationHandle(this);
    }
}

void TerraActivationHandle::elapseBy(millis_t delta)
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
                if (actuator) { actuator->setNeedsUpdate(); }
            }
        }
        elapsed += delta;
    }
}
