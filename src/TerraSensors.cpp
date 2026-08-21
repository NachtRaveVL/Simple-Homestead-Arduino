/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Sensors
*/

#include "TerraSensors.h"
#include "TerraUtils.h"

TerraSensor::TerraSensor(Terra_SensorType sensorType, Terra_Unit unit,
                         uint32_t key, const TerraString &name)
    : TerraObject(Terra_ObjectType_Sensor, key, name), _sensorType(sensorType),
      _measurement(0.0f, unit, 0, false), _driver(), _updateIntervalMs(1000), _lastReadAt(0)
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
    return staleAfterMs && (!_measurement.valid || terraElapsed(now, _measurement.timestamp, staleAfterMs));
}

void TerraSensor::update(uint32_t now)
{
    if (!_enabled || !_driver) return;
    if (_lastReadAt && _updateIntervalMs && !terraElapsed(now, _lastReadAt, _updateIntervalMs)) return;

    TerraMeasurement measurement = _driver->read(now);
    _lastReadAt = now;
    if (measurement.valid) _measurement = measurement;
}

TerraAnalogSensor::TerraAnalogSensor(Terra_Unit unit, uint32_t key, const TerraString &name)
    : TerraSensor(Terra_SensorType_Analog, unit, key, name), _rawMinimum(0.0f), _rawMaximum(1.0f),
      _valueMinimum(0.0f), _valueMaximum(1.0f), _calibrated(false)
{ }

bool TerraAnalogSensor::setCalibration(float rawMinimum, float rawMaximum,
                                       float valueMinimum, float valueMaximum)
{
    if (isFPEqual(rawMinimum, rawMaximum)) return false;
    _rawMinimum = rawMinimum;
    _rawMaximum = rawMaximum;
    _valueMinimum = valueMinimum;
    _valueMaximum = valueMaximum;
    _calibrated = true;
    return true;
}

bool TerraAnalogSensor::getCalibration(float &rawMinimum, float &rawMaximum,
                                       float &valueMinimum, float &valueMaximum) const
{
    if (!_calibrated) return false;
    rawMinimum = _rawMinimum;
    rawMaximum = _rawMaximum;
    valueMinimum = _valueMinimum;
    valueMaximum = _valueMaximum;
    return true;
}

void TerraAnalogSensor::update(uint32_t now)
{
    TerraSensor::update(now);
    if (_calibrated && _measurement.valid) {
        float ratio = (_measurement.value - _rawMinimum) / (_rawMaximum - _rawMinimum);
        _measurement.value = _valueMinimum + ratio * (_valueMaximum - _valueMinimum);
    }
}

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
    return _hasReport && (!_staleAfterMs || !terraElapsed(now, _lastReportAt, _staleAfterMs));
}

void TerraRemoteSensor::update(uint32_t now)
{
    if (!isOnline(now)) _measurement.valid = false;
}
