/*  Terraduino: Full/RW UI scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraduinoUI_H
#define TerraduinoUI_H

class TerraduinoFullUI;
typedef TerraduinoFullUI TerraduinoUI;

#include "../shared/TerraduinoUI.h"

class TerraduinoFullUI : public TerraduinoBaseUI {
public:
    TerraduinoFullUI(String deviceUUID,
                 UIControlSetup uiControlSetup = UIControlSetup(),
                 UIDisplaySetup uiDisplaySetup = UIDisplaySetup(),
                 bool isActiveLowIO = true,
                 bool allowInterruptableIO = true,
                 bool enableTcUnicodeFonts = false,
                 bool enableBufferedVRAM = false);
    virtual ~TerraduinoFullUI();

    void addRemote(Terra_RemoteControl rcType,
                   UARTDeviceSetup rcSetup = UARTDeviceSetup(),
                   uint16_t rcServerPort = TERRA_UI_REMOTESERVER_PORT);

    virtual bool isFullUI() override;
};

#endif // /ifndef TerraduinoUI_H
#endif
