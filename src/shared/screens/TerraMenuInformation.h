/*  Terraduino: Information menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraMenuInformation_H
#define TerraMenuInformation_H

#include "../TerraduinoUI.h"

// UI stub: Domain-specific information menu behavior is intentionally not implemented yet.
class TerraMenuInformation : public TerraMenu
{
public:
    TerraMenuInformation() = default;
    virtual ~TerraMenuInformation() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef TerraMenuInformation_H
#endif
