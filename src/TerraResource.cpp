/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Resources
*/

#include "TerraResource.h"
#include "TerraUtils.h"

TerraResource::TerraResource(Terra_ResourceType type, uint32_t key, const TerraString &name)
    : TerraObject(Terra_ObjectType_Resource, key, name), _type(type), _state(Terra_ResourceState_Reserve),
      _level(0.0f), _reserveLevel(10.0f), _lowLevel(25.0f), _highLevel(90.0f) { }

bool TerraResource::setThresholds(float reserveLevel, float lowLevel, float highLevel) {
    if (reserveLevel < 0.0f || reserveLevel > lowLevel || lowLevel >= highLevel || highLevel > 100.0f) return false;
    _reserveLevel = reserveLevel;
    _lowLevel = lowLevel;
    _highLevel = highLevel;
    updateState();
    return true;
}

void TerraResource::setLevel(float level) {
    _level = terraClamp(level, 0.0f, 100.0f);
    updateState();
}

void TerraResource::setFault(const TerraString &message) {
    TerraObject::setFault(message);
    updateState();
}

void TerraResource::clearFault() {
    TerraObject::clearFault();
    updateState();
}

void TerraResource::updateState() {
    if (_fault) { _state = Terra_ResourceState_Fault; }
    else if (_level <= _reserveLevel) { _state = Terra_ResourceState_Reserve; }
    else if (_level <= _lowLevel) { _state = Terra_ResourceState_Low; }
    else if (_level >= _highLevel) { _state = Terra_ResourceState_High; }
    else { _state = Terra_ResourceState_Normal; }
}
