/*  Terraduino: TFT_eSPI Overview Screen.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraOverviewTFT_H
#define TerraOverviewTFT_H

class TerraOverviewTFT;

#include "../TerraduinoUI.h"

class TerraOverviewTFT : public TerraOverview {
public:
    TerraOverviewTFT(TerraDisplayTFTeSPI *display, const void *clockFont, const void *detailFont);
    virtual ~TerraOverviewTFT();

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override;

protected:
    TFT_eSPI &_gfx;
    TfteSpiDrawable &_drawable;
    const void *_clockFont;
    const void *_detailFont;
};

#endif // /ifndef TerraOverviewTFT_H
#endif
