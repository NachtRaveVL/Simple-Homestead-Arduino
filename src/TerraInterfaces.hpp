/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Interfaces
*/

#ifndef TerraInterfaces_HPP
#define TerraInterfaces_HPP

#include "TerraInterfaces.h"

class TerraNullSink : public TerraTextSink {
public:
    void write(const TerraString &) override { }
};

#endif
