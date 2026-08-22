/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Actuators
*/

#ifndef TerraActuators_H
#define TerraActuators_H

class TerraActuator;
class TerraRelayActuator;
class TerraVariableActuator;
class TerraPump;
class TerraSumpPump;

struct TerraActuatorData;

#include "TerraObject.h"
#include "TerraActivation.h"
#include "TerraAttachments.h"
#include "TerraPins.h"
#include "TerraDatas.h"
#include <math.h>

// Actuator Base
// Base controlled output using a shared output driver and resident activation requests.
class TerraActuator : public TerraObject {
public:
    TerraActuator(Terra_ActuatorType actuatorType = Terra_ActuatorType_Undefined,
                  uint32_t key = TERRA_INVALID_KEY,
                  const TerraString &name = TerraString());
    const enum : signed char { Relay, RelayPump, Variable, Unknown = -1 } classType; // Actuator class type

    TerraActuator(Terra_ActuatorType actuatorType,
                  tposi_t actuatorIndex,
                  int classTypeIn = Unknown);
    TerraActuator(const TerraActuatorData *dataIn);
    virtual ~TerraActuator();

    virtual void update(uint32_t now = millis()) override;

    virtual bool getCanEnable();
    virtual float getDriveIntensity() const = 0;
    virtual bool isEnabled(float tolerance = 0.0f) const = 0;

    inline Terra_ActuatorType getActuatorType() const { return _id.objTypeAs.actuatorType; }
    inline tposi_t getActuatorIndex() const { return _id.posIndex; }

    void setEnableMode(Terra_EnableMode mode);
    inline Terra_EnableMode getEnableMode() const { return _enableMode; }
    inline void setNeedsUpdate() { _needsUpdate = true; }
    inline bool needsUpdate() const { return _needsUpdate; }

    TerraActivationHandle activate(Terra_DirectionMode direction = Terra_DirectionMode_Forward,
                                   float intensity = 1.0f,
                                   millis_t duration = (millis_t)-1,
                                   bool force = false);
    void setOutput(float intensity, millis_t duration = (millis_t)-1,
                   uint32_t now = millis());
    void off();

protected:
    bool _enabled;                                          // Actuator enabled state
    bool _needsUpdate;                                      // Activation-resolution stale flag
    Terra_EnableMode _enableMode;                           // Activation enablement mode
    Vector<TerraActivationHandle *> _handles;               // Active handles
    TerraActivationHandle _directActivation;                // Direct user request

    virtual void _enableActuator(float intensity = 1.0f) = 0;
    virtual void _disableActuator() = 0;

    virtual TerraData *allocateData() const override;
    virtual void saveToData(TerraData *dataOut) const override;

    friend struct TerraActivationHandle;
};

// Relay Actuator
class TerraRelayActuator : public TerraActuator {
public:
    TerraRelayActuator(Terra_ActuatorType actuatorType,
                       tposi_t actuatorIndex,
                       TerraDigitalPin outputPin,
                       int classTypeIn = Relay);
    TerraRelayActuator(const TerraActuatorData *dataIn);
    virtual ~TerraRelayActuator();

    virtual bool getCanEnable() override;
    virtual float getDriveIntensity() const override { return _enabled ? 1.0f : 0.0f; }
    virtual bool isEnabled(float tolerance = 0.0f) const override { (void)tolerance; return _enabled; }

    inline const TerraDigitalPin &getOutputPin() const { return _outputPin; }

protected:
    TerraDigitalPin _outputPin;                             // Digital output pin

    virtual void _enableActuator(float intensity = 1.0f) override;
    virtual void _disableActuator() override;
    virtual void saveToData(TerraData *dataOut) const override;
};

// Variable Actuator
class TerraVariableActuator : public TerraActuator {
public:
    TerraVariableActuator(Terra_ActuatorType actuatorType,
                          tposi_t actuatorIndex,
                          TerraAnalogPin outputPin);
    TerraVariableActuator(const TerraActuatorData *dataIn);
    virtual ~TerraVariableActuator();

    virtual bool getCanEnable() override;
    virtual float getDriveIntensity() const override { return _intensity; }
    virtual bool isEnabled(float tolerance = 0.0f) const override { return fabsf(_intensity) > tolerance; }

    inline const TerraAnalogPin &getOutputPin() const { return _outputPin; }

protected:
    TerraAnalogPin _outputPin;                              // Analog/PWM output pin
    float _intensity;                                       // Current normalized output

    virtual void _enableActuator(float intensity = 1.0f) override;
    virtual void _disableActuator() override;
    virtual void saveToData(TerraData *dataOut) const override;
};

// Pump Actuator
class TerraPump : public TerraRelayActuator {
public:
    TerraPump(tposi_t actuatorIndex,
              TerraDigitalPin outputPin,
              Terra_ActuatorType actuatorType = Terra_ActuatorType_Pump,
              int classTypeIn = RelayPump);
    TerraPump(const TerraActuatorData *dataIn);

    void setMaxContinuousRuntime(uint32_t maxMs) { _maxContinuousMs = maxMs; bumpRevisionIfNeeded(); }
    inline uint32_t getMaxContinuousRuntime() const { return _maxContinuousMs; }
    virtual void update(uint32_t now = millis()) override;

protected:
    uint32_t _maxContinuousMs;                              // Maximum continuous runtime
    uint32_t _startedAt;                                    // Continuous-run start timestamp

    virtual void saveToData(TerraData *dataOut) const override;
};

// Sump Pump Actuator
class TerraSumpPump : public TerraPump {
public:
    TerraSumpPump(tposi_t actuatorIndex,
                  TerraDigitalPin outputPin);
    TerraSumpPump(const TerraActuatorData *dataIn);

    bool configureLevels(float startPercent, float stopPercent,
                         float alarmPercent = TERRA_SUMP_ALARM_LEVEL_PERCENT);
    template<class T> inline void setLevelSensor(const SharedPtr<T> &sensor) { _levelSensor.setObject(sensor); }
    inline TerraSensorAttachment &getLevelSensorAttachment() { return _levelSensor; }
    inline const TerraSensorAttachment &getLevelSensorAttachment() const { return _levelSensor; }
    virtual void update(uint32_t now = millis()) override;
    virtual void unresolveAny(TerraObject *object) override;

    inline float getStartLevelPercent() const { return _startLevelPercent; }
    inline float getStopLevelPercent() const { return _stopLevelPercent; }
    inline float getAlarmLevelPercent() const { return _alarmLevelPercent; }
    inline float getLastLevelPercent() const { return _lastLevelPercent; }
    inline bool hasValidLevel() const { return _levelValid; }
    inline bool hasHighWaterAlarm() const { return _highWaterAlarm; }

protected:
    TerraSensorAttachment _levelSensor;                     // Sump level sensor
    float _startLevelPercent;                               // Pump start level
    float _stopLevelPercent;                                // Pump stop level
    float _alarmLevelPercent;                               // High-water alarm level
    float _lastLevelPercent;                                // Last valid level
    bool _levelValid;                                       // Valid level state
    bool _highWaterAlarm;                                   // High-water alarm state

    virtual void saveToData(TerraData *dataOut) const override;
};

class TerraValve : public TerraRelayActuator {
public:
    TerraValve(tposi_t actuatorIndex, TerraDigitalPin outputPin)
        : TerraRelayActuator(Terra_ActuatorType_Valve, actuatorIndex, outputPin) { ; }
    inline void open(millis_t duration = (millis_t)-1) { setOutput(1.0f, duration); }
    inline void close() { off(); }
};

class TerraDiverter : public TerraRelayActuator {
public:
    TerraDiverter(tposi_t actuatorIndex, TerraDigitalPin outputPin)
        : TerraRelayActuator(Terra_ActuatorType_Diverter, actuatorIndex, outputPin) { ; }
    inline void routePrimary() { off(); }
    inline void routeSecondary() { setOutput(1.0f); }
};

class TerraHeater : public TerraRelayActuator {
public:
    TerraHeater(tposi_t actuatorIndex, TerraDigitalPin outputPin)
        : TerraRelayActuator(Terra_ActuatorType_Heater, actuatorIndex, outputPin) { ; }
};

class TerraCirculator : public TerraPump {
public:
    TerraCirculator(tposi_t actuatorIndex, TerraDigitalPin outputPin)
        : TerraPump(actuatorIndex, outputPin, Terra_ActuatorType_Circulator) { ; }
};

#endif // /ifndef TerraActuators_H
