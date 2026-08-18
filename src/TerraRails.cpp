/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Power Rails
*/

#include "TerraRails.h"

TerraPowerRail::TerraPowerRail(float nominalVoltage, uint32_t key, const TerraString &name, Terra_RailType railType)
    : TerraObject(Terra_ObjectType_PowerRail, key, name), _railType(railType), _nominalVoltage(nominalVoltage), _measuredVoltage(0.0f),
      _railEnabled(false), _callback(nullptr), _context(nullptr) { }

void TerraPowerRail::setEnabledState(bool enabled) {
    _railEnabled = enabled;
    if (_callback) _callback(_context, enabled ? 1.0f : 0.0f);
}
