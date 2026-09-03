/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Activation
*/

#include "Terraduino.h"

TerraActivationHandle::TerraActivationHandle(SharedPtr<TerraActuator> actuatorIn, Terra_DirectionMode direction, float intensity, millis_t duration, bool force)
    : actuator(nullptr), activation(direction, constrain(intensity, 0.0f, 1.0f), duration, (force ? Terra_ActivationFlags_Forced : Terra_ActivationFlags_None)),
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

TerraActivationHandle &TerraActivationHandle::operator=(SharedPtr<TerraActuator> actuatorIn)
{
    if (actuator != actuatorIn && isValid()) {
        if (actuator) { unset(); } else { checkTime = 0; }

        actuator = actuatorIn;

        if (actuator) { actuator->_handles.push_back(this); actuator->setNeedsUpdate(); }
    }
    return *this;
}

void TerraActivationHandle::unset()
{
    if (isActive()) { elapseTo(); }
    checkTime = 0;

    if (actuator) {
        for (auto handleIter = actuator->_handles.end(); handleIter != actuator->_handles.begin();) {
            --handleIter;
            if ((*handleIter) == this) {
                actuator->_handles.erase(handleIter);
                break;
            }
        }
        actuator->setNeedsUpdate();
        actuator = nullptr;
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
                actuator->setNeedsUpdate();
            }
        }
        elapsed += delta;
    }
}
