/*  Terraduino: LCD overview scaffold.
    Copyright (C) 2026 NachtRaveVL
*/
#ifndef TerraOverviewLCD_H
#define TerraOverviewLCD_H
class TerraOverviewLCD : public TerraOverview {
public:
    explicit TerraOverviewLCD(TerraDisplayDriver *display, const void *clockFont = nullptr, const void *detailFont = nullptr) : TerraOverview(display) { (void)clockFont; (void)detailFont; }
    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override { (void)isLandscape; (void)screenSize; _needsFullRedraw = false; }
};
#endif
