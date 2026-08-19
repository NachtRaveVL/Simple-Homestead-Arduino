/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Sensors
*/

#include "TerraSensors.h"
#include "TerraUtils.h"

TerraSensor::TerraSensor(Terra_SensorType sensorType, Terra_Unit unit, uint32_t key, const TerraString &name)
    : TerraObject(Terra_ObjectType_Sensor, key, name), _sensorType(sensorType),
      _measurement(0.0f, unit, 0, false), _driver(nullptr), _ownsDriver(false),
      _readCallback(nullptr), _readContext(nullptr), _updateIntervalMs(1000), _lastReadAt(0) { }

TerraSensor::~TerraSensor() {
    if (_ownsDriver) delete _driver;
}

void TerraSensor::setDriver(TerraInputDriver *driver, bool takeOwnership) {
    if (_driver != driver && _ownsDriver) delete _driver;
    _driver = driver;
    _ownsDriver = driver && takeOwnership;
    if (_driver) _driver->begin();
}

void TerraSensor::setReadCallback(TerraReadCallback callback, void *context) {
    _readCallback = callback;
    _readContext = context;
}

void TerraSensor::setMeasurement(float value, Terra_Unit unit, uint32_t timestamp, bool valid) {
    _measurement = TerraMeasurement(value, unit, timestamp, valid);
    _lastReadAt = timestamp;
}

bool TerraSensor::isStale(uint32_t now, uint32_t staleAfterMs) const {
    uint32_t threshold = staleAfterMs ? staleAfterMs : (_updateIntervalMs ? _updateIntervalMs * 3UL : TERRA_DEFAULT_REMOTE_STALE_MS);
    return !_measurement.valid || terraElapsed(now, _measurement.timestamp, threshold);
}

void TerraSensor::update(uint32_t now) {
    if (!_enabled || _fault || (!_driver && !_readCallback)) return;
    if (!_lastReadAt || terraElapsed(now, _lastReadAt, _updateIntervalMs)) {
        if (_driver) {
            TerraMeasurement measurement = _driver->read(now);
            setMeasurement(measurement.value, measurement.unit, measurement.timestamp, measurement.valid);
        } else {
            float value = _readCallback(_readContext);
            setMeasurement(value, _measurement.unit, now, !isnan(value));
        }
    }
}


TerraAnalogSensor::TerraAnalogSensor(Terra_Unit unit, uint32_t key, const TerraString &name)
    : TerraSensor(Terra_SensorType_Analog, unit, key, name), _rawMinimum(0.0f), _rawMaximum(1023.0f),
      _valueMinimum(0.0f), _valueMaximum(100.0f), _calibrated(false) { }

bool TerraAnalogSensor::setCalibration(float rawMinimum, float rawMaximum, float valueMinimum, float valueMaximum) {
    if (isFPEqual(rawMinimum, rawMaximum)) return false;
    _rawMinimum = rawMinimum;
    _rawMaximum = rawMaximum;
    _valueMinimum = valueMinimum;
    _valueMaximum = valueMaximum;
    _calibrated = true;
    return true;
}

bool TerraAnalogSensor::getCalibration(float &rawMinimum, float &rawMaximum,
                                       float &valueMinimum, float &valueMaximum) const {
    if (!_calibrated) return false;
    rawMinimum = _rawMinimum;
    rawMaximum = _rawMaximum;
    valueMinimum = _valueMinimum;
    valueMaximum = _valueMaximum;
    return true;
}

void TerraAnalogSensor::update(uint32_t now) {
    if (!_enabled || _fault || (!_driver && !_readCallback)) return;
    if (!_lastReadAt || terraElapsed(now, _lastReadAt, _updateIntervalMs)) {
        float raw;
        Terra_Unit unit = _measurement.unit;
        bool valid = true;
        if (_driver) {
            TerraMeasurement measurement = _driver->read(now);
            raw = measurement.value;
            valid = measurement.valid;
            unit = measurement.unit == Terra_Unit_Undefined ? _measurement.unit : measurement.unit;
        } else {
            raw = _readCallback(_readContext);
            valid = !isnan(raw);
        }
        if (!valid || isnan(raw)) {
            setMeasurement(raw, unit, now, false);
            return;
        }
        float value = _calibrated ? terraMapFloat(raw, _rawMinimum, _rawMaximum, _valueMinimum, _valueMaximum) : raw;
        setMeasurement(value, unit, now, true);
    }
}

TerraBinarySensor::TerraBinarySensor(uint32_t key, const TerraString &name)
    : TerraSensor(Terra_SensorType_Binary, Terra_Unit_Raw, key, name) { }

void TerraBinarySensor::setState(bool active, uint32_t timestamp) {
    setMeasurement(active ? 1.0f : 0.0f, Terra_Unit_Raw, timestamp, true);
}

TerraRemoteSensor::TerraRemoteSensor(Terra_SensorType reportedType, Terra_Unit unit, uint32_t key, const TerraString &name)
    : TerraSensor(Terra_SensorType_Remote, unit, key, name), _reportedType(reportedType),
      _staleAfterMs(TERRA_DEFAULT_REMOTE_STALE_MS), _lastReportAt(0), _hasReport(false) { }

void TerraRemoteSensor::receiveReport(float value, Terra_Unit unit, uint32_t reportTime, bool valid) {
    _lastReportAt = reportTime;
    _hasReport = true;
    setMeasurement(value, unit, reportTime, valid);
    if (valid) clearFault();
    else setFault(TerraString("remote sensor reported invalid data"));
}

bool TerraRemoteSensor::isOnline(uint32_t now) const {
    return _hasReport && !terraElapsed(now, _lastReportAt, _staleAfterMs);
}

void TerraRemoteSensor::update(uint32_t now) {
    if (_hasReport && terraElapsed(now, _lastReportAt, _staleAfterMs)) {
        _measurement.valid = false;
        setFault(TerraString("remote sensor stale"));
    }
}
