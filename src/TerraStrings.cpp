/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Strings
*/

#include "Terraduino.h"
#include "TerraUtils.h"

static char _terraBlank = '\0';
const char *TStr_Blank = &_terraBlank;

static StringProvider _terraStringProvider = nullptr;
static void *_terraStringProviderContext = nullptr;

void setStringProvider(StringProvider provider, void *context)
{
    _terraStringProvider = provider;
    _terraStringProviderContext = context;
}

String stringFromPGM(Terra_String strNum)
{
#ifndef TERRA_DISABLE_BUILTIN_DATA
    return stringFromPGMAddr(pgmAddrForStr(strNum));
#else
    String value;
    return _terraStringProvider && _terraStringProvider(_terraStringProviderContext, strNum, &value) ? value : String();
#endif
}

String stringFromPGMAddr(const char *flashStr)
{
    if (!flashStr) { return String(); }
#if defined(ARDUINO)
    String value;
    char ch;
    while ((ch = (char)pgm_read_byte(flashStr++)) != '\0') { value += ch; }
    return value;
#else
    return String(flashStr);
#endif
}

#ifndef TERRA_DISABLE_BUILTIN_DATA
const char *pgmAddrForStr(Terra_String strNum)
{
    switch (strNum) {
        case TStr_ColonSpace: {
            static const char flashStr[] PROGMEM = ": ";
            return flashStr;
        }
        case TStr_DoubleSpace: {
            static const char flashStr[] PROGMEM = "  ";
            return flashStr;
        }
        case TStr_csv: {
            static const char flashStr[] PROGMEM = "csv";
            return flashStr;
        }
        case TStr_dat: {
            static const char flashStr[] PROGMEM = "dat";
            return flashStr;
        }
        case TStr_Disabled: {
            static const char flashStr[] PROGMEM = "Disabled";
            return flashStr;
        }
        case TStr_raw: {
            static const char flashStr[] PROGMEM = "raw";
            return flashStr;
        }
        case TStr_txt: {
            static const char flashStr[] PROGMEM = "txt";
            return flashStr;
        }
        case TStr_Undefined: {
            static const char flashStr[] PROGMEM = "Undefined";
            return flashStr;
        }
        case TStr_null: {
            static const char flashStr[] PROGMEM = "null";
            return flashStr;
        }
        case TStr_Default_SystemName: {
            static const char flashStr[] PROGMEM = "Terraduino";
            return flashStr;
        }
        case TStr_Default_ConfigFilename: {
            static const char flashStr[] PROGMEM = "Terraduino.cfg";
            return flashStr;
        }
        case TStr_Err_AllocationFailure: {
            static const char flashStr[] PROGMEM = "Allocation failure";
            return flashStr;
        }
        case TStr_Err_AlreadyInitialized: {
            static const char flashStr[] PROGMEM = "Already initialized";
            return flashStr;
        }
        case TStr_Err_AssertionFailure: {
            static const char flashStr[] PROGMEM = "Assertion failure";
            return flashStr;
        }
        case TStr_Err_ExportFailure: {
            static const char flashStr[] PROGMEM = "Export failure";
            return flashStr;
        }
        case TStr_Err_ImportFailure: {
            static const char flashStr[] PROGMEM = "Import failure";
            return flashStr;
        }
        case TStr_Err_InitializationFailure: {
            static const char flashStr[] PROGMEM = "Initialization failure";
            return flashStr;
        }
        case TStr_Err_InvalidParameter: {
            static const char flashStr[] PROGMEM = "Invalid parameter";
            return flashStr;
        }
        case TStr_Err_InvalidPinOrType: {
            static const char flashStr[] PROGMEM = "Invalid pin or type";
            return flashStr;
        }
        case TStr_Err_MeasurementFailure: {
            static const char flashStr[] PROGMEM = "Measurement failure";
            return flashStr;
        }
        case TStr_Err_MissingLinkage: {
            static const char flashStr[] PROGMEM = "Missing linkage";
            return flashStr;
        }
        case TStr_Err_NoPositionsAvailable: {
            static const char flashStr[] PROGMEM = "No positions available";
            return flashStr;
        }
        case TStr_Err_NotConfiguredProperly: {
            static const char flashStr[] PROGMEM = "Not configured properly";
            return flashStr;
        }
        case TStr_Err_NotYetInitialized: {
            static const char flashStr[] PROGMEM = "Not yet initialized";
            return flashStr;
        }
        case TStr_Err_OperationFailure: {
            static const char flashStr[] PROGMEM = "Operation failure";
            return flashStr;
        }
        case TStr_Err_UnsupportedOperation: {
            static const char flashStr[] PROGMEM = "Unsupported operation";
            return flashStr;
        }
        case TStr_Log_RTCBatteryFailure: {
            static const char flashStr[] PROGMEM = "RTC battery failure";
            return flashStr;
        }
        case TStr_Log_SystemDataSaved: {
            static const char flashStr[] PROGMEM = "System data saved";
            return flashStr;
        }
        case TStr_Log_SystemUptime: {
            static const char flashStr[] PROGMEM = "System uptime";
            return flashStr;
        }
        case TStr_Log_CalculatedPumping: {
            static const char flashStr[] PROGMEM = " pumping request:";
            return flashStr;
        }
        case TStr_Log_MeasuredPumping: {
            static const char flashStr[] PROGMEM = " pumping result:";
            return flashStr;
        }
        case TStr_Log_HasDisabled: {
            static const char flashStr[] PROGMEM = " has disabled";
            return flashStr;
        }
        case TStr_Log_HasEnabled: {
            static const char flashStr[] PROGMEM = " has enabled";
            return flashStr;
        }
        case TStr_Log_Prefix_Info: {
            static const char flashStr[] PROGMEM = "[INFO] ";
            return flashStr;
        }
        case TStr_Log_Prefix_Warning: {
            static const char flashStr[] PROGMEM = "[WARN] ";
            return flashStr;
        }
        case TStr_Log_Prefix_Error: {
            static const char flashStr[] PROGMEM = "[ERROR] ";
            return flashStr;
        }
        case TStr_Log_Field_Destination_Reservoir: {
            static const char flashStr[] PROGMEM = "  To reservoir: ";
            return flashStr;
        }
        case TStr_Log_Field_Source_Reservoir: {
            static const char flashStr[] PROGMEM = "  From reservoir: ";
            return flashStr;
        }
        case TStr_Log_Field_Time_Calculated: {
            static const char flashStr[] PROGMEM = "  Pump run time: ";
            return flashStr;
        }
        case TStr_Log_Field_Time_Measured: {
            static const char flashStr[] PROGMEM = "  Elapsed time: ";
            return flashStr;
        }
        case TStr_Log_Field_Vol_Calculated: {
            static const char flashStr[] PROGMEM = "  Est. pumped vol.: ";
            return flashStr;
        }
        case TStr_Log_Field_Vol_Measured: {
            static const char flashStr[] PROGMEM = "  Act. pumped vol.: ";
            return flashStr;
        }
        case TStr_Key_ActiveLow: {
            static const char flashStr[] PROGMEM = "activeLow";
            return flashStr;
        }
        case TStr_Key_AirTemperatureSensor: {
            static const char flashStr[] PROGMEM = "airTemperatureSensor";
            return flashStr;
        }
        case TStr_Key_AlwaysFilled: {
            static const char flashStr[] PROGMEM = "alwaysFilled";
            return flashStr;
        }
        case TStr_Key_AreaSquareMeters: {
            static const char flashStr[] PROGMEM = "areaSquareMeters";
            return flashStr;
        }
        case TStr_Key_AutosaveEnabled: {
            static const char flashStr[] PROGMEM = "autosaveEnabled";
            return flashStr;
        }
        case TStr_Key_AutosaveFallback: {
            static const char flashStr[] PROGMEM = "autosaveFallback";
            return flashStr;
        }
        case TStr_Key_AutosaveInterval: {
            static const char flashStr[] PROGMEM = "autosaveInterval";
            return flashStr;
        }
        case TStr_Key_BitRes: {
            static const char flashStr[] PROGMEM = "bitRes";
            return flashStr;
        }
        case TStr_Key_CalibrationUnits: {
            static const char flashStr[] PROGMEM = "calibrationUnits";
            return flashStr;
        }
        case TStr_Key_Channel: {
            static const char flashStr[] PROGMEM = "channel";
            return flashStr;
        }
        case TStr_Key_Circulator: {
            static const char flashStr[] PROGMEM = "circulator";
            return flashStr;
        }
        case TStr_Key_CollectionEfficiency: {
            static const char flashStr[] PROGMEM = "collectionEfficiency";
            return flashStr;
        }
        case TStr_Key_ContinuousFlowRate: {
            static const char flashStr[] PROGMEM = "continuousFlowRate";
            return flashStr;
        }
        case TStr_Key_ContinuousPowerUsage: {
            static const char flashStr[] PROGMEM = "continuousPowerUsage";
            return flashStr;
        }
        case TStr_Key_CtrlInMode: {
            static const char flashStr[] PROGMEM = "ctrlInMode";
            return flashStr;
        }
        case TStr_Key_DataFilePrefix: {
            static const char flashStr[] PROGMEM = "dataFilePrefix";
            return flashStr;
        }
        case TStr_Key_Destination: {
            static const char flashStr[] PROGMEM = "destination";
            return flashStr;
        }
        case TStr_Key_DestinationStartPercent: {
            static const char flashStr[] PROGMEM = "destinationStartPercent";
            return flashStr;
        }
        case TStr_Key_DestinationStopPercent: {
            static const char flashStr[] PROGMEM = "destinationStopPercent";
            return flashStr;
        }
        case TStr_Key_DetriggerDelay: {
            static const char flashStr[] PROGMEM = "detriggerDelay";
            return flashStr;
        }
        case TStr_Key_DetriggerTol: {
            static const char flashStr[] PROGMEM = "detriggerTol";
            return flashStr;
        }
        case TStr_Key_DispOutMode: {
            static const char flashStr[] PROGMEM = "dispOutMode";
            return flashStr;
        }
        case TStr_Key_EmptyTrigger: {
            static const char flashStr[] PROGMEM = "emptyTrigger";
            return flashStr;
        }
        case TStr_Key_EnableMode: {
            static const char flashStr[] PROGMEM = "enableMode";
            return flashStr;
        }
        case TStr_Key_FilledTrigger: {
            static const char flashStr[] PROGMEM = "filledTrigger";
            return flashStr;
        }
        case TStr_Key_FlowRateSensor: {
            static const char flashStr[] PROGMEM = "flowRateSensor";
            return flashStr;
        }
        case TStr_Key_FlowRateUnits: {
            static const char flashStr[] PROGMEM = "flowRateUnits";
            return flashStr;
        }
        case TStr_Key_FlowSensor: {
            static const char flashStr[] PROGMEM = "flowSensor";
            return flashStr;
        }
        case TStr_Key_HighTrigger: {
            static const char flashStr[] PROGMEM = "highTrigger";
            return flashStr;
        }
        case TStr_Key_HumiditySensor: {
            static const char flashStr[] PROGMEM = "humiditySensor";
            return flashStr;
        }
        case TStr_Key_Id: {
            static const char flashStr[] PROGMEM = "id";
            return flashStr;
        }
        case TStr_Key_InputPin: {
            static const char flashStr[] PROGMEM = "inputPin";
            return flashStr;
        }
        case TStr_Key_LevelSensor: {
            static const char flashStr[] PROGMEM = "levelSensor";
            return flashStr;
        }
        case TStr_Key_LimitTrigger: {
            static const char flashStr[] PROGMEM = "limitTrigger";
            return flashStr;
        }
        case TStr_Key_Location: {
            static const char flashStr[] PROGMEM = "location";
            return flashStr;
        }
        case TStr_Key_LogFilePrefix: {
            static const char flashStr[] PROGMEM = "logFilePrefix";
            return flashStr;
        }
        case TStr_Key_LogLevel: {
            static const char flashStr[] PROGMEM = "logLevel";
            return flashStr;
        }
        case TStr_Key_Logger: {
            static const char flashStr[] PROGMEM = "logger";
            return flashStr;
        }
        case TStr_Key_LogToSDCard: {
            static const char flashStr[] PROGMEM = "logToSDCard";
            return flashStr;
        }
        case TStr_Key_LogToWiFiStorage: {
            static const char flashStr[] PROGMEM = "logToWiFiStorage";
            return flashStr;
        }
        case TStr_Key_LowTrigger: {
            static const char flashStr[] PROGMEM = "lowTrigger";
            return flashStr;
        }
        case TStr_Key_MACAddress: {
            static const char flashStr[] PROGMEM = "macAddress";
            return flashStr;
        }
        case TStr_Key_MaxActiveAtOnce: {
            static const char flashStr[] PROGMEM = "maxActiveAtOnce";
            return flashStr;
        }
        case TStr_Key_MaxContinuousMs: {
            static const char flashStr[] PROGMEM = "maxContinuousMs";
            return flashStr;
        }
        case TStr_Key_MaximumFlowLpm: {
            static const char flashStr[] PROGMEM = "maximumFlowLpm";
            return flashStr;
        }
        case TStr_Key_MaxPower: {
            static const char flashStr[] PROGMEM = "maxPower";
            return flashStr;
        }
        case TStr_Key_MaxStoreTempC: {
            static const char flashStr[] PROGMEM = "maxStoreTempC";
            return flashStr;
        }
        case TStr_Key_MaxTemperature: {
            static const char flashStr[] PROGMEM = "maxTemperature";
            return flashStr;
        }
        case TStr_Key_MaxVolume: {
            static const char flashStr[] PROGMEM = "maxVolume";
            return flashStr;
        }
        case TStr_Key_MeasurementRow: {
            static const char flashStr[] PROGMEM = "measurementRow";
            return flashStr;
        }
        case TStr_Key_MeasurementUnits: {
            static const char flashStr[] PROGMEM = "measurementUnits";
            return flashStr;
        }
        case TStr_Key_MeasureMode: {
            static const char flashStr[] PROGMEM = "measureMode";
            return flashStr;
        }
        case TStr_Key_MinimumFlowLpm: {
            static const char flashStr[] PROGMEM = "minimumFlowLpm";
            return flashStr;
        }
        case TStr_Key_Mode: {
            static const char flashStr[] PROGMEM = "mode";
            return flashStr;
        }
        case TStr_Key_Multiplier: {
            static const char flashStr[] PROGMEM = "multiplier";
            return flashStr;
        }
        case TStr_Key_OffDifferentialC: {
            static const char flashStr[] PROGMEM = "offDifferentialC";
            return flashStr;
        }
        case TStr_Key_Offset: {
            static const char flashStr[] PROGMEM = "offset";
            return flashStr;
        }
        case TStr_Key_OnDifferentialC: {
            static const char flashStr[] PROGMEM = "onDifferentialC";
            return flashStr;
        }
        case TStr_Key_OutputPin: {
            static const char flashStr[] PROGMEM = "outputPin";
            return flashStr;
        }
        case TStr_Key_OutputReservoir: {
            static const char flashStr[] PROGMEM = "outputReservoir";
            return flashStr;
        }
        case TStr_Key_Pin: {
            static const char flashStr[] PROGMEM = "pin";
            return flashStr;
        }
        case TStr_Key_PollingInterval: {
            static const char flashStr[] PROGMEM = "pollingInterval";
            return flashStr;
        }
        case TStr_Key_PowerUsageSensor: {
            static const char flashStr[] PROGMEM = "powerUsageSensor";
            return flashStr;
        }
        case TStr_Key_PowerUnits: {
            static const char flashStr[] PROGMEM = "powerUnits";
            return flashStr;
        }
        case TStr_Key_PressureSensor: {
            static const char flashStr[] PROGMEM = "pressureSensor";
            return flashStr;
        }
        case TStr_Key_PublishToSDCard: {
            static const char flashStr[] PROGMEM = "pubToSDCard";
            return flashStr;
        }
        case TStr_Key_PublishToWiFiStorage: {
            static const char flashStr[] PROGMEM = "pubToWiFiStorage";
            return flashStr;
        }
        case TStr_Key_Publisher: {
            static const char flashStr[] PROGMEM = "publisher";
            return flashStr;
        }
        case TStr_Key_PWMChannel: {
            static const char flashStr[] PROGMEM = "pwmChannel";
            return flashStr;
        }
        case TStr_Key_PWMFrequency: {
            static const char flashStr[] PROGMEM = "pwmFrequency";
            return flashStr;
        }
        case TStr_Key_Pump: {
            static const char flashStr[] PROGMEM = "pump";
            return flashStr;
        }
        case TStr_Key_RailName: {
            static const char flashStr[] PROGMEM = "railName";
            return flashStr;
        }
        case TStr_Key_RainfallSensor: {
            static const char flashStr[] PROGMEM = "rainfallSensor";
            return flashStr;
        }
        case TStr_Key_RainRateSensor: {
            static const char flashStr[] PROGMEM = "rainRateSensor";
            return flashStr;
        }
        case TStr_Key_ReportInterval: {
            static const char flashStr[] PROGMEM = "reportInterval";
            return flashStr;
        }
        case TStr_Key_ReportedType: {
            static const char flashStr[] PROGMEM = "reportedType";
            return flashStr;
        }
        case TStr_Key_ReservoirName: {
            static const char flashStr[] PROGMEM = "reservoirName";
            return flashStr;
        }
        case TStr_Key_Revision: {
            static const char flashStr[] PROGMEM = "revision";
            return flashStr;
        }
        case TStr_Key_Scheduler: {
            static const char flashStr[] PROGMEM = "scheduler";
            return flashStr;
        }
        case TStr_Key_SensorName: {
            static const char flashStr[] PROGMEM = "sensorName";
            return flashStr;
        }
        case TStr_Key_SolarRadiationSensor: {
            static const char flashStr[] PROGMEM = "solarRadiationSensor";
            return flashStr;
        }
        case TStr_Key_Source: {
            static const char flashStr[] PROGMEM = "source";
            return flashStr;
        }
        case TStr_Key_SourceTemperatureSensor: {
            static const char flashStr[] PROGMEM = "sourceTemperatureSensor";
            return flashStr;
        }
        case TStr_Key_StaleAfterMs: {
            static const char flashStr[] PROGMEM = "staleAfterMs";
            return flashStr;
        }
        case TStr_Key_Store: {
            static const char flashStr[] PROGMEM = "store";
            return flashStr;
        }
        case TStr_Key_SumpAlarmPercent: {
            static const char flashStr[] PROGMEM = "sumpAlarmPercent";
            return flashStr;
        }
        case TStr_Key_SumpStartPercent: {
            static const char flashStr[] PROGMEM = "sumpStartPercent";
            return flashStr;
        }
        case TStr_Key_SumpStopPercent: {
            static const char flashStr[] PROGMEM = "sumpStopPercent";
            return flashStr;
        }
        case TStr_Key_SystemMode: {
            static const char flashStr[] PROGMEM = "systemMode";
            return flashStr;
        }
        case TStr_Key_SystemName: {
            static const char flashStr[] PROGMEM = "systemName";
            return flashStr;
        }
        case TStr_Key_TemperatureSensor: {
            static const char flashStr[] PROGMEM = "temperatureSensor";
            return flashStr;
        }
        case TStr_Key_TemperatureUnits: {
            static const char flashStr[] PROGMEM = "temperatureUnits";
            return flashStr;
        }
        case TStr_Key_TimeZoneOffset: {
            static const char flashStr[] PROGMEM = "timeZoneOffset";
            return flashStr;
        }
        case TStr_Key_Timestamp: {
            static const char flashStr[] PROGMEM = "timestamp";
            return flashStr;
        }
        case TStr_Key_Tolerance: {
            static const char flashStr[] PROGMEM = "tolerance";
            return flashStr;
        }
        case TStr_Key_ToleranceHigh: {
            static const char flashStr[] PROGMEM = "toleranceHigh";
            return flashStr;
        }
        case TStr_Key_ToleranceLow: {
            static const char flashStr[] PROGMEM = "toleranceLow";
            return flashStr;
        }
        case TStr_Key_TriggerBelow: {
            static const char flashStr[] PROGMEM = "triggerBelow";
            return flashStr;
        }
        case TStr_Key_TriggerOutside: {
            static const char flashStr[] PROGMEM = "triggerOutside";
            return flashStr;
        }
        case TStr_Key_Type: {
            static const char flashStr[] PROGMEM = "type";
            return flashStr;
        }
        case TStr_Key_Units: {
            static const char flashStr[] PROGMEM = "units";
            return flashStr;
        }
        case TStr_Key_Value: {
            static const char flashStr[] PROGMEM = "value";
            return flashStr;
        }
        case TStr_Key_Version: {
            static const char flashStr[] PROGMEM = "version";
            return flashStr;
        }
        case TStr_Key_VolumeSensor: {
            static const char flashStr[] PROGMEM = "volumeSensor";
            return flashStr;
        }
        case TStr_Key_VolumeUnits: {
            static const char flashStr[] PROGMEM = "volumeUnits";
            return flashStr;
        }
        case TStr_Key_WaterTemperatureSensor: {
            static const char flashStr[] PROGMEM = "waterTemperatureSensor";
            return flashStr;
        }
        case TStr_Key_WiFiPassword: {
            static const char flashStr[] PROGMEM = "wifiPassword";
            return flashStr;
        }
        case TStr_Key_WiFiPasswordSeed: {
            static const char flashStr[] PROGMEM = "wifiPasswordSeed";
            return flashStr;
        }
        case TStr_Key_WiFiSSID: {
            static const char flashStr[] PROGMEM = "wifiSSID";
            return flashStr;
        }
        case TStr_Key_WindDirectionSensor: {
            static const char flashStr[] PROGMEM = "windDirectionSensor";
            return flashStr;
        }
        case TStr_Key_WindSpeedSensor: {
            static const char flashStr[] PROGMEM = "windSpeedSensor";
            return flashStr;
        }
        case TStr_Enum_AC110V: {
            static const char flashStr[] PROGMEM = "AC110V";
            return flashStr;
        }
        case TStr_Enum_AC220V: {
            static const char flashStr[] PROGMEM = "AC220V";
            return flashStr;
        }
        case TStr_Enum_Active: {
            static const char flashStr[] PROGMEM = "Active";
            return flashStr;
        }
        case TStr_Enum_Analog: {
            static const char flashStr[] PROGMEM = "Analog";
            return flashStr;
        }
        case TStr_Enum_AnalogInput: {
            static const char flashStr[] PROGMEM = "AnalogInput";
            return flashStr;
        }
        case TStr_Enum_AnalogJoystick: {
            static const char flashStr[] PROGMEM = "AnalogJoystick";
            return flashStr;
        }
        case TStr_Enum_AnalogOutput: {
            static const char flashStr[] PROGMEM = "AnalogOutput";
            return flashStr;
        }
        case TStr_Enum_Angle: {
            static const char flashStr[] PROGMEM = "Angle";
            return flashStr;
        }
        case TStr_Enum_AscOrder: {
            static const char flashStr[] PROGMEM = "AscOrder";
            return flashStr;
        }
        case TStr_Enum_Automatic: {
            static const char flashStr[] PROGMEM = "Automatic";
            return flashStr;
        }
        case TStr_Enum_Average: {
            static const char flashStr[] PROGMEM = "Average";
            return flashStr;
        }
        case TStr_Enum_Binary: {
            static const char flashStr[] PROGMEM = "Binary";
            return flashStr;
        }
        case TStr_Enum_Circulator: {
            static const char flashStr[] PROGMEM = "Circulator";
            return flashStr;
        }
        case TStr_Enum_Complete: {
            static const char flashStr[] PROGMEM = "Complete";
            return flashStr;
        }
        case TStr_Enum_Count: {
            static const char flashStr[] PROGMEM = "Count";
            return flashStr;
        }
        case TStr_Enum_Current: {
            static const char flashStr[] PROGMEM = "Current";
            return flashStr;
        }
        case TStr_Enum_Custom: {
            static const char flashStr[] PROGMEM = "Custom";
            return flashStr;
        }
        case TStr_Enum_CustomOLED: {
            static const char flashStr[] PROGMEM = "CustomOLED";
            return flashStr;
        }
        case TStr_Enum_DC12V: {
            static const char flashStr[] PROGMEM = "DC12V";
            return flashStr;
        }
        case TStr_Enum_DC24V: {
            static const char flashStr[] PROGMEM = "DC24V";
            return flashStr;
        }
        case TStr_Enum_DC3V3: {
            static const char flashStr[] PROGMEM = "DC3V3";
            return flashStr;
        }
        case TStr_Enum_DC48V: {
            static const char flashStr[] PROGMEM = "DC48V";
            return flashStr;
        }
        case TStr_Enum_DC5V: {
            static const char flashStr[] PROGMEM = "DC5V";
            return flashStr;
        }
        case TStr_Enum_DescOrder: {
            static const char flashStr[] PROGMEM = "DescOrder";
            return flashStr;
        }
        case TStr_Enum_Digital: {
            static const char flashStr[] PROGMEM = "Digital";
            return flashStr;
        }
        case TStr_Enum_DigitalInput: {
            static const char flashStr[] PROGMEM = "DigitalInput";
            return flashStr;
        }
        case TStr_Enum_DigitalInputPullDown: {
            static const char flashStr[] PROGMEM = "DigitalInputPullDown";
            return flashStr;
        }
        case TStr_Enum_DigitalInputPullUp: {
            static const char flashStr[] PROGMEM = "DigitalInputPullUp";
            return flashStr;
        }
        case TStr_Enum_DigitalOutput: {
            static const char flashStr[] PROGMEM = "DigitalOutput";
            return flashStr;
        }
        case TStr_Enum_DigitalOutputPushPull: {
            static const char flashStr[] PROGMEM = "DigitalOutputPushPull";
            return flashStr;
        }
        case TStr_Enum_Display: {
            static const char flashStr[] PROGMEM = "Display";
            return flashStr;
        }
        case TStr_Enum_Distance: {
            static const char flashStr[] PROGMEM = "Distance";
            return flashStr;
        }
        case TStr_Enum_Diverter: {
            static const char flashStr[] PROGMEM = "Diverter";
            return flashStr;
        }
        case TStr_Enum_Energy: {
            static const char flashStr[] PROGMEM = "Energy";
            return flashStr;
        }
        case TStr_Enum_Equal: {
            static const char flashStr[] PROGMEM = "Equal";
            return flashStr;
        }
        case TStr_Enum_Fan: {
            static const char flashStr[] PROGMEM = "Fan";
            return flashStr;
        }
        case TStr_Enum_Fault: {
            static const char flashStr[] PROGMEM = "Fault";
            return flashStr;
        }
        case TStr_Enum_Flow: {
            static const char flashStr[] PROGMEM = "Flow";
            return flashStr;
        }
        case TStr_Enum_FlowSensor: {
            static const char flashStr[] PROGMEM = "FlowSensor";
            return flashStr;
        }
        case TStr_Enum_GreaterOrEqual: {
            static const char flashStr[] PROGMEM = "GreaterOrEqual";
            return flashStr;
        }
        case TStr_Enum_GreaterThan: {
            static const char flashStr[] PROGMEM = "GreaterThan";
            return flashStr;
        }
        case TStr_Enum_Heater: {
            static const char flashStr[] PROGMEM = "Heater";
            return flashStr;
        }
        case TStr_Enum_High: {
            static const char flashStr[] PROGMEM = "High";
            return flashStr;
        }
        case TStr_Enum_Highest: {
            static const char flashStr[] PROGMEM = "Highest";
            return flashStr;
        }
        case TStr_Enum_Humidity: {
            static const char flashStr[] PROGMEM = "Humidity";
            return flashStr;
        }
        case TStr_Enum_HumiditySensor: {
            static const char flashStr[] PROGMEM = "HumiditySensor";
            return flashStr;
        }
        case TStr_Enum_Idle: {
            static const char flashStr[] PROGMEM = "Idle";
            return flashStr;
        }
        case TStr_Enum_IL3820: {
            static const char flashStr[] PROGMEM = "IL3820";
            return flashStr;
        }
        case TStr_Enum_IL3820V2: {
            static const char flashStr[] PROGMEM = "IL3820V2";
            return flashStr;
        }
        case TStr_Enum_ILI9341: {
            static const char flashStr[] PROGMEM = "ILI9341";
            return flashStr;
        }
        case TStr_Enum_Imperial: {
            static const char flashStr[] PROGMEM = "Imperial";
            return flashStr;
        }
        case TStr_Enum_Inactive: {
            static const char flashStr[] PROGMEM = "Inactive";
            return flashStr;
        }
        case TStr_Enum_InOrder: {
            static const char flashStr[] PROGMEM = "InOrder";
            return flashStr;
        }
        case TStr_Enum_IOExpander: {
            static const char flashStr[] PROGMEM = "IOExpander";
            return flashStr;
        }
        case TStr_Enum_Irradiance: {
            static const char flashStr[] PROGMEM = "Irradiance";
            return flashStr;
        }
        case TStr_Enum_LCD16x2: {
            static const char flashStr[] PROGMEM = "LCD16x2";
            return flashStr;
        }
        case TStr_Enum_LCD20x4: {
            static const char flashStr[] PROGMEM = "LCD20x4";
            return flashStr;
        }
        case TStr_Enum_Leak: {
            static const char flashStr[] PROGMEM = "Leak";
            return flashStr;
        }
        case TStr_Enum_LeakSensor: {
            static const char flashStr[] PROGMEM = "LeakSensor";
            return flashStr;
        }
        case TStr_Enum_LessOrEqual: {
            static const char flashStr[] PROGMEM = "LessOrEqual";
            return flashStr;
        }
        case TStr_Enum_LessThan: {
            static const char flashStr[] PROGMEM = "LessThan";
            return flashStr;
        }
        case TStr_Enum_Level: {
            static const char flashStr[] PROGMEM = "Level";
            return flashStr;
        }
        case TStr_Enum_LevelSensor: {
            static const char flashStr[] PROGMEM = "LevelSensor";
            return flashStr;
        }
        case TStr_Enum_LiquidFlowRate: {
            static const char flashStr[] PROGMEM = "LiquidFlowRate";
            return flashStr;
        }
        case TStr_Enum_LiquidVolume: {
            static const char flashStr[] PROGMEM = "LiquidVolume";
            return flashStr;
        }
        case TStr_Enum_Low: {
            static const char flashStr[] PROGMEM = "Low";
            return flashStr;
        }
        case TStr_Enum_Lowest: {
            static const char flashStr[] PROGMEM = "Lowest";
            return flashStr;
        }
        case TStr_Enum_Manual: {
            static const char flashStr[] PROGMEM = "Manual";
            return flashStr;
        }
        case TStr_Enum_Matrix2x2: {
            static const char flashStr[] PROGMEM = "Matrix2x2";
            return flashStr;
        }
        case TStr_Enum_Matrix3x4: {
            static const char flashStr[] PROGMEM = "Matrix3x4";
            return flashStr;
        }
        case TStr_Enum_Matrix4x4: {
            static const char flashStr[] PROGMEM = "Matrix4x4";
            return flashStr;
        }
        case TStr_Enum_Metric: {
            static const char flashStr[] PROGMEM = "Metric";
            return flashStr;
        }
        case TStr_Enum_Multiply: {
            static const char flashStr[] PROGMEM = "Multiply";
            return flashStr;
        }
        case TStr_Enum_Municipal: {
            static const char flashStr[] PROGMEM = "Municipal";
            return flashStr;
        }
        case TStr_Enum_Network: {
            static const char flashStr[] PROGMEM = "Network";
            return flashStr;
        }
        case TStr_Enum_Normal: {
            static const char flashStr[] PROGMEM = "Normal";
            return flashStr;
        }
        case TStr_Enum_NotEqual: {
            static const char flashStr[] PROGMEM = "NotEqual";
            return flashStr;
        }
        case TStr_Enum_Percentile: {
            static const char flashStr[] PROGMEM = "Percentile";
            return flashStr;
        }
        case TStr_Enum_Power: {
            static const char flashStr[] PROGMEM = "Power";
            return flashStr;
        }
        case TStr_Enum_Pressure: {
            static const char flashStr[] PROGMEM = "Pressure";
            return flashStr;
        }
        case TStr_Enum_PressureSensor: {
            static const char flashStr[] PROGMEM = "PressureSensor";
            return flashStr;
        }
        case TStr_Enum_Pump: {
            static const char flashStr[] PROGMEM = "Pump";
            return flashStr;
        }
        case TStr_Enum_Radio: {
            static const char flashStr[] PROGMEM = "Radio";
            return flashStr;
        }
        case TStr_Enum_Rainfall: {
            static const char flashStr[] PROGMEM = "Rainfall";
            return flashStr;
        }
        case TStr_Enum_RainfallSensor: {
            static const char flashStr[] PROGMEM = "RainfallSensor";
            return flashStr;
        }
        case TStr_Enum_RainRate: {
            static const char flashStr[] PROGMEM = "RainRate";
            return flashStr;
        }
        case TStr_Enum_Rainwater: {
            static const char flashStr[] PROGMEM = "Rainwater";
            return flashStr;
        }
        case TStr_Enum_Raw: {
            static const char flashStr[] PROGMEM = "Raw";
            return flashStr;
        }
        case TStr_Enum_Reclaimed: {
            static const char flashStr[] PROGMEM = "Reclaimed";
            return flashStr;
        }
        case TStr_Enum_Remote: {
            static const char flashStr[] PROGMEM = "Remote";
            return flashStr;
        }
        case TStr_Enum_RemoteControl: {
            static const char flashStr[] PROGMEM = "RemoteControl";
            return flashStr;
        }
        case TStr_Enum_Requested: {
            static const char flashStr[] PROGMEM = "Requested";
            return flashStr;
        }
        case TStr_Enum_Reserve: {
            static const char flashStr[] PROGMEM = "Reserve";
            return flashStr;
        }
        case TStr_Enum_ResistiveTouch: {
            static const char flashStr[] PROGMEM = "ResistiveTouch";
            return flashStr;
        }
        case TStr_Enum_RevOrder: {
            static const char flashStr[] PROGMEM = "RevOrder";
            return flashStr;
        }
        case TStr_Enum_RotaryEncoder: {
            static const char flashStr[] PROGMEM = "RotaryEncoder";
            return flashStr;
        }
        case TStr_Enum_RTC: {
            static const char flashStr[] PROGMEM = "RTC";
            return flashStr;
        }
        case TStr_Enum_Scientific: {
            static const char flashStr[] PROGMEM = "Scientific";
            return flashStr;
        }
        case TStr_Enum_SH1106: {
            static const char flashStr[] PROGMEM = "SH1106";
            return flashStr;
        }
        case TStr_Enum_SolarRadiation: {
            static const char flashStr[] PROGMEM = "SolarRadiation";
            return flashStr;
        }
        case TStr_Enum_SolarRadiationSensor: {
            static const char flashStr[] PROGMEM = "SolarRadiationSensor";
            return flashStr;
        }
        case TStr_Enum_Speed: {
            static const char flashStr[] PROGMEM = "Speed";
            return flashStr;
        }
        case TStr_Enum_SSD1305: {
            static const char flashStr[] PROGMEM = "SSD1305";
            return flashStr;
        }
        case TStr_Enum_SSD1305x32Ada: {
            static const char flashStr[] PROGMEM = "SSD1305x32Ada";
            return flashStr;
        }
        case TStr_Enum_SSD1305x64Ada: {
            static const char flashStr[] PROGMEM = "SSD1305x64Ada";
            return flashStr;
        }
        case TStr_Enum_SSD1306: {
            static const char flashStr[] PROGMEM = "SSD1306";
            return flashStr;
        }
        case TStr_Enum_SSD1607: {
            static const char flashStr[] PROGMEM = "SSD1607";
            return flashStr;
        }
        case TStr_Enum_ST7735: {
            static const char flashStr[] PROGMEM = "ST7735";
            return flashStr;
        }
        case TStr_Enum_ST7789: {
            static const char flashStr[] PROGMEM = "ST7789";
            return flashStr;
        }
        case TStr_Enum_Storage: {
            static const char flashStr[] PROGMEM = "Storage";
            return flashStr;
        }
        case TStr_Enum_Stored: {
            static const char flashStr[] PROGMEM = "Stored";
            return flashStr;
        }
        case TStr_Enum_SumpPump: {
            static const char flashStr[] PROGMEM = "SumpPump";
            return flashStr;
        }
        case TStr_Enum_Surface: {
            static const char flashStr[] PROGMEM = "Surface";
            return flashStr;
        }
        case TStr_Enum_Tank: {
            static const char flashStr[] PROGMEM = "Tank";
            return flashStr;
        }
        case TStr_Enum_Temperature: {
            static const char flashStr[] PROGMEM = "Temperature";
            return flashStr;
        }
        case TStr_Enum_TemperatureSensor: {
            static const char flashStr[] PROGMEM = "TemperatureSensor";
            return flashStr;
        }
        case TStr_Enum_TFTTouch: {
            static const char flashStr[] PROGMEM = "TFTTouch";
            return flashStr;
        }
        case TStr_Enum_Thermal: {
            static const char flashStr[] PROGMEM = "Thermal";
            return flashStr;
        }
        case TStr_Enum_ThermalReservoir: {
            static const char flashStr[] PROGMEM = "ThermalReservoir";
            return flashStr;
        }
        case TStr_Enum_TouchScreen: {
            static const char flashStr[] PROGMEM = "TouchScreen";
            return flashStr;
        }
        case TStr_Enum_Unknown: {
            static const char flashStr[] PROGMEM = "Unknown";
            return flashStr;
        }
        case TStr_Enum_UpDownButtons: {
            static const char flashStr[] PROGMEM = "UpDownButtons";
            return flashStr;
        }
        case TStr_Enum_UpDownESP32Touch: {
            static const char flashStr[] PROGMEM = "UpDownESP32Touch";
            return flashStr;
        }
        case TStr_Enum_Valve: {
            static const char flashStr[] PROGMEM = "Valve";
            return flashStr;
        }
        case TStr_Enum_Variable: {
            static const char flashStr[] PROGMEM = "Variable";
            return flashStr;
        }
        case TStr_Enum_Voltage: {
            static const char flashStr[] PROGMEM = "Voltage";
            return flashStr;
        }
        case TStr_Enum_Water: {
            static const char flashStr[] PROGMEM = "Water";
            return flashStr;
        }
        case TStr_Enum_WaterReservoir: {
            static const char flashStr[] PROGMEM = "WaterReservoir";
            return flashStr;
        }
        case TStr_Enum_Well: {
            static const char flashStr[] PROGMEM = "Well";
            return flashStr;
        }
        case TStr_Enum_WindDirection: {
            static const char flashStr[] PROGMEM = "WindDirection";
            return flashStr;
        }
        case TStr_Enum_WindDirectionSensor: {
            static const char flashStr[] PROGMEM = "WindDirectionSensor";
            return flashStr;
        }
        case TStr_Enum_WindSpeed: {
            static const char flashStr[] PROGMEM = "WindSpeed";
            return flashStr;
        }
        case TStr_Enum_WindSpeedSensor: {
            static const char flashStr[] PROGMEM = "WindSpeedSensor";
            return flashStr;
        }

        case TStr_Unit_Degree: {
            static const char flashStr[] PROGMEM = "deg";
            return flashStr;
        }
        case TStr_Unit_Feet: {
            static const char flashStr[] PROGMEM = "ft";
            return flashStr;
        }
        case TStr_Unit_Gallons: {
            static const char flashStr[] PROGMEM = "gal";
            return flashStr;
        }
        case TStr_Unit_Hectopascals: {
            static const char flashStr[] PROGMEM = "hPa";
            return flashStr;
        }
        case TStr_Unit_Inches: {
            static const char flashStr[] PROGMEM = "in";
            return flashStr;
        }
        case TStr_Unit_KilometersPerHour: {
            static const char flashStr[] PROGMEM = "km/h";
            return flashStr;
        }
        case TStr_Unit_Kilopascals: {
            static const char flashStr[] PROGMEM = "kPa";
            return flashStr;
        }
        case TStr_Unit_KilowattHours: {
            static const char flashStr[] PROGMEM = "kWh";
            return flashStr;
        }
        case TStr_Unit_MilesPerHour: {
            static const char flashStr[] PROGMEM = "mph";
            return flashStr;
        }
        case TStr_Unit_Millimeters: {
            static const char flashStr[] PROGMEM = "mm";
            return flashStr;
        }
        case TStr_Unit_Minutes: {
            static const char flashStr[] PROGMEM = "min";
            return flashStr;
        }
        case TStr_Unit_PerHour: {
            static const char flashStr[] PROGMEM = "/h";
            return flashStr;
        }
        case TStr_Unit_PerMinute: {
            static const char flashStr[] PROGMEM = "/min";
            return flashStr;
        }
        case TStr_Unit_PerSecond: {
            static const char flashStr[] PROGMEM = "/s";
            return flashStr;
        }
        case TStr_Unit_PoundsPerSquareInch: {
            static const char flashStr[] PROGMEM = "psi";
            return flashStr;
        }
        case TStr_Unit_Radians: {
            static const char flashStr[] PROGMEM = "rad";
            return flashStr;
        }
        case TStr_Unit_WattsPerSquareMeter: {
            static const char flashStr[] PROGMEM = "W/m2";
            return flashStr;
        }
        case TStr_Count: break;
    }
    return TStr_Blank;
}
#endif

bool terraStringIdEqualsIgnoreCase(const String &value, Terra_String stringId)
{
    return terraStringEqualsIgnoreCase(value, SFP(stringId));
}
