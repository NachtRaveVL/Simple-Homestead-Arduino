/*  Terraduino: GFX overview scaffold.
    Copyright (C) 2026 NachtRaveVL
*/
#ifndef TerraOverviewGFX_H
#define TerraOverviewGFX_H
template<class T> class TerraOverviewGFX : public TerraOverview {
public:
    explicit TerraOverviewGFX(TerraDisplayDriver *display, const void *clockFont = nullptr, const void *detailFont = nullptr) : TerraOverview(display) { (void)clockFont; (void)detailFont; }
    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override { (void)isLandscape; (void)screenSize; _needsFullRedraw = false; }
};
#endif
