/*  Terraduino: Minimal/RO UI scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraduinoUI_H
#define TerraduinoUI_H

class TerraduinoMinUI;
typedef TerraduinoMinUI TerraduinoUI;

#include "../shared/TerraduinoUI.h"

class TerraduinoMinUI : public TerraduinoBaseUI {
public:
    TerraduinoMinUI(String deviceUUID,
                UIControlSetup uiControlSetup = UIControlSetup(),
                UIDisplaySetup uiDisplaySetup = UIDisplaySetup(),
                bool isActiveLowIO = true,
                bool allowInterruptableIO = true,
                bool enableTcUnicodeFonts = false,
                bool enableBufferedVRAM = false);
    virtual ~TerraduinoMinUI();

    void allocateStandardControls();
    void allocateESP32TouchControl();
    void allocateResistiveTouchControl();
    void allocateTouchscreenControl();
    void allocateTFTTouchControl();

    void allocateLCDDisplay();
    void allocateSSD1305Display();
    void allocateSSD1305x32AdaDisplay();
    void allocateSSD1305x64AdaDisplay();
    void allocateSSD1306Display();
    void allocateSH1106Display();
    void allocateCustomOLEDDisplay();
    void allocateSSD1607Display();
    void allocateIL3820Display();
    void allocateIL3820V2Display();
    void allocateST7735Display();
    void allocateST7789Display();
    void allocateILI9341Display();
    void allocateTFTDisplay();

    void addSerialRemote(UARTDeviceSetup rcSetup = UARTDeviceSetup());
    void addSimhubRemote(UARTDeviceSetup rcSetup = UARTDeviceSetup());
    void addWiFiRemote(uint16_t rcServerPort = TERRA_UI_REMOTESERVER_PORT);
    void addEthernetRemote(uint16_t rcServerPort = TERRA_UI_REMOTESERVER_PORT);

    virtual bool isFullUI() override;
};

#endif // /ifndef TerraduinoUI_H
#endif
