/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Drivers
*/

#ifndef TerraDrivers_H
#define TerraDrivers_H

#include "TerraMeasurements.h"
#include "TerraPins.h"
#include "TerraCallback.hh"

typedef float (*TerraReadCallback)(void *context);
typedef void (*TerraWriteCallback)(void *context, float value);

// Input Driver Base
// Hardware/callback input bridge. Calibration remains sensor-owned.
class TerraInputDriver {
public:
    virtual ~TerraInputDriver() { ; }
    virtual void begin() { ; }
    virtual TerraSingleMeasurement read(uint32_t now = millis()) = 0;
    virtual bool getPinData(TerraPinData &dataOut) const { (void)dataOut; return false; }
};

// Output Driver Base
// Hardware/callback output bridge. Activation arbitration remains actuator-owned.
class TerraOutputDriver {
public:
    virtual ~TerraOutputDriver() { ; }
    virtual void begin() { ; }
    virtual bool write(float value) = 0;
    virtual bool getPinData(TerraPinData &dataOut) const { (void)dataOut; return false; }
    virtual int getMaximumRaw() const { return 0; }
};

class TerraCallbackInputDriver : public TerraInputDriver {
public:
    TerraCallbackInputDriver(TerraReadCallback callback = nullptr,
                             void *context = nullptr,
                             Terra_Unit units = Terra_Unit_Raw);
    void setCallback(TerraReadCallback callback, void *context = nullptr);
    virtual TerraSingleMeasurement read(uint32_t now = millis()) override;

protected:
    TerraReadCallback _callback;                            // Configured callback
    void *_context;                                         // Callback context, not owned
    Terra_Unit _units;                                      // Driver measurement units
};

class TerraAnalogInputDriver : public TerraInputDriver {
public:
    TerraAnalogInputDriver(uint8_t pin = TERRA_INVALID_PIN,
                           uint8_t bitRes = 10);
    virtual void begin() override;
    virtual TerraSingleMeasurement read(uint32_t now = millis()) override;
    virtual bool getPinData(TerraPinData &dataOut) const override;

protected:
    TerraAnalogPin _pin;                                    // Configured analog input pin
};

class TerraDigitalInputDriver : public TerraInputDriver {
public:
    TerraDigitalInputDriver(uint8_t pin = TERRA_INVALID_PIN,
                            bool activeLow = false,
                            Terra_PinMode pinMode = Terra_PinMode_Digital_Input,
                            Terra_Unit units = Terra_Unit_Raw);
    virtual void begin() override;
    virtual TerraSingleMeasurement read(uint32_t now = millis()) override;
    virtual bool getPinData(TerraPinData &dataOut) const override;

protected:
    TerraDigitalPin _pin;                                   // Configured digital input pin
    Terra_Unit _units;                                      // Driver measurement units
};

class TerraCallbackOutputDriver : public TerraOutputDriver {
public:
    TerraCallbackOutputDriver(TerraWriteCallback callback = nullptr,
                              void *context = nullptr);
    void setCallback(TerraWriteCallback callback, void *context = nullptr);
    virtual bool write(float value) override;

protected:
    TerraWriteCallback _callback;                           // Configured callback
    void *_context;                                         // Callback context, not owned
};

class TerraDigitalOutputDriver : public TerraOutputDriver {
public:
    TerraDigitalOutputDriver(uint8_t pin = TERRA_INVALID_PIN,
                             bool activeLow = false);
    virtual void begin() override;
    virtual bool write(float value) override;
    virtual bool getPinData(TerraPinData &dataOut) const override;

protected:
    TerraDigitalPin _pin;                                   // Configured digital output pin
};

class TerraAnalogOutputDriver : public TerraOutputDriver {
public:
    TerraAnalogOutputDriver(uint8_t pin = TERRA_INVALID_PIN,
                            uint8_t bitRes = 8);
    virtual void begin() override;
    virtual bool write(float value) override;
    virtual bool getPinData(TerraPinData &dataOut) const override;
    virtual int getMaximumRaw() const override;

protected:
    TerraAnalogPin _pin;                                    // Configured analog/PWM output pin
};

#endif // /ifndef TerraDrivers_H
