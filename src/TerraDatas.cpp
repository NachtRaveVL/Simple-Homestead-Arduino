/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Object Data
*/

#include "Terraduino.h"

TerraData *_allocateDataFromBaseDecode(const TerraData &baseDecode)
{
    TerraData *retVal = nullptr;

    if (baseDecode.isStandardData()) {
        if (baseDecode.isSystemData()) {
            retVal = new TerraSystemData();
        } else if (baseDecode.isCalibrationData()) {
            retVal = new TerraCalibrationData();
        } else if (baseDecode.isUIData()) {
            retVal = new TerraUIData();
        }
    } else if (baseDecode.isObjectData()) {
        retVal = _allocateDataForObjType(baseDecode.id.object.idType, baseDecode.id.object.classType);
    }

    TERRA_SOFT_ASSERT(retVal, F("Unknown data decode"));
    if (retVal) {
        retVal->id = baseDecode.id;
        TERRA_SOFT_ASSERT(retVal->_version >= baseDecode._version, F("Data version mismatch"));
        retVal->_revision = baseDecode._revision;
        return retVal;
    }
    return new TerraData(baseDecode);
}

TerraData *_allocateDataForObjType(int8_t idType, int8_t classType)
{
    switch (idType) {
        case (tid_t)Terra_ObjectType_Actuator:
            switch (classType) {
                case (tid_t)TerraActuator::Relay:
                    return new TerraActuatorData();
                case (tid_t)TerraActuator::RelayPump:
                    return new TerraActuatorData();
                case (tid_t)TerraActuator::Variable:
                    return new TerraActuatorData();
                default: break;
            }
            break;

        case (tid_t)Terra_ObjectType_Sensor:
            switch (classType) {
                case (tid_t)TerraSensor::Value:
                    return new TerraSensorData();
                case (tid_t)TerraSensor::Binary:
                    return new TerraSensorData();
                case (tid_t)TerraSensor::Analog:
                    return new TerraSensorData();
                case (tid_t)TerraSensor::Remote:
                    return new TerraSensorData();
                default: break;
            }
            break;

        case (tid_t)Terra_ObjectType_Reservoir:
            switch (classType) {
                case (tid_t)TerraReservoir::Base:
                    return new TerraReservoirData();
                case (tid_t)TerraReservoir::WaterStorage:
                    return new TerraWaterStorageData();
                case (tid_t)TerraReservoir::Cistern:
                    return new TerraCisternData();
                case (tid_t)TerraReservoir::WaterSource:
                    return new TerraWaterSourceData();
                case (tid_t)TerraReservoir::ThermalStore:
                    return new TerraThermalStoreData();
                default: break;
            }
            break;

        case (tid_t)Terra_ObjectType_WaterRoute:
            switch (classType) {
                case (tid_t)TerraWaterRoute::Route:
                    return new TerraWaterRouteData();
                default: break;
            }
            break;

        case (tid_t)Terra_ObjectType_RainCatchment:
            switch (classType) {
                case (tid_t)TerraRainCatchment::Catchment:
                    return new TerraRainCatchmentData();
                default: break;
            }
            break;

        case (tid_t)Terra_ObjectType_ThermalLoop:
            switch (classType) {
                case (tid_t)TerraThermalLoop::Loop:
                    return new TerraThermalLoopData();
                default: break;
            }
            break;

        case (tid_t)Terra_ObjectType_Environment:
            switch (classType) {
                case (tid_t)TerraEnvironment::Standard:
                    return new TerraEnvironmentData();
                default: break;
            }
            break;

        case (tid_t)Terra_ObjectType_Rail:
            switch (classType) {
                case (tid_t)TerraRail::Simple:
                    return new TerraSimpleRailData();
                case (tid_t)TerraRail::Regulated:
                    return new TerraRegulatedRailData();
                default: break;
            }
            break;

        default: break;
    }

    return nullptr;
}

TerraSystemData::TerraSystemData()
    : TerraData('T','S','Y','S', 1),
      systemMode(Terra_SystemMode_Undefined), measureMode(Terra_MeasurementMode_Undefined),
      dispOutMode(Terra_DisplayOutputMode_Undefined), ctrlInMode(Terra_ControlInputMode_Undefined),
      systemName{0}, timeZoneOffset(0), pollingInterval(TERRA_DATA_LOOP_INTERVAL),
      autosaveEnabled(Terra_Autosave_Disabled), autosaveFallback(Terra_Autosave_Disabled), autosaveInterval(TERRA_SYS_AUTOSAVE_INTERVAL),
      wifiSSID{0}, wifiPassword{0}, wifiPasswordSeed(0),
      macAddress{0},
      latitude(DBL_UNDEF), longitude(DBL_UNDEF), altitude(DBL_UNDEF)
{
    _size = sizeof(*this);
    TERRA_HARD_ASSERT(isSystemData(), SFP(TStr_Err_OperationFailure));
    strncpy(systemName, SFP(TStr_Default_SystemName).c_str(), TERRA_NAME_MAXSIZE);
}

void TerraSystemData::toJSONObject(JsonObject &objectOut) const
{
    TerraData::toJSONObject(objectOut);

    objectOut[SFP(TStr_Key_SystemMode)] = systemModeToString(systemMode);
    objectOut[SFP(TStr_Key_MeasureMode)] = measurementModeToString(measureMode);
    #ifdef TERRA_USE_GUI
        objectOut[SFP(TStr_Key_DispOutMode)] = displayOutputModeToString(dispOutMode);
        objectOut[SFP(TStr_Key_CtrlInMode)] = controlInputModeToString(ctrlInMode);
    #else
        objectOut[SFP(TStr_Key_DispOutMode)] = displayOutputModeToString(Terra_DisplayOutputMode_Disabled);
        objectOut[SFP(TStr_Key_CtrlInMode)] = controlInputModeToString(Terra_ControlInputMode_Disabled);
    #endif
    if (systemName[0]) { objectOut[SFP(TStr_Key_SystemName)] = charsToString(systemName, TERRA_NAME_MAXSIZE); }
    if (timeZoneOffset != 0) { objectOut[SFP(TStr_Key_TimeZoneOffset)] = timeZoneOffset; }
    if (pollingInterval && pollingInterval != TERRA_DATA_LOOP_INTERVAL) { objectOut[SFP(TStr_Key_PollingInterval)] = pollingInterval; }
    if (autosaveEnabled != Terra_Autosave_Disabled) { objectOut[SFP(TStr_Key_AutosaveEnabled)] = autosaveEnabled; }
    if (autosaveFallback != Terra_Autosave_Disabled) { objectOut[SFP(TStr_Key_AutosaveFallback)] = autosaveFallback; }
    if (autosaveInterval && autosaveInterval != TERRA_SYS_AUTOSAVE_INTERVAL) { objectOut[SFP(TStr_Key_AutosaveInterval)] = autosaveInterval; }
    if (wifiSSID[0]) { objectOut[SFP(TStr_Key_WiFiSSID)] = charsToString(wifiSSID, TERRA_NAME_MAXSIZE); }
    if (wifiPasswordSeed) {
        objectOut[SFP(TStr_Key_WiFiPassword)] = hexStringFromBytes(wifiPassword, TERRA_NAME_MAXSIZE);
        objectOut[SFP(TStr_Key_WiFiPasswordSeed)] = wifiPasswordSeed;
    } else if (wifiPassword[0]) {
        objectOut[SFP(TStr_Key_WiFiPassword)] = charsToString((const char *)wifiPassword, TERRA_NAME_MAXSIZE);
    }
    if (!arrayElementsEqual<uint8_t>(macAddress, 6, 0)) {
        objectOut[SFP(TStr_Key_MACAddress)] = commaStringFromArray(macAddress, 6);
    }
    if (latitude != DBL_UNDEF && longitude != DBL_UNDEF) {
        if (altitude != DBL_UNDEF) {
            double loc[3] = {latitude,longitude,altitude};
            objectOut[SFP(TStr_Key_Location)] = commaStringFromArray(loc, 3);
        } else {
            double loc[2] = {latitude,longitude};
            objectOut[SFP(TStr_Key_Location)] = commaStringFromArray(loc, 2);
        }
    }

    JsonObject schedulerObj = objectOut.createNestedObject(SFP(TStr_Key_Scheduler));
    scheduler.toJSONObject(schedulerObj); if (!schedulerObj.size()) { objectOut.remove(SFP(TStr_Key_Scheduler)); }
    JsonObject loggerObj = objectOut.createNestedObject(SFP(TStr_Key_Logger));
    logger.toJSONObject(loggerObj); if (!loggerObj.size()) { objectOut.remove(SFP(TStr_Key_Logger)); }
    JsonObject publisherObj = objectOut.createNestedObject(SFP(TStr_Key_Publisher));
    publisher.toJSONObject(publisherObj); if (!publisherObj.size()) { objectOut.remove(SFP(TStr_Key_Publisher)); }
}

void TerraSystemData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraData::fromJSONObject(objectIn);

    systemMode = systemModeFromString(objectIn[SFP(TStr_Key_SystemMode)]);
    measureMode = measurementModeFromString(objectIn[SFP(TStr_Key_MeasureMode)]);
    #ifdef TERRA_USE_GUI
        dispOutMode = displayOutputModeFromString(objectIn[SFP(TStr_Key_DispOutMode)]);
        ctrlInMode = controlInputModeFromString(objectIn[SFP(TStr_Key_CtrlInMode)]);
    #else
        dispOutMode = Terra_DisplayOutputMode_Disabled;
        ctrlInMode = Terra_ControlInputMode_Disabled;
    #endif
    const char *systemNameStr = objectIn[SFP(TStr_Key_SystemName)];
    if (systemNameStr && systemNameStr[0]) { strncpy(systemName, systemNameStr, TERRA_NAME_MAXSIZE); }
    timeZoneOffset = objectIn[SFP(TStr_Key_TimeZoneOffset)] | timeZoneOffset;
    pollingInterval = objectIn[SFP(TStr_Key_PollingInterval)] | pollingInterval;
    autosaveEnabled = objectIn[SFP(TStr_Key_AutosaveEnabled)] | autosaveEnabled;
    autosaveFallback = objectIn[SFP(TStr_Key_AutosaveFallback)] | autosaveFallback;
    autosaveInterval = objectIn[SFP(TStr_Key_AutosaveInterval)] | autosaveInterval;
    const char *wifiSSIDStr = objectIn[SFP(TStr_Key_WiFiSSID)];
    if (wifiSSIDStr && wifiSSIDStr[0]) { strncpy(wifiSSID, wifiSSIDStr, TERRA_NAME_MAXSIZE); }
    const char *wifiPasswordStr = objectIn[SFP(TStr_Key_WiFiPassword)];
    wifiPasswordSeed = objectIn[SFP(TStr_Key_WiFiPasswordSeed)] | wifiPasswordSeed;
    if (wifiPasswordStr && wifiPasswordSeed) { hexStringToBytes(String(wifiPasswordStr), wifiPassword, TERRA_NAME_MAXSIZE); }
    else if (wifiPasswordStr && wifiPasswordStr[0]) { strncpy((char *)wifiPassword, wifiPasswordStr, TERRA_NAME_MAXSIZE); wifiPasswordSeed = 0; }
    JsonVariantConst macAddressVar = objectIn[SFP(TStr_Key_MACAddress)];
    commaStringToArray(macAddressVar, macAddress, 6);
    JsonVariantConst locationVar = objectIn[SFP(TStr_Key_Location)];
    if (!locationVar.isNull()) {
        auto commaCount = occurrencesInString(locationVar.as<String>(), String(','));
        if (commaCount == 2) {
            double loc[3]; commaStringToArray(locationVar, loc, 3);
            latitude = loc[0]; longitude = loc[1]; altitude = loc[2];
        } else if (commaCount == 1) {
            double loc[2]; commaStringToArray(locationVar, loc, 2);
            latitude = loc[0]; longitude = loc[1];
        }
    }

    JsonObjectConst schedulerObj = objectIn[SFP(TStr_Key_Scheduler)];
    if (!schedulerObj.isNull()) { scheduler.fromJSONObject(schedulerObj); }
    JsonObjectConst loggerObj = objectIn[SFP(TStr_Key_Logger)];
    if (!loggerObj.isNull()) { logger.fromJSONObject(loggerObj); }
    JsonObjectConst publisherObj = objectIn[SFP(TStr_Key_Publisher)];
    if (!publisherObj.isNull()) { publisher.fromJSONObject(publisherObj); }
}


TerraCalibrationData::TerraCalibrationData()
    : TerraData('T','C','A','L', 1),
      ownerName{0}, calibrationUnits(Terra_UnitsType_Undefined),
      multiplier(1.0f), offset(0.0f)
{
    _size = sizeof(*this);
    TERRA_HARD_ASSERT(isCalibrationData(), SFP(TStr_Err_OperationFailure));
}

TerraCalibrationData::TerraCalibrationData(TerraIdentity ownerId, Terra_UnitsType calibrationUnitsIn)
    : TerraData('T','C','A','L', 1),
      ownerName{0}, calibrationUnits(calibrationUnitsIn),
      multiplier(1.0f), offset(0.0f)
{
    _size = sizeof(*this);
    TERRA_HARD_ASSERT(isCalibrationData(), SFP(TStr_Err_OperationFailure));
    if (ownerId) {
        strncpy(ownerName, ownerId.keyString.c_str(), TERRA_NAME_MAXSIZE);
    }
}

void TerraCalibrationData::toJSONObject(JsonObject &objectOut) const
{
    TerraData::toJSONObject(objectOut);

    if (ownerName[0]) { objectOut[SFP(TStr_Key_SensorName)] = charsToString(ownerName, TERRA_NAME_MAXSIZE); }
    if (calibrationUnits != Terra_UnitsType_Undefined) { objectOut[SFP(TStr_Key_CalibrationUnits)] = unitsTypeToSymbol(calibrationUnits); }
    objectOut[SFP(TStr_Key_Multiplier)] = multiplier;
    objectOut[SFP(TStr_Key_Offset)] = offset;
}

void TerraCalibrationData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraData::fromJSONObject(objectIn);

    const char *ownerNameStr = objectIn[SFP(TStr_Key_SensorName)];
    if (ownerNameStr && ownerNameStr[0]) { strncpy(ownerName, ownerNameStr, TERRA_NAME_MAXSIZE); }
    calibrationUnits = unitsTypeFromSymbol(objectIn[SFP(TStr_Key_CalibrationUnits)]);
    multiplier = objectIn[SFP(TStr_Key_Multiplier)] | multiplier;
    offset = objectIn[SFP(TStr_Key_Offset)] | offset;
}

void TerraCalibrationData::setFromTwoPoints(float point1MeasuredAt, float point1CalibratedTo,
                                            float point2MeasuredAt, float point2CalibratedTo)
{
    float aTerm = point2CalibratedTo - point1CalibratedTo;
    float bTerm = point2MeasuredAt - point1MeasuredAt;
    TERRA_SOFT_ASSERT(!isFPEqual(bTerm, 0.0f), SFP(TStr_Err_InvalidParameter));
    if (!isFPEqual(bTerm, 0.0f)) {
        multiplier = aTerm / bTerm;
        offset = ((aTerm * point2MeasuredAt) + (bTerm * point1CalibratedTo)) / bTerm;
        bumpRevisionIfNeeded();
    }
}
