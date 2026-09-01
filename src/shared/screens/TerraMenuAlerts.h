/*  Terraduino: Alerts menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraMenuAlerts_H
#define TerraMenuAlerts_H

#include "../TerraduinoUI.h"

// UI stub: Domain-specific alerts menu behavior is intentionally not implemented yet.
class TerraMenuAlerts : public TerraMenu
{
public:
    TerraMenuAlerts() = default;
    virtual ~TerraMenuAlerts() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef TerraMenuAlerts_H
#endif
