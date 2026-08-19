/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Types
*/

#ifndef TerraTypes_H
#define TerraTypes_H

#include "TerraPlatform.h"

// Runtime object categories.
enum Terra_ObjectType : uint8_t {
    Terra_ObjectType_Undefined = 0,             ///< Undefined object
    Terra_ObjectType_Sensor,                    ///< Sensor input
    Terra_ObjectType_Actuator,                  ///< Controlled output
    Terra_ObjectType_Resource,                  ///< Generic resource state
    Terra_ObjectType_WaterStorage,              ///< Water tank, cistern, or reservoir
    Terra_ObjectType_WaterSource,               ///< Water supply source
    Terra_ObjectType_WaterRoute,                ///< Water transfer route
    Terra_ObjectType_RainCatchment,             ///< Roof or other rainfall catchment
    Terra_ObjectType_ThermalStore,              ///< Thermal energy store
    Terra_ObjectType_ThermalLoop,               ///< Thermal circulation loop
    Terra_ObjectType_Environment,               ///< Local weather/environment observations
    Terra_ObjectType_PowerRail                  ///< Low-voltage equipment rail
};

// Managed resource categories.
enum Terra_ResourceType : uint8_t {
    Terra_ResourceType_Undefined = 0,           ///< Undefined resource
    Terra_ResourceType_Water,                   ///< Stored or available water
    Terra_ResourceType_Thermal                  ///< Stored thermal energy
};

// Normalized resource state.
enum Terra_ResourceState : uint8_t {
    Terra_ResourceState_Unknown = 0,            ///< No reliable state
    Terra_ResourceState_Normal,                 ///< Within normal operating band
    Terra_ResourceState_Low,                    ///< Below low threshold
    Terra_ResourceState_Reserve,                ///< At or below protected reserve
    Terra_ResourceState_High,                   ///< At or above high threshold
    Terra_ResourceState_Fault                   ///< Resource or measurement fault
};

// Water source categories.
enum Terra_WaterSourceType : uint8_t {
    Terra_WaterSourceType_Undefined = 0,        ///< Undefined water source
    Terra_WaterSourceType_Rainwater,            ///< Rainwater catchment
    Terra_WaterSourceType_Well,                 ///< Groundwater well
    Terra_WaterSourceType_Municipal,            ///< Municipal or utility supply
    Terra_WaterSourceType_Surface,              ///< Pond, creek, lake, or other surface source
    Terra_WaterSourceType_Reclaimed,            ///< Reclaimed or greywater source
    Terra_WaterSourceType_Stored                ///< Previously stored water source
};

// Water storage categories.
enum Terra_WaterStorageType : uint8_t {
    Terra_WaterStorageType_Undefined = 0,       ///< Generic or unspecified storage
    Terra_WaterStorageType_Tank,                ///< General-purpose water tank
    Terra_WaterStorageType_Cistern,             ///< Rainwater or bulk-water cistern
    Terra_WaterStorageType_Reservoir            ///< Managed reservoir or header storage
};

// Sensor categories.
enum Terra_SensorType : uint8_t {
    Terra_SensorType_Undefined = 0,             ///< Undefined sensor
    Terra_SensorType_Binary,                    ///< Generic binary state
    Terra_SensorType_Analog,                    ///< Generic analog reading
    Terra_SensorType_Temperature,               ///< Temperature
    Terra_SensorType_Humidity,                  ///< Relative humidity
    Terra_SensorType_Pressure,                  ///< Fluid or barometric pressure
    Terra_SensorType_Rainfall,                  ///< Rainfall amount or rate
    Terra_SensorType_Flow,                      ///< Water flow rate
    Terra_SensorType_Level,                     ///< Tank/resource level
    Terra_SensorType_WindSpeed,                 ///< Wind speed
    Terra_SensorType_WindDirection,             ///< Wind direction
    Terra_SensorType_SolarRadiation,            ///< Solar irradiance
    Terra_SensorType_Voltage,                   ///< Electrical voltage
    Terra_SensorType_Current,                   ///< Electrical current
    Terra_SensorType_Leak,                      ///< Leak/flood state
    Terra_SensorType_Remote                     ///< Transport-neutral remote reading
};

// Actuator categories.
enum Terra_ActuatorType : uint8_t {
    Terra_ActuatorType_Undefined = 0,           ///< Undefined actuator
    Terra_ActuatorType_Digital,                 ///< Generic digital output
    Terra_ActuatorType_Variable,                ///< Generic normalized variable output
    Terra_ActuatorType_Pump,                    ///< Water pump
    Terra_ActuatorType_Valve,                   ///< On/off valve
    Terra_ActuatorType_Diverter,                ///< Two-way or proportional diverter
    Terra_ActuatorType_Heater,                  ///< Heater output
    Terra_ActuatorType_Circulator,              ///< Thermal circulation pump
    Terra_ActuatorType_SumpPump                 ///< Level-controlled sump pump
};

// Preferred reporting system.
enum Terra_MeasurementMode : int8_t {
    Terra_MeasurementMode_Imperial = 0,         ///< US/imperial presentation units
    Terra_MeasurementMode_Metric,               ///< Metric presentation units
    Terra_MeasurementMode_Scientific,           ///< SI/scientific presentation units
    Terra_MeasurementMode_Count,                ///< Number of concrete modes
    Terra_MeasurementMode_Undefined = -1        ///< Unspecified mode
};

// How concurrent actuator requests are combined.
enum Terra_EnableMode : int8_t {
    Terra_EnableMode_Highest = 0,               ///< Highest request wins
    Terra_EnableMode_Lowest,                    ///< Lowest request wins
    Terra_EnableMode_Average,                   ///< Average active requests
    Terra_EnableMode_Multiply,                  ///< Multiply active requests
    Terra_EnableMode_InOrder,                   ///< First active request wins
    Terra_EnableMode_RevOrder,                  ///< Last active request wins
    Terra_EnableMode_Count,                     ///< Number of concrete modes
    Terra_EnableMode_Undefined = -1             ///< Unspecified mode
};

// Portable pin direction/mode settings.
enum Terra_PinMode : int8_t {
    Terra_PinMode_Digital_Input = 0,            ///< Digital input
    Terra_PinMode_Digital_Input_PullUp,         ///< Digital input with pull-up
    Terra_PinMode_Digital_Input_PullDown,       ///< Digital input with pull-down where supported
    Terra_PinMode_Digital_Output,               ///< Digital output
    Terra_PinMode_Analog_Input,                 ///< Analog input
    Terra_PinMode_Analog_Output,                ///< Analog/PWM output
    Terra_PinMode_Count,                        ///< Number of concrete modes
    Terra_PinMode_Undefined = -1                ///< Unspecified mode
};

// Measurement units.
enum Terra_Unit : uint8_t {
    Terra_Unit_Undefined = 0,                   ///< Undefined unit
    Terra_Unit_Raw,                             ///< Raw/dimensionless value
    Terra_Unit_Percent,                         ///< Percent
    Terra_Unit_Celsius,                         ///< Degrees Celsius
    Terra_Unit_Fahrenheit,                      ///< Degrees Fahrenheit
    Terra_Unit_Kelvin,                          ///< Kelvin
    Terra_Unit_Liters,                          ///< Liters
    Terra_Unit_GallonsUS,                       ///< US gallons
    Terra_Unit_LitersPerMinute,                 ///< Liters per minute
    Terra_Unit_GallonsPerMinute,                ///< US gallons per minute
    Terra_Unit_Kilopascals,                     ///< Kilopascals
    Terra_Unit_PSI,                             ///< Pounds per square inch
    Terra_Unit_Hectopascals,                    ///< Hectopascals
    Terra_Unit_Millimeters,                     ///< Millimeters
    Terra_Unit_Inches,                          ///< Inches
    Terra_Unit_MillimetersPerHour,              ///< Millimeters per hour
    Terra_Unit_InchesPerHour,                   ///< Inches per hour
    Terra_Unit_Watts,                           ///< Watts
    Terra_Unit_WattsPerSquareMeter,             ///< Watts per square meter
    Terra_Unit_KilowattHours,                   ///< Kilowatt-hours
    Terra_Unit_MetersPerSecond,                 ///< Meters per second
    Terra_Unit_KilometersPerHour,               ///< Kilometers per hour
    Terra_Unit_MilesPerHour,                    ///< Miles per hour
    Terra_Unit_Degrees,                         ///< Degrees
    Terra_Unit_Volts,                           ///< Volts
    Terra_Unit_Amps                             ///< Amperes
};

// Measurement unit families.
enum Terra_UnitsCategory : int8_t {
    Terra_UnitsCategory_Raw = 0,                ///< Raw/dimensionless values
    Terra_UnitsCategory_Percentile,             ///< Percent values
    Terra_UnitsCategory_Temperature,            ///< Temperature
    Terra_UnitsCategory_LiquidVolume,           ///< Liquid volume
    Terra_UnitsCategory_LiquidFlowRate,         ///< Liquid flow rate
    Terra_UnitsCategory_Pressure,               ///< Pressure
    Terra_UnitsCategory_Distance,               ///< Distance/rainfall depth
    Terra_UnitsCategory_RainRate,               ///< Rainfall rate
    Terra_UnitsCategory_Power,                  ///< Power
    Terra_UnitsCategory_Irradiance,             ///< Solar irradiance
    Terra_UnitsCategory_Energy,                 ///< Energy
    Terra_UnitsCategory_Speed,                  ///< Linear speed
    Terra_UnitsCategory_Angle,                  ///< Angle/direction
    Terra_UnitsCategory_Voltage,                ///< Voltage
    Terra_UnitsCategory_Current,                ///< Electrical current
    Terra_UnitsCategory_Count,                  ///< Number of concrete categories
    Terra_UnitsCategory_Undefined = -1          ///< Unspecified category
};

// Power rail categories used for reporting and configuration.
enum Terra_RailType : int8_t {
    Terra_RailType_Custom = 0,                  ///< User-defined nominal rail
    Terra_RailType_DC3V3,                       ///< 3.3 V DC rail
    Terra_RailType_DC5V,                        ///< 5 V DC rail
    Terra_RailType_DC12V,                       ///< 12 V DC rail
    Terra_RailType_DC24V,                       ///< 24 V DC rail
    Terra_RailType_Count,                       ///< Number of concrete rail types
    Terra_RailType_Undefined = -1               ///< Unspecified rail
};

// Trigger comparison operations.
enum Terra_Comparison : uint8_t {
    Terra_Comparison_LessThan = 0,              ///< Less than
    Terra_Comparison_LessOrEqual,               ///< Less than or equal
    Terra_Comparison_GreaterThan,               ///< Greater than
    Terra_Comparison_GreaterOrEqual,            ///< Greater than or equal
    Terra_Comparison_Equal,                     ///< Approximately equal
    Terra_Comparison_NotEqual                   ///< Not approximately equal
};

// Trigger output state.
enum Terra_TriggerState : uint8_t {
    Terra_TriggerState_Inactive = 0,            ///< Trigger inactive
    Terra_TriggerState_Active,                  ///< Trigger active
    Terra_TriggerState_Fault                    ///< Trigger input invalid/faulted
};

// Logging severity.
enum Terra_LogLevel : uint8_t {
    Terra_LogLevel_Debug = 0,                   ///< Debug detail
    Terra_LogLevel_Info,                        ///< Informational event
    Terra_LogLevel_Warning,                     ///< Warning
    Terra_LogLevel_Error                        ///< Error
};

// Optional module categories.
enum Terra_ModuleType : uint8_t {
    Terra_ModuleType_Undefined = 0,             ///< Undefined module
    Terra_ModuleType_RTC,                       ///< Real-time clock
    Terra_ModuleType_Storage,                   ///< EEPROM/flash/SD storage
    Terra_ModuleType_Display,                   ///< Local display/UI
    Terra_ModuleType_Network,                   ///< WiFi/Ethernet network interface
    Terra_ModuleType_Radio,                     ///< LoRa/other radio link
    Terra_ModuleType_IOExpander                 ///< GPIO/ADC expander
};

// Controller operating mode.
enum Terra_ControlMode : uint8_t {
    Terra_ControlMode_Manual = 0,               ///< Application/user code drives outputs
    Terra_ControlMode_Automatic,                ///< Local automation active
    Terra_ControlMode_Disabled                  ///< Controller updates suspended
};

// Water route state.
enum Terra_RouteState : uint8_t {
    Terra_RouteState_Idle = 0,                  ///< Route idle
    Terra_RouteState_Requested,                 ///< Route requested to start
    Terra_RouteState_Active,                    ///< Route actively transferring
    Terra_RouteState_Complete,                  ///< Destination target reached
    Terra_RouteState_Fault                      ///< Route faulted
};

// Common relationships between homestead objects.
enum Terra_AttachmentRole : uint8_t {
    Terra_AttachmentRole_Undefined = 0,         ///< Undefined relationship
    Terra_AttachmentRole_PrimarySensor,         ///< Primary sensor
    Terra_AttachmentRole_SecondarySensor,       ///< Secondary/redundant sensor
    Terra_AttachmentRole_LevelSensor,           ///< Level sensor
    Terra_AttachmentRole_FlowSensor,            ///< Flow sensor
    Terra_AttachmentRole_TemperatureSensor,     ///< Temperature sensor
    Terra_AttachmentRole_HumiditySensor,        ///< Humidity sensor
    Terra_AttachmentRole_PressureSensor,        ///< Pressure sensor
    Terra_AttachmentRole_RainfallSensor,        ///< Rainfall sensor
    Terra_AttachmentRole_WindSpeedSensor,       ///< Wind speed sensor
    Terra_AttachmentRole_WindDirectionSensor,   ///< Wind direction sensor
    Terra_AttachmentRole_SolarRadiationSensor,  ///< Solar irradiance sensor
    Terra_AttachmentRole_LeakSensor,            ///< Leak/flood sensor
    Terra_AttachmentRole_Pump,                  ///< Pump actuator
    Terra_AttachmentRole_Valve,                 ///< Valve actuator
    Terra_AttachmentRole_Diverter,              ///< Diverter actuator
    Terra_AttachmentRole_Heater,                ///< Heater actuator
    Terra_AttachmentRole_Circulator             ///< Circulation pump
};

#endif // /ifndef TerraTypes_H
