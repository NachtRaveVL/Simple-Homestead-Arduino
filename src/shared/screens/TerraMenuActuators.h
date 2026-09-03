/*  Terraduino: Actuators menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraMenuActuators_H
#define TerraMenuActuators_H

#include "../TerraduinoUI.h"

// UI stub: Domain-specific actuators menu behavior is intentionally not implemented yet.
class TerraMenuActuators : public TerraMenu
{
public:
    TerraMenuActuators() = default;
    virtual ~TerraMenuActuators() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef TerraMenuActuators_H
#endif
