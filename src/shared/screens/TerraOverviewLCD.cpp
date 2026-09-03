/*  Terraduino: LCD Overview Screen.
    Copyright (C) 2026 NachtRaveVL
*/

#include "../TerraduinoUI.h"
#ifdef TERRA_USE_GUI

TerraOverviewLCD::TerraOverviewLCD(TerraDisplayLiquidCrystal *display)
    : TerraOverview(display), _lcd(display->getLCD())
{ ; }

TerraOverviewLCD::~TerraOverviewLCD()
{ ; }

void TerraOverviewLCD::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    // todo
}

#endif
