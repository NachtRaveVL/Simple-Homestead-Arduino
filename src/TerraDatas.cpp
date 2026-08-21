/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Object Data
*/

#include "TerraDatas.h"
#include "TerraData.h"
#include "TerraStrings.h"
#include "TerraUtils.h"

static TerraString terraNumber(long value)
{
#if defined(ARDUINO)
    return String(value);
#else
    return std::to_string(value);
#endif
}

static TerraString terraUnsigned(uint32_t value)
{
#if defined(ARDUINO)
    return String(value);
#else
    return std::to_string(value);
#endif
}

static TerraString terraFloat(float value)
{
    return terraFloatToString(value, 6);
}

static TerraString terraBool(bool value)
{
    return value ? TerraString("true") : TerraString("false");
}

static TerraString objectFields(const TerraObjectData &data)
{
    return TerraString("\"key\":") + terraUnsigned(data.key) +
           ",\"objectType\":\"" + terraObjectTypeToString(data.objectType) +
           "\",\"name\":\"" + terraJsonEscape(data.name) +
           "\",\"enabled\":" + terraBool(data.enabled);
}

static bool parseObjectFields(const TerraString &json, TerraObjectData &data)
{
    long key = 0;
    TerraString objectType;
    if (!terraJsonExtractLong(json, "key", key) || key < 0) return false;
    if (!terraJsonExtractString(json, "objectType", objectType)) return false;
    if (!terraJsonExtractString(json, "name", data.name)) return false;
    if (!terraJsonExtractBool(json, "enabled", data.enabled)) return false;
    data.key = (uint32_t)key;
    data.objectType = terraObjectTypeFromString(objectType);
    return terraStringEqualsIgnoreCase(objectType, terraObjectTypeToString(data.objectType));
}

static void copyObjectFields(const TerraObjectData &from, TerraObjectData &to)
{
    to.key = from.key;
    to.name = from.name;
    to.enabled = from.enabled;
}

TerraObjectData::TerraObjectData()
    : key(TERRA_INVALID_KEY), objectType(Terra_ObjectType_Undefined), name(), enabled(true)
{ }

TerraString TerraObjectData::toJSON() const
{
    return TerraString("{") + objectFields(*this) + "}";
}

bool TerraObjectData::fromJSON(const TerraString &json)
{
    return parseObjectFields(json, *this);
}

TerraSensorData::TerraSensorData()
    : TerraObjectData(), sensorType(Terra_SensorType_Undefined), reportedType(Terra_SensorType_Undefined), unit(Terra_Unit_Raw),
      updateIntervalMs(1000), staleAfterMs(0), hasPinDriver(false),
      pinSetup(TERRA_INVALID_PIN, Terra_PinMode_Undefined, false),
      driverCalibrated(false), driverRawMinimum(0.0f), driverRawMaximum(1023.0f),
      driverValueMinimum(0.0f), driverValueMaximum(100.0f),
      sensorCalibrated(false), sensorRawMinimum(0.0f), sensorRawMaximum(1023.0f),
      sensorValueMinimum(0.0f), sensorValueMaximum(100.0f)
{
    objectType = Terra_ObjectType_Sensor;
}

TerraString TerraSensorData::toJSON() const
{
    return TerraString("{") + objectFields(*this) +
           ",\"sensorType\":\"" + terraSensorTypeToString(sensorType) +
           "\",\"reportedType\":\"" + terraSensorTypeToString(reportedType) +
           "\",\"unit\":\"" + terraUnitToString(unit) +
           "\",\"updateIntervalMs\":" + terraUnsigned(updateIntervalMs) +
           ",\"staleAfterMs\":" + terraUnsigned(staleAfterMs) +
           ",\"hasPinDriver\":" + terraBool(hasPinDriver) +
           ",\"pin\":" + terraNumber(pinSetup.pin) +
           ",\"pinMode\":\"" + terraPinModeToString(pinSetup.mode) +
           "\",\"activeLow\":" + terraBool(pinSetup.activeLow) +
           ",\"driverCalibrated\":" + terraBool(driverCalibrated) +
           ",\"driverRawMinimum\":" + terraFloat(driverRawMinimum) +
           ",\"driverRawMaximum\":" + terraFloat(driverRawMaximum) +
           ",\"driverValueMinimum\":" + terraFloat(driverValueMinimum) +
           ",\"driverValueMaximum\":" + terraFloat(driverValueMaximum) +
           ",\"sensorCalibrated\":" + terraBool(sensorCalibrated) +
           ",\"sensorRawMinimum\":" + terraFloat(sensorRawMinimum) +
           ",\"sensorRawMaximum\":" + terraFloat(sensorRawMaximum) +
           ",\"sensorValueMinimum\":" + terraFloat(sensorValueMinimum) +
           ",\"sensorValueMaximum\":" + terraFloat(sensorValueMaximum) + "}";
}

bool TerraSensorData::fromJSON(const TerraString &json)
{
    TerraString sensorTypeStr, reportedTypeStr, unitStr, pinModeStr;
    long updateInterval = 0, staleAfter = 0, pin = 0;
    if (!parseObjectFields(json, *this) || objectType != Terra_ObjectType_Sensor) return false;
    if (!terraJsonExtractString(json, "sensorType", sensorTypeStr)) return false;
    if (!terraJsonExtractString(json, "reportedType", reportedTypeStr)) return false;
    if (!terraJsonExtractString(json, "unit", unitStr)) return false;
    if (!terraJsonExtractLong(json, "updateIntervalMs", updateInterval) || updateInterval < 0) return false;
    if (!terraJsonExtractLong(json, "staleAfterMs", staleAfter) || staleAfter < 0) return false;
    if (!terraJsonExtractBool(json, "hasPinDriver", hasPinDriver)) return false;
    if (!terraJsonExtractLong(json, "pin", pin) || pin < 0 || pin > 255) return false;
    if (!terraJsonExtractString(json, "pinMode", pinModeStr)) return false;
    if (!terraJsonExtractBool(json, "activeLow", pinSetup.activeLow)) return false;
    if (!terraJsonExtractBool(json, "driverCalibrated", driverCalibrated)) return false;
    if (!terraJsonExtractFloat(json, "driverRawMinimum", driverRawMinimum)) return false;
    if (!terraJsonExtractFloat(json, "driverRawMaximum", driverRawMaximum)) return false;
    if (!terraJsonExtractFloat(json, "driverValueMinimum", driverValueMinimum)) return false;
    if (!terraJsonExtractFloat(json, "driverValueMaximum", driverValueMaximum)) return false;
    if (!terraJsonExtractBool(json, "sensorCalibrated", sensorCalibrated)) return false;
    if (!terraJsonExtractFloat(json, "sensorRawMinimum", sensorRawMinimum)) return false;
    if (!terraJsonExtractFloat(json, "sensorRawMaximum", sensorRawMaximum)) return false;
    if (!terraJsonExtractFloat(json, "sensorValueMinimum", sensorValueMinimum)) return false;
    if (!terraJsonExtractFloat(json, "sensorValueMaximum", sensorValueMaximum)) return false;
    sensorType = terraSensorTypeFromString(sensorTypeStr);
    reportedType = terraSensorTypeFromString(reportedTypeStr);
    unit = terraUnitFromString(unitStr);
    pinSetup.pin = (uint8_t)pin;
    pinSetup.mode = terraPinModeFromString(pinModeStr);
    if (!terraStringEqualsIgnoreCase(sensorTypeStr, terraSensorTypeToString(sensorType))) return false;
    if (!terraStringEqualsIgnoreCase(reportedTypeStr, terraSensorTypeToString(reportedType))) return false;
    if (!terraStringEqualsIgnoreCase(unitStr, terraUnitToString(unit))) return false;
    if (!terraStringEqualsIgnoreCase(pinModeStr, terraPinModeToString(pinSetup.mode))) return false;
    if (hasPinDriver && (!pinSetup.isValid() || pinSetup.isOutput())) return false;
    if (driverCalibrated && isFPEqual(driverRawMinimum, driverRawMaximum)) return false;
    if (sensorCalibrated && (sensorType != Terra_SensorType_Analog || isFPEqual(sensorRawMinimum, sensorRawMaximum))) return false;
    updateIntervalMs = (uint32_t)updateInterval;
    staleAfterMs = (uint32_t)staleAfter;
    return true;
}

TerraActuatorData::TerraActuatorData()
    : TerraObjectData(), actuatorType(Terra_ActuatorType_Undefined),
      enableMode(Terra_EnableMode_Highest), maxContinuousMs(0), hasPinDriver(false),
      pinSetup(TERRA_INVALID_PIN, Terra_PinMode_Undefined, false), maximumRaw(255),
      sumpStartPercent(TERRA_SUMP_START_LEVEL_PERCENT), sumpStopPercent(TERRA_SUMP_STOP_LEVEL_PERCENT),
      sumpAlarmPercent(TERRA_SUMP_ALARM_LEVEL_PERCENT), levelSensorKey(TERRA_INVALID_KEY)
{
    objectType = Terra_ObjectType_Actuator;
}

TerraString TerraActuatorData::toJSON() const
{
    TerraString json = TerraString("{") + objectFields(*this) +
                       ",\"actuatorType\":\"" + terraActuatorTypeToString(actuatorType) +
                       "\",\"enableMode\":\"" + terraEnableModeToString(enableMode) +
                       "\",\"maxContinuousMs\":" + terraUnsigned(maxContinuousMs) +
                       ",\"hasPinDriver\":" + terraBool(hasPinDriver) +
                       ",\"pin\":" + terraNumber(pinSetup.pin) +
                       ",\"pinMode\":\"" + terraPinModeToString(pinSetup.mode) +
                       "\",\"activeLow\":" + terraBool(pinSetup.activeLow) +
                       ",\"maximumRaw\":" + terraNumber(maximumRaw);
    if (actuatorType == Terra_ActuatorType_SumpPump) {
        json += ",\"sumpStartPercent\":" + terraFloat(sumpStartPercent) +
                ",\"sumpStopPercent\":" + terraFloat(sumpStopPercent) +
                ",\"sumpAlarmPercent\":" + terraFloat(sumpAlarmPercent) +
                ",\"levelSensorKey\":" + terraUnsigned(levelSensorKey);
    }
    return json + "}";
}

bool TerraActuatorData::fromJSON(const TerraString &json)
{
    TerraString actuatorTypeStr, enableModeStr, pinModeStr;
    long maxRuntime = 0, pin = 0, maxRaw = 0;
    if (!parseObjectFields(json, *this) || objectType != Terra_ObjectType_Actuator) return false;
    if (!terraJsonExtractString(json, "actuatorType", actuatorTypeStr)) return false;
    if (!terraJsonExtractString(json, "enableMode", enableModeStr)) return false;
    if (!terraJsonExtractLong(json, "maxContinuousMs", maxRuntime) || maxRuntime < 0) return false;
    if (!terraJsonExtractBool(json, "hasPinDriver", hasPinDriver)) return false;
    if (!terraJsonExtractLong(json, "pin", pin) || pin < 0 || pin > 255) return false;
    if (!terraJsonExtractString(json, "pinMode", pinModeStr)) return false;
    if (!terraJsonExtractBool(json, "activeLow", pinSetup.activeLow)) return false;
    if (!terraJsonExtractLong(json, "maximumRaw", maxRaw) || maxRaw <= 0 || maxRaw > 65535) return false;
    actuatorType = terraActuatorTypeFromString(actuatorTypeStr);
    enableMode = terraEnableModeFromString(enableModeStr);
    pinSetup.pin = (uint8_t)pin;
    pinSetup.mode = terraPinModeFromString(pinModeStr);
    if (!terraStringEqualsIgnoreCase(actuatorTypeStr, terraActuatorTypeToString(actuatorType))) return false;
    if (!terraStringEqualsIgnoreCase(enableModeStr, terraEnableModeToString(enableMode))) return false;
    if (!terraStringEqualsIgnoreCase(pinModeStr, terraPinModeToString(pinSetup.mode))) return false;
    if (hasPinDriver && (!pinSetup.isValid() || !pinSetup.isOutput())) return false;
    if (actuatorType == Terra_ActuatorType_SumpPump) {
        long levelSensor = 0;
        if (!terraJsonExtractFloat(json, "sumpStartPercent", sumpStartPercent) ||
            !terraJsonExtractFloat(json, "sumpStopPercent", sumpStopPercent) ||
            !terraJsonExtractFloat(json, "sumpAlarmPercent", sumpAlarmPercent) ||
            !terraJsonExtractLong(json, "levelSensorKey", levelSensor) || levelSensor < 0 ||
            sumpStopPercent < 0.0f || sumpStopPercent >= sumpStartPercent ||
            sumpStartPercent >= sumpAlarmPercent || sumpAlarmPercent > 100.0f) return false;
        levelSensorKey = (uint32_t)levelSensor;
    }
    maxContinuousMs = (uint32_t)maxRuntime;
    maximumRaw = (int)maxRaw;
    return true;
}

TerraResourceData::TerraResourceData()
    : TerraObjectData(), resourceType(Terra_ResourceType_Undefined), level(0.0f),
      reserveLevel(10.0f), lowLevel(25.0f), highLevel(90.0f)
{
    objectType = Terra_ObjectType_Resource;
}

TerraString TerraResourceData::toJSON() const
{
    return TerraString("{") + objectFields(*this) +
           ",\"resourceType\":\"" + terraResourceTypeToString(resourceType) +
           "\",\"level\":" + terraFloat(level) +
           ",\"reserveLevel\":" + terraFloat(reserveLevel) +
           ",\"lowLevel\":" + terraFloat(lowLevel) +
           ",\"highLevel\":" + terraFloat(highLevel) + "}";
}

bool TerraResourceData::fromJSON(const TerraString &json)
{
    TerraString type;
    if (!parseObjectFields(json, *this)) return false;
    if (!terraJsonExtractString(json, "resourceType", type)) return false;
    if (!terraJsonExtractFloat(json, "level", level)) return false;
    if (!terraJsonExtractFloat(json, "reserveLevel", reserveLevel)) return false;
    if (!terraJsonExtractFloat(json, "lowLevel", lowLevel)) return false;
    if (!terraJsonExtractFloat(json, "highLevel", highLevel)) return false;
    resourceType = terraResourceTypeFromString(type);
    if (!terraStringEqualsIgnoreCase(type, terraResourceTypeToString(resourceType))) return false;
    return level >= 0.0f && level <= 100.0f && reserveLevel >= 0.0f && reserveLevel <= lowLevel && lowLevel < highLevel && highLevel <= 100.0f;
}

TerraWaterStorageData::TerraWaterStorageData()
    : TerraResourceData(), storageType(Terra_WaterStorageType_Undefined), capacityLiters(0.0f),
      fillStartPercent(35.0f), fillStopPercent(90.0f), overflowPercent(99.0f),
      levelSensorKey(TERRA_INVALID_KEY)
{
    objectType = Terra_ObjectType_WaterStorage;
    resourceType = Terra_ResourceType_Water;
}

TerraString TerraWaterStorageData::toJSON() const
{
    return TerraString("{") + objectFields(*this) +
           ",\"resourceType\":\"" + terraResourceTypeToString(resourceType) +
           "\",\"level\":" + terraFloat(level) +
           ",\"reserveLevel\":" + terraFloat(reserveLevel) +
           ",\"lowLevel\":" + terraFloat(lowLevel) +
           ",\"highLevel\":" + terraFloat(highLevel) +
           ",\"storageType\":\"" + terraWaterStorageTypeToString(storageType) + "\"" +
           ",\"capacityLiters\":" + terraFloat(capacityLiters) +
           ",\"fillStartPercent\":" + terraFloat(fillStartPercent) +
           ",\"fillStopPercent\":" + terraFloat(fillStopPercent) +
           ",\"overflowPercent\":" + terraFloat(overflowPercent) +
           ",\"levelSensorKey\":" + terraUnsigned(levelSensorKey) + "}";
}

bool TerraWaterStorageData::fromJSON(const TerraString &json)
{
    TerraResourceData base;
    TerraString type;
    long levelSensor = 0;
    if (!base.fromJSON(json)) return false;
    if (base.objectType != Terra_ObjectType_WaterStorage || base.resourceType != Terra_ResourceType_Water) return false;
    if (!terraJsonExtractString(json, "storageType", type)) return false;
    if (!terraJsonExtractFloat(json, "capacityLiters", capacityLiters) || capacityLiters < 0.0f) return false;
    if (!terraJsonExtractFloat(json, "fillStartPercent", fillStartPercent)) return false;
    if (!terraJsonExtractFloat(json, "fillStopPercent", fillStopPercent)) return false;
    if (!terraJsonExtractFloat(json, "overflowPercent", overflowPercent)) return false;
    if (!terraJsonExtractLong(json, "levelSensorKey", levelSensor) || levelSensor < 0) return false;
    storageType = terraWaterStorageTypeFromString(type);
    if (!terraStringEqualsIgnoreCase(type, terraWaterStorageTypeToString(storageType))) return false;
    if (fillStartPercent < 0.0f || fillStartPercent >= fillStopPercent ||
        fillStopPercent > overflowPercent || overflowPercent > 100.0f) return false;
    copyObjectFields(base, *this);
    objectType = Terra_ObjectType_WaterStorage;
    resourceType = Terra_ResourceType_Water;
    level = base.level;
    reserveLevel = base.reserveLevel;
    lowLevel = base.lowLevel;
    highLevel = base.highLevel;
    levelSensorKey = (uint32_t)levelSensor;
    return true;
}

TerraCisternData::TerraCisternData()
    : TerraWaterStorageData()
{
    storageType = Terra_WaterStorageType_Cistern;
}

TerraString TerraCisternData::toJSON() const
{
    return TerraWaterStorageData::toJSON();
}
bool TerraCisternData::fromJSON(const TerraString &json)
{
    return TerraWaterStorageData::fromJSON(json) && storageType == Terra_WaterStorageType_Cistern;
}

TerraWaterSourceData::TerraWaterSourceData()
    : TerraObjectData(), sourceType(Terra_WaterSourceType_Undefined), priority(0), available(true),
      level(100.0f), reserveLevel(0.0f), maximumFlowLpm(0.0f), levelSensorKey(TERRA_INVALID_KEY)
{
    objectType = Terra_ObjectType_WaterSource;
}

TerraString TerraWaterSourceData::toJSON() const
{
    return TerraString("{") + objectFields(*this) +
           ",\"sourceType\":\"" + terraWaterSourceTypeToString(sourceType) +
           "\",\"priority\":" + terraNumber(priority) +
           ",\"available\":" + terraBool(available) +
           ",\"level\":" + terraFloat(level) +
           ",\"reserveLevel\":" + terraFloat(reserveLevel) +
           ",\"maximumFlowLpm\":" + terraFloat(maximumFlowLpm) +
           ",\"levelSensorKey\":" + terraUnsigned(levelSensorKey) + "}";
}

bool TerraWaterSourceData::fromJSON(const TerraString &json)
{
    TerraString type;
    long priorityValue = 0, levelSensor = 0;
    if (!parseObjectFields(json, *this)) return false;
    if (!terraJsonExtractString(json, "sourceType", type)) return false;
    if (!terraJsonExtractLong(json, "priority", priorityValue) || priorityValue < 0 || priorityValue > 255) return false;
    if (!terraJsonExtractBool(json, "available", available)) return false;
    if (!terraJsonExtractFloat(json, "level", level) || level < 0.0f || level > 100.0f) return false;
    if (!terraJsonExtractFloat(json, "reserveLevel", reserveLevel) || reserveLevel < 0.0f || reserveLevel > 100.0f) return false;
    if (!terraJsonExtractFloat(json, "maximumFlowLpm", maximumFlowLpm) || maximumFlowLpm < 0.0f) return false;
    if (!terraJsonExtractLong(json, "levelSensorKey", levelSensor) || levelSensor < 0) return false;
    if (objectType != Terra_ObjectType_WaterSource) return false;
    sourceType = terraWaterSourceTypeFromString(type);
    if (!terraStringEqualsIgnoreCase(type, terraWaterSourceTypeToString(sourceType))) return false;
    priority = (uint8_t)priorityValue;
    levelSensorKey = (uint32_t)levelSensor;
    return true;
}

TerraWaterRouteData::TerraWaterRouteData()
    : TerraObjectData(), sourceKey(TERRA_INVALID_KEY), destinationKey(TERRA_INVALID_KEY),
      pumpKey(TERRA_INVALID_KEY), flowSensorKey(TERRA_INVALID_KEY),
      destinationStartPercent(40.0f), destinationStopPercent(90.0f), minimumFlowLpm(0.0f),
      maximumFlowLpm(0.0f), routeState(Terra_RouteState_Idle)
{
    objectType = Terra_ObjectType_WaterRoute;
}

TerraString TerraWaterRouteData::toJSON() const
{
    return TerraString("{") + objectFields(*this) +
           ",\"sourceKey\":" + terraUnsigned(sourceKey) +
           ",\"destinationKey\":" + terraUnsigned(destinationKey) +
           ",\"pumpKey\":" + terraUnsigned(pumpKey) +
           ",\"flowSensorKey\":" + terraUnsigned(flowSensorKey) +
           ",\"destinationStartPercent\":" + terraFloat(destinationStartPercent) +
           ",\"destinationStopPercent\":" + terraFloat(destinationStopPercent) +
           ",\"minimumFlowLpm\":" + terraFloat(minimumFlowLpm) +
           ",\"maximumFlowLpm\":" + terraFloat(maximumFlowLpm) +
           ",\"routeState\":\"" + terraRouteStateToString(routeState) + "\"}";
}

bool TerraWaterRouteData::fromJSON(const TerraString &json)
{
    long source = 0, destination = 0, pump = 0, flowSensor = 0;
    TerraString state;
    if (!parseObjectFields(json, *this)) return false;
    if (!terraJsonExtractLong(json, "sourceKey", source) || source < 0) return false;
    if (!terraJsonExtractLong(json, "destinationKey", destination) || destination < 0) return false;
    if (!terraJsonExtractLong(json, "pumpKey", pump) || pump < 0) return false;
    if (!terraJsonExtractLong(json, "flowSensorKey", flowSensor) || flowSensor < 0) return false;
    if (!terraJsonExtractFloat(json, "destinationStartPercent", destinationStartPercent)) return false;
    if (!terraJsonExtractFloat(json, "destinationStopPercent", destinationStopPercent)) return false;
    if (!terraJsonExtractFloat(json, "minimumFlowLpm", minimumFlowLpm)) return false;
    if (!terraJsonExtractFloat(json, "maximumFlowLpm", maximumFlowLpm)) return false;
    if (!terraJsonExtractString(json, "routeState", state)) return false;
    if (destinationStartPercent < 0.0f || destinationStartPercent > destinationStopPercent || destinationStopPercent > 100.0f) return false;
    if (minimumFlowLpm < 0.0f || maximumFlowLpm < 0.0f) return false;
    if (maximumFlowLpm > 0.0f && minimumFlowLpm > maximumFlowLpm) return false;
    if (objectType != Terra_ObjectType_WaterRoute) return false;
    sourceKey = (uint32_t)source;
    destinationKey = (uint32_t)destination;
    pumpKey = (uint32_t)pump;
    flowSensorKey = (uint32_t)flowSensor;
    routeState = terraRouteStateFromString(state);
    return terraStringEqualsIgnoreCase(state, terraRouteStateToString(routeState));
}

TerraRainCatchmentData::TerraRainCatchmentData()
    : TerraObjectData(), areaSquareMeters(0.0f), collectionEfficiency(0.85f)
{
    objectType = Terra_ObjectType_RainCatchment;
}

TerraString TerraRainCatchmentData::toJSON() const
{
    return TerraString("{") + objectFields(*this) +
           ",\"areaSquareMeters\":" + terraFloat(areaSquareMeters) +
           ",\"collectionEfficiency\":" + terraFloat(collectionEfficiency) + "}";
}

bool TerraRainCatchmentData::fromJSON(const TerraString &json)
{
    if (!parseObjectFields(json, *this)) return false;
    if (objectType != Terra_ObjectType_RainCatchment) return false;
    if (!terraJsonExtractFloat(json, "areaSquareMeters", areaSquareMeters) || areaSquareMeters < 0.0f) return false;
    if (!terraJsonExtractFloat(json, "collectionEfficiency", collectionEfficiency) ||
        collectionEfficiency < 0.0f || collectionEfficiency > 1.0f) return false;
    return true;
}

TerraThermalStoreData::TerraThermalStoreData()
    : TerraResourceData(), temperatureC(0.0f), temperatureSensorKey(TERRA_INVALID_KEY), minimumTargetC(0.0f),
      maximumTargetC(80.0f), absoluteMaximumC(95.0f)
{
    objectType = Terra_ObjectType_ThermalStore;
    resourceType = Terra_ResourceType_Thermal;
}

TerraString TerraThermalStoreData::toJSON() const
{
    return TerraString("{") + objectFields(*this) +
           ",\"resourceType\":\"" + terraResourceTypeToString(resourceType) +
           "\",\"level\":" + terraFloat(level) +
           ",\"reserveLevel\":" + terraFloat(reserveLevel) +
           ",\"lowLevel\":" + terraFloat(lowLevel) +
           ",\"highLevel\":" + terraFloat(highLevel) +
           ",\"temperatureC\":" + terraFloat(temperatureC) +
           ",\"temperatureSensorKey\":" + terraUnsigned(temperatureSensorKey) +
           ",\"minimumTargetC\":" + terraFloat(minimumTargetC) +
           ",\"maximumTargetC\":" + terraFloat(maximumTargetC) +
           ",\"absoluteMaximumC\":" + terraFloat(absoluteMaximumC) + "}";
}

bool TerraThermalStoreData::fromJSON(const TerraString &json)
{
    TerraResourceData base;
    long temperatureSensor = 0;
    if (!base.fromJSON(json)) return false;
    if (base.objectType != Terra_ObjectType_ThermalStore || base.resourceType != Terra_ResourceType_Thermal) return false;
    if (!terraJsonExtractFloat(json, "temperatureC", temperatureC)) return false;
    if (!terraJsonExtractLong(json, "temperatureSensorKey", temperatureSensor) || temperatureSensor < 0) return false;
    if (!terraJsonExtractFloat(json, "minimumTargetC", minimumTargetC)) return false;
    if (!terraJsonExtractFloat(json, "maximumTargetC", maximumTargetC)) return false;
    if (!terraJsonExtractFloat(json, "absoluteMaximumC", absoluteMaximumC)) return false;
    if (minimumTargetC > maximumTargetC || absoluteMaximumC < maximumTargetC) return false;
    copyObjectFields(base, *this);
    objectType = Terra_ObjectType_ThermalStore;
    resourceType = Terra_ResourceType_Thermal;
    level = base.level;
    reserveLevel = base.reserveLevel;
    lowLevel = base.lowLevel;
    highLevel = base.highLevel;
    temperatureSensorKey = (uint32_t)temperatureSensor;
    return true;
}

TerraThermalLoopData::TerraThermalLoopData()
    : TerraObjectData(), sourceTemperatureSensorKey(TERRA_INVALID_KEY),
      thermalStoreKey(TERRA_INVALID_KEY), circulatorKey(TERRA_INVALID_KEY),
      onDifferentialC(8.0f), offDifferentialC(3.0f), maxStoreTempC(80.0f)
{
    objectType = Terra_ObjectType_ThermalLoop;
}

TerraString TerraThermalLoopData::toJSON() const
{
    return TerraString("{") + objectFields(*this) +
           ",\"sourceTemperatureSensorKey\":" + terraUnsigned(sourceTemperatureSensorKey) +
           ",\"thermalStoreKey\":" + terraUnsigned(thermalStoreKey) +
           ",\"circulatorKey\":" + terraUnsigned(circulatorKey) +
           ",\"onDifferentialC\":" + terraFloat(onDifferentialC) +
           ",\"offDifferentialC\":" + terraFloat(offDifferentialC) +
           ",\"maxStoreTempC\":" + terraFloat(maxStoreTempC) + "}";
}

bool TerraThermalLoopData::fromJSON(const TerraString &json)
{
    long sourceTemperatureSensor = 0, thermalStore = 0, circulator = 0;
    if (!parseObjectFields(json, *this) || objectType != Terra_ObjectType_ThermalLoop) return false;
    if (!terraJsonExtractLong(json, "sourceTemperatureSensorKey", sourceTemperatureSensor) || sourceTemperatureSensor < 0) return false;
    if (!terraJsonExtractLong(json, "thermalStoreKey", thermalStore) || thermalStore < 0) return false;
    if (!terraJsonExtractLong(json, "circulatorKey", circulator) || circulator < 0) return false;
    if (!terraJsonExtractFloat(json, "onDifferentialC", onDifferentialC)) return false;
    if (!terraJsonExtractFloat(json, "offDifferentialC", offDifferentialC)) return false;
    if (!terraJsonExtractFloat(json, "maxStoreTempC", maxStoreTempC)) return false;
    sourceTemperatureSensorKey = (uint32_t)sourceTemperatureSensor;
    thermalStoreKey = (uint32_t)thermalStore;
    circulatorKey = (uint32_t)circulator;
    return offDifferentialC >= 0.0f && onDifferentialC > offDifferentialC;
}

TerraPowerRailData::TerraPowerRailData()
    : TerraObjectData(), railType(Terra_RailType_Custom), nominalVoltage(0.0f)
{
    objectType = Terra_ObjectType_PowerRail;
}

TerraString TerraPowerRailData::toJSON() const
{
    return TerraString("{") + objectFields(*this) +
           ",\"railType\":\"" + terraRailTypeToString(railType) +
           "\",\"nominalVoltage\":" + terraFloat(nominalVoltage) + "}";
}

bool TerraPowerRailData::fromJSON(const TerraString &json)
{
    TerraString railTypeStr;
    if (!parseObjectFields(json, *this) || objectType != Terra_ObjectType_PowerRail) return false;
    if (!terraJsonExtractString(json, "railType", railTypeStr)) return false;
    if (!terraJsonExtractFloat(json, "nominalVoltage", nominalVoltage) || nominalVoltage < 0.0f) return false;
    railType = terraRailTypeFromString(railTypeStr);
    return terraStringEqualsIgnoreCase(railTypeStr, terraRailTypeToString(railType));
}

TerraEnvironmentData::TerraEnvironmentData()
    : TerraObjectData(),
      airTemperatureSensorKey(TERRA_INVALID_KEY), humiditySensorKey(TERRA_INVALID_KEY),
      pressureSensorKey(TERRA_INVALID_KEY), rainfallSensorKey(TERRA_INVALID_KEY),
      rainRateSensorKey(TERRA_INVALID_KEY), windSpeedSensorKey(TERRA_INVALID_KEY),
      windDirectionSensorKey(TERRA_INVALID_KEY), solarRadiationSensorKey(TERRA_INVALID_KEY)
{
    objectType = Terra_ObjectType_Environment;
}

TerraString TerraEnvironmentData::toJSON() const
{
    return TerraString("{") + objectFields(*this) +
           ",\"airTemperatureSensorKey\":" + terraUnsigned(airTemperatureSensorKey) +
           ",\"humiditySensorKey\":" + terraUnsigned(humiditySensorKey) +
           ",\"pressureSensorKey\":" + terraUnsigned(pressureSensorKey) +
           ",\"rainfallSensorKey\":" + terraUnsigned(rainfallSensorKey) +
           ",\"rainRateSensorKey\":" + terraUnsigned(rainRateSensorKey) +
           ",\"windSpeedSensorKey\":" + terraUnsigned(windSpeedSensorKey) +
           ",\"windDirectionSensorKey\":" + terraUnsigned(windDirectionSensorKey) +
           ",\"solarRadiationSensorKey\":" + terraUnsigned(solarRadiationSensorKey) + "}";
}

bool TerraEnvironmentData::fromJSON(const TerraString &json)
{
    long airTemperatureSensor = 0, humiditySensor = 0, pressureSensor = 0, rainfallSensor = 0;
    long rainRateSensor = 0, windSpeedSensor = 0, windDirectionSensor = 0, solarRadiationSensor = 0;
    if (!parseObjectFields(json, *this) || objectType != Terra_ObjectType_Environment) return false;
    if (!terraJsonExtractLong(json, "airTemperatureSensorKey", airTemperatureSensor) || airTemperatureSensor < 0) return false;
    if (!terraJsonExtractLong(json, "humiditySensorKey", humiditySensor) || humiditySensor < 0) return false;
    if (!terraJsonExtractLong(json, "pressureSensorKey", pressureSensor) || pressureSensor < 0) return false;
    if (!terraJsonExtractLong(json, "rainfallSensorKey", rainfallSensor) || rainfallSensor < 0) return false;
    if (!terraJsonExtractLong(json, "rainRateSensorKey", rainRateSensor) || rainRateSensor < 0) return false;
    if (!terraJsonExtractLong(json, "windSpeedSensorKey", windSpeedSensor) || windSpeedSensor < 0) return false;
    if (!terraJsonExtractLong(json, "windDirectionSensorKey", windDirectionSensor) || windDirectionSensor < 0) return false;
    if (!terraJsonExtractLong(json, "solarRadiationSensorKey", solarRadiationSensor) || solarRadiationSensor < 0) return false;
    airTemperatureSensorKey = (uint32_t)airTemperatureSensor;
    humiditySensorKey = (uint32_t)humiditySensor;
    pressureSensorKey = (uint32_t)pressureSensor;
    rainfallSensorKey = (uint32_t)rainfallSensor;
    rainRateSensorKey = (uint32_t)rainRateSensor;
    windSpeedSensorKey = (uint32_t)windSpeedSensor;
    windDirectionSensorKey = (uint32_t)windDirectionSensor;
    solarRadiationSensorKey = (uint32_t)solarRadiationSensor;
    return true;
}
