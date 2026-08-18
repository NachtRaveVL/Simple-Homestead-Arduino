/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Common Inlines
*/

#ifndef TerraInlines_HH
#define TerraInlines_HH

#include "TerraFactory.h"


template <typename T>
inline T *terraObjectAs(TerraFactory &factory, uint32_t key) {
    return static_cast<T *>(factory.findObjectByKey(key));
}

template <typename T>
inline const T *terraObjectAs(const TerraFactory &factory, uint32_t key) {
    return static_cast<const T *>(factory.findObjectByKey(key));
}

#endif
