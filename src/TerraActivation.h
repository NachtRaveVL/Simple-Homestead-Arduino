/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Activation
*/

#ifndef TerraActivation_H
#define TerraActivation_H

#include "TerraPlatform.h"

class TerraActuator;

// Activation Data
// Activation setup data that defines a normalized actuator-output request.
struct TerraActivation {
    float intensity;                                        // Normalized driving intensity ([0.0,1.0])
    uint32_t duration;                                      // Duration remaining, milliseconds; -1 untimed, 0 finished

    inline TerraActivation(float intensityIn = 0.0f, uint32_t durationIn = 0)
        : intensity(intensityIn < 0.0f ? 0.0f : intensityIn > 1.0f ? 1.0f : intensityIn), duration(durationIn) { ; }

    inline bool isValid() const { return intensity > 0.0f; }
    inline bool isDone() const { return duration == 0; }
    inline bool isUntimed() const { return duration == (uint32_t)-1; }
    inline float getDriveIntensity() const { return intensity; }
};

// Activation Handle
// Resident actuator request. Handles must stay memory resident while assigned to an
// actuator so the actuator can combine them with other active requests.
struct TerraActivationHandle {
    TerraActuator *actuator;                                // Actuator owner, set only when activation requested
    TerraActivation activation;                             // Activation data
    uint32_t checkTime;                                     // Last active check time, else 0 when not started
    uint32_t elapsed;                                       // Elapsed active time, milliseconds

    TerraActivationHandle(TerraActuator *actuatorIn = nullptr,
                          float intensity = 0.0f,
                          uint32_t duration = 0);
    TerraActivationHandle(const TerraActivationHandle &handle);
    ~TerraActivationHandle();

    TerraActivationHandle &operator=(TerraActuator *actuatorIn);
    inline TerraActivationHandle &operator=(const TerraActivation &activationIn) { activation = activationIn; return *this; }
    inline TerraActivationHandle &operator=(const TerraActivationHandle &handle) { activation = handle.activation; return operator=(handle.actuator); }

    // Disconnects activation from an actuator.
    void unset();

    // Elapses activation by delta, updating remaining duration and elapsed time.
    void elapseBy(uint32_t delta);
    inline void elapseTo(uint32_t time = terraNZMillis()) { elapseBy(time - checkTime); }

    inline bool isActive() const { return actuator && checkTime; }
    inline bool isValid() const { return activation.isValid(); }
    inline bool isDone() const { return activation.isDone(); }
    inline bool isUntimed() const { return activation.isUntimed(); }
    inline uint32_t getTimeLeft() const { return activation.duration; }
    inline uint32_t getTimeActive(uint32_t time = terraNZMillis()) const { return isActive() ? (time - checkTime) + elapsed : elapsed; }
    inline float getDriveIntensity() const { return activation.getDriveIntensity(); }
};

#endif // /ifndef TerraActivation_H
