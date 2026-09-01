/*  Terraduino: TFT overview scaffold.
    Copyright (C) 2026 NachtRaveVL
*/
#ifndef TerraOverviewTFT_H
#define TerraOverviewTFT_H
class TerraOverviewTFT : public TerraOverview {
public:
    explicit TerraOverviewTFT(TerraDisplayDriver *display, const void *clockFont = nullptr, const void *detailFont = nullptr) : TerraOverview(display) { (void)clockFont; (void)detailFont; }
    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override { (void)isLandscape; (void)screenSize; _needsFullRedraw = false; }
};
#endif
