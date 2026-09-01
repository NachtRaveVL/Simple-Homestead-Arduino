/*  Terraduino: Water menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraMenuWater_H
#define TerraMenuWater_H

#include "../TerraduinoUI.h"

// UI stub: Domain-specific water menu behavior is intentionally not implemented yet.
class TerraMenuWater : public TerraMenu
{
public:
    TerraMenuWater() = default;
    virtual ~TerraMenuWater() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef TerraMenuWater_H
#endif
