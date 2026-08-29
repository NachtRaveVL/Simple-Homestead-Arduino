/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Actuators
*/

#ifndef TerraActuators_H
#define TerraActuators_H

class TerraActuator;
class TerraRelayActuator;
class TerraRelayPumpActuator;
class TerraVariableActuator;
//class TerraVariablePumpActuator;

struct TerraActuatorData;
struct TerraPumpActuatorData;

#include "Terraduino.h"
#include "TerraDatas.h"
#include "TerraActivation.h"

// Creates actuator object from passed actuator data (return ownership transfer - user code *must* delete returned object)
extern TerraActuator *newActuatorObjectFromData(const TerraActuatorData *dataIn);


// Actuator Base
// Base controlled output using a shared output driver and resident activation requests.
// This is the base class for all actuators, which defines how the actuator is identified,
// where it lives, and what it's attached to.
class TerraActuator : public TerraObject,
                      public TerraActuatorObjectInterface,
                      public TerraParentRailAttachmentInterface,
                      public TerraParentReservoirAttachmentInterface {
public:
    const enum : signed char { Relay, RelayPump, Variable, VariablePump, Unknown = -1 } classType; // Actuator class type
    inline bool isRelayClass() const { return classType == Relay; }
    inline bool isRelayPumpClass() const { return classType == RelayPump; }
    inline bool isVariableClass() const { return classType == Variable; }
    inline bool isVariablePumpClass() const { return classType == VariablePump; }
    inline bool isAnyBinaryClass() const { return isRelayClass() || isRelayPumpClass(); }
    inline bool isAnyVariableClass() const { return isVariableClass() || isVariablePumpClass(); }
    inline bool isAnyPumpClass() const { return isRelayPumpClass() || isVariablePumpClass(); }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    TerraActuator(Terra_ActuatorType actuatorType,
                  tposi_t actuatorIndex,
                  int classTypeIn = Unknown);
    TerraActuator(const TerraActuatorData *dataIn);

    virtual void update(uint32_t now = millis()) override;

    virtual bool getCanEnable() override;

    // Activating actuators is done through activation handles, which must stay memory
    // resident in order for the actuator to pick up and process it. Enablement mode
    // affects how handles are processed - in parallel, or in serial - and what the
    // applied output is. See TerraActuatorAttachment for an abstraction of this process.
    inline TerraActivationHandle enableActuator(Terra_DirectionMode direction, float intensity = 1.0f, millis_t duration = (millis_t)-1, bool force = false) { return TerraActivationHandle(::getSharedPtr<TerraActuator>(this), direction, intensity, duration, force); }
    inline TerraActivationHandle enableActuator(float value, millis_t duration = (millis_t)-1, bool force = false) { return enableActuator(Terra_DirectionMode_Forward, calibrationInvTransform(value), duration, force); }
    inline TerraActivationHandle enableActuator(millis_t duration = (millis_t)-1, bool force = false) { return enableActuator(Terra_DirectionMode_Forward, 1.0f, duration, force); }

    void setEnableMode(Terra_EnableMode mode);
    inline Terra_EnableMode getEnableMode() const { return _enableMode; }

    inline bool isSerialMode() const { return _enableMode >= Terra_EnableMode_Serial; }
    inline bool isPumpType() const { return isAnyPumpClass(); }
    inline bool isBidirectionalType() const { return false; }

    virtual void setContinuousPowerUsage(TerraSingleMeasurement contPowerUsage) override;
    virtual const TerraSingleMeasurement &getContinuousPowerUsage() override;

    virtual TerraAttachment &getParentRailAttachment() override;
    virtual TerraAttachment &getParentReservoirAttachment() override;

    void setUserCalibrationData(const TerraCalibrationData *userCalibrationData);
    inline const TerraCalibrationData *getUserCalibrationData() const { return _calibrationData; }

    // Transformation methods that convert from normalized driving intensity/driver value to calibration units
    inline float calibrationTransform(float value) const { return _calibrationData ? _calibrationData->transform(value) : value; }
    inline void calibrationTransform(float *valueInOut, Terra_UnitsType *unitsOut = nullptr) const { if (valueInOut && _calibrationData) { _calibrationData->transform(valueInOut, unitsOut); } }
    inline TerraSingleMeasurement calibrationTransform(TerraSingleMeasurement measurement) { return _calibrationData ? TerraSingleMeasurement(_calibrationData->transform(measurement.value), _calibrationData->calibrationUnits, measurement.timestamp, measurement.frame) : measurement; }
    inline void calibrationTransform(TerraSingleMeasurement *measurementInOut) const { if (measurementInOut && _calibrationData) { _calibrationData->transform(measurementInOut); } }

    // Transformation methods that convert from calibration units to normalized driving intensity/driver value
    inline float calibrationInvTransform(float value) const { return _calibrationData ? _calibrationData->inverseTransform(value) : value; }
    inline void calibrationInvTransform(float *valueInOut, Terra_UnitsType *unitsOut = nullptr) const { if (valueInOut && _calibrationData) { _calibrationData->inverseTransform(valueInOut, unitsOut); } }
    inline TerraSingleMeasurement calibrationInvTransform(TerraSingleMeasurement measurement) { return _calibrationData ? TerraSingleMeasurement(_calibrationData->inverseTransform(measurement.value), _calibrationData->calibrationUnits, measurement.timestamp, measurement.frame) : measurement; }
    inline void calibrationInvTransform(TerraSingleMeasurement *measurementInOut) const { if (measurementInOut && _calibrationData) { _calibrationData->inverseTransform(measurementInOut); } }

    inline float getCalibratedValue() const { return calibrationTransform(getDriveIntensity()); }

    virtual float getDriveIntensity() const = 0;
    virtual bool isEnabled(float tolerance = 0.0f) const = 0;

    inline Terra_ActuatorType getActuatorType() const { return _id.objTypeAs.actuatorType; }
    inline tposi_t getActuatorIndex() const { return _id.posIndex; }

    inline void setNeedsUpdate() { _needsUpdate = true; }
    inline bool needsUpdate() const { return _needsUpdate; }

    Signal<TerraActuator *, TERRA_ACTUATOR_SIGNAL_SLOTS> &getActivationSignal();

protected:
    bool _enabled;                                          // Actuator enabled state
    bool _needsUpdate;                                      // Activation-resolution stale flag
    Terra_EnableMode _enableMode;                           // Activation enablement mode
    Vector<TerraActivationHandle *> _handles;               // Active handles
    TerraSingleMeasurement _contPowerUsage;                 // Continuous power draw
    TerraAttachment _parentRail;                            // Parent power rail attachment
    TerraAttachment _parentReservoir;                       // Parent reservoir attachment
    const TerraCalibrationData *_calibrationData;           // Calibration data
    Signal<TerraActuator *, TERRA_ACTUATOR_SIGNAL_SLOTS> _activateSignal; // Activation update signal

    virtual TerraData *allocateData() const override;
    virtual void saveToData(TerraData *dataOut) const override;

    virtual void handleActivation();

    friend struct TerraActivationHandle;
};

// Relay Actuator
// This actuator acts as a standard on/off switch, typically paired with a variety of
// different equipment from pumps to fans and heaters.
class TerraRelayActuator : public TerraActuator {
public:
    TerraRelayActuator(Terra_ActuatorType actuatorType,
                       tposi_t actuatorIndex,
                       TerraDigitalPin outputPin,
                       int classTypeIn = Relay);
    TerraRelayActuator(const TerraActuatorData *dataIn);
    virtual ~TerraRelayActuator();

    virtual bool getCanEnable() override;
    virtual float getDriveIntensity() const override;
    virtual bool isEnabled(float tolerance = 0.0f) const override;

    inline const TerraDigitalPin &getOutputPin() const { return _outputPin; }

protected:
    TerraDigitalPin _outputPin;                             // Digital output pin

    virtual void saveToData(TerraData *dataOut) const override;

    virtual void _enableActuator(float intensity = 1.0f) override;
    virtual void _disableActuator() override;
};

// Relay Pump Actuator
// This actuator acts as a resource-transfer pump and attaches to both an input and output
// reservoir. Pumps using this class are on/off and do not contain any variable flow control,
// but can be paired with a flow sensor for more precise pumping calculations.
class TerraRelayPumpActuator : public TerraRelayActuator,
                               public TerraPumpObjectInterface,
                               public TerraFlowRateUnitsInterfaceStorage,
                               public TerraWaterFlowRateSensorAttachmentInterface {
public:
    TerraRelayPumpActuator(Terra_ActuatorType actuatorType,
                           tposi_t actuatorIndex,
                           TerraDigitalPin outputPin,
                           int classTypeIn = RelayPump);
    TerraRelayPumpActuator(const TerraPumpActuatorData *dataIn);

    virtual void update(uint32_t now = millis()) override;

    virtual bool getCanEnable() override;

    virtual bool canPump(float volume, Terra_UnitsType volumeUnits = Terra_UnitsType_Undefined) override;
    virtual TerraActivationHandle pump(float volume, Terra_UnitsType volumeUnits = Terra_UnitsType_Undefined) override;
    virtual bool canPump(millis_t time) override;
    virtual TerraActivationHandle pump(millis_t time) override;

    virtual void setFlowRateUnits(Terra_UnitsType flowRateUnits) override;

    virtual TerraAttachment &getSourceReservoirAttachment() override;
    virtual TerraAttachment &getDestinationReservoirAttachment() override;

    virtual void setContinuousFlowRate(TerraSingleMeasurement contFlowRate) override;
    virtual const TerraSingleMeasurement &getContinuousFlowRate() override;

    virtual TerraSensorAttachment &getFlowRateSensorAttachment() override;

protected:
    TerraSingleMeasurement _contFlowRate;                   // Continuous flow rate
    TerraSensorAttachment _flowRate;                        // Flow rate sensor attachment
    TerraAttachment _destReservoir;                         // Destination output reservoir

    float _pumpVolumeAccum;                                 // Accumulator for total volume of fluid pumped
    millis_t _pumpTimeStart;                                // Time millis pump was activated at
    millis_t _pumpTimeAccum;                                // Time millis pump has been accumulated up to

    virtual void saveToData(TerraData *dataOut) const override;

    virtual void handleActivation() override;

    virtual void handlePumpTime(millis_t time) override;
};

// Variable Actuator
// This actuator acts as a simple variable ranged dial, typically paired with a variety of
// different equipment that allows analog throttle or position control.
class TerraVariableActuator : public TerraActuator {
public:
    TerraVariableActuator(Terra_ActuatorType actuatorType,
                          tposi_t actuatorIndex,
                          TerraAnalogPin outputPin,
                          int classTypeIn = Variable);
    TerraVariableActuator(const TerraActuatorData *dataIn);
    virtual ~TerraVariableActuator();

    virtual bool getCanEnable() override;
    virtual float getDriveIntensity() const override;
    virtual bool isEnabled(float tolerance = 0.0f) const override;

    inline const TerraAnalogPin &getOutputPin() const { return _outputPin; }

protected:
    TerraAnalogPin _outputPin;                              // Analog/PWM output pin
    float _intensity;                                       // Current normalized output

    virtual void saveToData(TerraData *dataOut) const override;

    virtual void _enableActuator(float intensity = 1.0f) override;
    virtual void _disableActuator() override;
};

// Variable/Throttled Pump Actuator
// This actuator acts as a throttleable resource-transfer pump and attaches to both an input
// and output reservoir. Pumps using this class have variable flow control but also can be
// paired with a flow sensor for more precise pumping calculations.
//class TerraVariablePumpActuator : public TerraVariableActuator, public TerraPumpObjectInterface, public TerraWaterFlowRateSensorAttachmentInterface {
// TODO: Port alongside HydroVariablePumpActuator once Hydruino implements it.
//};

// Actuator Serialization Data
struct TerraActuatorData : public TerraObjectData {
    TerraPinData outputPin;                                 // Output pin
    Terra_EnableMode enableMode;                            // Activation enablement mode
    TerraMeasurementData contPowerUsage;                    // Continuous power usage
    char railName[TERRA_NAME_MAXSIZE];                      // Parent rail
    char reservoirName[TERRA_NAME_MAXSIZE];                 // Parent reservoir

    TerraActuatorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Pump Actuator Serialization Data
struct TerraPumpActuatorData : public TerraActuatorData {
    Terra_UnitsType flowRateUnits;                          // Flow rate units
    TerraMeasurementData contFlowRate;                      // Continuous flow rate
    char destReservoir[TERRA_NAME_MAXSIZE];                 // Destination reservoir
    char flowRateSensor[TERRA_NAME_MAXSIZE];                // Flow rate sensor

    TerraPumpActuatorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef TerraActuators_H
