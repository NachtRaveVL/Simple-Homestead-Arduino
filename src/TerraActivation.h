/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Activation
*/

#ifndef TerraActivation_H
#define TerraActivation_H

class TerraActuator;

// Activation Flags
enum Terra_ActivationFlags : uint8_t {
    Terra_ActivationFlags_Forced = 0x01,                    // Force enable / ignore cursory enable checks
    Terra_ActivationFlags_None = 0x00                       // Placeholder
};

// Activation Setup
// Stores one normalized actuator-output request and optional duration.
struct TerraActivation {
    float intensity;                                        // Normalized driving intensity [0.0,1.0]
    millis_t duration;                                      // Duration remaining, milliseconds, -1 for unlimited, 0 for finished
    Terra_ActivationFlags flags;                            // Activation flags

    TerraActivation(float intensityIn = 0.0f,
                    millis_t durationIn = 0,
                    Terra_ActivationFlags flagsIn = Terra_ActivationFlags_None)
        : intensity(constrain(intensityIn, 0.0f, 1.0f)), duration(durationIn), flags(flagsIn) { }

    inline bool isValid() const { return intensity > FLT_EPSILON; }
    inline bool isDone() const { return duration == millis_none; }
    inline bool isUntimed() const { return duration == (millis_t)-1; }
    inline bool isForced() const { return flags & Terra_ActivationFlags_Forced; }
    inline float getDriveIntensity() const { return intensity; }
};

// Activation Handle
// Resident actuator request used by attachment points. The handle must stay memory
// resident while active so the actuator can aggregate it with other requests.
struct TerraActivationHandle {
    TerraActuator *actuator;                                // Actuator owner; attachment retains shared object
    TerraActivation activation;                             // Activation data
    millis_t checkTime;                                     // Last active check timestamp, else 0 for not started
    millis_t elapsed;                                       // Elapsed active time accumulator

    TerraActivationHandle(TerraActuator *actuatorIn = nullptr,
                          float intensity = 0.0f,
                          millis_t duration = 0,
                          bool force = false);
    TerraActivationHandle(const TerraActivationHandle &other);
    ~TerraActivationHandle();

    void setActuator(TerraActuator *actuatorIn);
    void setup(float intensity, millis_t duration = (millis_t)-1, bool force = false);
    void enable();
    void unset();
    void elapseBy(millis_t delta);
    inline void elapseTo(millis_t time = millis()) { elapseBy(time - checkTime); }

    inline bool isActive() const { return actuator && checkTime; }
    inline bool isValid() const { return activation.isValid(); }
    inline bool isDone() const { return activation.isDone(); }
    inline bool isUntimed() const { return activation.isUntimed(); }
    inline bool isForced() const { return activation.isForced(); }
    inline millis_t getTimeLeft() const { return activation.duration; }
    inline millis_t getTimeActive(millis_t time = millis()) const { return isActive() ? (time - checkTime) + elapsed : elapsed; }
    inline float getDriveIntensity() const { return activation.getDriveIntensity(); }
};

#endif
