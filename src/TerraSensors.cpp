/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Sensors
*/

#include "Terraduino.h"

TerraSensor::TerraSensor(Terra_SensorType sensorType, tposi_t sensorIndex, Terra_UnitsType units, int classTypeIn)
    : TerraObject(TerraIdentity(sensorType, sensorIndex)), TerraMeasurementUnitsInterfaceStorageSingle(units),
      classType(static_cast<decltype(Value)>(classTypeIn)),
      _lastMeasurement(0.0f, units), _calibrationData(nullptr)
{ ; }

TerraSensor::TerraSensor(const TerraSensorData *dataIn)
    : TerraObject((const TerraObjectData *)dataIn),
      TerraMeasurementUnitsInterfaceStorageSingle(dataIn ? dataIn->measurementUnits : Terra_UnitsType_Undefined),
      classType(static_cast<decltype(Value)>(dataIn ? (int)dataIn->id.object.classType : (int)Unknown)),
      _lastMeasurement(0.0f, dataIn ? dataIn->measurementUnits : Terra_UnitsType_Undefined),
      _calibrationData(nullptr)
{ ; }

bool TerraSensor::takeMeasurement(bool force)
{
    if (!force && !needsPolling()) { return _lastMeasurement.isSet(); }
    return _lastMeasurement.isSet();
}

const TerraMeasurement *TerraSensor::getMeasurement(bool poll)
{
    if (poll) { takeMeasurement(); }
    return &_lastMeasurement;
}

bool TerraSensor::needsPolling(tframe_t allowance) const
{
    return getController() ? getController()->isPollingFrameOld(_lastMeasurement.frame, allowance) : !_lastMeasurement.isSet();
}

void TerraSensor::update(uint32_t now)
{
    if (needsPolling()) { takeMeasurement(); }
    (void)now;
}

void TerraSensor::setMeasurement(float value, Terra_UnitsType units, uint32_t timestamp, bool valid)
{
    _lastMeasurement = TerraSingleMeasurement(value, units, timestamp,
                                              valid && getController() ? getController()->getPollingFrame() :
                                              valid ? (tframe_t)1 : tframe_none);
    _measurementSignal.fire(&_lastMeasurement);
    if (getPublisher()) { getPublisher()->publishData(getKey(), _lastMeasurement); }
}

bool TerraSensor::isStale(uint32_t now, uint32_t staleAfterMs) const
{
    return staleAfterMs && (!_lastMeasurement.isSet() ||
           (uint32_t)(now - _lastMeasurement.timestamp) >= staleAfterMs);
}

void TerraSensor::setUserCalibrationData(TerraCalibrationData *userCalibrationData)
{
    if (_calibrationData && _calibrationData != userCalibrationData) { bumpRevisionIfNeeded(); }
    if (getController()) {
        if (userCalibrationData && getController()->setUserCalibrationData(userCalibrationData)) {
            _calibrationData = getController()->getUserCalibrationData(getKey());
        } else if (!userCalibrationData && _calibrationData && getController()->dropUserCalibrationData(_calibrationData)) {
            _calibrationData = nullptr;
        }
    } else {
        _calibrationData = userCalibrationData;
    }
}

TerraBinarySensor::TerraBinarySensor(Terra_SensorType sensorType, tposi_t sensorIndex,
                                     TerraDigitalPin inputPin)
    : TerraSensor(sensorType, sensorIndex, Terra_UnitsType_Raw_1, Binary), _inputPin(inputPin)
{
    _inputPin.init();
}

TerraBinarySensor::TerraBinarySensor(const TerraSensorData *dataIn)
    : TerraSensor(dataIn), _inputPin(&dataIn->inputPin)
{
    _inputPin.init();
}

bool TerraBinarySensor::takeMeasurement(bool force)
{
    if (!force && !needsPolling()) { return _lastMeasurement.isSet(); }
    bool active = _inputPin.isActive();
    uint32_t now = millis();
    _lastMeasurement = TerraSingleMeasurement(active ? 1.0f : 0.0f, getMeasurementUnits(), now,
                                              getController() ? getController()->getPollingFrame() : (tframe_t)1);
    _measurementSignal.fire(&_lastMeasurement);
    if (getPublisher()) { getPublisher()->publishData(getKey(), _lastMeasurement); }
    return true;
}

TerraAnalogSensor::TerraAnalogSensor(Terra_SensorType sensorType, tposi_t sensorIndex,
                                     TerraAnalogPin inputPin, Terra_UnitsType units)
    : TerraSensor(sensorType, sensorIndex, units, Analog), _inputPin(inputPin)
{
    _inputPin.init();
}

TerraAnalogSensor::TerraAnalogSensor(const TerraSensorData *dataIn)
    : TerraSensor(dataIn), _inputPin(&dataIn->inputPin)
{
    _inputPin.init();
}

bool TerraAnalogSensor::takeMeasurement(bool force)
{
    if (!force && !needsPolling()) { return _lastMeasurement.isSet(); }
    uint32_t now = millis();
    TerraSingleMeasurement measurement(_inputPin.analogRead(), Terra_UnitsType_Raw_1, now,
                                       getController() ? getController()->getPollingFrame() : (tframe_t)1);
    calibrationTransform(&measurement);
    if (measurement.units != getMeasurementUnits() && canConvertUnits(measurement.units, getMeasurementUnits())) {
        measurement.toUnits(getMeasurementUnits());
    }
    _lastMeasurement = measurement;
    _measurementSignal.fire(&_lastMeasurement);
    if (getPublisher()) { getPublisher()->publishData(getKey(), _lastMeasurement); }
    return true;
}

TerraRemoteSensor::TerraRemoteSensor(Terra_SensorType reportedType, tposi_t sensorIndex,
                                     Terra_UnitsType units)
    : TerraSensor(Terra_SensorType_Remote, sensorIndex, units, Remote), _reportedType(reportedType),
      _staleAfterMs(TERRA_DEFAULT_REMOTE_STALE_MS), _lastReportAt(0), _hasReport(false)
{ ; }

void TerraRemoteSensor::receiveReport(float value, Terra_UnitsType units, uint32_t reportTime, bool valid)
{
    _hasReport = true;
    _lastReportAt = reportTime;
    setMeasurement(value, units, reportTime, valid);
}

bool TerraRemoteSensor::isOnline(uint32_t now) const
{
    return _hasReport && (!_staleAfterMs || (uint32_t)(now - _lastReportAt) < _staleAfterMs);
}

void TerraRemoteSensor::update(uint32_t now)
{
    if (!isOnline(now)) { _lastMeasurement.frame = tframe_none; }
}

Signal<const TerraMeasurement *, TERRA_SENSOR_SIGNAL_SLOTS> &TerraSensor::getMeasurementSignal()
{
    return _measurementSignal;
}

TerraData *TerraSensor::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void TerraSensor::saveToData(TerraData *dataOut) const
{
    TerraObject::saveToData(dataOut);
    dataOut->id.object.classType = (tid_t)classType;
    static_cast<TerraSensorData *>(dataOut)->measurementUnits = getMeasurementUnits();
}

void TerraBinarySensor::saveToData(TerraData *dataOut) const
{
    TerraSensor::saveToData(dataOut);
    _inputPin.saveToData(&static_cast<TerraSensorData *>(dataOut)->inputPin);
}

void TerraAnalogSensor::saveToData(TerraData *dataOut) const
{
    TerraSensor::saveToData(dataOut);
    _inputPin.saveToData(&static_cast<TerraSensorData *>(dataOut)->inputPin);
}

TerraRemoteSensor::TerraRemoteSensor(const TerraSensorData *dataIn)
    : TerraSensor(dataIn),
      _reportedType(dataIn ? dataIn->reportedType : Terra_SensorType_Undefined),
      _staleAfterMs(dataIn ? dataIn->staleAfterMs : TERRA_DEFAULT_REMOTE_STALE_MS),
      _lastReportAt(0), _hasReport(false)
{ ; }

void TerraRemoteSensor::saveToData(TerraData *dataOut) const
{
    TerraSensor::saveToData(dataOut);
    auto data = static_cast<TerraSensorData *>(dataOut);
    data->reportedType = _reportedType;
    data->staleAfterMs = _staleAfterMs;
}

TerraSensor *newSensorObjectFromData(const TerraSensorData *dataIn)
{
    if (!dataIn) { return nullptr; }

    switch (dataIn->id.object.classType) {
        case (tid_t)TerraSensor::Value:
            return new TerraSensor(dataIn);
        case (tid_t)TerraSensor::Binary:
            return new TerraBinarySensor(dataIn);
        case (tid_t)TerraSensor::Analog:
            return new TerraAnalogSensor(dataIn);
        case (tid_t)TerraSensor::Remote:
            return new TerraRemoteSensor(dataIn);
        default:
            return nullptr;
    }
}

TerraSensorData::TerraSensorData()
    : TerraObjectData(), measurementUnits(Terra_UnitsType_Undefined), reportedType(Terra_SensorType_Undefined),
      staleAfterMs(TERRA_DEFAULT_REMOTE_STALE_MS), inputPin()
{
    _size = sizeof(*this);
}

void TerraSensorData::toJSONObject(JsonObject &objectOut) const
{
    TerraObjectData::toJSONObject(objectOut);
    objectOut["measurementUnits"] = (int)measurementUnits;
    if (reportedType != Terra_SensorType_Undefined) { objectOut["reportedType"] = (int)reportedType; }
    if (inputPin.isSet()) {
        JsonObject pinObj = objectOut.createNestedObject("inputPin");
        inputPin.toJSONObject(pinObj);
    }
    if (staleAfterMs) { objectOut["staleAfterMs"] = staleAfterMs; }
}

void TerraSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraObjectData::fromJSONObject(objectIn);
    measurementUnits = (Terra_UnitsType)(objectIn["measurementUnits"] | (int)measurementUnits);
    reportedType = (Terra_SensorType)(objectIn["reportedType"] | (int)reportedType);
    JsonObjectConst pinObj = objectIn["inputPin"].as<JsonObjectConst>();
    if (!pinObj.isNull()) { inputPin.fromJSONObject(pinObj); }
    staleAfterMs = objectIn["staleAfterMs"] | staleAfterMs;
}
