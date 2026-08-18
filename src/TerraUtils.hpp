/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Utilities
*/

#ifndef TerraUtils_HPP
#define TerraUtils_HPP

#include "TerraUtils.h"

inline float terraPercentToFraction(float percent) {
    return terraClamp(percent, 0.0f, 100.0f) / 100.0f;
}

inline float terraFractionToPercent(float fraction) {
    return terraClamp(fraction, 0.0f, 1.0f) * 100.0f;
}

#endif
