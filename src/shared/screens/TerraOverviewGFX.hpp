/*  Terraduino: AdafruitGFX Overview Screen.
    Copyright (C) 2026 NachtRaveVL
*/

#include "../TerraduinoUI.h"
#ifdef TERRA_USE_GUI

template <class T>
TerraOverviewGFX<T>::TerraOverviewGFX(TerraDisplayAdafruitGFX<T> *display, const void *clockFont, const void *detailFont)
    : TerraOverview(display), _gfx(display->getGfx()), _drawable(display->getDrawable()), _clockFont(clockFont), _detailFont(detailFont)
{ ; }

template <class T>
TerraOverviewGFX<T>::~TerraOverviewGFX()
{ ; }

template <class T>
void TerraOverviewGFX<T>::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    // todo
}

#endif
