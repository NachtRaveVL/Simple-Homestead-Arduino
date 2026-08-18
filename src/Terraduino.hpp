/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Controller
*/

#ifndef Terraduino_HPP
#define Terraduino_HPP

#include "Terraduino.h"

inline bool terraRegisterAll(Terraduino &controller, TerraObject **objects, uint8_t count) {
    for (uint8_t i = 0; i < count; ++i) if (!controller.registerObject(objects[i])) return false;
    return true;
}

#endif
