/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Object
*/

#include "TerraObject.h"

TerraObject::TerraObject(Terra_ObjectType objectType, uint32_t key, const TerraString &name)
    : _objectType(objectType), _key(key), _name(name), _enabled(true), _fault(false), _faultMessage() { }

void TerraObject::setFault(const TerraString &message) {
    _fault = true;
    _faultMessage = message;
}

void TerraObject::clearFault() {
    _fault = false;
    _faultMessage = TerraString();
}

void TerraObject::setEnabled(bool enabled) {
    _enabled = enabled;
}
