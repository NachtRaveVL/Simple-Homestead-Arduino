/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Actuators
*/

#ifndef TerraActuators_H
#define TerraActuators_H

#include "TerraObject.h"
#include "TerraActivation.h"
#include "TerraCallback.hh"
#include "TerraDrivers.h"

// Actuator Base
// Base controlled output using a callback or output driver.
class TerraActuator : public TerraObject {
public:
    TerraActuator(Terra_ActuatorType actuatorType = Terra_ActuatorType_Undefined,
                  uint32_t key = TERRA_INVALID_KEY,
                  const TerraString &name = TerraString());
    virtual ~TerraActuator();

    void setWriteCallback(TerraWriteCallback callback, void *context = nullptr);
    void setDriver(TerraOutputDriver *driver, bool takeOwnership = false);
    TerraOutputDriver *getDriver() const { return _driver; }
    void setEnabled(bool enabled) override;
    virtual void setOutput(float intensity, uint32_t durationMs = 0, uint32_t now = terraMillis());
    void setOutputRequests(const float *requests, uint8_t count, uint32_t durationMs = 0, uint32_t now = terraMillis());
    virtual void off();
    bool isActive() const { return _activation.isActive(); }
    float getOutput() const { return _activation.getIntensity(); }
    Terra_ActuatorType getActuatorType() const { return _actuatorType; }
    void setEnableMode(Terra_EnableMode mode) { _enableMode = mode; }
    Terra_EnableMode getEnableMode() const { return _enableMode; }
    void update(uint32_t now = terraMillis()) override;

protected:
    Terra_ActuatorType _actuatorType;                       // Actuator type
    TerraActivation _activation;                            // Current actuator activation
    TerraOutputDriver *_driver;
    bool _ownsDriver;                                       // Driver ownership flag
    TerraWriteCallback _writeCallback;                      // Actuator write callback
    Terra_EnableMode _enableMode;                           // Request aggregation mode
    void *_writeContext;
};

// Pump Actuator
// Adds a continuous-runtime safety limit to a controlled pump output.
class TerraPump : public TerraActuator {
public:
    TerraPump(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString())
        : TerraActuator(Terra_ActuatorType_Pump, key, name), _maxContinuousMs(0), _startedAt(0) { }
    void setMaxContinuousRuntime(uint32_t maxMs) { _maxContinuousMs = maxMs; }
    uint32_t getMaxContinuousRuntime() const { return _maxContinuousMs; }
    void setOutput(float intensity, uint32_t durationMs = 0, uint32_t now = terraMillis()) override;
    void update(uint32_t now = terraMillis()) override;

protected:
    uint32_t _maxContinuousMs;                              // Maximum continuous runtime, milliseconds
    uint32_t _startedAt;                                    // Continuous run start timestamp
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
    void routePrimary() { setOutput(0.0f); }
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
