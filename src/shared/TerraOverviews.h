/*  Terraduino: Overview screens.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Overview Screens
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraOverviews_H
#define TerraOverviews_H

class TerraOverview;
class TerraOverviewLCD;
class TerraOverviewOLED;
template<class T> class TerraOverviewGFX;
class TerraOverviewTFT;

#include "TerraduinoUI.h"

// Overview Screen Base
// Overview screen class that manages the default at-a-glance system overview.
// Meant to be able to be deleted on a moments notice to transition back into menu.
class TerraOverview {
public:
    inline TerraOverview(TerraDisplayDriver *display) : _display(display), _needsFullRedraw(true) { ; }
    virtual ~TerraOverview() = default;

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) = 0;

    inline void setNeedsFullRedraw() { _needsFullRedraw = true; }

protected:
    TerraDisplayDriver *_display;
    bool _needsFullRedraw;
};

#include "screens/TerraOverviewGFX.h"
#include "screens/TerraOverviewLCD.h"
#include "screens/TerraOverviewOLED.h"
#include "screens/TerraOverviewTFT.h"

#endif // /ifndef TerraOverviews_H
#endif
