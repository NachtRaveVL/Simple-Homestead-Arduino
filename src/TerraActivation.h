/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Activation
*/

#ifndef TerraActivation_H
#define TerraActivation_H

#include "TerraPlatform.h"

class TerraActivation {
public:
    TerraActivation();

    void activate(float intensity = 1.0f, uint32_t durationMs = 0, uint32_t now = terraMillis());
    void deactivate();
    void update(uint32_t now = terraMillis());

    bool isActive() const { return _active; }
    float getIntensity() const { return _intensity; }
    uint32_t getStartedAt() const { return _startedAt; }
    uint32_t getDurationMs() const { return _durationMs; }

protected:
    bool _active;                                           // Active
    float _intensity;                                       // Intensity
    uint32_t _startedAt;                                    // Continuous run start timestamp
    uint32_t _durationMs;                                   // Duration milliseconds
};

#endif
