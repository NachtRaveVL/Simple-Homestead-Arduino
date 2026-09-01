/*  Terraduino: Scheduling menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraMenuScheduling_H
#define TerraMenuScheduling_H

#include "../TerraduinoUI.h"

// UI stub: Domain-specific scheduling menu behavior is intentionally not implemented yet.
class TerraMenuScheduling : public TerraMenu
{
public:
    TerraMenuScheduling() = default;
    virtual ~TerraMenuScheduling() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef TerraMenuScheduling_H
#endif
