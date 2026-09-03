/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Remote Controls
*/

#include "TerraduinoUI.h"
#ifdef TERRA_USE_GUI

TerraRemoteSerialControl::TerraRemoteSerialControl(UARTDeviceSetup serialSetup)
    : _serialTransport(serialSetup.serial), _serialInitializer(), _serialConnection(_serialTransport, _serialInitializer)
{ ; }

BaseRemoteServerConnection *TerraRemoteSerialControl::getConnection()
{
    return &_serialConnection;
}


TerraRemoteSimhubControl::TerraRemoteSimhubControl(UARTDeviceSetup serialSetup, menuid_t statusMenuId)
    : _simhubConnection(serialSetup.serial, statusMenuId)
{ ; }

BaseRemoteServerConnection *TerraRemoteSimhubControl::getConnection()
{
    return &_simhubConnection;
}


#ifdef TERRA_USE_WIFI

TerraRemoteWiFiControl::TerraRemoteWiFiControl(uint16_t listeningPort)
    : _rcServer(listeningPort), _netInitialisation(&_rcServer), _netTransport(), _netConnection(_netTransport, _netInitialisation)
{ ; }

BaseRemoteServerConnection *TerraRemoteWiFiControl::getConnection()
{
    return &_netConnection;
}

#endif // /ifdef TERRA_USE_WIFI


#ifdef TERRA_USE_ETHERNET

TerraRemoteEthernetControl::TerraRemoteEthernetControl(uint16_t listeningPort)
    : _rcServer(listeningPort), _netInitialisation(&_rcServer), _netTransport(), _netConnection(_netTransport, _netInitialisation)
{ ; }

BaseRemoteServerConnection *TerraRemoteEthernetControl::getConnection()
{
    return &_netConnection;
}

#endif // /ifdef TERRA_USE_ETHERNET

#endif
