/*  Terraduino: Home menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraMenuHome_H
#define TerraMenuHome_H

#include "../TerraduinoUI.h"

// UI stub: Domain-specific home menu behavior is intentionally not implemented yet.
class TerraHomeMenu : public TerraMenu
{
public:
    TerraHomeMenu() = default;
    virtual ~TerraHomeMenu() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }

    void unloadSubMenus() { ; }
};

#endif // /ifndef TerraMenuHome_H
#endif
