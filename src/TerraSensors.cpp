/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Sensors
*/

#include "Terraduino.h"
#include "TerraUtils.h"

TerraSensor::TerraSensor(Terra_SensorType sensorType, Terra_Unit unit,
                         uint32_t key, const TerraString &name)
    : TerraObject(Terra_ObjectType_Sensor, key, name), _sensorType(sensorType),
      _measurement(0.0f, unit, 0, false), _driver(), _updateIntervalMs(1000), _lastReadAt(0),
      _rawMinimum(0.0f), _rawMaximum(1.0f), _valueMinimum(0.0f), _valueMaximum(1.0f),
      _calibrationUnit(unit), _calibrated(false)
{ }

void TerraSensor::setDriver(const SharedPtr<TerraInputDriver> &driver)
{
    _driver = driver;
    if (_driver) _driver->begin();
}

void TerraSensor::setMeasurement(float value, Terra_Unit unit, uint32_t timestamp, bool valid)
{
    _measurement = TerraMeasurement(value, unit, timestamp, valid);
    _lastReadAt = timestamp;
}

bool TerraSensor::isStale(uint32_t now, uint32_t staleAfterMs) const
{
    return staleAfterMs && (!_measurement.valid || (uint32_t)(now - _measurement.timestamp) >= staleAfterMs);
}

bool TerraSensor::setCalibration(float rawMinimum, float rawMaximum,
                                 float valueMinimum, float valueMaximum,
                                 Terra_Unit unit)
{
    if (isFPEqual(rawMinimum, rawMaximum)) return false;
    _rawMinimum = rawMinimum;
    _rawMaximum = rawMaximum;
    _valueMinimum = valueMinimum;
    _valueMaximum = valueMaximum;
    _calibrationUnit = unit != Terra_Unit_Undefined ? unit : _measurement.unit;
    _measurement.unit = _calibrationUnit;
    _calibrated = true;
    return true;
}

void TerraSensor::clearCalibration()
{
    _calibrated = false;
    _rawMinimum = 0.0f;
    _rawMaximum = 1.0f;
    _valueMinimum = 0.0f;
    _valueMaximum = 1.0f;
    _calibrationUnit = _measurement.unit;
}

bool TerraSensor::getCalibration(float &rawMinimum, float &rawMaximum,
                                 float &valueMinimum, float &valueMaximum) const
{
    if (!_calibrated) return false;
    rawMinimum = _rawMinimum;
    rawMaximum = _rawMaximum;
    valueMinimum = _valueMinimum;
    valueMaximum = _valueMaximum;
    return true;
}

void TerraSensor::handleDriverMeasurement(const TerraMeasurement &measurement)
{
    if (!_calibrated) {
        _measurement = measurement;
        return;
    }

    float ratio = (measurement.value - _rawMinimum) / (_rawMaximum - _rawMinimum);
    _measurement = TerraMeasurement(_valueMinimum + ratio * (_valueMaximum - _valueMinimum),
                                    _calibrationUnit, measurement.timestamp, measurement.valid);
}

void TerraSensor::update(uint32_t now)
{
    if (!_enabled || !_driver) return;
    if (_lastReadAt && _updateIntervalMs && (uint32_t)(now - _lastReadAt) < _updateIntervalMs) return;

    TerraMeasurement measurement = _driver->read(now);
    _lastReadAt = now;
    if (measurement.valid) handleDriverMeasurement(measurement);
}

TerraAnalogSensor::TerraAnalogSensor(Terra_Unit unit, uint32_t key, const TerraString &name)
    : TerraSensor(Terra_SensorType_Analog, unit, key, name)
{ }

TerraBinarySensor::TerraBinarySensor(uint32_t key, const TerraString &name)
    : TerraSensor(Terra_SensorType_Binary, Terra_Unit_Raw, key, name)
{ }

void TerraBinarySensor::setState(bool active, uint32_t timestamp)
{
    setMeasurement(active ? 1.0f : 0.0f, Terra_Unit_Raw, timestamp, true);
}

TerraRemoteSensor::TerraRemoteSensor(Terra_SensorType reportedType, Terra_Unit unit,
                                     uint32_t key, const TerraString &name)
    : TerraSensor(Terra_SensorType_Remote, unit, key, name), _reportedType(reportedType),
      _staleAfterMs(TERRA_DEFAULT_REMOTE_STALE_MS), _lastReportAt(0), _hasReport(false)
{ }

void TerraRemoteSensor::receiveReport(float value, Terra_Unit unit, uint32_t reportTime, bool valid)
{
    _hasReport = true;
    _lastReportAt = reportTime;
    setMeasurement(value, unit, reportTime, valid);
}

bool TerraRemoteSensor::isOnline(uint32_t now) const
{
    return _hasReport && (!_staleAfterMs || (uint32_t)(now - _lastReportAt) < _staleAfterMs);
}

void TerraRemoteSensor::update(uint32_t now)
{
    if (!isOnline(now)) _measurement.valid = false;
}
