/*  Terraduino: UI strings/prototypes.
    Copyright (C) 2026 NachtRaveVL
    Terraduino UI Strings/Prototypes
*/

#include <Terraduino.h>
#ifdef TERRA_USE_GUI
#ifndef TerraUIStrings_H
#define TerraUIStrings_H

enum TerraUI_String : unsigned short {
    TUIStr_Keys_MatrixActions,

    TUIStr_Item_Actuators,
    TUIStr_Item_AddNew,
    TUIStr_Item_Alerts,
    TUIStr_Item_AllowRemoteCtrl,
    TUIStr_Item_Altitude,
    TUIStr_Item_AssignByDHCP,
    TUIStr_Item_AssignByHostname,
    TUIStr_Item_AutosavePrimary,
    TUIStr_Item_AutosaveSecondary,
    TUIStr_Item_BackToOverview,
    TUIStr_Item_BatteryFailure,
    TUIStr_Item_Board,
    TUIStr_Item_BrokerPort,
    TUIStr_Item_Browse,
    TUIStr_Item_Calibrations,
    TUIStr_Item_ControlMode,
    TUIStr_Item_ControllerIP,
    TUIStr_Item_Controls,
    TUIStr_Item_DSTAddHour,
    TUIStr_Item_DataPolling,
    TUIStr_Item_Date,
    TUIStr_Item_Debug,
    TUIStr_Item_DisplayMode,
    TUIStr_Item_EEPROM,
    TUIStr_Item_Firmware,
    TUIStr_Item_FreeMemory,
    TUIStr_Item_GPSPolling,
    TUIStr_Item_General,
    TUIStr_Item_Information,
    TUIStr_Item_JoystickXMid,
    TUIStr_Item_JoystickXTol,
    TUIStr_Item_JoystickYMid,
    TUIStr_Item_JoystickYTol,
    TUIStr_Item_LatDegrees,
    TUIStr_Item_Library,
    TUIStr_Item_LocalTime,
    TUIStr_Item_Location,
    TUIStr_Item_LongMinutes,
    TUIStr_Item_MACAddr0x,
    TUIStr_Item_MQTTBroker,
    TUIStr_Item_Measurements,
    TUIStr_Item_Name,
    TUIStr_Item_Networking,
    TUIStr_Item_PowerRails,
    TUIStr_Item_RTC,
    TUIStr_Item_RemoteCtrlPort,
    TUIStr_Item_Scheduling,
    TUIStr_Item_SDCard,
    TUIStr_Item_SimhubConnected,
    TUIStr_Item_Sensors,
    TUIStr_Item_Settings,
    TUIStr_Item_Size,
    TUIStr_Item_System,
    TUIStr_Item_SystemMode,
    TUIStr_Item_SystemName,
    TUIStr_Item_Time,
    TUIStr_Item_TimeZone,
    TUIStr_Item_ToggleBadConn,
    TUIStr_Item_ToggleFastTime,
    TUIStr_Item_TriggerAutosave,
    TUIStr_Item_TriggerLowMem,
    TUIStr_Item_TriggerSDCleanup,
    TUIStr_Item_TriggerSigTime,
    TUIStr_Item_TriggerSigLocation,
    TUIStr_Item_Uptime,
    TUIStr_Item_WiFiPass,
    TUIStr_Item_WiFiSSID,
    TUIStr_Item_Water,
    TUIStr_Item_Environment,
    TUIStr_Item_Thermal,

    TUIStr_Enum_Autosave,
    TUIStr_Enum_DataPolling,
    TUIStr_Enum_GPSPolling,
    TUIStr_Enum_Measurements,
    TUIStr_Enum_SystemMode,
    TUIStr_Enum_TimeZone,

    TUIStr_Unit_MSL,
    TUIStr_Unit_Percent,

    TUIStr_Count
};

extern String stringFromPGM(TerraUI_String strNum);
extern void beginUIStringsFromEEPROM(uint16_t uiDataAddress);
extern void beginUIStringsFromSDCard(String uiDataFilePrefix);
#ifndef TERRA_DISABLE_BUILTIN_DATA
const char *pgmAddrForStr(TerraUI_String strNum);
#endif
extern size_t enumListPitch(const char *enumData);
inline size_t enumListPitch(TerraUI_String strNum) { return enumListPitch(CFP(strNum)); }

#endif // /ifndef TerraUIStrings_H
#endif
