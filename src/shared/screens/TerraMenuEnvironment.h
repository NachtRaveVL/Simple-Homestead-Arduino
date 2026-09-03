/*  Terraduino: Environment menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraMenuEnvironment_H
#define TerraMenuEnvironment_H

#include "../TerraduinoUI.h"

// UI stub: Domain-specific environment menu behavior is intentionally not implemented yet.
class TerraMenuEnvironment : public TerraMenu
{
public:
    TerraMenuEnvironment() = default;
    virtual ~TerraMenuEnvironment() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef TerraMenuEnvironment_H
#endif
