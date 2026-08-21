/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Strings
*/

#ifndef TerraStrings_H
#define TerraStrings_H

#include "TerraTypes.h"

// Common library strings. Built-in values live in program Flash where supported.
enum Terra_String : uint16_t {
    TStr_Undefined,                       ///< "Undefined"
    TStr_Unknown,                         ///< "Unknown"
    TStr_Sensor,                          ///< "Sensor"
    TStr_Actuator,                        ///< "Actuator"
    TStr_Resource,                        ///< "Resource"
    TStr_WaterStorage,                    ///< "WaterStorage"
    TStr_WaterSource,                     ///< "WaterSource"
    TStr_WaterRoute,                      ///< "WaterRoute"
    TStr_RainCatchment,                   ///< "RainCatchment"
    TStr_ThermalStore,                    ///< "ThermalStore"
    TStr_ThermalLoop,                     ///< "ThermalLoop"
    TStr_Environment,                     ///< "Environment"
    TStr_PowerRail,                       ///< "PowerRail"
    TStr_Water,                           ///< "Water"
    TStr_Thermal,                         ///< "Thermal"
    TStr_Normal,                          ///< "Normal"
    TStr_Low,                             ///< "Low"
    TStr_Reserve,                         ///< "Reserve"
    TStr_High,                            ///< "High"
    TStr_Fault,                           ///< "Fault"
    TStr_Rainwater,                       ///< "Rainwater"
    TStr_Well,                            ///< "Well"
    TStr_Municipal,                       ///< "Municipal"
    TStr_Surface,                         ///< "Surface"
    TStr_Reclaimed,                       ///< "Reclaimed"
    TStr_Stored,                          ///< "Stored"
    TStr_Tank,                            ///< "Tank"
    TStr_Cistern,                         ///< "Cistern"
    TStr_Reservoir,                       ///< "Reservoir"
    TStr_Binary,                          ///< "Binary"
    TStr_Analog,                          ///< "Analog"
    TStr_Temperature,                     ///< "Temperature"
    TStr_Humidity,                        ///< "Humidity"
    TStr_Pressure,                        ///< "Pressure"
    TStr_Rainfall,                        ///< "Rainfall"
    TStr_Flow,                            ///< "Flow"
    TStr_Level,                           ///< "Level"
    TStr_WindSpeed,                       ///< "WindSpeed"
    TStr_WindDirection,                   ///< "WindDirection"
    TStr_SolarRadiation,                  ///< "SolarRadiation"
    TStr_Leak,                            ///< "Leak"
    TStr_Remote,                          ///< "Remote"
    TStr_Digital,                         ///< "Digital"
    TStr_Variable,                        ///< "Variable"
    TStr_Pump,                            ///< "Pump"
    TStr_Valve,                           ///< "Valve"
    TStr_Diverter,                        ///< "Diverter"
    TStr_Heater,                          ///< "Heater"
    TStr_Circulator,                      ///< "Circulator"
    TStr_SumpPump,                        ///< "SumpPump"
    TStr_raw,                             ///< "raw"
    TStr_Percent,                         ///< "%"
    TStr_C,                               ///< "C"
    TStr_F,                               ///< "F"
    TStr_K,                               ///< "K"
    TStr_L,                               ///< "L"
    TStr_gal,                             ///< "gal"
    TStr_LPerMin,                         ///< "L/min"
    TStr_galPerMin,                       ///< "gal/min"
    TStr_kPa,                             ///< "kPa"
    TStr_psi,                             ///< "psi"
    TStr_hPa,                             ///< "hPa"
    TStr_mm,                              ///< "mm"
    TStr_in,                              ///< "in"
    TStr_mmPerH,                          ///< "mm/h"
    TStr_W,                               ///< "W"
    TStr_WPerM2,                          ///< "W/m2"
    TStr_kWh,                             ///< "kWh"
    TStr_mPerS,                           ///< "m/s"
    TStr_kmPerH,                          ///< "km/h"
    TStr_mph,                             ///< "mph"
    TStr_deg,                             ///< "deg"
    TStr_V,                               ///< "V"
    TStr_A,                               ///< "A"
    TStr_inPerH,                          ///< "in/h"
    TStr_LessThan,                        ///< "LessThan"
    TStr_LessOrEqual,                     ///< "LessOrEqual"
    TStr_GreaterThan,                     ///< "GreaterThan"
    TStr_GreaterOrEqual,                  ///< "GreaterOrEqual"
    TStr_Equal,                           ///< "Equal"
    TStr_NotEqual,                        ///< "NotEqual"
    TStr_Inactive,                        ///< "Inactive"
    TStr_Active,                          ///< "Active"
    TStr_DEBUG,                           ///< "DEBUG"
    TStr_INFO,                            ///< "INFO"
    TStr_WARN,                            ///< "WARN"
    TStr_ERROR,                           ///< "ERROR"
    TStr_RTC,                             ///< "RTC"
    TStr_Storage,                         ///< "Storage"
    TStr_Display,                         ///< "Display"
    TStr_Network,                         ///< "Network"
    TStr_Radio,                           ///< "Radio"
    TStr_IOExpander,                      ///< "IOExpander"
    TStr_Manual,                          ///< "Manual"
    TStr_Automatic,                       ///< "Automatic"
    TStr_Disabled,                        ///< "Disabled"
    TStr_Imperial,                        ///< "Imperial"
    TStr_Metric,                          ///< "Metric"
    TStr_Scientific,                      ///< "Scientific"
    TStr_Highest,                         ///< "Highest"
    TStr_Lowest,                          ///< "Lowest"
    TStr_Average,                         ///< "Average"
    TStr_Multiply,                        ///< "Multiply"
    TStr_InOrder,                         ///< "InOrder"
    TStr_RevOrder,                        ///< "RevOrder"
    TStr_DigitalInput,                    ///< "DigitalInput"
    TStr_DigitalInputPullUp,              ///< "DigitalInputPullUp"
    TStr_DigitalInputPullDown,            ///< "DigitalInputPullDown"
    TStr_DigitalOutput,                   ///< "DigitalOutput"
    TStr_AnalogInput,                     ///< "AnalogInput"
    TStr_AnalogOutput,                    ///< "AnalogOutput"
    TStr_Raw,                             ///< "Raw"
    TStr_Percentile,                      ///< "Percentile"
    TStr_LiquidVolume,                    ///< "LiquidVolume"
    TStr_LiquidFlowRate,                  ///< "LiquidFlowRate"
    TStr_Distance,                        ///< "Distance"
    TStr_RainRate,                        ///< "RainRate"
    TStr_Power,                           ///< "Power"
    TStr_Irradiance,                      ///< "Irradiance"
    TStr_Energy,                          ///< "Energy"
    TStr_Speed,                           ///< "Speed"
    TStr_Angle,                           ///< "Angle"
    TStr_Voltage,                         ///< "Voltage"
    TStr_Current,                         ///< "Current"
    TStr_Custom,                          ///< "Custom"
    TStr_DC3V3,                           ///< "DC3V3"
    TStr_DC5V,                            ///< "DC5V"
    TStr_DC12V,                           ///< "DC12V"
    TStr_DC24V,                           ///< "DC24V"
    TStr_Idle,                            ///< "Idle"
    TStr_Requested,                       ///< "Requested"
    TStr_Complete,                        ///< "Complete"
    TStr_PrimarySensor,                   ///< "PrimarySensor"
    TStr_SecondarySensor,                 ///< "SecondarySensor"
    TStr_LevelSensor,                     ///< "LevelSensor"
    TStr_FlowSensor,                      ///< "FlowSensor"
    TStr_TemperatureSensor,               ///< "TemperatureSensor"
    TStr_HumiditySensor,                  ///< "HumiditySensor"
    TStr_PressureSensor,                  ///< "PressureSensor"
    TStr_RainfallSensor,                  ///< "RainfallSensor"
    TStr_RainRateSensor,                  ///< "RainRateSensor"
    TStr_WindSpeedSensor,                 ///< "WindSpeedSensor"
    TStr_WindDirectionSensor,             ///< "WindDirectionSensor"
    TStr_SolarRadiationSensor,            ///< "SolarRadiationSensor"
    TStr_LeakSensor,                      ///< "LeakSensor"
    TStr_Terraduino,                      ///< "Terraduino"
    TStr_System,                          ///< "system"
    TStr_Initialized,                     ///< "initialized"
    TStr_Launched,                        ///< "launched"
    TStr_Suspended,                       ///< "suspended"
    TStr_RouteFault,                      ///< "route fault"
    TStr_SourceUnavailable,               ///< "source unavailable"
    TStr_SourceReserveProtected,          ///< "source reserve protected"
    TStr_DestinationTargetReached,        ///< "destination target reached"
    TStr_ContinuingFillToStopLevel,       ///< "continuing fill to stop level"
    TStr_DestinationRequestsFill,         ///< "destination requests fill"
    TStr_WithinDestinationBand,           ///< "within destination band"
    TStr_MaxContinuousRuntimeExceeded,    ///< "maximum continuous runtime exceeded"
    TStr_RemoteSensorInvalid,             ///< "remote sensor reported invalid data"
    TStr_RemoteSensorStale,               ///< "remote sensor stale"
    TStr_RouteFlowOutsideLimits,          ///< "route flow outside limits"
    TStr_UnexpectedFlowWhileIdle,         ///< "unexpected flow while route idle"
    TStr_Empty,                           ///< ""
    TStr_Null,                            ///< "null"
    TStr_Csv,                             ///< "csv"
    TStr_Dat,                             ///< "dat"
    TStr_Txt,                             ///< "txt"
    TStr_ColonSpace,                      ///< ": "
    TStr_CountLabel,                      ///< "Count"
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
TerraString terraResourceTypeToString(Terra_ResourceType value);
TerraString terraResourceStateToString(Terra_ResourceState value);
TerraString terraWaterSourceTypeToString(Terra_WaterSourceType value);
TerraString terraWaterStorageTypeToString(Terra_WaterStorageType value);
TerraString terraSensorTypeToString(Terra_SensorType value);
TerraString terraActuatorTypeToString(Terra_ActuatorType value);
TerraString terraMeasurementModeToString(Terra_MeasurementMode value);
TerraString terraEnableModeToString(Terra_EnableMode value);
TerraString terraPinModeToString(Terra_PinMode value);
TerraString terraUnitToString(Terra_Unit value);
TerraString terraUnitsCategoryToString(Terra_UnitsCategory value);
TerraString terraRailTypeToString(Terra_RailType value);
TerraString terraComparisonToString(Terra_Comparison value);
TerraString terraTriggerStateToString(Terra_TriggerState value);
TerraString terraLogLevelToString(Terra_LogLevel value);
TerraString terraModuleTypeToString(Terra_ModuleType value);
TerraString terraControlModeToString(Terra_ControlMode value);
TerraString terraRouteStateToString(Terra_RouteState value);

Terra_ObjectType terraObjectTypeFromString(const TerraString &value);
Terra_ResourceType terraResourceTypeFromString(const TerraString &value);
Terra_ResourceState terraResourceStateFromString(const TerraString &value);
Terra_WaterSourceType terraWaterSourceTypeFromString(const TerraString &value);
Terra_WaterStorageType terraWaterStorageTypeFromString(const TerraString &value);
Terra_SensorType terraSensorTypeFromString(const TerraString &value);
Terra_ActuatorType terraActuatorTypeFromString(const TerraString &value);
Terra_MeasurementMode terraMeasurementModeFromString(const TerraString &value);
Terra_EnableMode terraEnableModeFromString(const TerraString &value);
Terra_PinMode terraPinModeFromString(const TerraString &value);
Terra_Unit terraUnitFromString(const TerraString &value);
Terra_UnitsCategory terraUnitsCategoryFromString(const TerraString &value);
Terra_RailType terraRailTypeFromString(const TerraString &value);
Terra_Comparison terraComparisonFromString(const TerraString &value);
Terra_TriggerState terraTriggerStateFromString(const TerraString &value);
Terra_LogLevel terraLogLevelFromString(const TerraString &value);
Terra_ModuleType terraModuleTypeFromString(const TerraString &value);
Terra_ControlMode terraControlModeFromString(const TerraString &value);
Terra_RouteState terraRouteStateFromString(const TerraString &value);

#endif // /ifndef TerraStrings_H
