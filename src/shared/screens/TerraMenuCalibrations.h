/*  Terraduino: Calibrations menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraMenuCalibrations_H
#define TerraMenuCalibrations_H

#include "../TerraduinoUI.h"

// UI stub: Domain-specific calibrations menu behavior is intentionally not implemented yet.
class TerraMenuCalibrations : public TerraMenu
{
public:
    TerraMenuCalibrations() = default;
    virtual ~TerraMenuCalibrations() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef TerraMenuCalibrations_H
#endif
