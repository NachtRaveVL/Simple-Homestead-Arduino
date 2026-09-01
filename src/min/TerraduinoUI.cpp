/*  Terraduino: Minimal/RO UI scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include "TerraduinoUI.h"
#ifdef TERRA_USE_GUI

TerraduinoMinUI::TerraduinoMinUI(String deviceUUID, UIControlSetup uiControlSetup, UIDisplaySetup uiDisplaySetup, bool isActiveLowIO, bool allowInterruptableIO, bool enableTcUnicodeFonts, bool enableBufferedVRAM)
    : TerraduinoBaseUI(deviceUUID, uiControlSetup, uiDisplaySetup, isActiveLowIO, allowInterruptableIO, enableTcUnicodeFonts, enableBufferedVRAM)
{ ; }

TerraduinoMinUI::~TerraduinoMinUI()
{ ; }

void TerraduinoMinUI::allocateStandardControls() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateESP32TouchControl() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateResistiveTouchControl() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateTouchscreenControl() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateTFTTouchControl() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateLCDDisplay() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateSSD1305Display() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateSSD1305x32AdaDisplay() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateSSD1305x64AdaDisplay() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateSSD1306Display() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateSH1106Display() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateCustomOLEDDisplay() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateSSD1607Display() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateIL3820Display() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateIL3820V2Display() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateST7735Display() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateST7789Display() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateILI9341Display() { /* UI stub: device allocation intentionally deferred. */ }
void TerraduinoMinUI::allocateTFTDisplay() { /* UI stub: device allocation intentionally deferred. */ }

void TerraduinoMinUI::addSerialRemote(UARTDeviceSetup rcSetup)
{
    auto remoteControl = new TerraRemoteSerialControl(rcSetup);
    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(getApplicationInfo()); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else if (remoteControl) { delete remoteControl; }
}

void TerraduinoMinUI::addSimhubRemote(UARTDeviceSetup rcSetup)
{
    auto remoteControl = new TerraRemoteSimhubControl(rcSetup, TERRA_UI_SIMHUB_STATUS_MENU_ID);
    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(getApplicationInfo()); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else if (remoteControl) { delete remoteControl; }
}

void TerraduinoMinUI::addWiFiRemote(uint16_t rcServerPort)
{
#ifdef TERRA_USE_WIFI
    auto remoteControl = new TerraRemoteWiFiControl(rcServerPort);
    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(getApplicationInfo()); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else if (remoteControl) { delete remoteControl; }
#else
    (void)rcServerPort;
#endif
}

void TerraduinoMinUI::addEthernetRemote(uint16_t rcServerPort)
{
#ifdef TERRA_USE_ETHERNET
    auto remoteControl = new TerraRemoteEthernetControl(rcServerPort);
    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(getApplicationInfo()); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else if (remoteControl) { delete remoteControl; }
#else
    (void)rcServerPort;
#endif
}

bool TerraduinoMinUI::isFullUI()
{
    return false;
}

#endif
