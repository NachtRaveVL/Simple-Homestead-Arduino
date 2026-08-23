/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Activation
*/

#ifndef TerraActivation_H
#define TerraActivation_H

struct TerraActivation;
struct TerraActivationHandle;
class TerraActuator;

#include "Terraduino.h"

// Activation Flags
enum Terra_ActivationFlags : uint8_t {
    Terra_ActivationFlags_Forced = 0x01,                    // Force enable / ignore cursory enable checks
    Terra_ActivationFlags_None = 0x00                       // Placeholder
};

// Activation Setup
// Stores one normalized actuator-output request and optional duration.
struct TerraActivation {
    Terra_DirectionMode direction;                          // Normalized driving direction
    float intensity;                                        // Normalized driving intensity [0.0,1.0]
    millis_t duration;                                      // Duration remaining, -1 unlimited, 0 finished
    Terra_ActivationFlags flags;                            // Activation flags

    TerraActivation(Terra_DirectionMode directionIn, float intensityIn,
                    millis_t durationIn, Terra_ActivationFlags flagsIn)
        : direction(directionIn), intensity(constrain(intensityIn, 0.0f, 1.0f)),
          duration(durationIn), flags(flagsIn) { ; }
    TerraActivation()
        : TerraActivation(Terra_DirectionMode_Undefined, 0.0f, 0, Terra_ActivationFlags_None) { ; }

    inline bool isValid() const { return direction != Terra_DirectionMode_Undefined; }
    inline bool isDone() const { return duration == millis_none; }
    inline bool isUntimed() const { return duration == (millis_t)-1; }
    inline bool isForced() const { return flags & Terra_ActivationFlags_Forced; }
    inline float getDriveIntensity() const { return direction == Terra_DirectionMode_Forward ? intensity :
                                                    direction == Terra_DirectionMode_Reverse ? -intensity : 0.0f; }
};

// Activation Handle
struct TerraActivationHandle {
    SharedPtr<TerraActuator> actuator;                      // Actuator owner
    TerraActivation activation;                             // Activation data
    millis_t checkTime;                                     // Last active check timestamp
    millis_t elapsed;                                       // Elapsed active time accumulator

    TerraActivationHandle(SharedPtr<TerraActuator> actuatorIn,
                          Terra_DirectionMode direction,
                          float intensity = 1.0f,
                          millis_t duration = (millis_t)-1,
                          bool force = false);
    inline TerraActivationHandle()
        : TerraActivationHandle(nullptr, Terra_DirectionMode_Undefined, 0.0f, 0, false) { ; }
    TerraActivationHandle(const TerraActivationHandle &handle);
    ~TerraActivationHandle();

    TerraActivationHandle &operator=(SharedPtr<TerraActuator> actuatorIn);
    inline TerraActivationHandle &operator=(const TerraActivation &activationIn) { activation = activationIn; return *this; }
    inline TerraActivationHandle &operator=(const TerraActivationHandle &handle) { activation = handle.activation; return operator=(handle.actuator); }

    void unset();
    void elapseBy(millis_t delta);
    inline void elapseTo(millis_t time = millis()) { if (checkTime) { elapseBy(time - checkTime); } }

    inline bool isActive() const { return actuator && checkTime; }
    inline bool isValid() const { return activation.isValid(); }
    inline bool isDone() const { return activation.isDone(); }
    inline bool isUntimed() const { return activation.isUntimed(); }
    inline bool isForced() const { return activation.isForced(); }
    inline millis_t getTimeLeft() const { return activation.duration; }
    inline millis_t getTimeActive(millis_t time = millis()) const { return isActive() ? (time - checkTime) + elapsed : elapsed; }
    inline float getDriveIntensity() const { return activation.getDriveIntensity(); }
};

#endif // /ifndef TerraActivation_H
