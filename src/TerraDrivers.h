/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Drivers
*/

#ifndef TerraDrivers_H
#define TerraDrivers_H

#include "TerraMeasurements.h"
#include "TerraPins.h"
#include "TerraCallback.hh"

class TerraInputDriver {
public:
    virtual ~TerraInputDriver() { }
    virtual void begin() { }
    virtual TerraMeasurement read(uint32_t now = terraMillis()) = 0;
    virtual bool getPinSetup(TerraPinSetup &setup) const { (void)setup; return false; }
    virtual bool getCalibration(float &rawMinimum, float &rawMaximum,
                                float &valueMinimum, float &valueMaximum) const {
        (void)rawMinimum; (void)rawMaximum; (void)valueMinimum; (void)valueMaximum;
        return false;
    }
};

class TerraOutputDriver {
public:
    virtual ~TerraOutputDriver() { }
    virtual void begin() { }
    virtual bool write(float value) = 0;
    virtual bool getPinSetup(TerraPinSetup &setup) const { (void)setup; return false; }
    virtual int getMaximumRaw() const { return 0; }
};

class TerraCallbackInputDriver : public TerraInputDriver {
public:
    TerraCallbackInputDriver(TerraReadCallback callback = nullptr,
                             void *context = nullptr,
                             Terra_Unit unit = Terra_Unit_Raw);  // Measurement unit
    void setCallback(TerraReadCallback callback, void *context = nullptr);
    TerraMeasurement read(uint32_t now = terraMillis()) override;

protected:
    TerraReadCallback _callback;                            // Configured callback
    void *_context;
    Terra_Unit _unit;                                       // Driver measurement unit
};

class TerraAnalogInputDriver : public TerraInputDriver {
public:
    TerraAnalogInputDriver(uint8_t pin = TERRA_INVALID_PIN,
                           Terra_Unit unit = Terra_Unit_Raw);  // Measurement unit
    void begin() override;
    bool setCalibration(float rawMinimum, float rawMaximum,
                        float valueMinimum, float valueMaximum);
    TerraMeasurement read(uint32_t now = terraMillis()) override;
    bool getPinSetup(TerraPinSetup &setup) const override;
    bool getCalibration(float &rawMinimum, float &rawMaximum,
                        float &valueMinimum, float &valueMaximum) const override;

protected:
    TerraAnalogPin _pin;                                    // Configured I/O pin
    Terra_Unit _unit;                                       // Driver measurement unit
    float _rawMinimum;                                      // Raw calibration minimum
    float _rawMaximum;                                      // Raw calibration maximum
    float _valueMinimum;                                    // Calibrated output minimum
    float _valueMaximum;                                    // Calibrated output maximum
    bool _calibrated;                                       // Calibration configured flag
};

class TerraDigitalInputDriver : public TerraInputDriver {
public:
    TerraDigitalInputDriver(const TerraPinSetup &setup = TerraPinSetup(),
                            Terra_Unit unit = Terra_Unit_Raw);  // Measurement unit
    void begin() override;
    TerraMeasurement read(uint32_t now = terraMillis()) override;
    bool getPinSetup(TerraPinSetup &setup) const override { setup = _pin.getSetup(); return setup.isValid(); }

protected:
    TerraDigitalPin _pin;                                   // Configured I/O pin
    Terra_Unit _unit;                                       // Driver measurement unit
};

class TerraCallbackOutputDriver : public TerraOutputDriver {
public:
    TerraCallbackOutputDriver(TerraWriteCallback callback = nullptr,
                              void *context = nullptr);
    void setCallback(TerraWriteCallback callback, void *context = nullptr);
    bool write(float value) override;

protected:
    TerraWriteCallback _callback;                           // Configured callback
    void *_context;
};

class TerraDigitalOutputDriver : public TerraOutputDriver {
public:
    TerraDigitalOutputDriver(const TerraPinSetup &setup = TerraPinSetup());
    void begin() override;
    bool write(float value) override;
    bool getLastState() const { return _pin.getLastState(); }
    bool getPinSetup(TerraPinSetup &setup) const override { setup = _pin.getSetup(); return setup.isValid(); }

protected:
    TerraDigitalPin _pin;                                   // Configured I/O pin
};

class TerraAnalogOutputDriver : public TerraOutputDriver {
public:
    TerraAnalogOutputDriver(uint8_t pin = TERRA_INVALID_PIN, int maximumRaw = 255);
    void begin() override;
    bool write(float value) override;
    int getLastRawValue() const { return _pin.getLastValue(); }
    bool getPinSetup(TerraPinSetup &setup) const override {
        setup = TerraPinSetup(_pin.getPin(), Terra_PinMode_Analog_Output, false);
        return setup.isValid();
    }
    int getMaximumRaw() const override { return _maximumRaw; }

protected:
    TerraAnalogPin _pin;                      ///< Analog/PWM output pin
    int _maximumRaw;                          ///< Raw value corresponding to full-scale output
};

#endif // /ifndef TerraDrivers_H
