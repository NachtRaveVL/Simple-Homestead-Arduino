/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Sensors
*/

#ifndef TerraSensors_H
#define TerraSensors_H

#include "TerraObject.h"
#include "TerraCallback.hh"
#include "TerraMeasurements.h"
#include "TerraDrivers.h"

// Sensor Base
// Base class for local, callback, pin-backed, or remote homestead measurements.
class TerraSensor : public TerraObject, public TerraMeasurementProvider {
public:
    TerraSensor(Terra_SensorType sensorType = Terra_SensorType_Undefined,
                Terra_Unit unit = Terra_Unit_Raw,
                uint32_t key = TERRA_INVALID_KEY,
                const TerraString &name = TerraString());
    virtual ~TerraSensor();

    void setReadCallback(TerraReadCallback callback, void *context = nullptr);
    void setDriver(TerraInputDriver *driver, bool takeOwnership = false);
    TerraInputDriver *getDriver() const { return _driver; }
    void setUpdateInterval(uint32_t intervalMs) { _updateIntervalMs = intervalMs; }
    uint32_t getUpdateInterval() const { return _updateIntervalMs; }
    void setMeasurement(float value, Terra_Unit unit, uint32_t timestamp = terraMillis(), bool valid = true);
    TerraMeasurement getMeasurement() const override { return _measurement; }
    Terra_SensorType getSensorType() const { return _sensorType; }
    bool isStale(uint32_t now = terraMillis(), uint32_t staleAfterMs = 0) const;
    void update(uint32_t now = terraMillis()) override;

protected:
    Terra_SensorType _sensorType;                           // Sensor type
    TerraMeasurement _measurement;                          // Latest sensor measurement
    TerraInputDriver *_driver;
    bool _ownsDriver;                                       // Driver ownership flag
    TerraReadCallback _readCallback;                        // Sensor read callback
    void *_readContext;
    uint32_t _updateIntervalMs;                             // Sensor update interval, milliseconds
    uint32_t _lastReadAt;                                   // Last sensor read timestamp
};

// Analog Sensor
// Adds a linear calibration range to a generic scalar sensor.
class TerraAnalogSensor : public TerraSensor {
public:
    TerraAnalogSensor(Terra_Unit unit = Terra_Unit_Raw,
                      uint32_t key = TERRA_INVALID_KEY,
                      const TerraString &name = TerraString());
    bool setCalibration(float rawMinimum, float rawMaximum, float valueMinimum, float valueMaximum);
    bool getCalibration(float &rawMinimum, float &rawMaximum,
                        float &valueMinimum, float &valueMaximum) const;
    void update(uint32_t now = terraMillis()) override;

protected:
    float _rawMinimum;                                      // Raw calibration minimum
    float _rawMaximum;                                      // Raw calibration maximum
    float _valueMinimum;                                    // Calibrated output minimum
    float _valueMaximum;                                    // Calibrated output maximum
    bool _calibrated;                                       // Calibration configured flag
};

// Binary Sensor
// Stores a simple active/inactive measurement.
class TerraBinarySensor : public TerraSensor {
public:
    TerraBinarySensor(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString());
    void setState(bool active, uint32_t timestamp = terraMillis());
    bool isActive() const { return _measurement.valid && _measurement.value >= 0.5f; }
};

class TerraTemperatureSensor : public TerraSensor {
public:
    TerraTemperatureSensor(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString())
        : TerraSensor(Terra_SensorType_Temperature, Terra_Unit_Celsius, key, name) { }
};

class TerraHumiditySensor : public TerraSensor {
public:
    TerraHumiditySensor(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString())
        : TerraSensor(Terra_SensorType_Humidity, Terra_Unit_Percent, key, name) { }
};

class TerraLevelSensor : public TerraSensor {
public:
    TerraLevelSensor(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString())
        : TerraSensor(Terra_SensorType_Level, Terra_Unit_Percent, key, name) { }
};

class TerraPressureSensor : public TerraSensor {
public:
    TerraPressureSensor(Terra_Unit unit = Terra_Unit_Kilopascals,
                        uint32_t key = TERRA_INVALID_KEY,
                        const TerraString &name = TerraString())
        : TerraSensor(Terra_SensorType_Pressure, unit, key, name) { }
};

class TerraFlowSensor : public TerraSensor {
public:
    TerraFlowSensor(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString())
        : TerraSensor(Terra_SensorType_Flow, Terra_Unit_LitersPerMinute, key, name) { }
};

class TerraRainfallSensor : public TerraSensor {
public:
    TerraRainfallSensor(bool rate = false, uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString())
        : TerraSensor(Terra_SensorType_Rainfall, rate ? Terra_Unit_MillimetersPerHour : Terra_Unit_Millimeters, key, name) { }
};

class TerraWindSpeedSensor : public TerraSensor {
public:
    TerraWindSpeedSensor(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString())
        : TerraSensor(Terra_SensorType_WindSpeed, Terra_Unit_MetersPerSecond, key, name) { }
};

class TerraWindDirectionSensor : public TerraSensor {
public:
    TerraWindDirectionSensor(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString())
        : TerraSensor(Terra_SensorType_WindDirection, Terra_Unit_Degrees, key, name) { }
};

class TerraSolarRadiationSensor : public TerraSensor {
public:
    TerraSolarRadiationSensor(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString())
        : TerraSensor(Terra_SensorType_SolarRadiation, Terra_Unit_WattsPerSquareMeter, key, name) { }
};

class TerraVoltageSensor : public TerraSensor {
public:
    TerraVoltageSensor(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString())
        : TerraSensor(Terra_SensorType_Voltage, Terra_Unit_Volts, key, name) { }
};

class TerraCurrentSensor : public TerraSensor {
public:
    TerraCurrentSensor(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString())
        : TerraSensor(Terra_SensorType_Current, Terra_Unit_Amps, key, name) { }
};

class TerraLeakSensor : public TerraBinarySensor {
public:
    TerraLeakSensor(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString())
        : TerraBinarySensor(key, name) { _sensorType = Terra_SensorType_Leak; }
};

// Remote Sensor
// Stores a transport-neutral remote reading and tracks link staleness.
class TerraRemoteSensor : public TerraSensor {
public:
    TerraRemoteSensor(Terra_SensorType reportedType = Terra_SensorType_Remote,
                      Terra_Unit unit = Terra_Unit_Raw,
                      uint32_t key = TERRA_INVALID_KEY,
                      const TerraString &name = TerraString());

    void receiveReport(float value, Terra_Unit unit, uint32_t reportTime = terraMillis(), bool valid = true);
    void setStaleAfter(uint32_t staleAfterMs) { _staleAfterMs = staleAfterMs; }
    uint32_t getStaleAfter() const { return _staleAfterMs; }
    Terra_SensorType getReportedType() const { return _reportedType; }
    uint32_t getLastReportTime() const { return _lastReportAt; }
    bool isOnline(uint32_t now = terraMillis()) const;
    void update(uint32_t now = terraMillis()) override;

protected:
    Terra_SensorType _reportedType;                         // Remote reported sensor type
    uint32_t _staleAfterMs;                                 // Remote stale timeout, milliseconds
    uint32_t _lastReportAt;                                 // Last remote report timestamp
    bool _hasReport;                                        // Remote report received flag
};

#endif
