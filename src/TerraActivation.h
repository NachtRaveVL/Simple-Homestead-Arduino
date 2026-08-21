/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Activation
*/

#ifndef TerraActivation_H
#define TerraActivation_H

#include "TerraPlatform.h"

class TerraActuator;

// Activation Setup
// Stores one normalized actuator-output request and optional duration.
class TerraActivation {
public:
    TerraActivation(float intensity = 0.0f, uint32_t durationMs = 0)
        : _intensity(intensity), _durationMs(durationMs) { }

    inline float getIntensity() const { return _intensity; }
    inline uint32_t getDurationMs() const { return _durationMs; }
    inline bool isUntimed() const { return _durationMs == 0; }

protected:
    float _intensity;                                       // Requested normalized intensity
    uint32_t _durationMs;                                   // Requested duration, milliseconds

    friend class TerraActivationHandle;
};

// Activation Handle
// Resident actuator request used by attachment points. The handle must stay memory
// resident while active so the actuator can aggregate it with other requests.
class TerraActivationHandle {
public:
    TerraActivationHandle(TerraActuator *actuator = nullptr,
                          float intensity = 0.0f,
                          uint32_t durationMs = 0);
    TerraActivationHandle(const TerraActivationHandle &other);
    ~TerraActivationHandle();

    void setActuator(TerraActuator *actuator);
    void setup(float intensity, uint32_t durationMs = 0);
    void enable(uint32_t now = terraMillis());
    void unset();
    void update(uint32_t now = terraMillis());

    inline bool isActive() const { return _actuator && _active; }
    inline float getIntensity() const { return _activation.getIntensity(); }

protected:
    TerraActuator *_actuator;                               // Actuator owner; attachment retains shared object
    TerraActivation _activation;                            // Requested activation
    uint32_t _startedAt;                                    // Activation start millis
    bool _active;                                           // Active request flag

    friend class TerraActuator;
};

#endif
