/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Activation
*/

#include "TerraActivation.h"
#include "TerraUtils.h"

TerraActivation::TerraActivation()
    : _active(false), _intensity(0.0f), _startedAt(0), _durationMs(0) { }

void TerraActivation::activate(float intensity, uint32_t durationMs, uint32_t now) {
    _active = true;
    _intensity = terraClamp(intensity, 0.0f, 1.0f);
    _startedAt = now;
    _durationMs = durationMs;
}

void TerraActivation::deactivate() {
    _active = false;
    _intensity = 0.0f;
    _durationMs = 0;
}

void TerraActivation::update(uint32_t now) {
    if (_active && _durationMs && terraElapsed(now, _startedAt, _durationMs)) deactivate();
}
