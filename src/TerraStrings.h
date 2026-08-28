/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Strings
*/

#ifndef TerraStrings_H
#define TerraStrings_H

#include "Terraduino.h"

// Strings Enumeration Table
enum Terra_String : uint16_t {
    TStr_ColonSpace,                        ///< ": "
    TStr_DoubleSpace,                       ///< "  "
    TStr_csv,                               ///< "csv"
    TStr_dat,                               ///< "dat"
    TStr_Disabled,                          ///< "Disabled"
    TStr_raw,                               ///< "raw"
    TStr_txt,                               ///< "txt"
    TStr_Undefined,                         ///< "Undefined"
    TStr_null,                              ///< "null"
    TStr_Empty,                             ///< ""
    TStr_public,                            ///< "public"
    TStr_In,                                ///< " in "
    TStr_HARD,                              ///< " HARD"

    TStr_Default_SystemName,                ///< "Terraduino"
    TStr_Default_ConfigFilename,            ///< "Terraduino.cfg"

    TStr_Err_AllocationFailure,             ///< "Allocation failure"
    TStr_Err_AlreadyInitialized,            ///< "Already initialized"
    TStr_Err_AssertionFailure,              ///< "Assertion failure"
    TStr_Err_DataVersionMismatch,           ///< "Data version mismatch"
    TStr_Err_ExportFailure,                 ///< "Export failure"
    TStr_Err_HashingCollision,              ///< "Hashing collision"
    TStr_Err_ImportFailure,                 ///< "Import failure"
    TStr_Err_InitializationFailure,         ///< "Initialization failure"
    TStr_Err_InvalidParameter,              ///< "Invalid parameter"
    TStr_Err_InvalidPinOrType,              ///< "Invalid pin or type"
    TStr_Err_MeasurementFailure,            ///< "Measurement failure"
    TStr_Err_MissingLinkage,                ///< "Missing linkage"
    TStr_Err_NoPositionsAvailable,          ///< "No positions available"
    TStr_Err_NotConfiguredProperly,         ///< "Not configured properly"
    TStr_Err_NotYetInitialized,             ///< "Not yet initialized"
    TStr_Err_OperationFailure,              ///< "Operation failure"
    TStr_Err_RTCDefaultAddressOnly,         ///< "RTClib only supports the default I2C RTC address"
    TStr_Err_SumpLevelInvalid,              ///< "sump level invalid"
    TStr_Err_UnknownDataDecode,             ///< "Unknown data decode"
    TStr_Err_UnsupportedOperation,          ///< "Unsupported operation"

    TStr_Log_RTCBatteryFailure,             ///< "RTC battery failure"
    TStr_Log_SystemDataSaved,               ///< "System data saved"
    TStr_Log_SystemUptime,                  ///< "System uptime"

    TStr_Log_Prefix_Info,                   ///< "[INFO] "
    TStr_Log_Prefix_Warning,                ///< "[WARN] "
    TStr_Log_Prefix_Error,                  ///< "[ERROR] "

    TStr_System,                            ///< "system"
    TStr_RTCLog,                            ///< "rtc"
    TStr_Initialized,                       ///< "initialized"
    TStr_Launched,                          ///< "launched"
    TStr_Suspended,                         ///< "suspended"
    TStr_RouteFault,                        ///< "route fault"
    TStr_SourceUnavailable,                 ///< "source unavailable"
    TStr_SourceReserveProtected,            ///< "source reserve protected"
    TStr_DestinationTargetReached,          ///< "destination target reached"
    TStr_ContinuingFillToStopLevel,         ///< "continuing fill to stop level"
    TStr_DestinationRequestsFill,           ///< "destination requests fill"
    TStr_WithinDestinationBand,             ///< "within destination band"
    TStr_MaxContinuousRuntimeExceeded,      ///< "maximum continuous runtime exceeded"
    TStr_RemoteSensorInvalid,               ///< "remote sensor reported invalid data"
    TStr_RemoteSensorStale,                 ///< "remote sensor stale"
    TStr_RouteFlowOutsideLimits,            ///< "route flow outside limits"
    TStr_UnexpectedFlowWhileIdle,           ///< "unexpected flow while route idle"

    TStr_Key_ActiveLow,                     ///< "activeLow"
    TStr_Key_AirTemperatureSensor,          ///< "airTemperatureSensor"
    TStr_Key_AlwaysFilled,                  ///< "alwaysFilled"
    TStr_Key_AreaSquareMeters,              ///< "areaSquareMeters"
    TStr_Key_AutosaveEnabled,               ///< "autosaveEnabled"
    TStr_Key_AutosaveFallback,              ///< "autosaveFallback"
    TStr_Key_AutosaveInterval,              ///< "autosaveInterval"
    TStr_Key_BitRes,                        ///< "bitRes"
    TStr_Key_CalibrationUnits,              ///< "calibrationUnits"
    TStr_Key_Circulator,                    ///< "circulator"
    TStr_Key_CollectionEfficiency,          ///< "collectionEfficiency"
    TStr_Key_CtrlInMode,                    ///< "ctrlInMode"
    TStr_Key_DataFilePrefix,                ///< "dataFilePrefix"
    TStr_Key_Destination,                   ///< "destination"
    TStr_Key_DestinationStartPercent,       ///< "destinationStartPercent"
    TStr_Key_DestinationStopPercent,        ///< "destinationStopPercent"
    TStr_Key_DetriggerDelay,                ///< "detriggerDelay"
    TStr_Key_DetriggerTol,                  ///< "detriggerTol"
    TStr_Key_DispOutMode,                   ///< "dispOutMode"
    TStr_Key_EmptyTrigger,                  ///< "emptyTrigger"
    TStr_Key_EnableMode,                    ///< "enableMode"
    TStr_Key_FilledTrigger,                 ///< "filledTrigger"
    TStr_Key_FlowSensor,                    ///< "flowSensor"
    TStr_Key_HighTrigger,                   ///< "highTrigger"
    TStr_Key_HumiditySensor,                ///< "humiditySensor"
    TStr_Key_Id,                            ///< "id"
    TStr_Key_InputPin,                      ///< "inputPin"
    TStr_Key_LevelSensor,                   ///< "levelSensor"
    TStr_Key_LimitTrigger,                  ///< "limitTrigger"
    TStr_Key_Location,                      ///< "location"
    TStr_Key_LogFilePrefix,                 ///< "logFilePrefix"
    TStr_Key_LogLevel,                      ///< "logLevel"
    TStr_Key_Logger,                        ///< "logger"
    TStr_Key_LogToSDCard,                   ///< "logToSDCard"
    TStr_Key_LogToWiFiStorage,              ///< "logToWiFiStorage"
    TStr_Key_LowTrigger,                    ///< "lowTrigger"
    TStr_Key_MACAddress,                    ///< "macAddress"
    TStr_Key_MaxActiveAtOnce,               ///< "maxActiveAtOnce"
    TStr_Key_MaxContinuousMs,               ///< "maxContinuousMs"
    TStr_Key_MaximumFlowLpm,                ///< "maximumFlowLpm"
    TStr_Key_MaxPower,                      ///< "maxPower"
    TStr_Key_MaxStoreTempC,                 ///< "maxStoreTempC"
    TStr_Key_MaxTemperature,                ///< "maxTemperature"
    TStr_Key_MaxVolume,                     ///< "maxVolume"
    TStr_Key_MeasurementRow,                ///< "measurementRow"
    TStr_Key_MeasurementUnits,              ///< "measurementUnits"
    TStr_Key_MeasureMode,                   ///< "measureMode"
    TStr_Key_MinimumFlowLpm,                ///< "minimumFlowLpm"
    TStr_Key_Mode,                          ///< "mode"
    TStr_Key_Multiplier,                    ///< "multiplier"
    TStr_Key_OffDifferentialC,              ///< "offDifferentialC"
    TStr_Key_Offset,                        ///< "offset"
    TStr_Key_OnDifferentialC,               ///< "onDifferentialC"
    TStr_Key_OutputPin,                     ///< "outputPin"
    TStr_Key_Pin,                           ///< "pin"
    TStr_Key_PollingInterval,               ///< "pollingInterval"
    TStr_Key_PowerUsageSensor,              ///< "powerUsageSensor"
    TStr_Key_PowerUnits,                    ///< "powerUnits"
    TStr_Key_PressureSensor,                ///< "pressureSensor"
    TStr_Key_PublishToSDCard,               ///< "pubToSDCard"
    TStr_Key_PublishToWiFiStorage,          ///< "pubToWiFiStorage"
    TStr_Key_Publisher,                     ///< "publisher"
    TStr_Key_Pump,                          ///< "pump"
    TStr_Key_RainfallSensor,                ///< "rainfallSensor"
    TStr_Key_RainRateSensor,                ///< "rainRateSensor"
    TStr_Key_ReportedType,                  ///< "reportedType"
    TStr_Key_Revision,                      ///< "revision"
    TStr_Key_Scheduler,                     ///< "scheduler"
    TStr_Key_SensorName,                    ///< "sensorName"
    TStr_Key_SolarRadiationSensor,          ///< "solarRadiationSensor"
    TStr_Key_Source,                        ///< "source"
    TStr_Key_SourceTemperatureSensor,       ///< "sourceTemperatureSensor"
    TStr_Key_StaleAfterMs,                  ///< "staleAfterMs"
    TStr_Key_Store,                         ///< "store"
    TStr_Key_SumpAlarmPercent,              ///< "sumpAlarmPercent"
    TStr_Key_SumpStartPercent,              ///< "sumpStartPercent"
    TStr_Key_SumpStopPercent,               ///< "sumpStopPercent"
    TStr_Key_SystemMode,                    ///< "systemMode"
    TStr_Key_SystemName,                    ///< "systemName"
    TStr_Key_TemperatureSensor,             ///< "temperatureSensor"
    TStr_Key_TemperatureUnits,              ///< "temperatureUnits"
    TStr_Key_TimeZoneOffset,                ///< "timeZoneOffset"
    TStr_Key_Timestamp,                     ///< "timestamp"
    TStr_Key_Tolerance,                     ///< "tolerance"
    TStr_Key_ToleranceHigh,                 ///< "toleranceHigh"
    TStr_Key_ToleranceLow,                  ///< "toleranceLow"
    TStr_Key_TriggerBelow,                  ///< "triggerBelow"
    TStr_Key_TriggerOutside,                ///< "triggerOutside"
    TStr_Key_Type,                          ///< "type"
    TStr_Key_Units,                         ///< "units"
    TStr_Key_Value,                         ///< "value"
    TStr_Key_Version,                       ///< "version"
    TStr_Key_VolumeSensor,                  ///< "volumeSensor"
    TStr_Key_VolumeUnits,                   ///< "volumeUnits"
    TStr_Key_WaterTemperatureSensor,        ///< "waterTemperatureSensor"
    TStr_Key_WiFiPassword,                  ///< "wifiPassword"
    TStr_Key_WiFiPasswordSeed,              ///< "wifiPasswordSeed"
    TStr_Key_WiFiSSID,                      ///< "wifiSSID"
    TStr_Key_WindDirectionSensor,           ///< "windDirectionSensor"
    TStr_Key_WindSpeedSensor,               ///< "windSpeedSensor"

    TStr_Unknown,                           ///< "Unknown"
    TStr_Sensor,                            ///< "Sensor"
    TStr_Actuator,                          ///< "Actuator"
    TStr_Resource,                          ///< "Resource"
    TStr_WaterStorage,                      ///< "WaterStorage"
    TStr_WaterSource,                       ///< "WaterSource"
    TStr_WaterRoute,                        ///< "WaterRoute"
    TStr_RainCatchment,                     ///< "RainCatchment"
    TStr_WaterReservoir,                    ///< "WaterReservoir"
    TStr_ThermalReservoir,                  ///< "ThermalReservoir"
    TStr_ThermalLoop,                       ///< "ThermalLoop"
    TStr_Environment,                       ///< "Environment"
    TStr_PowerRail,                         ///< "PowerRail"
    TStr_Water,                             ///< "Water"
    TStr_Thermal,                           ///< "Thermal"
    TStr_Normal,                            ///< "Normal"
    TStr_Low,                               ///< "Low"
    TStr_Reserve,                           ///< "Reserve"
    TStr_High,                              ///< "High"
    TStr_Fault,                             ///< "Fault"
    TStr_Rainwater,                         ///< "Rainwater"
    TStr_Well,                              ///< "Well"
    TStr_Municipal,                         ///< "Municipal"
    TStr_Surface,                           ///< "Surface"
    TStr_Reclaimed,                         ///< "Reclaimed"
    TStr_Stored,                            ///< "Stored"
    TStr_Tank,                              ///< "Tank"
    TStr_Cistern,                           ///< "Cistern"
    TStr_Reservoir,                         ///< "Reservoir"
    TStr_Binary,                            ///< "Binary"
    TStr_Analog,                            ///< "Analog"
    TStr_Temperature,                       ///< "Temperature"
    TStr_Humidity,                          ///< "Humidity"
    TStr_Pressure,                          ///< "Pressure"
    TStr_Rainfall,                          ///< "Rainfall"
    TStr_Flow,                              ///< "Flow"
    TStr_Level,                             ///< "Level"
    TStr_WindSpeed,                         ///< "WindSpeed"
    TStr_WindDirection,                     ///< "WindDirection"
    TStr_SolarRadiation,                    ///< "SolarRadiation"
    TStr_Leak,                              ///< "Leak"
    TStr_Remote,                            ///< "Remote"
    TStr_Digital,                           ///< "Digital"
    TStr_Variable,                          ///< "Variable"
    TStr_Pump,                              ///< "Pump"
    TStr_Valve,                             ///< "Valve"
    TStr_Diverter,                          ///< "Diverter"
    TStr_Heater,                            ///< "Heater"
    TStr_Circulator,                        ///< "Circulator"
    TStr_SumpPump,                          ///< "SumpPump"
    TStr_LessThan,                          ///< "LessThan"
    TStr_LessOrEqual,                       ///< "LessOrEqual"
    TStr_GreaterThan,                       ///< "GreaterThan"
    TStr_GreaterOrEqual,                    ///< "GreaterOrEqual"
    TStr_Equal,                             ///< "Equal"
    TStr_NotEqual,                          ///< "NotEqual"
    TStr_Inactive,                          ///< "Inactive"
    TStr_Active,                            ///< "Active"
    TStr_DEBUG,                             ///< "DEBUG"
    TStr_INFO,                              ///< "INFO"
    TStr_WARN,                              ///< "WARN"
    TStr_ERROR,                             ///< "ERROR"
    TStr_RTC,                               ///< "RTC"
    TStr_Storage,                           ///< "Storage"
    TStr_Display,                           ///< "Display"
    TStr_Network,                           ///< "Network"
    TStr_Radio,                             ///< "Radio"
    TStr_IOExpander,                        ///< "IOExpander"
    TStr_Manual,                            ///< "Manual"
    TStr_Automatic,                         ///< "Automatic"
    TStr_Imperial,                          ///< "Imperial"
    TStr_Metric,                            ///< "Metric"
    TStr_Scientific,                        ///< "Scientific"
    TStr_Highest,                           ///< "Highest"
    TStr_Lowest,                            ///< "Lowest"
    TStr_Average,                           ///< "Average"
    TStr_Multiply,                          ///< "Multiply"
    TStr_InOrder,                           ///< "InOrder"
    TStr_RevOrder,                          ///< "RevOrder"
    TStr_DigitalInput,                      ///< "DigitalInput"
    TStr_DigitalInputPullUp,                ///< "DigitalInputPullUp"
    TStr_DigitalInputPullDown,              ///< "DigitalInputPullDown"
    TStr_DigitalOutput,                     ///< "DigitalOutput"
    TStr_AnalogInput,                       ///< "AnalogInput"
    TStr_AnalogOutput,                      ///< "AnalogOutput"
    TStr_Raw,                               ///< "Raw"
    TStr_Percentile,                        ///< "Percentile"
    TStr_LiquidVolume,                      ///< "LiquidVolume"
    TStr_LiquidFlowRate,                    ///< "LiquidFlowRate"
    TStr_Distance,                          ///< "Distance"
    TStr_RainRate,                          ///< "RainRate"
    TStr_Power,                             ///< "Power"
    TStr_Irradiance,                        ///< "Irradiance"
    TStr_Energy,                            ///< "Energy"
    TStr_Speed,                             ///< "Speed"
    TStr_Angle,                             ///< "Angle"
    TStr_Voltage,                           ///< "Voltage"
    TStr_Current,                           ///< "Current"
    TStr_Custom,                            ///< "Custom"
    TStr_DC3V3,                             ///< "DC3V3"
    TStr_DC5V,                              ///< "DC5V"
    TStr_DC12V,                             ///< "DC12V"
    TStr_DC24V,                             ///< "DC24V"
    TStr_Idle,                              ///< "Idle"
    TStr_Requested,                         ///< "Requested"
    TStr_Complete,                          ///< "Complete"
    TStr_PrimarySensor,                     ///< "PrimarySensor"
    TStr_SecondarySensor,                   ///< "SecondarySensor"
    TStr_LevelSensor,                       ///< "LevelSensor"
    TStr_FlowSensor,                        ///< "FlowSensor"
    TStr_TemperatureSensor,                 ///< "TemperatureSensor"
    TStr_HumiditySensor,                    ///< "HumiditySensor"
    TStr_PressureSensor,                    ///< "PressureSensor"
    TStr_RainfallSensor,                    ///< "RainfallSensor"
    TStr_RainRateSensor,                    ///< "RainRateSensor"
    TStr_WindSpeedSensor,                   ///< "WindSpeedSensor"
    TStr_WindDirectionSensor,               ///< "WindDirectionSensor"
    TStr_SolarRadiationSensor,              ///< "SolarRadiationSensor"
    TStr_LeakSensor,                        ///< "LeakSensor"
    TStr_CountLabel,                        ///< "Count"

    TStr_Percent,                           ///< "%"
    TStr_C,                                 ///< "C"
    TStr_F,                                 ///< "F"
    TStr_K,                                 ///< "K"
    TStr_L,                                 ///< "L"
    TStr_gal,                               ///< "gal"
    TStr_LPerMin,                           ///< "L/min"
    TStr_galPerMin,                         ///< "gal/min"
    TStr_kPa,                               ///< "kPa"
    TStr_psi,                               ///< "psi"
    TStr_hPa,                               ///< "hPa"
    TStr_mm,                                ///< "mm"
    TStr_in,                                ///< "in"
    TStr_mmPerH,                            ///< "mm/h"
    TStr_W,                                 ///< "W"
    TStr_WPerM2,                            ///< "W/m2"
    TStr_kWh,                               ///< "kWh"
    TStr_mPerS,                             ///< "m/s"
    TStr_kmPerH,                            ///< "km/h"
    TStr_mph,                               ///< "mph"
    TStr_deg,                               ///< "deg"
    TStr_V,                                 ///< "V"
    TStr_A,                                 ///< "A"
    TStr_inPerH,                            ///< "in/h"

    TStr_Count
};

extern const char *TStr_Blank;

typedef bool (*TerraStringProvider)(void *context, Terra_String stringId, TerraString *valueOut);
void setTerraStringProvider(TerraStringProvider provider, void *context = nullptr);
TerraString stringFromPGM(Terra_String strNum);
TerraString stringFromPGMAddr(const char *flashStr);
#define SFP(strNum) stringFromPGM((strNum))

#ifndef TERRA_DISABLE_BUILTIN_DATA
const char *pgmAddrForStr(Terra_String strNum);
#define CFP(strNum) pgmAddrForStr((strNum))
#else
#define CFP(strNum) SFP(strNum).c_str()
#endif

bool terraStringIdEqualsIgnoreCase(const TerraString &value, Terra_String stringId);

TerraString terraObjectTypeToString(Terra_ObjectType value);
TerraString TerraReservoirTypeToString(Terra_ReservoirType value);
TerraString TerraReservoirStateToString(Terra_ResourceState value);
TerraString terraWaterSourceTypeToString(Terra_WaterSourceType value);
TerraString terraWaterStorageTypeToString(Terra_WaterStorageType value);
TerraString terraSensorTypeToString(Terra_SensorType value);
TerraString terraActuatorTypeToString(Terra_ActuatorType value);
TerraString terraMeasurementModeToString(Terra_MeasurementMode value);
TerraString terraEnableModeToString(Terra_EnableMode value);
TerraString terraPinModeToString(Terra_PinMode value);
TerraString terraUnitToString(Terra_UnitsType value);
TerraString terraUnitsCategoryToString(Terra_UnitsCategory value);
TerraString terraRailTypeToString(Terra_RailType value);
TerraString terraComparisonToString(Terra_Comparison value);
TerraString terraTriggerStateToString(Terra_TriggerState value);
TerraString terraLogLevelToString(Terra_LogLevel value);
TerraString terraModuleTypeToString(Terra_ModuleType value);
TerraString terraControlModeToString(Terra_ControlMode value);
TerraString terraRouteStateToString(Terra_RouteState value);
TerraString terraAttachmentRoleToString(Terra_AttachmentRole value);

Terra_ObjectType terraObjectTypeFromString(const TerraString &value);
Terra_ReservoirType TerraReservoirTypeFromString(const TerraString &value);
Terra_ResourceState TerraReservoirStateFromString(const TerraString &value);
Terra_WaterSourceType terraWaterSourceTypeFromString(const TerraString &value);
Terra_WaterStorageType terraWaterStorageTypeFromString(const TerraString &value);
Terra_SensorType terraSensorTypeFromString(const TerraString &value);
Terra_ActuatorType terraActuatorTypeFromString(const TerraString &value);
Terra_MeasurementMode terraMeasurementModeFromString(const TerraString &value);
Terra_EnableMode terraEnableModeFromString(const TerraString &value);
Terra_PinMode terraPinModeFromString(const TerraString &value);
Terra_UnitsType unitsTypeFromSymbol(const TerraString &value);
Terra_UnitsCategory terraUnitsCategoryFromString(const TerraString &value);
Terra_RailType terraRailTypeFromString(const TerraString &value);
Terra_Comparison terraComparisonFromString(const TerraString &value);
Terra_TriggerState terraTriggerStateFromString(const TerraString &value);
Terra_LogLevel terraLogLevelFromString(const TerraString &value);
Terra_ModuleType terraModuleTypeFromString(const TerraString &value);
Terra_ControlMode terraControlModeFromString(const TerraString &value);
Terra_RouteState terraRouteStateFromString(const TerraString &value);
Terra_AttachmentRole terraAttachmentRoleFromString(const TerraString &value);

#endif // /ifndef TerraStrings_H
