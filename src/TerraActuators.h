/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Actuators
*/

#ifndef TerraActuators_H
#define TerraActuators_H

#include "TerraObject.h"
#include "TerraActivation.h"
#include "TerraDrivers.h"
#include "TerraAttachments.h"

// Actuator Base
// Base controlled output using a shared output driver and resident activation requests.
class TerraActuator : public TerraObject {
public:
    TerraActuator(Terra_ActuatorType actuatorType = Terra_ActuatorType_Undefined,
                  uint32_t key = TERRA_INVALID_KEY,
                  const TerraString &name = TerraString());
    virtual ~TerraActuator();

    void setDriver(const SharedPtr<TerraOutputDriver> &driver);
    SharedPtr<TerraOutputDriver> getDriver() const { return _driver; }
    void setEnabled(bool enabled) override;
    virtual void setOutput(float intensity, uint32_t durationMs = 0, uint32_t now = millis());
    virtual void off();
    bool isActive() const { return _output > FLT_EPSILON; }
    float getOutput() const { return _output; }
    Terra_ActuatorType getActuatorType() const { return _actuatorType; }
    inline void setEnableMode(Terra_EnableMode mode)
    {
        _enableMode = mode;
        resolveActivations();
    }
    Terra_EnableMode getEnableMode() const { return _enableMode; }
    void update(uint32_t now = millis()) override;

    bool addActivationHandle(TerraActivationHandle *handle);
    bool removeActivationHandle(TerraActivationHandle *handle);
    void resolveActivations();

protected:
    Terra_ActuatorType _actuatorType;                       // Actuator type
    SharedPtr<TerraOutputDriver> _driver;                   // Output driver sub-object
    Terra_EnableMode _enableMode;                           // Request aggregation mode
    TerraActivationHandle *_handles[TERRA_MAX_ATTACHMENTS]; // Active attachment requests
    TerraActivationHandle _directActivation;                // Direct user request
    float _output;                                          // Applied normalized output

    void applyOutput(float intensity);
};

// Pump Actuator
// Adds a continuous-runtime safety limit to a controlled pump output.
class TerraPump : public TerraActuator {
public:
    TerraPump(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString());
    void setMaxContinuousRuntime(uint32_t maxMs) { _maxContinuousMs = maxMs; }
    uint32_t getMaxContinuousRuntime() const { return _maxContinuousMs; }
    void update(uint32_t now = millis()) override;

protected:
    uint32_t _maxContinuousMs;                              // Maximum continuous runtime, milliseconds
    uint32_t _startedAt;                                    // Continuous run start timestamp
};

// Sump Pump Actuator
// Adds level hysteresis, high-water alarm state, invalid-level fail-safe shutdown,
// and continuous-runtime protection to a normal pump output. Level sensing is attached
// directly so normal operation only requires controller update().
class TerraSumpPump : public TerraPump {
public:
    TerraSumpPump(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString());

    bool configureLevels(float startPercent, float stopPercent,
                         float alarmPercent = TERRA_SUMP_ALARM_LEVEL_PERCENT);
    template<class T> inline void setLevelSensor(const SharedPtr<T> &sensor) { _levelSensor.setObject(sensor); }
    inline TerraSensorAttachment &getLevelSensorAttachment() { return _levelSensor; }
    inline const TerraSensorAttachment &getLevelSensorAttachment() const { return _levelSensor; }
    void update(uint32_t now = millis()) override;
    void unresolveAny(TerraObject *object) override;

    float getStartLevelPercent() const { return _startLevelPercent; }
    float getStopLevelPercent() const { return _stopLevelPercent; }
    float getAlarmLevelPercent() const { return _alarmLevelPercent; }
    float getLastLevelPercent() const { return _lastLevelPercent; }
    bool hasValidLevel() const { return _levelValid; }
    bool hasHighWaterAlarm() const { return _highWaterAlarm; }

protected:
    TerraSensorAttachment _levelSensor;                     // Sump level sensor attachment
    float _startLevelPercent;                               // Pump start level, percent
    float _stopLevelPercent;                                // Pump stop level, percent
    float _alarmLevelPercent;                               // High-water alarm level, percent
    float _lastLevelPercent;                                // Last valid level, percent
    bool _levelValid;                                       // Valid level state
    bool _highWaterAlarm;                                   // High-water alarm state

    void initLevelSensorKey(uint32_t key) { _levelSensor.initObject(key); }

    friend class TerraFactory;
};

class TerraVariableActuator : public TerraActuator {
public:
    TerraVariableActuator(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString())
        : TerraActuator(Terra_ActuatorType_Variable, key, name) { }
};

class TerraValve : public TerraActuator {
public:
    TerraValve(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString())
        : TerraActuator(Terra_ActuatorType_Valve, key, name) { }
    void open(uint32_t durationMs = 0) { setOutput(1.0f, durationMs); }
    void close() { off(); }
};

class TerraDiverter : public TerraActuator {
public:
    TerraDiverter(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString())
        : TerraActuator(Terra_ActuatorType_Diverter, key, name) { }
    void routePrimary() { off(); }
    void routeSecondary() { setOutput(1.0f); }
};

class TerraHeater : public TerraActuator {
public:
    TerraHeater(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString())
        : TerraActuator(Terra_ActuatorType_Heater, key, name) { }
};

class TerraCirculator : public TerraPump {
public:
    TerraCirculator(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString())
        : TerraPump(key, name) { _actuatorType = Terra_ActuatorType_Circulator; }
};

#endif
