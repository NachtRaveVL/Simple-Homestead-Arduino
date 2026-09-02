/*  Terraduino: U8g2 OLED Overview Screen.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraOverviewOLED_H
#define TerraOverviewOLED_H

class TerraOverviewOLED;

#include "../TerraduinoUI.h"

class TerraOverviewOLED : public TerraOverview {
public:
    TerraOverviewOLED(TerraDisplayU8g2OLED *display, const void *clockFont, const void *detailFont);
    virtual ~TerraOverviewOLED();

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override;

protected:
    U8G2 &_gfx;
    #ifdef TERRA_UI_ENABLE_STCHROMA_LDTC
        StChromaArtDrawable &_drawable;
    #else
        U8g2Drawable &_drawable;
    #endif
    const void *_clockFont;
    const void *_detailFont;
};

#endif // /ifndef TerraOverviewOLED_H
#endif
