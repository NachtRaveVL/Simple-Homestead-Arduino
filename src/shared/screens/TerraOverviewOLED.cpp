/*  Terraduino: U8g2 OLED Overview Screen.
    Copyright (C) 2026 NachtRaveVL
*/

#include "../TerraduinoUI.h"
#ifdef TERRA_USE_GUI

TerraOverviewOLED::TerraOverviewOLED(TerraDisplayU8g2OLED *display, const void *clockFont, const void *detailFont)
    : TerraOverview(display), _gfx(display->getGfx()), _drawable(display->getDrawable()), _clockFont(clockFont), _detailFont(detailFont)
{ ; }

TerraOverviewOLED::~TerraOverviewOLED()
{ ; }

void TerraOverviewOLED::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    // todo
}

#endif
