/*  Terraduino: AdafruitGFX Overview Screen.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraOverviewGFX_H
#define TerraOverviewGFX_H

template<class T> class TerraOverviewGFX;

#include "../TerraduinoUI.h"

template<class T>
class TerraOverviewGFX : public TerraOverview {
public:
    TerraOverviewGFX(TerraDisplayAdafruitGFX<T> *display, const void *clockFont = nullptr, const void *detailFont = nullptr);
    virtual ~TerraOverviewGFX();

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override;

protected:
    T &_gfx;
    AdafruitDrawable<T> &_drawable;
    const void *_clockFont;
    const void *_detailFont;
};

#endif // /ifndef TerraOverviewGFX_H
#endif
