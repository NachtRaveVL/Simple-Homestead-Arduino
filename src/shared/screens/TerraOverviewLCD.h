/*  Terraduino: LCD Overview Screen.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraOverviewLCD_H
#define TerraOverviewLCD_H

class TerraOverviewLCD;

#include "../TerraduinoUI.h"

class TerraOverviewLCD : public TerraOverview {
public:
    TerraOverviewLCD(TerraDisplayLiquidCrystal *display);
    virtual ~TerraOverviewLCD();

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override;

protected:
    LiquidCrystal &_lcd;
};

#endif // /ifndef TerraOverviewLCD_H
#endif
