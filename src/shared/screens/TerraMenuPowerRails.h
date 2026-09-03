/*  Terraduino: PowerRails menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraMenuPowerRails_H
#define TerraMenuPowerRails_H

#include "../TerraduinoUI.h"

// UI stub: Domain-specific powerrails menu behavior is intentionally not implemented yet.
class TerraMenuPowerRails : public TerraMenu
{
public:
    TerraMenuPowerRails() = default;
    virtual ~TerraMenuPowerRails() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef TerraMenuPowerRails_H
#endif
