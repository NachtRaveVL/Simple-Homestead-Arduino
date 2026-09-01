/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Remote Controls
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraRemoteControls_H
#define TerraRemoteControls_H

class TerraRemoteControl;
class TerraRemoteSerialControl;
class TerraRemoteSimhubControl;
#ifdef TERRA_USE_WIFI
class TerraRemoteWiFiControl;
#endif
#ifdef TERRA_USE_ETHERNET
class TerraRemoteEthernetControl;
#endif

#include "TerraduinoUI.h"

// Remote Control Base
// Base remote control class.
class TerraRemoteControl {
public:
    virtual ~TerraRemoteControl() = default;

    // Connection accessor
    virtual BaseRemoteServerConnection *getConnection() = 0;
};


// Serial UART Remote Control
// Manages remote control over serial UART.
class TerraRemoteSerialControl : public TerraRemoteControl {
public:
    TerraRemoteSerialControl(UARTDeviceSetup serialSetup);
    virtual ~TerraRemoteSerialControl() = default;

    virtual BaseRemoteServerConnection *getConnection() override;

protected:
    SerialTagValueTransport _serialTransport;
    NoInitialisationNeeded _serialInitializer;
    TagValueRemoteServerConnection _serialConnection;
};


// Simhub Connector Remote Control
// Manages remote control over simhub connector.
class TerraRemoteSimhubControl : public TerraRemoteControl {
public:
    TerraRemoteSimhubControl(UARTDeviceSetup serialSetup, menuid_t statusMenuId);
    virtual ~TerraRemoteSimhubControl() = default;

    virtual BaseRemoteServerConnection *getConnection() override;

protected:
    SimHubRemoteConnection _simhubConnection;
};


#ifdef TERRA_USE_WIFI
// WiFi Remote Control
// Manages remote control over a WiFi connection.
class TerraRemoteWiFiControl : public TerraRemoteControl {
public:
    TerraRemoteWiFiControl(uint16_t listeningPort = TERRA_UI_REMOTESERVER_PORT);
    virtual ~TerraRemoteWiFiControl() = default;

    virtual BaseRemoteServerConnection *getConnection() override;

    inline WiFiServer &getRCServer() { return _rcServer; }

protected:
    WiFiServer _rcServer;
    WiFiInitialisation _netInitialisation;
    WiFiTagValTransport _netTransport;
    TagValueRemoteServerConnection _netConnection;
};
#endif


#ifdef TERRA_USE_ETHERNET
// Ethernet Remote Control
// Manages remote control over an Ethernet connection.
class TerraRemoteEthernetControl : public TerraRemoteControl {
public:
    TerraRemoteEthernetControl(uint16_t listeningPort = TERRA_UI_REMOTESERVER_PORT);
    virtual ~TerraRemoteEthernetControl() = default;

    virtual BaseRemoteServerConnection *getConnection() override;

    inline EthernetServer &getRCServer() { return _rcServer; }

protected:
    EthernetServer _rcServer;
    EthernetInitialisation _netInitialisation;
    EthernetTagValTransport _netTransport;
    TagValueRemoteServerConnection _netConnection;
};
#endif

#endif // /ifndef TerraRemoteControls_H
#endif
