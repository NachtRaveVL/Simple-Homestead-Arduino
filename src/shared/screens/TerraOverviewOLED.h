/*  Terraduino: OLED overview scaffold.
    Copyright (C) 2026 NachtRaveVL
*/
#ifndef TerraOverviewOLED_H
#define TerraOverviewOLED_H
class TerraOverviewOLED : public TerraOverview {
public:
    explicit TerraOverviewOLED(TerraDisplayDriver *display, const void *clockFont = nullptr, const void *detailFont = nullptr) : TerraOverview(display) { (void)clockFont; (void)detailFont; }
    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override { (void)isLandscape; (void)screenSize; _needsFullRedraw = false; }
};
#endif
