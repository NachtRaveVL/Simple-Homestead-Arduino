/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino UI Data
*/

#include "Terraduino.h"
#include "TerraUIData.h"

TerraUIData::TerraUIData()
    : TerraData('T','U','I','D', 1),
      updatesPerSec(TERRA_UI_UPDATE_SPEED),
      displayTheme(Terra_DisplayTheme_Undefined),
      titleMode(Terra_TitleMode_Undefined),
      analogSlider(false), editingIcons(false),
      joystickCalib{0.5f,0.5f,0.05f}, touchscreenCalib{0}
{
    _size = sizeof(*this);
}

void TerraUIData::toJSONObject(JsonObject &objectOut) const
{
    TerraData::toJSONObject(objectOut);

    if (updatesPerSec != TERRA_UI_UPDATE_SPEED) { objectOut["updatesPerSec"] = updatesPerSec; }
    if (displayTheme != Terra_DisplayTheme_Undefined) { objectOut["displayTheme"] = displayTheme; }
    if (!isFPEqual(joystickCalib[0], 0.5f) || !isFPEqual(joystickCalib[1], 0.5f) || !isFPEqual(joystickCalib[2], 0.05f)) { objectOut["joystickCalib"] = commaStringFromArray(joystickCalib, 3); }
}

void TerraUIData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraData::fromJSONObject(objectIn);

    updatesPerSec = objectIn["updatesPerSec"] | updatesPerSec;
    displayTheme = objectIn["displayTheme"] | displayTheme;
    JsonVariantConst joystickCalibVar = objectIn["joystickCalib"];
    commaStringToArray(joystickCalibVar, joystickCalib, 3);
}
