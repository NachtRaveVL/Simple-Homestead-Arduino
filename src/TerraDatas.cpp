/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Object Data
*/

#include "Terraduino.h"
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
    TerraString result = TerraString("\"key\":") + terraUnsigned(data.key) +
           ",\"objectType\":\"" + terraObjectTypeToString(data.objectType) +
           "\",\"name\":\"" + terraJsonEscape(data.name) +
           "\",\"enabled\":" + terraBool(data.enabled) +
           ",\"attachmentCount\":" + terraNumber(data.attachmentCount);
    for (uint8_t i = 0; i < data.attachmentCount && i < TERRA_MAX_ATTACHMENTS; ++i) {
        TerraString prefix = TerraString("attachment") + terraNumber(i);
        result += TerraString(",\"") + prefix + "Key\":" + terraUnsigned(data.attachments[i].objectKey);
        result += TerraString(",\"") + prefix + "Role\":\"" + terraAttachmentRoleToString(data.attachments[i].role) + "\"";
    }
    return result;
}

static bool parseObjectFields(const TerraString &json, TerraObjectData &data)
{
    long key = 0, attachmentCount = 0;
    TerraString objectType;
    if (!terraJsonExtractLong(json, "key", key) || key < 0) return false;
    if (!terraJsonExtractString(json, "objectType", objectType)) return false;
    if (!terraJsonExtractString(json, "name", data.name)) return false;
    if (!terraJsonExtractBool(json, "enabled", data.enabled)) return false;
    if (!terraJsonExtractLong(json, "attachmentCount", attachmentCount) ||
        attachmentCount < 0 || attachmentCount > TERRA_MAX_ATTACHMENTS) return false;
    data.key = (uint32_t)key;
    data.objectType = terraObjectTypeFromString(objectType);
    if (!terraStringEqualsIgnoreCase(objectType, terraObjectTypeToString(data.objectType))) return false;
    data.attachmentCount = (uint8_t)attachmentCount;
    for (uint8_t i = 0; i < data.attachmentCount; ++i) {
        TerraString prefix = TerraString("attachment") + terraNumber(i);
        TerraString keyName = prefix + "Key";
        TerraString roleName = prefix + "Role";
        TerraString roleString;
        long objectKey = 0;
        if (!terraJsonExtractLong(json, keyName.c_str(), objectKey) || objectKey <= 0) return false;
        if (!terraJsonExtractString(json, roleName.c_str(), roleString)) return false;
        data.attachments[i].objectKey = (uint32_t)objectKey;
        data.attachments[i].role = terraAttachmentRoleFromString(roleString);
        if (!terraStringEqualsIgnoreCase(roleString, terraAttachmentRoleToString(data.attachments[i].role)) ||
            data.attachments[i].role == Terra_AttachmentRole_Undefined) return false;
    }
    for (uint8_t i = data.attachmentCount; i < TERRA_MAX_ATTACHMENTS; ++i) data.attachments[i] = TerraAttachmentData();
    return true;
}

static void copyObjectFields(const TerraObjectData &from, TerraObjectData &to)
{
    to.key = from.key;
    to.name = from.name;
    to.enabled = from.enabled;
    to.attachmentCount = from.attachmentCount;
    for (uint8_t i = 0; i < TERRA_MAX_ATTACHMENTS; ++i) to.attachments[i] = from.attachments[i];
}

TerraObjectData::TerraObjectData()
    : key(TERRA_INVALID_KEY), objectType(Terra_ObjectType_Undefined), name(), enabled(true), attachmentCount(0)
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
    if (sensorCalibrated && isFPEqual(sensorRawMinimum, sensorRawMaximum)) return false;
    updateIntervalMs = (uint32_t)updateInterval;
    staleAfterMs = (uint32_t)staleAfter;
    return true;
}

TerraActuatorData::TerraActuatorData()
    : TerraObjectData(), actuatorType(Terra_ActuatorType_Undefined),
      enableMode(Terra_EnableMode_Highest), maxContinuousMs(0), hasPinDriver(false),
      pinSetup(TERRA_INVALID_PIN, Terra_PinMode_Undefined, false), maximumRaw(255),
      sumpStartPercent(TERRA_SUMP_START_LEVEL_PERCENT), sumpStopPercent(TERRA_SUMP_STOP_LEVEL_PERCENT),
      sumpAlarmPercent(TERRA_SUMP_ALARM_LEVEL_PERCENT)
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
        json += ",\"sumpStartPercent\":" + terraNumber(sumpStartPercent) +
                ",\"sumpStopPercent\":" + terraNumber(sumpStopPercent) +
                ",\"sumpAlarmPercent\":" + terraNumber(sumpAlarmPercent);
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
        if (!terraJsonExtractFloat(json, "sumpStartPercent", sumpStartPercent) ||
            !terraJsonExtractFloat(json, "sumpStopPercent", sumpStopPercent) ||
            !terraJsonExtractFloat(json, "sumpAlarmPercent", sumpAlarmPercent) ||
            sumpStopPercent < 0.0f || sumpStopPercent >= sumpStartPercent ||
            sumpStartPercent >= sumpAlarmPercent || sumpAlarmPercent > 100.0f) return false;
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
      fillStartPercent(35.0f), fillStopPercent(90.0f), overflowPercent(99.0f)
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
           ",\"overflowPercent\":" + terraFloat(overflowPercent) + "}";
}

bool TerraWaterStorageData::fromJSON(const TerraString &json)
{
    TerraResourceData base;
    TerraString type;
    if (!base.fromJSON(json)) return false;
    if (base.objectType != Terra_ObjectType_WaterStorage || base.resourceType != Terra_ResourceType_Water) return false;
    if (!terraJsonExtractString(json, "storageType", type)) return false;
    if (!terraJsonExtractFloat(json, "capacityLiters", capacityLiters) || capacityLiters < 0.0f) return false;
    if (!terraJsonExtractFloat(json, "fillStartPercent", fillStartPercent)) return false;
    if (!terraJsonExtractFloat(json, "fillStopPercent", fillStopPercent)) return false;
    if (!terraJsonExtractFloat(json, "overflowPercent", overflowPercent)) return false;
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
      level(100.0f), reserveLevel(0.0f), maximumFlowLpm(0.0f)
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
           ",\"maximumFlowLpm\":" + terraFloat(maximumFlowLpm) + "}";
}

bool TerraWaterSourceData::fromJSON(const TerraString &json)
{
    TerraString type;
    long priorityValue = 0;
    if (!parseObjectFields(json, *this)) return false;
    if (!terraJsonExtractString(json, "sourceType", type)) return false;
    if (!terraJsonExtractLong(json, "priority", priorityValue) || priorityValue < 0 || priorityValue > 255) return false;
    if (!terraJsonExtractBool(json, "available", available)) return false;
    if (!terraJsonExtractFloat(json, "level", level) || level < 0.0f || level > 100.0f) return false;
    if (!terraJsonExtractFloat(json, "reserveLevel", reserveLevel) || reserveLevel < 0.0f || reserveLevel > 100.0f) return false;
    if (!terraJsonExtractFloat(json, "maximumFlowLpm", maximumFlowLpm) || maximumFlowLpm < 0.0f) return false;
    if (objectType != Terra_ObjectType_WaterSource) return false;
    sourceType = terraWaterSourceTypeFromString(type);
    if (!terraStringEqualsIgnoreCase(type, terraWaterSourceTypeToString(sourceType))) return false;
    priority = (uint8_t)priorityValue;
    return true;
}

TerraWaterRouteData::TerraWaterRouteData()
    : TerraObjectData(), sourceKey(TERRA_INVALID_KEY), destinationKey(TERRA_INVALID_KEY),
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
           ",\"destinationStartPercent\":" + terraFloat(destinationStartPercent) +
           ",\"destinationStopPercent\":" + terraFloat(destinationStopPercent) +
           ",\"minimumFlowLpm\":" + terraFloat(minimumFlowLpm) +
           ",\"maximumFlowLpm\":" + terraFloat(maximumFlowLpm) +
           ",\"routeState\":\"" + terraRouteStateToString(routeState) + "\"}";
}

bool TerraWaterRouteData::fromJSON(const TerraString &json)
{
    long source = 0, destination = 0;
    TerraString state;
    if (!parseObjectFields(json, *this)) return false;
    if (!terraJsonExtractLong(json, "sourceKey", source) || source < 0) return false;
    if (!terraJsonExtractLong(json, "destinationKey", destination) || destination < 0) return false;
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
    : TerraResourceData(), temperatureC(0.0f), minimumTargetC(0.0f),
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
           ",\"minimumTargetC\":" + terraFloat(minimumTargetC) +
           ",\"maximumTargetC\":" + terraFloat(maximumTargetC) +
           ",\"absoluteMaximumC\":" + terraFloat(absoluteMaximumC) + "}";
}

bool TerraThermalStoreData::fromJSON(const TerraString &json)
{
    TerraResourceData base;
    if (!base.fromJSON(json)) return false;
    if (base.objectType != Terra_ObjectType_ThermalStore || base.resourceType != Terra_ResourceType_Thermal) return false;
    if (!terraJsonExtractFloat(json, "temperatureC", temperatureC)) return false;
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
    return true;
}

TerraThermalLoopData::TerraThermalLoopData()
    : TerraObjectData(), onDifferentialC(8.0f), offDifferentialC(3.0f), maxStoreTempC(80.0f)
{
    objectType = Terra_ObjectType_ThermalLoop;
}

TerraString TerraThermalLoopData::toJSON() const
{
    return TerraString("{") + objectFields(*this) +
           ",\"onDifferentialC\":" + terraFloat(onDifferentialC) +
           ",\"offDifferentialC\":" + terraFloat(offDifferentialC) +
           ",\"maxStoreTempC\":" + terraFloat(maxStoreTempC) + "}";
}

bool TerraThermalLoopData::fromJSON(const TerraString &json)
{
    if (!parseObjectFields(json, *this) || objectType != Terra_ObjectType_ThermalLoop) return false;
    if (!terraJsonExtractFloat(json, "onDifferentialC", onDifferentialC)) return false;
    if (!terraJsonExtractFloat(json, "offDifferentialC", offDifferentialC)) return false;
    if (!terraJsonExtractFloat(json, "maxStoreTempC", maxStoreTempC)) return false;
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
    : TerraObjectData()
{
    objectType = Terra_ObjectType_Environment;
}

TerraString TerraEnvironmentData::toJSON() const
{
    return TerraString("{") + objectFields(*this) + "}";
}

bool TerraEnvironmentData::fromJSON(const TerraString &json)
{
    return parseObjectFields(json, *this) && objectType == Terra_ObjectType_Environment;
}
