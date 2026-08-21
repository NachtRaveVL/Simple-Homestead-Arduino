/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Common Inlines
*/

#ifndef TerraInlines_HH
#define TerraInlines_HH

#include "TerraModules.h"


template <typename T>
inline T *terraObjectAs(TerraObjectRegistration &objects, uint32_t key) {
    return static_cast<T *>(objects.findObjectByKey(key));
}

template <typename T>
inline const T *terraObjectAs(const TerraObjectRegistration &objects, uint32_t key) {
    return static_cast<const T *>(objects.findObjectByKey(key));
}

#endif
