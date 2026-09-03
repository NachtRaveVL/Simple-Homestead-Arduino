/*  Terraduino: TFT_eSPI Overview Screen.
    Copyright (C) 2026 NachtRaveVL
*/

#include "../TerraduinoUI.h"
#ifdef TERRA_USE_GUI

TerraOverviewTFT::TerraOverviewTFT(TerraDisplayTFTeSPI *display, const void *clockFont, const void *detailFont)
    : TerraOverview(display), _gfx(display->getGfx()), _drawable(display->getDrawable()), _clockFont(clockFont), _detailFont(detailFont)
{ ; }

TerraOverviewTFT::~TerraOverviewTFT()
{ ; }

void TerraOverviewTFT::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    // todo
}

#endif
