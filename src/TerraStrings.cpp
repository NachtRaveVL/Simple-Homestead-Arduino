/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Strings
*/

#include "Terraduino.h"
#include "TerraUtils.h"
#include <ctype.h>

static size_t terraStringLength(const TerraString &value)
{
#if defined(ARDUINO)
    return value.length();
#else
    return value.size();
#endif
}

static char terraTrieChar(const TerraString &value, size_t index)
{
    if (index >= terraStringLength(value)) { return '\0'; }
    return (char)tolower((unsigned char)value[index]);
}


static char _terraBlank = '\0';
const char *TStr_Blank = &_terraBlank;

static TerraStringProvider _terraStringProvider = nullptr;
static void *_terraStringProviderContext = nullptr;

void setTerraStringProvider(TerraStringProvider provider, void *context)
{
    _terraStringProvider = provider;
    _terraStringProviderContext = context;
}

TerraString stringFromPGM(Terra_String strNum)
{
#ifndef TERRA_DISABLE_BUILTIN_DATA
    return stringFromPGMAddr(pgmAddrForStr(strNum));
#else
    TerraString value;
    return _terraStringProvider && _terraStringProvider(_terraStringProviderContext, strNum, &value) ? value : TerraString();
#endif
}

TerraString stringFromPGMAddr(const char *flashStr)
{
    if (!flashStr) { return TerraString(); }
#if defined(ARDUINO)
    TerraString value;
    char ch;
    while ((ch = (char)pgm_read_byte(flashStr++)) != '\0') { value += ch; }
    return value;
#else
    return TerraString(flashStr);
#endif
}

#ifndef TERRA_DISABLE_BUILTIN_DATA
const char *pgmAddrForStr(Terra_String strNum)
{
    switch (strNum) {
        case TStr_Undefined: {
            static const char flashStr[] PROGMEM = "Undefined";
            return flashStr;
        }
        case TStr_Unknown: {
            static const char flashStr[] PROGMEM = "Unknown";
            return flashStr;
        }
        case TStr_Sensor: {
            static const char flashStr[] PROGMEM = "Sensor";
            return flashStr;
        }
        case TStr_Actuator: {
            static const char flashStr[] PROGMEM = "Actuator";
            return flashStr;
        }
        case TStr_Resource: {
            static const char flashStr[] PROGMEM = "Resource";
            return flashStr;
        }
        case TStr_WaterStorage: {
            static const char flashStr[] PROGMEM = "WaterStorage";
            return flashStr;
        }
        case TStr_WaterSource: {
            static const char flashStr[] PROGMEM = "WaterSource";
            return flashStr;
        }
        case TStr_WaterRoute: {
            static const char flashStr[] PROGMEM = "WaterRoute";
            return flashStr;
        }
        case TStr_RainCatchment: {
            static const char flashStr[] PROGMEM = "RainCatchment";
            return flashStr;
        }
        case TStr_ThermalStore: {
            static const char flashStr[] PROGMEM = "ThermalStore";
            return flashStr;
        }
        case TStr_ThermalLoop: {
            static const char flashStr[] PROGMEM = "ThermalLoop";
            return flashStr;
        }
        case TStr_Environment: {
            static const char flashStr[] PROGMEM = "Environment";
            return flashStr;
        }
        case TStr_PowerRail: {
            static const char flashStr[] PROGMEM = "PowerRail";
            return flashStr;
        }
        case TStr_Water: {
            static const char flashStr[] PROGMEM = "Water";
            return flashStr;
        }
        case TStr_Thermal: {
            static const char flashStr[] PROGMEM = "Thermal";
            return flashStr;
        }
        case TStr_Normal: {
            static const char flashStr[] PROGMEM = "Normal";
            return flashStr;
        }
        case TStr_Low: {
            static const char flashStr[] PROGMEM = "Low";
            return flashStr;
        }
        case TStr_Reserve: {
            static const char flashStr[] PROGMEM = "Reserve";
            return flashStr;
        }
        case TStr_High: {
            static const char flashStr[] PROGMEM = "High";
            return flashStr;
        }
        case TStr_Fault: {
            static const char flashStr[] PROGMEM = "Fault";
            return flashStr;
        }
        case TStr_Rainwater: {
            static const char flashStr[] PROGMEM = "Rainwater";
            return flashStr;
        }
        case TStr_Well: {
            static const char flashStr[] PROGMEM = "Well";
            return flashStr;
        }
        case TStr_Municipal: {
            static const char flashStr[] PROGMEM = "Municipal";
            return flashStr;
        }
        case TStr_Surface: {
            static const char flashStr[] PROGMEM = "Surface";
            return flashStr;
        }
        case TStr_Reclaimed: {
            static const char flashStr[] PROGMEM = "Reclaimed";
            return flashStr;
        }
        case TStr_Stored: {
            static const char flashStr[] PROGMEM = "Stored";
            return flashStr;
        }
        case TStr_Tank: {
            static const char flashStr[] PROGMEM = "Tank";
            return flashStr;
        }
        case TStr_Cistern: {
            static const char flashStr[] PROGMEM = "Cistern";
            return flashStr;
        }
        case TStr_Reservoir: {
            static const char flashStr[] PROGMEM = "Reservoir";
            return flashStr;
        }
        case TStr_Binary: {
            static const char flashStr[] PROGMEM = "Binary";
            return flashStr;
        }
        case TStr_Analog: {
            static const char flashStr[] PROGMEM = "Analog";
            return flashStr;
        }
        case TStr_Temperature: {
            static const char flashStr[] PROGMEM = "Temperature";
            return flashStr;
        }
        case TStr_Humidity: {
            static const char flashStr[] PROGMEM = "Humidity";
            return flashStr;
        }
        case TStr_Pressure: {
            static const char flashStr[] PROGMEM = "Pressure";
            return flashStr;
        }
        case TStr_Rainfall: {
            static const char flashStr[] PROGMEM = "Rainfall";
            return flashStr;
        }
        case TStr_Flow: {
            static const char flashStr[] PROGMEM = "Flow";
            return flashStr;
        }
        case TStr_Level: {
            static const char flashStr[] PROGMEM = "Level";
            return flashStr;
        }
        case TStr_WindSpeed: {
            static const char flashStr[] PROGMEM = "WindSpeed";
            return flashStr;
        }
        case TStr_WindDirection: {
            static const char flashStr[] PROGMEM = "WindDirection";
            return flashStr;
        }
        case TStr_SolarRadiation: {
            static const char flashStr[] PROGMEM = "SolarRadiation";
            return flashStr;
        }
        case TStr_Leak: {
            static const char flashStr[] PROGMEM = "Leak";
            return flashStr;
        }
        case TStr_Remote: {
            static const char flashStr[] PROGMEM = "Remote";
            return flashStr;
        }
        case TStr_Digital: {
            static const char flashStr[] PROGMEM = "Digital";
            return flashStr;
        }
        case TStr_Variable: {
            static const char flashStr[] PROGMEM = "Variable";
            return flashStr;
        }
        case TStr_Pump: {
            static const char flashStr[] PROGMEM = "Pump";
            return flashStr;
        }
        case TStr_Valve: {
            static const char flashStr[] PROGMEM = "Valve";
            return flashStr;
        }
        case TStr_Diverter: {
            static const char flashStr[] PROGMEM = "Diverter";
            return flashStr;
        }
        case TStr_Heater: {
            static const char flashStr[] PROGMEM = "Heater";
            return flashStr;
        }
        case TStr_Circulator: {
            static const char flashStr[] PROGMEM = "Circulator";
            return flashStr;
        }
        case TStr_SumpPump: {
            static const char flashStr[] PROGMEM = "SumpPump";
            return flashStr;
        }
        case TStr_raw: {
            static const char flashStr[] PROGMEM = "raw";
            return flashStr;
        }
        case TStr_Percent: {
            static const char flashStr[] PROGMEM = "%";
            return flashStr;
        }
        case TStr_C: {
            static const char flashStr[] PROGMEM = "C";
            return flashStr;
        }
        case TStr_F: {
            static const char flashStr[] PROGMEM = "F";
            return flashStr;
        }
        case TStr_K: {
            static const char flashStr[] PROGMEM = "K";
            return flashStr;
        }
        case TStr_L: {
            static const char flashStr[] PROGMEM = "L";
            return flashStr;
        }
        case TStr_gal: {
            static const char flashStr[] PROGMEM = "gal";
            return flashStr;
        }
        case TStr_LPerMin: {
            static const char flashStr[] PROGMEM = "L/min";
            return flashStr;
        }
        case TStr_galPerMin: {
            static const char flashStr[] PROGMEM = "gal/min";
            return flashStr;
        }
        case TStr_kPa: {
            static const char flashStr[] PROGMEM = "kPa";
            return flashStr;
        }
        case TStr_psi: {
            static const char flashStr[] PROGMEM = "psi";
            return flashStr;
        }
        case TStr_hPa: {
            static const char flashStr[] PROGMEM = "hPa";
            return flashStr;
        }
        case TStr_mm: {
            static const char flashStr[] PROGMEM = "mm";
            return flashStr;
        }
        case TStr_in: {
            static const char flashStr[] PROGMEM = "in";
            return flashStr;
        }
        case TStr_mmPerH: {
            static const char flashStr[] PROGMEM = "mm/h";
            return flashStr;
        }
        case TStr_W: {
            static const char flashStr[] PROGMEM = "W";
            return flashStr;
        }
        case TStr_WPerM2: {
            static const char flashStr[] PROGMEM = "W/m2";
            return flashStr;
        }
        case TStr_kWh: {
            static const char flashStr[] PROGMEM = "kWh";
            return flashStr;
        }
        case TStr_mPerS: {
            static const char flashStr[] PROGMEM = "m/s";
            return flashStr;
        }
        case TStr_kmPerH: {
            static const char flashStr[] PROGMEM = "km/h";
            return flashStr;
        }
        case TStr_mph: {
            static const char flashStr[] PROGMEM = "mph";
            return flashStr;
        }
        case TStr_deg: {
            static const char flashStr[] PROGMEM = "deg";
            return flashStr;
        }
        case TStr_V: {
            static const char flashStr[] PROGMEM = "V";
            return flashStr;
        }
        case TStr_A: {
            static const char flashStr[] PROGMEM = "A";
            return flashStr;
        }
        case TStr_inPerH: {
            static const char flashStr[] PROGMEM = "in/h";
            return flashStr;
        }
        case TStr_LessThan: {
            static const char flashStr[] PROGMEM = "LessThan";
            return flashStr;
        }
        case TStr_LessOrEqual: {
            static const char flashStr[] PROGMEM = "LessOrEqual";
            return flashStr;
        }
        case TStr_GreaterThan: {
            static const char flashStr[] PROGMEM = "GreaterThan";
            return flashStr;
        }
        case TStr_GreaterOrEqual: {
            static const char flashStr[] PROGMEM = "GreaterOrEqual";
            return flashStr;
        }
        case TStr_Equal: {
            static const char flashStr[] PROGMEM = "Equal";
            return flashStr;
        }
        case TStr_NotEqual: {
            static const char flashStr[] PROGMEM = "NotEqual";
            return flashStr;
        }
        case TStr_Inactive: {
            static const char flashStr[] PROGMEM = "Inactive";
            return flashStr;
        }
        case TStr_Active: {
            static const char flashStr[] PROGMEM = "Active";
            return flashStr;
        }
        case TStr_DEBUG: {
            static const char flashStr[] PROGMEM = "DEBUG";
            return flashStr;
        }
        case TStr_INFO: {
            static const char flashStr[] PROGMEM = "INFO";
            return flashStr;
        }
        case TStr_WARN: {
            static const char flashStr[] PROGMEM = "WARN";
            return flashStr;
        }
        case TStr_ERROR: {
            static const char flashStr[] PROGMEM = "ERROR";
            return flashStr;
        }
        case TStr_RTC: {
            static const char flashStr[] PROGMEM = "RTC";
            return flashStr;
        }
        case TStr_Storage: {
            static const char flashStr[] PROGMEM = "Storage";
            return flashStr;
        }
        case TStr_Display: {
            static const char flashStr[] PROGMEM = "Display";
            return flashStr;
        }
        case TStr_Network: {
            static const char flashStr[] PROGMEM = "Network";
            return flashStr;
        }
        case TStr_Radio: {
            static const char flashStr[] PROGMEM = "Radio";
            return flashStr;
        }
        case TStr_IOExpander: {
            static const char flashStr[] PROGMEM = "IOExpander";
            return flashStr;
        }
        case TStr_Manual: {
            static const char flashStr[] PROGMEM = "Manual";
            return flashStr;
        }
        case TStr_Automatic: {
            static const char flashStr[] PROGMEM = "Automatic";
            return flashStr;
        }
        case TStr_Disabled: {
            static const char flashStr[] PROGMEM = "Disabled";
            return flashStr;
        }
        case TStr_Imperial: {
            static const char flashStr[] PROGMEM = "Imperial";
            return flashStr;
        }
        case TStr_Metric: {
            static const char flashStr[] PROGMEM = "Metric";
            return flashStr;
        }
        case TStr_Scientific: {
            static const char flashStr[] PROGMEM = "Scientific";
            return flashStr;
        }
        case TStr_Highest: {
            static const char flashStr[] PROGMEM = "Highest";
            return flashStr;
        }
        case TStr_Lowest: {
            static const char flashStr[] PROGMEM = "Lowest";
            return flashStr;
        }
        case TStr_Average: {
            static const char flashStr[] PROGMEM = "Average";
            return flashStr;
        }
        case TStr_Multiply: {
            static const char flashStr[] PROGMEM = "Multiply";
            return flashStr;
        }
        case TStr_InOrder: {
            static const char flashStr[] PROGMEM = "InOrder";
            return flashStr;
        }
        case TStr_RevOrder: {
            static const char flashStr[] PROGMEM = "RevOrder";
            return flashStr;
        }
        case TStr_DigitalInput: {
            static const char flashStr[] PROGMEM = "DigitalInput";
            return flashStr;
        }
        case TStr_DigitalInputPullUp: {
            static const char flashStr[] PROGMEM = "DigitalInputPullUp";
            return flashStr;
        }
        case TStr_DigitalInputPullDown: {
            static const char flashStr[] PROGMEM = "DigitalInputPullDown";
            return flashStr;
        }
        case TStr_DigitalOutput: {
            static const char flashStr[] PROGMEM = "DigitalOutput";
            return flashStr;
        }
        case TStr_AnalogInput: {
            static const char flashStr[] PROGMEM = "AnalogInput";
            return flashStr;
        }
        case TStr_AnalogOutput: {
            static const char flashStr[] PROGMEM = "AnalogOutput";
            return flashStr;
        }
        case TStr_Raw: {
            static const char flashStr[] PROGMEM = "Raw";
            return flashStr;
        }
        case TStr_Percentile: {
            static const char flashStr[] PROGMEM = "Percentile";
            return flashStr;
        }
        case TStr_LiquidVolume: {
            static const char flashStr[] PROGMEM = "LiquidVolume";
            return flashStr;
        }
        case TStr_LiquidFlowRate: {
            static const char flashStr[] PROGMEM = "LiquidFlowRate";
            return flashStr;
        }
        case TStr_Distance: {
            static const char flashStr[] PROGMEM = "Distance";
            return flashStr;
        }
        case TStr_RainRate: {
            static const char flashStr[] PROGMEM = "RainRate";
            return flashStr;
        }
        case TStr_Power: {
            static const char flashStr[] PROGMEM = "Power";
            return flashStr;
        }
        case TStr_Irradiance: {
            static const char flashStr[] PROGMEM = "Irradiance";
            return flashStr;
        }
        case TStr_Energy: {
            static const char flashStr[] PROGMEM = "Energy";
            return flashStr;
        }
        case TStr_Speed: {
            static const char flashStr[] PROGMEM = "Speed";
            return flashStr;
        }
        case TStr_Angle: {
            static const char flashStr[] PROGMEM = "Angle";
            return flashStr;
        }
        case TStr_Voltage: {
            static const char flashStr[] PROGMEM = "Voltage";
            return flashStr;
        }
        case TStr_Current: {
            static const char flashStr[] PROGMEM = "Current";
            return flashStr;
        }
        case TStr_Custom: {
            static const char flashStr[] PROGMEM = "Custom";
            return flashStr;
        }
        case TStr_DC3V3: {
            static const char flashStr[] PROGMEM = "DC3V3";
            return flashStr;
        }
        case TStr_DC5V: {
            static const char flashStr[] PROGMEM = "DC5V";
            return flashStr;
        }
        case TStr_DC12V: {
            static const char flashStr[] PROGMEM = "DC12V";
            return flashStr;
        }
        case TStr_DC24V: {
            static const char flashStr[] PROGMEM = "DC24V";
            return flashStr;
        }
        case TStr_Idle: {
            static const char flashStr[] PROGMEM = "Idle";
            return flashStr;
        }
        case TStr_Requested: {
            static const char flashStr[] PROGMEM = "Requested";
            return flashStr;
        }
        case TStr_Complete: {
            static const char flashStr[] PROGMEM = "Complete";
            return flashStr;
        }
        case TStr_PrimarySensor: {
            static const char flashStr[] PROGMEM = "PrimarySensor";
            return flashStr;
        }
        case TStr_SecondarySensor: {
            static const char flashStr[] PROGMEM = "SecondarySensor";
            return flashStr;
        }
        case TStr_LevelSensor: {
            static const char flashStr[] PROGMEM = "LevelSensor";
            return flashStr;
        }
        case TStr_FlowSensor: {
            static const char flashStr[] PROGMEM = "FlowSensor";
            return flashStr;
        }
        case TStr_TemperatureSensor: {
            static const char flashStr[] PROGMEM = "TemperatureSensor";
            return flashStr;
        }
        case TStr_HumiditySensor: {
            static const char flashStr[] PROGMEM = "HumiditySensor";
            return flashStr;
        }
        case TStr_PressureSensor: {
            static const char flashStr[] PROGMEM = "PressureSensor";
            return flashStr;
        }
        case TStr_RainfallSensor: {
            static const char flashStr[] PROGMEM = "RainfallSensor";
            return flashStr;
        }
        case TStr_RainRateSensor: {
            static const char flashStr[] PROGMEM = "RainRateSensor";
            return flashStr;
        }
        case TStr_WindSpeedSensor: {
            static const char flashStr[] PROGMEM = "WindSpeedSensor";
            return flashStr;
        }
        case TStr_WindDirectionSensor: {
            static const char flashStr[] PROGMEM = "WindDirectionSensor";
            return flashStr;
        }
        case TStr_SolarRadiationSensor: {
            static const char flashStr[] PROGMEM = "SolarRadiationSensor";
            return flashStr;
        }
        case TStr_LeakSensor: {
            static const char flashStr[] PROGMEM = "LeakSensor";
            return flashStr;
        }
        case TStr_Terraduino: {
            static const char flashStr[] PROGMEM = "Terraduino";
            return flashStr;
        }
        case TStr_System: {
            static const char flashStr[] PROGMEM = "system";
            return flashStr;
        }
        case TStr_Initialized: {
            static const char flashStr[] PROGMEM = "initialized";
            return flashStr;
        }
        case TStr_Launched: {
            static const char flashStr[] PROGMEM = "launched";
            return flashStr;
        }
        case TStr_Suspended: {
            static const char flashStr[] PROGMEM = "suspended";
            return flashStr;
        }
        case TStr_RouteFault: {
            static const char flashStr[] PROGMEM = "route fault";
            return flashStr;
        }
        case TStr_SourceUnavailable: {
            static const char flashStr[] PROGMEM = "source unavailable";
            return flashStr;
        }
        case TStr_SourceReserveProtected: {
            static const char flashStr[] PROGMEM = "source reserve protected";
            return flashStr;
        }
        case TStr_DestinationTargetReached: {
            static const char flashStr[] PROGMEM = "destination target reached";
            return flashStr;
        }
        case TStr_ContinuingFillToStopLevel: {
            static const char flashStr[] PROGMEM = "continuing fill to stop level";
            return flashStr;
        }
        case TStr_DestinationRequestsFill: {
            static const char flashStr[] PROGMEM = "destination requests fill";
            return flashStr;
        }
        case TStr_WithinDestinationBand: {
            static const char flashStr[] PROGMEM = "within destination band";
            return flashStr;
        }
        case TStr_MaxContinuousRuntimeExceeded: {
            static const char flashStr[] PROGMEM = "maximum continuous runtime exceeded";
            return flashStr;
        }
        case TStr_RemoteSensorInvalid: {
            static const char flashStr[] PROGMEM = "remote sensor reported invalid data";
            return flashStr;
        }
        case TStr_RemoteSensorStale: {
            static const char flashStr[] PROGMEM = "remote sensor stale";
            return flashStr;
        }
        case TStr_RouteFlowOutsideLimits: {
            static const char flashStr[] PROGMEM = "route flow outside limits";
            return flashStr;
        }
        case TStr_UnexpectedFlowWhileIdle: {
            static const char flashStr[] PROGMEM = "unexpected flow while route idle";
            return flashStr;
        }
        case TStr_Empty: {
            static const char flashStr[] PROGMEM = "";
            return flashStr;
        }
        case TStr_Null: {
            static const char flashStr[] PROGMEM = "null";
            return flashStr;
        }
        case TStr_Csv: {
            static const char flashStr[] PROGMEM = "csv";
            return flashStr;
        }
        case TStr_Dat: {
            static const char flashStr[] PROGMEM = "dat";
            return flashStr;
        }
        case TStr_Txt: {
            static const char flashStr[] PROGMEM = "txt";
            return flashStr;
        }
        case TStr_ColonSpace: {
            static const char flashStr[] PROGMEM = ": ";
            return flashStr;
        }
        case TStr_CountLabel: {
            static const char flashStr[] PROGMEM = "Count";
            return flashStr;
        }
        case TStr_Count: break;
    }
    return TStr_Blank;
}
#endif

bool terraStringIdEqualsIgnoreCase(const TerraString &value, Terra_String stringId)
{
    return terraStringEqualsIgnoreCase(value, SFP(stringId));
}

TerraString terraObjectTypeToString(Terra_ObjectType value)
{
    switch (value) {
        case Terra_ObjectType_Undefined: return SFP(TStr_Undefined);
        case Terra_ObjectType_Sensor: return SFP(TStr_Sensor);
        case Terra_ObjectType_Actuator: return SFP(TStr_Actuator);
        case Terra_ObjectType_Resource: return SFP(TStr_Resource);
        case Terra_ObjectType_WaterStorage: return SFP(TStr_WaterStorage);
        case Terra_ObjectType_WaterSource: return SFP(TStr_WaterSource);
        case Terra_ObjectType_WaterRoute: return SFP(TStr_WaterRoute);
        case Terra_ObjectType_RainCatchment: return SFP(TStr_RainCatchment);
        case Terra_ObjectType_ThermalStore: return SFP(TStr_ThermalStore);
        case Terra_ObjectType_ThermalLoop: return SFP(TStr_ThermalLoop);
        case Terra_ObjectType_Environment: return SFP(TStr_Environment);
        case Terra_ObjectType_PowerRail: return SFP(TStr_PowerRail);
    }
    return SFP(TStr_Undefined);
}

TerraString TerraReservoirTypeToString(Terra_ReservoirType value)
{
    switch (value) {
        case Terra_ReservoirType_Undefined: return SFP(TStr_Undefined);
        case Terra_ReservoirType_Water: return SFP(TStr_Water);
        case Terra_ReservoirType_Thermal: return SFP(TStr_Thermal);
    }
    return SFP(TStr_Undefined);
}

TerraString TerraReservoirStateToString(Terra_ResourceState value)
{
    switch (value) {
        case Terra_ResourceState_Unknown: return SFP(TStr_Unknown);
        case Terra_ResourceState_Normal: return SFP(TStr_Normal);
        case Terra_ResourceState_Low: return SFP(TStr_Low);
        case Terra_ResourceState_Reserve: return SFP(TStr_Reserve);
        case Terra_ResourceState_High: return SFP(TStr_High);
        case Terra_ResourceState_Fault: return SFP(TStr_Fault);
    }
    return SFP(TStr_Unknown);
}

TerraString terraWaterSourceTypeToString(Terra_WaterSourceType value)
{
    switch (value) {
        case Terra_WaterSourceType_Undefined: return SFP(TStr_Undefined);
        case Terra_WaterSourceType_Rainwater: return SFP(TStr_Rainwater);
        case Terra_WaterSourceType_Well: return SFP(TStr_Well);
        case Terra_WaterSourceType_Municipal: return SFP(TStr_Municipal);
        case Terra_WaterSourceType_Surface: return SFP(TStr_Surface);
        case Terra_WaterSourceType_Reclaimed: return SFP(TStr_Reclaimed);
        case Terra_WaterSourceType_Stored: return SFP(TStr_Stored);
    }
    return SFP(TStr_Undefined);
}

TerraString terraWaterStorageTypeToString(Terra_WaterStorageType value)
{
    switch (value) {
        case Terra_WaterStorageType_Undefined: return SFP(TStr_Undefined);
        case Terra_WaterStorageType_Tank: return SFP(TStr_Tank);
        case Terra_WaterStorageType_Cistern: return SFP(TStr_Cistern);
        case Terra_WaterStorageType_Reservoir: return SFP(TStr_Reservoir);
    }
    return SFP(TStr_Undefined);
}

TerraString terraSensorTypeToString(Terra_SensorType value)
{
    switch (value) {
        case Terra_SensorType_Undefined: return SFP(TStr_Undefined);
        case Terra_SensorType_Binary: return SFP(TStr_Binary);
        case Terra_SensorType_Analog: return SFP(TStr_Analog);
        case Terra_SensorType_Temperature: return SFP(TStr_Temperature);
        case Terra_SensorType_Humidity: return SFP(TStr_Humidity);
        case Terra_SensorType_Pressure: return SFP(TStr_Pressure);
        case Terra_SensorType_Rainfall: return SFP(TStr_Rainfall);
        case Terra_SensorType_Flow: return SFP(TStr_Flow);
        case Terra_SensorType_Level: return SFP(TStr_Level);
        case Terra_SensorType_WindSpeed: return SFP(TStr_WindSpeed);
        case Terra_SensorType_WindDirection: return SFP(TStr_WindDirection);
        case Terra_SensorType_SolarRadiation: return SFP(TStr_SolarRadiation);
        case Terra_SensorType_Voltage: return SFP(TStr_Voltage);
        case Terra_SensorType_Current: return SFP(TStr_Current);
        case Terra_SensorType_Leak: return SFP(TStr_Leak);
        case Terra_SensorType_Remote: return SFP(TStr_Remote);
    }
    return SFP(TStr_Undefined);
}

TerraString terraActuatorTypeToString(Terra_ActuatorType value)
{
    switch (value) {
        case Terra_ActuatorType_Undefined: return SFP(TStr_Undefined);
        case Terra_ActuatorType_Digital: return SFP(TStr_Digital);
        case Terra_ActuatorType_Variable: return SFP(TStr_Variable);
        case Terra_ActuatorType_Pump: return SFP(TStr_Pump);
        case Terra_ActuatorType_Valve: return SFP(TStr_Valve);
        case Terra_ActuatorType_Diverter: return SFP(TStr_Diverter);
        case Terra_ActuatorType_Heater: return SFP(TStr_Heater);
        case Terra_ActuatorType_Circulator: return SFP(TStr_Circulator);
        case Terra_ActuatorType_SumpPump: return SFP(TStr_SumpPump);
    }
    return SFP(TStr_Undefined);
}

TerraString terraMeasurementModeToString(Terra_MeasurementMode value)
{
    switch (value) {
        case Terra_MeasurementMode_Imperial: return SFP(TStr_Imperial);
        case Terra_MeasurementMode_Metric: return SFP(TStr_Metric);
        case Terra_MeasurementMode_Scientific: return SFP(TStr_Scientific);
        case Terra_MeasurementMode_Count: return SFP(TStr_CountLabel);
        case Terra_MeasurementMode_Undefined: return SFP(TStr_Undefined);
    }
    return SFP(TStr_Undefined);
}

TerraString terraEnableModeToString(Terra_EnableMode value)
{
    switch (value) {
        case Terra_EnableMode_Highest: return SFP(TStr_Highest);
        case Terra_EnableMode_Lowest: return SFP(TStr_Lowest);
        case Terra_EnableMode_Average: return SFP(TStr_Average);
        case Terra_EnableMode_Multiply: return SFP(TStr_Multiply);
        case Terra_EnableMode_InOrder: return SFP(TStr_InOrder);
        case Terra_EnableMode_RevOrder: return SFP(TStr_RevOrder);
        case Terra_EnableMode_Count: return SFP(TStr_CountLabel);
        case Terra_EnableMode_Undefined: return SFP(TStr_Undefined);
    }
    return SFP(TStr_Undefined);
}

TerraString terraPinModeToString(Terra_PinMode value)
{
    switch (value) {
        case Terra_PinMode_Digital_Input: return SFP(TStr_DigitalInput);
        case Terra_PinMode_Digital_Input_PullUp: return SFP(TStr_DigitalInputPullUp);
        case Terra_PinMode_Digital_Input_PullDown: return SFP(TStr_DigitalInputPullDown);
        case Terra_PinMode_Digital_Output: return SFP(TStr_DigitalOutput);
        case Terra_PinMode_Analog_Input: return SFP(TStr_AnalogInput);
        case Terra_PinMode_Analog_Output: return SFP(TStr_AnalogOutput);
        case Terra_PinMode_Count: return SFP(TStr_CountLabel);
        case Terra_PinMode_Undefined: return SFP(TStr_Undefined);
    }
    return SFP(TStr_Undefined);
}

TerraString terraUnitToString(Terra_UnitsType value)
{
    switch (value) {
        case Terra_UnitsType_Undefined: return SFP(TStr_Undefined);
        case Terra_UnitsType_Raw: return SFP(TStr_raw);
        case Terra_UnitsType_Percent: return SFP(TStr_Percent);
        case Terra_UnitsType_Celsius: return SFP(TStr_C);
        case Terra_UnitsType_Fahrenheit: return SFP(TStr_F);
        case Terra_UnitsType_Kelvin: return SFP(TStr_K);
        case Terra_UnitsType_Liters: return SFP(TStr_L);
        case Terra_UnitsType_GallonsUS: return SFP(TStr_gal);
        case Terra_UnitsType_LitersPerMinute: return SFP(TStr_LPerMin);
        case Terra_UnitsType_GallonsPerMinute: return SFP(TStr_galPerMin);
        case Terra_UnitsType_Kilopascals: return SFP(TStr_kPa);
        case Terra_UnitsType_PSI: return SFP(TStr_psi);
        case Terra_UnitsType_Hectopascals: return SFP(TStr_hPa);
        case Terra_UnitsType_Millimeters: return SFP(TStr_mm);
        case Terra_UnitsType_Inches: return SFP(TStr_in);
        case Terra_UnitsType_MillimetersPerHour: return SFP(TStr_mmPerH);
        case Terra_UnitsType_InchesPerHour: return SFP(TStr_inPerH);
        case Terra_UnitsType_Watts: return SFP(TStr_W);
        case Terra_UnitsType_WattsPerSquareMeter: return SFP(TStr_WPerM2);
        case Terra_UnitsType_KilowattHours: return SFP(TStr_kWh);
        case Terra_UnitsType_MetersPerSecond: return SFP(TStr_mPerS);
        case Terra_UnitsType_KilometersPerHour: return SFP(TStr_kmPerH);
        case Terra_UnitsType_MilesPerHour: return SFP(TStr_mph);
        case Terra_UnitsType_Degrees: return SFP(TStr_deg);
        case Terra_UnitsType_Volts: return SFP(TStr_V);
        case Terra_UnitsType_Amps: return SFP(TStr_A);
    }
    return SFP(TStr_Undefined);
}

TerraString terraUnitsCategoryToString(Terra_UnitsCategory value)
{
    switch (value) {
        case Terra_UnitsCategory_Raw: return SFP(TStr_Raw);
        case Terra_UnitsCategory_Percentile: return SFP(TStr_Percentile);
        case Terra_UnitsCategory_Temperature: return SFP(TStr_Temperature);
        case Terra_UnitsCategory_LiquidVolume: return SFP(TStr_LiquidVolume);
        case Terra_UnitsCategory_LiquidFlowRate: return SFP(TStr_LiquidFlowRate);
        case Terra_UnitsCategory_Pressure: return SFP(TStr_Pressure);
        case Terra_UnitsCategory_Distance: return SFP(TStr_Distance);
        case Terra_UnitsCategory_RainRate: return SFP(TStr_RainRate);
        case Terra_UnitsCategory_Power: return SFP(TStr_Power);
        case Terra_UnitsCategory_Irradiance: return SFP(TStr_Irradiance);
        case Terra_UnitsCategory_Energy: return SFP(TStr_Energy);
        case Terra_UnitsCategory_Speed: return SFP(TStr_Speed);
        case Terra_UnitsCategory_Angle: return SFP(TStr_Angle);
        case Terra_UnitsCategory_Voltage: return SFP(TStr_Voltage);
        case Terra_UnitsCategory_Current: return SFP(TStr_Current);
        case Terra_UnitsCategory_Count: return SFP(TStr_CountLabel);
        case Terra_UnitsCategory_Undefined: return SFP(TStr_Undefined);
    }
    return SFP(TStr_Undefined);
}

TerraString terraRailTypeToString(Terra_RailType value)
{
    switch (value) {
        case Terra_RailType_Custom: return SFP(TStr_Custom);
        case Terra_RailType_DC3V3: return SFP(TStr_DC3V3);
        case Terra_RailType_DC5V: return SFP(TStr_DC5V);
        case Terra_RailType_DC12V: return SFP(TStr_DC12V);
        case Terra_RailType_DC24V: return SFP(TStr_DC24V);
        case Terra_RailType_Count: return SFP(TStr_CountLabel);
        case Terra_RailType_Undefined: return SFP(TStr_Undefined);
    }
    return SFP(TStr_Undefined);
}

TerraString terraComparisonToString(Terra_Comparison value)
{
    switch (value) {
        case Terra_Comparison_LessThan: return SFP(TStr_LessThan);
        case Terra_Comparison_LessOrEqual: return SFP(TStr_LessOrEqual);
        case Terra_Comparison_GreaterThan: return SFP(TStr_GreaterThan);
        case Terra_Comparison_GreaterOrEqual: return SFP(TStr_GreaterOrEqual);
        case Terra_Comparison_Equal: return SFP(TStr_Equal);
        case Terra_Comparison_NotEqual: return SFP(TStr_NotEqual);
    }
    return SFP(TStr_GreaterOrEqual);
}

TerraString terraTriggerStateToString(Terra_TriggerState value)
{
    switch (value) {
        case Terra_TriggerState_Inactive: return SFP(TStr_Inactive);
        case Terra_TriggerState_Active: return SFP(TStr_Active);
        case Terra_TriggerState_Fault: return SFP(TStr_Fault);
    }
    return SFP(TStr_Fault);
}

TerraString terraLogLevelToString(Terra_LogLevel value)
{
    switch (value) {
        case Terra_LogLevel_Debug: return SFP(TStr_DEBUG);
        case Terra_LogLevel_Info: return SFP(TStr_INFO);
        case Terra_LogLevel_Warning: return SFP(TStr_WARN);
        case Terra_LogLevel_Error: return SFP(TStr_ERROR);
    }
    return SFP(TStr_INFO);
}

TerraString terraModuleTypeToString(Terra_ModuleType value)
{
    switch (value) {
        case Terra_ModuleType_Undefined: return SFP(TStr_Undefined);
        case Terra_ModuleType_RTC: return SFP(TStr_RTC);
        case Terra_ModuleType_Storage: return SFP(TStr_Storage);
        case Terra_ModuleType_Display: return SFP(TStr_Display);
        case Terra_ModuleType_Network: return SFP(TStr_Network);
        case Terra_ModuleType_Radio: return SFP(TStr_Radio);
        case Terra_ModuleType_IOExpander: return SFP(TStr_IOExpander);
    }
    return SFP(TStr_Undefined);
}

TerraString terraControlModeToString(Terra_ControlMode value)
{
    switch (value) {
        case Terra_ControlMode_Manual: return SFP(TStr_Manual);
        case Terra_ControlMode_Automatic: return SFP(TStr_Automatic);
        case Terra_ControlMode_Disabled: return SFP(TStr_Disabled);
    }
    return SFP(TStr_Disabled);
}

TerraString terraRouteStateToString(Terra_RouteState value)
{
    switch (value) {
        case Terra_RouteState_Idle: return SFP(TStr_Idle);
        case Terra_RouteState_Requested: return SFP(TStr_Requested);
        case Terra_RouteState_Active: return SFP(TStr_Active);
        case Terra_RouteState_Complete: return SFP(TStr_Complete);
        case Terra_RouteState_Fault: return SFP(TStr_Fault);
    }
    return SFP(TStr_Idle);
}

TerraString terraAttachmentRoleToString(Terra_AttachmentRole value)
{
    switch (value) {
        case Terra_AttachmentRole_Undefined: return SFP(TStr_Undefined);
        case Terra_AttachmentRole_PrimarySensor: return SFP(TStr_PrimarySensor);
        case Terra_AttachmentRole_SecondarySensor: return SFP(TStr_SecondarySensor);
        case Terra_AttachmentRole_LevelSensor: return SFP(TStr_LevelSensor);
        case Terra_AttachmentRole_FlowSensor: return SFP(TStr_FlowSensor);
        case Terra_AttachmentRole_TemperatureSensor: return SFP(TStr_TemperatureSensor);
        case Terra_AttachmentRole_HumiditySensor: return SFP(TStr_HumiditySensor);
        case Terra_AttachmentRole_PressureSensor: return SFP(TStr_PressureSensor);
        case Terra_AttachmentRole_RainfallSensor: return SFP(TStr_RainfallSensor);
        case Terra_AttachmentRole_RainRateSensor: return SFP(TStr_RainRateSensor);
        case Terra_AttachmentRole_WindSpeedSensor: return SFP(TStr_WindSpeedSensor);
        case Terra_AttachmentRole_WindDirectionSensor: return SFP(TStr_WindDirectionSensor);
        case Terra_AttachmentRole_SolarRadiationSensor: return SFP(TStr_SolarRadiationSensor);
        case Terra_AttachmentRole_LeakSensor: return SFP(TStr_LeakSensor);
        case Terra_AttachmentRole_Storage: return SFP(TStr_Storage);
        case Terra_AttachmentRole_Pump: return SFP(TStr_Pump);
        case Terra_AttachmentRole_Valve: return SFP(TStr_Valve);
        case Terra_AttachmentRole_Diverter: return SFP(TStr_Diverter);
        case Terra_AttachmentRole_Heater: return SFP(TStr_Heater);
        case Terra_AttachmentRole_Circulator: return SFP(TStr_Circulator);
    }
    return SFP(TStr_Undefined);
}


// All remaining methods generated from minimum spanning trie

Terra_ObjectType terraObjectTypeFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 2)) {
        case 'd':
            return Terra_ObjectType_Undefined;
        case 'e':
            switch (terraTrieChar(value, 7)) {
                case 'l':
                    return Terra_ObjectType_ThermalLoop;
                case 's':
                    return Terra_ObjectType_ThermalStore;
            }
            return Terra_ObjectType_Undefined;
        case 'i':
            return Terra_ObjectType_RainCatchment;
        case 'n':
            return Terra_ObjectType_Sensor;
        case 's':
            return Terra_ObjectType_Resource;
        case 't':
            switch (terraTrieChar(value, 9)) {
                case '\0':
                    return Terra_ObjectType_Actuator;
                case 'a':
                    return Terra_ObjectType_WaterStorage;
                case 'c':
                    return Terra_ObjectType_WaterSource;
                case 'e':
                    return Terra_ObjectType_WaterRoute;
            }
            return Terra_ObjectType_Undefined;
        case 'v':
            return Terra_ObjectType_Environment;
        case 'w':
            return Terra_ObjectType_PowerRail;
    }
    return Terra_ObjectType_Undefined;
}

Terra_ReservoirType TerraReservoirTypeFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 0)) {
        case 't':
            return Terra_ReservoirType_Thermal;
        case 'u':
            return Terra_ReservoirType_Undefined;
        case 'w':
            return Terra_ReservoirType_Water;
    }
    return Terra_ReservoirType_Undefined;
}

Terra_ResourceState TerraReservoirStateFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 0)) {
        case 'f':
            return Terra_ResourceState_Fault;
        case 'h':
            return Terra_ResourceState_High;
        case 'l':
            return Terra_ResourceState_Low;
        case 'n':
            return Terra_ResourceState_Normal;
        case 'r':
            return Terra_ResourceState_Reserve;
        case 'u':
            return Terra_ResourceState_Unknown;
    }
    return Terra_ResourceState_Unknown;
}

Terra_WaterSourceType terraWaterSourceTypeFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 2)) {
        case 'c':
            return Terra_WaterSourceType_Reclaimed;
        case 'd':
            return Terra_WaterSourceType_Undefined;
        case 'i':
            return Terra_WaterSourceType_Rainwater;
        case 'l':
            return Terra_WaterSourceType_Well;
        case 'n':
            return Terra_WaterSourceType_Municipal;
        case 'o':
            return Terra_WaterSourceType_Stored;
        case 'r':
            return Terra_WaterSourceType_Surface;
    }
    return Terra_WaterSourceType_Undefined;
}

Terra_WaterStorageType terraWaterStorageTypeFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 0)) {
        case 'c':
            return Terra_WaterStorageType_Cistern;
        case 'r':
            return Terra_WaterStorageType_Reservoir;
        case 't':
            return Terra_WaterStorageType_Tank;
        case 'u':
            return Terra_WaterStorageType_Undefined;
    }
    return Terra_WaterStorageType_Undefined;
}

Terra_SensorType terraSensorTypeFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 8)) {
        case '\0':
            switch (terraTrieChar(value, 3)) {
                case 'a':
                    return Terra_SensorType_Binary;
                case 'e':
                    return Terra_SensorType_Level;
                case 'i':
                    return Terra_SensorType_Humidity;
                case 'k':
                    return Terra_SensorType_Leak;
                case 'l':
                    return Terra_SensorType_Analog;
                case 'n':
                    return Terra_SensorType_Rainfall;
                case 'o':
                    return Terra_SensorType_Remote;
                case 'r':
                    return Terra_SensorType_Current;
                case 's':
                    return Terra_SensorType_Pressure;
                case 't':
                    return Terra_SensorType_Voltage;
                case 'w':
                    return Terra_SensorType_Flow;
            }
            return Terra_SensorType_Undefined;
        case 'c':
            return Terra_SensorType_WindDirection;
        case 'd':
            switch (terraTrieChar(value, 0)) {
                case 'u':
                    return Terra_SensorType_Undefined;
                case 'w':
                    return Terra_SensorType_WindSpeed;
            }
            return Terra_SensorType_Undefined;
        case 'i':
            return Terra_SensorType_SolarRadiation;
        case 'u':
            return Terra_SensorType_Temperature;
    }
    return Terra_SensorType_Undefined;
}

Terra_ActuatorType terraActuatorTypeFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 4)) {
        case '\0':
            return Terra_ActuatorType_Pump;
        case 'a':
            return Terra_ActuatorType_Variable;
        case 'e':
            switch (terraTrieChar(value, 0)) {
                case 'h':
                    return Terra_ActuatorType_Heater;
                case 'v':
                    return Terra_ActuatorType_Valve;
            }
            return Terra_ActuatorType_Undefined;
        case 'f':
            return Terra_ActuatorType_Undefined;
        case 'p':
            return Terra_ActuatorType_SumpPump;
        case 'r':
            return Terra_ActuatorType_Diverter;
        case 't':
            return Terra_ActuatorType_Digital;
        case 'u':
            return Terra_ActuatorType_Circulator;
    }
    return Terra_ActuatorType_Undefined;
}

Terra_MeasurementMode terraMeasurementModeFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 0)) {
        case 'c':
            return Terra_MeasurementMode_Count;
        case 'i':
            return Terra_MeasurementMode_Imperial;
        case 'm':
            return Terra_MeasurementMode_Metric;
        case 's':
            return Terra_MeasurementMode_Scientific;
        case 'u':
            return Terra_MeasurementMode_Undefined;
    }
    return Terra_MeasurementMode_Undefined;
}

Terra_EnableMode terraEnableModeFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 0)) {
        case 'a':
            return Terra_EnableMode_Average;
        case 'c':
            return Terra_EnableMode_Count;
        case 'h':
            return Terra_EnableMode_Highest;
        case 'i':
            return Terra_EnableMode_InOrder;
        case 'l':
            return Terra_EnableMode_Lowest;
        case 'm':
            return Terra_EnableMode_Multiply;
        case 'r':
            return Terra_EnableMode_RevOrder;
        case 'u':
            return Terra_EnableMode_Undefined;
    }
    return Terra_EnableMode_Undefined;
}

Terra_PinMode terraPinModeFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 7)) {
        case '\0':
            return Terra_PinMode_Count;
        case 'e':
            return Terra_PinMode_Undefined;
        case 'i':
            switch (terraTrieChar(value, 16)) {
                case '\0':
                    return Terra_PinMode_Digital_Input;
                case 'd':
                    return Terra_PinMode_Digital_Input_PullDown;
                case 'u':
                    return Terra_PinMode_Digital_Input_PullUp;
            }
            return Terra_PinMode_Undefined;
        case 'n':
            return Terra_PinMode_Analog_Input;
        case 'o':
            return Terra_PinMode_Digital_Output;
        case 'u':
            return Terra_PinMode_Analog_Output;
    }
    return Terra_PinMode_Undefined;
}

Terra_UnitsType unitsTypeFromSymbol(const TerraString &value)
{
    switch (terraTrieChar(value, 3)) {
        case '\0':
            switch (terraTrieChar(value, 0)) {
                case '%':
                    return Terra_UnitsType_Percent;
                case 'a':
                    return Terra_UnitsType_Amps;
                case 'c':
                    return Terra_UnitsType_Celsius;
                case 'd':
                    return Terra_UnitsType_Degrees;
                case 'f':
                    return Terra_UnitsType_Fahrenheit;
                case 'g':
                    return Terra_UnitsType_GallonsUS;
                case 'h':
                    return Terra_UnitsType_Hectopascals;
                case 'i':
                    return Terra_UnitsType_Inches;
                case 'k':
                    switch (terraTrieChar(value, 1)) {
                        case '\0':
                            return Terra_UnitsType_Kelvin;
                        case 'p':
                            return Terra_UnitsType_Kilopascals;
                        case 'w':
                            return Terra_UnitsType_KilowattHours;
                    }
                    return Terra_UnitsType_Undefined;
                case 'l':
                    return Terra_UnitsType_Liters;
                case 'm':
                    switch (terraTrieChar(value, 1)) {
                        case '/':
                            return Terra_UnitsType_MetersPerSecond;
                        case 'm':
                            return Terra_UnitsType_Millimeters;
                        case 'p':
                            return Terra_UnitsType_MilesPerHour;
                    }
                    return Terra_UnitsType_Undefined;
                case 'p':
                    return Terra_UnitsType_PSI;
                case 'r':
                    return Terra_UnitsType_Raw;
                case 'v':
                    return Terra_UnitsType_Volts;
                case 'w':
                    return Terra_UnitsType_Watts;
            }
            return Terra_UnitsType_Undefined;
        case '/':
            return Terra_UnitsType_GallonsPerMinute;
        case '2':
            return Terra_UnitsType_WattsPerSquareMeter;
        case 'e':
            return Terra_UnitsType_Undefined;
        case 'h':
            switch (terraTrieChar(value, 0)) {
                case 'i':
                    return Terra_UnitsType_InchesPerHour;
                case 'k':
                    return Terra_UnitsType_KilometersPerHour;
                case 'm':
                    return Terra_UnitsType_MillimetersPerHour;
            }
            return Terra_UnitsType_Undefined;
        case 'i':
            return Terra_UnitsType_LitersPerMinute;
    }
    return Terra_UnitsType_Undefined;
}

Terra_UnitsCategory terraUnitsCategoryFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 6)) {
        case '\0':
            switch (terraTrieChar(value, 0)) {
                case 'a':
                    return Terra_UnitsCategory_Angle;
                case 'c':
                    return Terra_UnitsCategory_Count;
                case 'e':
                    return Terra_UnitsCategory_Energy;
                case 'p':
                    return Terra_UnitsCategory_Power;
                case 'r':
                    return Terra_UnitsCategory_Raw;
                case 's':
                    return Terra_UnitsCategory_Speed;
            }
            return Terra_UnitsCategory_Undefined;
        case 'a':
            switch (terraTrieChar(value, 0)) {
                case 'i':
                    return Terra_UnitsCategory_Irradiance;
                case 't':
                    return Terra_UnitsCategory_Temperature;
            }
            return Terra_UnitsCategory_Undefined;
        case 'c':
            return Terra_UnitsCategory_Distance;
        case 'e':
            return Terra_UnitsCategory_Voltage;
        case 'f':
            return Terra_UnitsCategory_LiquidFlowRate;
        case 'n':
            return Terra_UnitsCategory_Undefined;
        case 'r':
            return Terra_UnitsCategory_Pressure;
        case 't':
            switch (terraTrieChar(value, 0)) {
                case 'c':
                    return Terra_UnitsCategory_Current;
                case 'p':
                    return Terra_UnitsCategory_Percentile;
                case 'r':
                    return Terra_UnitsCategory_RainRate;
            }
            return Terra_UnitsCategory_Undefined;
        case 'v':
            return Terra_UnitsCategory_LiquidVolume;
    }
    return Terra_UnitsCategory_Undefined;
}

Terra_RailType terraRailTypeFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 2)) {
        case '1':
            return Terra_RailType_DC12V;
        case '2':
            return Terra_RailType_DC24V;
        case '3':
            return Terra_RailType_DC3V3;
        case '5':
            return Terra_RailType_DC5V;
        case 'd':
            return Terra_RailType_Undefined;
        case 's':
            return Terra_RailType_Custom;
        case 'u':
            return Terra_RailType_Count;
    }
    return Terra_RailType_Undefined;
}

Terra_Comparison terraComparisonFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 7)) {
        case '\0':
            return Terra_Comparison_Equal;
        case 'l':
            return Terra_Comparison_NotEqual;
        case 'n':
            return Terra_Comparison_LessThan;
        case 'o':
            return Terra_Comparison_GreaterOrEqual;
        case 'q':
            return Terra_Comparison_LessOrEqual;
        case 't':
            return Terra_Comparison_GreaterThan;
    }
    return Terra_Comparison_GreaterOrEqual;
}

Terra_TriggerState terraTriggerStateFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 0)) {
        case 'a':
            return Terra_TriggerState_Active;
        case 'f':
            return Terra_TriggerState_Fault;
        case 'i':
            return Terra_TriggerState_Inactive;
    }
    return Terra_TriggerState_Fault;
}

Terra_LogLevel terraLogLevelFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 0)) {
        case 'd':
            return Terra_LogLevel_Debug;
        case 'e':
            return Terra_LogLevel_Error;
        case 'i':
            return Terra_LogLevel_Info;
        case 'w':
            return Terra_LogLevel_Warning;
    }
    return Terra_LogLevel_Info;
}

Terra_ModuleType terraModuleTypeFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 3)) {
        case '\0':
            return Terra_ModuleType_RTC;
        case 'e':
            return Terra_ModuleType_Undefined;
        case 'i':
            return Terra_ModuleType_Radio;
        case 'p':
            return Terra_ModuleType_Display;
        case 'r':
            return Terra_ModuleType_Storage;
        case 'w':
            return Terra_ModuleType_Network;
        case 'x':
            return Terra_ModuleType_IOExpander;
    }
    return Terra_ModuleType_Undefined;
}

Terra_ControlMode terraControlModeFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 0)) {
        case 'a':
            return Terra_ControlMode_Automatic;
        case 'd':
            return Terra_ControlMode_Disabled;
        case 'm':
            return Terra_ControlMode_Manual;
    }
    return Terra_ControlMode_Disabled;
}

Terra_RouteState terraRouteStateFromString(const TerraString &value)
{
    switch (terraTrieChar(value, 0)) {
        case 'a':
            return Terra_RouteState_Active;
        case 'c':
            return Terra_RouteState_Complete;
        case 'f':
            return Terra_RouteState_Fault;
        case 'i':
            return Terra_RouteState_Idle;
        case 'r':
            return Terra_RouteState_Requested;
    }
    return Terra_RouteState_Idle;
}

Terra_AttachmentRole terraAttachmentRoleFromString(const TerraString &value)
{
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_PrimarySensor))) return Terra_AttachmentRole_PrimarySensor;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_SecondarySensor))) return Terra_AttachmentRole_SecondarySensor;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_LevelSensor))) return Terra_AttachmentRole_LevelSensor;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_FlowSensor))) return Terra_AttachmentRole_FlowSensor;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_TemperatureSensor))) return Terra_AttachmentRole_TemperatureSensor;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_HumiditySensor))) return Terra_AttachmentRole_HumiditySensor;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_PressureSensor))) return Terra_AttachmentRole_PressureSensor;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_RainfallSensor))) return Terra_AttachmentRole_RainfallSensor;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_RainRateSensor))) return Terra_AttachmentRole_RainRateSensor;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_WindSpeedSensor))) return Terra_AttachmentRole_WindSpeedSensor;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_WindDirectionSensor))) return Terra_AttachmentRole_WindDirectionSensor;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_SolarRadiationSensor))) return Terra_AttachmentRole_SolarRadiationSensor;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_LeakSensor))) return Terra_AttachmentRole_LeakSensor;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_Storage))) return Terra_AttachmentRole_Storage;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_Pump))) return Terra_AttachmentRole_Pump;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_Valve))) return Terra_AttachmentRole_Valve;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_Diverter))) return Terra_AttachmentRole_Diverter;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_Heater))) return Terra_AttachmentRole_Heater;
    if (terraStringEqualsIgnoreCase(value, SFP(TStr_Circulator))) return Terra_AttachmentRole_Circulator;
    return Terra_AttachmentRole_Undefined;
}
