/*  Terraduino: Thermal menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraMenuThermal_H
#define TerraMenuThermal_H

#include "../TerraduinoUI.h"

// UI stub: Domain-specific thermal menu behavior is intentionally not implemented yet.
class TerraMenuThermal : public TerraMenu
{
public:
    TerraMenuThermal() = default;
    virtual ~TerraMenuThermal() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef TerraMenuThermal_H
#endif
