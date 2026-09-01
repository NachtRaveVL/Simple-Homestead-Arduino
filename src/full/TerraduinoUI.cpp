/*  Terraduino: Full/RW UI scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include "TerraduinoUI.h"
#ifdef TERRA_USE_GUI

TerraduinoFullUI::TerraduinoFullUI(String deviceUUID, UIControlSetup uiControlSetup, UIDisplaySetup uiDisplaySetup, bool isActiveLowIO, bool allowInterruptableIO, bool enableTcUnicodeFonts, bool enableBufferedVRAM)
    : TerraduinoBaseUI(deviceUUID, uiControlSetup, uiDisplaySetup, isActiveLowIO, allowInterruptableIO, enableTcUnicodeFonts, enableBufferedVRAM)
{
    // UI stub: Automatic display/input allocation is intentionally deferred with the main UI implementation.
}

TerraduinoFullUI::~TerraduinoFullUI()
{ ; }

void TerraduinoFullUI::addRemote(Terra_RemoteControl rcType, UARTDeviceSetup rcSetup, uint16_t rcServerPort)
{
    TerraRemoteControl *remoteControl = nullptr;
    switch (rcType) {
        case Terra_RemoteControl_Serial: remoteControl = new TerraRemoteSerialControl(rcSetup); break;
        case Terra_RemoteControl_Simhub: remoteControl = new TerraRemoteSimhubControl(rcSetup, TERRA_UI_SIMHUB_STATUS_MENU_ID); break;
#ifdef TERRA_USE_WIFI
        case Terra_RemoteControl_WiFi: remoteControl = new TerraRemoteWiFiControl(rcServerPort); break;
#endif
#ifdef TERRA_USE_ETHERNET
        case Terra_RemoteControl_Ethernet: remoteControl = new TerraRemoteEthernetControl(rcServerPort); break;
#endif
        default: break;
    }

    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(getApplicationInfo()); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else if (remoteControl) {
        delete remoteControl;
    }
}

bool TerraduinoFullUI::isFullUI()
{
    return true;
}

#endif
