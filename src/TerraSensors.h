/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Sensors
*/

#ifndef TerraSensors_H
#define TerraSensors_H

class TerraSensor;
class TerraBinarySensor;
class TerraAnalogSensor;

struct TerraSensorData;
struct TerraCalibrationData;

#include "TerraObject.h"
#include "TerraMeasurements.h"
#include "TerraPins.h"
#include "TerraDatas.h"

// Creates sensor object from passed sensor data (return ownership transfer - user code *must* delete returned object)
extern TerraSensor *newSensorObjectFromData(const TerraSensorData *dataIn);

// Sensor Base
class TerraSensor : public TerraObject,
                    public TerraMeasurementUnitsInterfaceStorageSingle {
public:
    const enum : signed char { Value, Binary, Analog, Remote, Unknown = -1 } classType; // Sensor class type

    TerraSensor(Terra_SensorType sensorType,
                tposi_t sensorIndex,
                Terra_UnitsType units = Terra_UnitsType_Raw_1,
                int classTypeIn = Value);
    TerraSensor(const TerraSensorData *dataIn);
    virtual ~TerraSensor() { ; }

    virtual bool takeMeasurement(bool force = false);
    virtual const TerraMeasurement *getMeasurement(bool poll = false);
    virtual bool needsPolling(tframe_t allowance = 0) const;
    virtual void update(uint32_t now = millis()) override;

    void setMeasurement(float value, Terra_UnitsType units,
                        uint32_t timestamp = millis(), bool valid = true);
    bool isStale(uint32_t now = millis(), uint32_t staleAfterMs = 0) const;

    void setUserCalibrationData(TerraCalibrationData *userCalibrationData);
    inline const TerraCalibrationData *getUserCalibrationData() const { return _calibrationData; }

    inline float calibrationTransform(float value) const { return _calibrationData ? _calibrationData->transform(value) : value; }
    inline void calibrationTransform(float *valueInOut, Terra_UnitsType *unitsOut = nullptr) const { if (valueInOut && _calibrationData) { _calibrationData->transform(valueInOut, unitsOut); } }
    inline TerraSingleMeasurement calibrationTransform(TerraSingleMeasurement measurement) const { return _calibrationData ? _calibrationData->transform(measurement) : measurement; }
    inline void calibrationTransform(TerraSingleMeasurement *measurementInOut) const { if (measurementInOut && _calibrationData) { _calibrationData->transform(measurementInOut); } }

    inline float calibrationInvTransform(float value) const { return _calibrationData ? _calibrationData->inverseTransform(value) : value; }
    inline void calibrationInvTransform(float *valueInOut, Terra_UnitsType *unitsOut = nullptr) const { if (valueInOut && _calibrationData) { _calibrationData->inverseTransform(valueInOut, unitsOut); } }
    inline TerraSingleMeasurement calibrationInvTransform(TerraSingleMeasurement measurement) const { return _calibrationData ? _calibrationData->inverseTransform(measurement) : measurement; }
    inline void calibrationInvTransform(TerraSingleMeasurement *measurementInOut) const { if (measurementInOut && _calibrationData) { _calibrationData->inverseTransform(measurementInOut); } }

    Signal<const TerraMeasurement *, TERRA_SENSOR_SIGNAL_SLOTS> &getMeasurementSignal();

    inline Terra_SensorType getSensorType() const { return _id.objTypeAs.sensorType; }
    inline tposi_t getSensorIndex() const { return _id.posIndex; }

protected:
    TerraSingleMeasurement _lastMeasurement;                // Latest successful measurement
    const TerraCalibrationData *_calibrationData;           // Calibration data
    Signal<const TerraMeasurement *, TERRA_SENSOR_SIGNAL_SLOTS> _measurementSignal; // Measurement signal

    virtual TerraData *allocateData() const override;
    virtual void saveToData(TerraData *dataOut) const override;
};

// Binary Sensor
class TerraBinarySensor : public TerraSensor {
public:
    TerraBinarySensor(Terra_SensorType sensorType,
                      tposi_t sensorIndex,
                      TerraDigitalPin inputPin);
    TerraBinarySensor(const TerraSensorData *dataIn);

    virtual bool takeMeasurement(bool force = false) override;
    inline const TerraDigitalPin &getInputPin() const { return _inputPin; }
    inline bool isActive(bool poll = false) { return getMeasurementValue(getMeasurement(poll)) >= 0.5f; }

#ifndef ARDUINO
    inline void setSimulatedState(bool active) { _inputPin.setSimulatedState(active); }
#endif

protected:
    TerraDigitalPin _inputPin;                              // Digital input pin

    virtual void saveToData(TerraData *dataOut) const override;
};

// Analog Sensor
class TerraAnalogSensor : public TerraSensor {
public:
    TerraAnalogSensor(Terra_SensorType sensorType,
                      tposi_t sensorIndex,
                      TerraAnalogPin inputPin,
                      Terra_UnitsType units = Terra_UnitsType_Raw_1);
    TerraAnalogSensor(const TerraSensorData *dataIn);

    virtual bool takeMeasurement(bool force = false) override;
    inline const TerraAnalogPin &getInputPin() const { return _inputPin; }

#ifndef ARDUINO
    inline void setSimulatedValue(int value) { _inputPin.setSimulatedValue(value); }
#endif

protected:
    TerraAnalogPin _inputPin;                               // Analog input pin

    virtual void saveToData(TerraData *dataOut) const override;
};

class TerraTemperatureSensor : public TerraAnalogSensor {
public:
    TerraTemperatureSensor(tposi_t sensorIndex,
                           TerraAnalogPin inputPin = TerraAnalogPin())
        : TerraAnalogSensor(Terra_SensorType_Temperature, sensorIndex, inputPin, Terra_UnitsType_Temperature_Celsius) { ; }
};

class TerraHumiditySensor : public TerraAnalogSensor {
public:
    TerraHumiditySensor(tposi_t sensorIndex,
                        TerraAnalogPin inputPin = TerraAnalogPin())
        : TerraAnalogSensor(Terra_SensorType_Humidity, sensorIndex, inputPin, Terra_UnitsType_Percentile_100) { ; }
};

class TerraLevelSensor : public TerraAnalogSensor {
public:
    TerraLevelSensor(tposi_t sensorIndex,
                     TerraAnalogPin inputPin = TerraAnalogPin())
        : TerraAnalogSensor(Terra_SensorType_Level, sensorIndex, inputPin, Terra_UnitsType_Percentile_100) { ; }
};

class TerraPressureSensor : public TerraAnalogSensor {
public:
    TerraPressureSensor(tposi_t sensorIndex,
                        TerraAnalogPin inputPin = TerraAnalogPin(),
                        Terra_UnitsType units = Terra_UnitsType_Pressure_Kilopascals)
        : TerraAnalogSensor(Terra_SensorType_Pressure, sensorIndex, inputPin, units) { ; }
};

class TerraFlowSensor : public TerraAnalogSensor {
public:
    TerraFlowSensor(tposi_t sensorIndex,
                    TerraAnalogPin inputPin = TerraAnalogPin())
        : TerraAnalogSensor(Terra_SensorType_Flow, sensorIndex, inputPin, Terra_UnitsType_LitersPerMinute) { ; }
};

class TerraRainfallSensor : public TerraAnalogSensor {
public:
    TerraRainfallSensor(tposi_t sensorIndex,
                        TerraAnalogPin inputPin = TerraAnalogPin(),
                        bool rate = false)
        : TerraAnalogSensor(Terra_SensorType_Rainfall, sensorIndex, inputPin,
                            rate ? Terra_UnitsType_Speed_MillimetersPerHour : Terra_UnitsType_Distance_Millimeters) { ; }
};

class TerraWindSpeedSensor : public TerraAnalogSensor {
public:
    TerraWindSpeedSensor(tposi_t sensorIndex,
                         TerraAnalogPin inputPin = TerraAnalogPin())
        : TerraAnalogSensor(Terra_SensorType_WindSpeed, sensorIndex, inputPin, Terra_UnitsType_Speed_MetersPerSecond) { ; }
};

class TerraWindDirectionSensor : public TerraAnalogSensor {
public:
    TerraWindDirectionSensor(tposi_t sensorIndex,
                             TerraAnalogPin inputPin = TerraAnalogPin())
        : TerraAnalogSensor(Terra_SensorType_WindDirection, sensorIndex, inputPin, Terra_UnitsType_Angle_Degrees_360) { ; }
};

class TerraSolarRadiationSensor : public TerraAnalogSensor {
public:
    TerraSolarRadiationSensor(tposi_t sensorIndex,
                              TerraAnalogPin inputPin = TerraAnalogPin())
        : TerraAnalogSensor(Terra_SensorType_SolarRadiation, sensorIndex, inputPin, Terra_UnitsType_Irradiance_WattsPerSquareMeter) { ; }
};

class TerraVoltageSensor : public TerraAnalogSensor {
public:
    TerraVoltageSensor(tposi_t sensorIndex,
                       TerraAnalogPin inputPin = TerraAnalogPin())
        : TerraAnalogSensor(Terra_SensorType_Voltage, sensorIndex, inputPin, Terra_UnitsType_Power_Volts) { ; }
};

class TerraCurrentSensor : public TerraAnalogSensor {
public:
    TerraCurrentSensor(tposi_t sensorIndex,
                       TerraAnalogPin inputPin = TerraAnalogPin())
        : TerraAnalogSensor(Terra_SensorType_Current, sensorIndex, inputPin, Terra_UnitsType_Current_Amperage) { ; }
};

class TerraLeakSensor : public TerraBinarySensor {
public:
    TerraLeakSensor(tposi_t sensorIndex,
                    TerraDigitalPin inputPin = TerraDigitalPin())
        : TerraBinarySensor(Terra_SensorType_Leak, sensorIndex, inputPin) { ; }
};

// Remote Sensor
class TerraRemoteSensor : public TerraSensor {
public:
    TerraRemoteSensor(Terra_SensorType reportedType,
                      tposi_t sensorIndex,
                      Terra_UnitsType units = Terra_UnitsType_Raw_1);
    TerraRemoteSensor(const TerraSensorData *dataIn);

    void receiveReport(float value, Terra_UnitsType units,
                       uint32_t reportTime = millis(), bool valid = true);
    void setStaleAfter(uint32_t staleAfterMs) { _staleAfterMs = staleAfterMs; }
    inline uint32_t getStaleAfter() const { return _staleAfterMs; }
    inline Terra_SensorType getReportedType() const { return _reportedType; }
    inline uint32_t getLastReportTime() const { return _lastReportAt; }
    bool isOnline(uint32_t now = millis()) const;
    virtual void update(uint32_t now = millis()) override;

protected:
    Terra_SensorType _reportedType;                         // Remote reported sensor type
    uint32_t _staleAfterMs;                                 // Remote stale timeout
    uint32_t _lastReportAt;                                 // Last report timestamp
    bool _hasReport;                                        // Any report received flag

    virtual void saveToData(TerraData *dataOut) const override;
};

// Sensor Serialization Data
struct TerraSensorData : public TerraObjectData {
    Terra_UnitsType measurementUnits;                       // Measurement units
    Terra_SensorType reportedType;                          // Remote reported sensor type
    uint32_t staleAfterMs;                                  // Remote stale timeout
    TerraPinData inputPin;                                  // Input pin

    TerraSensorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef TerraSensors_H
