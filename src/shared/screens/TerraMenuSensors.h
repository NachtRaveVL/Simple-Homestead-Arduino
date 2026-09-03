/*  Terraduino: Sensors menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraMenuSensors_H
#define TerraMenuSensors_H

#include "../TerraduinoUI.h"

// UI stub: Domain-specific sensors menu behavior is intentionally not implemented yet.
class TerraMenuSensors : public TerraMenu
{
public:
    TerraMenuSensors() = default;
    virtual ~TerraMenuSensors() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef TerraMenuSensors_H
#endif
