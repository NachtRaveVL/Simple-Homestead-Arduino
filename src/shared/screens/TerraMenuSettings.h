/*  Terraduino: Settings menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraMenuSettings_H
#define TerraMenuSettings_H

#include "../TerraduinoUI.h"

// UI stub: Domain-specific settings menu behavior is intentionally not implemented yet.
class TerraMenuSettings : public TerraMenu
{
public:
    TerraMenuSettings() = default;
    virtual ~TerraMenuSettings() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef TerraMenuSettings_H
#endif
