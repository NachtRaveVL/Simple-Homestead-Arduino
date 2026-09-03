/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino UI Data
*/

#ifndef TerraUIData_H
#define TerraUIData_H

struct TerraUIData;

#include "Terraduino.h"
#include "TerraUIDefines.h"

// UI Serialization Data
// id: AUID. Terraduino UI data.
struct TerraUIData : public TerraData {
    uint8_t updatesPerSec;                                  // Updates per second (1-10, default: TERRA_UI_UPDATE_SPEED)
    Terra_DisplayTheme displayTheme;                        // Display theme (if supported)
    Terra_TitleMode titleMode;                              // Title mode
    bool analogSlider;                                      // Use analog slider
    bool editingIcons;                                      // Use editing icons
    float joystickCalib[3];                                 // Joystick calibration ({midX,midY,zeroTol}, default: {0.5,0.5,0.05})
    uint16_t touchscreenCalib[4];                           // Touchscreen calibration ({x0,y0,x1,y1}), default: {0,0,0,0})

    TerraUIData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef TerraUIData_H
