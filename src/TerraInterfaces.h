/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Interfaces
*/

#ifndef TerraInterfaces_H
#define TerraInterfaces_H

struct TerraJSONSerializableInterface;

class TerraObjInterface;
class TerraUIInterface;
class TerraRTCInterface;

struct TerraDigitalInputPinInterface;
struct TerraDigitalOutputPinInterface;
struct TerraAnalogInputPinInterface;
struct TerraAnalogOutputPinInterface;

class TerraAirConcentrateUnitsInterfaceStorage;
class TerraFlowRateUnitsInterfaceStorage;
class TerraMeasurementUnitsInterface;
template <size_t N = 1> class TerraMeasurementUnitsStorage;
class TerraMeasurementUnitsInterfaceStorageSingle;
class TerraMeasurementUnitsInterfaceStorageDouble;
class TerraMeasurementUnitsInterfaceStorageTriple;
class TerraPowerUnitsInterfaceStorage;
class TerraTemperatureUnitsInterfaceStorage;
class TerraVolumeUnitsInterfaceStorage;

class TerraActuatorObjectInterface;
class TerraSensorObjectInterface;
class TerraReservoirObjectInterface;
class TerraRailObjectInterface;
class TerraBalancerObjectInterface;
class TerraTriggerObjectInterface;

class TerraPumpObjectInterface;

class TerraParentActuatorAttachmentInterface;
class TerraParentSensorAttachmentInterface;
class TerraParentReservoirAttachmentInterface;
class TerraParentRailAttachmentInterface;

class TerraWaterReservoirAttachmentInterface;
class TerraThermalReservoirAttachmentInterface;

class TerraSensorAttachmentInterface;
class TerraAirCO2SensorAttachmentInterface;
class TerraAirTemperatureSensorAttachmentInterface;
class TerraMediumTemperatureSensorAttachmentInterface;
class TerraPowerProductionSensorAttachmentInterface;
class TerraPowerUsageSensorAttachmentInterface;
class TerraWaterFlowRateSensorAttachmentInterface;
class TerraWaterTemperatureSensorAttachmentInterface;
class TerraWaterVolumeSensorAttachmentInterface;

class TerraTriggerAttachmentInterface;
class TerraFilledTriggerAttachmentInterface;
class TerraEmptyTriggerAttachmentInterface;
class TerraLimitTriggerAttachmentInterface;

#include "Terraduino.h"

// JSON Serializable Interface
struct TerraJSONSerializableInterface {
    // Given a JSON element to fill in, writes self to JSON format.
    virtual void toJSONObject(JsonObject &objectOut) const = 0;

    // Given a JSON element to read from, reads overtop self from JSON format.
    virtual void fromJSONObject(JsonObjectConst &objectIn) = 0;
};


// Object Interface
class TerraObjInterface {
public:
    virtual void unresolveAny(TerraObject *obj) = 0;

    virtual TerraIdentity getId() const = 0;
    virtual tkey_t getKey() const = 0;
    virtual String getKeyString() const = 0;
    virtual SharedPtr<TerraObjInterface> getSharedPtr() const = 0;
    virtual SharedPtr<TerraObjInterface> getSharedPtrFor(const TerraObjInterface *obj) const = 0;

    virtual bool isObject() const = 0;
    inline bool isSubObject() const { return !isObject(); }
};

// UI Interface
class TerraUIInterface {
public:
    virtual TerraUIData *init(TerraUIData *data = nullptr) = 0;
    virtual void begin() = 0;

    virtual void setNeedsRedraw() = 0;
};

// RTC Module Interface
class TerraRTCInterface {
public:
    virtual ~TerraRTCInterface() { ; }
    virtual bool begin(TwoWire *wireInstance) = 0;
    virtual void adjust(const DateTime &dt) = 0;
    virtual bool lostPower(void) = 0;
    virtual DateTime now() = 0;
};


// Digital Input Pin Interface
struct TerraDigitalInputPinInterface {
    virtual ard_pinstatus_t digitalRead() = 0;
    inline int get() { return digitalRead(); }
};

// Digital Output Pin Interface
struct TerraDigitalOutputPinInterface {
    virtual void digitalWrite(ard_pinstatus_t status) = 0;
    inline void set(ard_pinstatus_t status) { digitalWrite(status); }
};

// Analog Input Pin Interface
struct TerraAnalogInputPinInterface {
    virtual float analogRead() = 0;
    virtual int analogRead_raw() = 0;
    inline float get() { return analogRead(); }
    inline int get_raw() { return analogRead_raw(); }
};

// Analog Output Pin Interface
struct TerraAnalogOutputPinInterface {
    virtual void analogWrite(float amount) = 0;
    virtual void analogWrite_raw(int amount) = 0;
    inline void set(float amount) { analogWrite(amount); }
    inline void set_raw(int amount) { analogWrite_raw(amount); }
};


// Air Concentrate Units Interface + Storage
class TerraAirConcentrateUnitsInterfaceStorage {
public:
    virtual void setAirConcentrateUnits(Terra_UnitsType airConcentrateUnits) = 0;
    inline Terra_UnitsType getAirConcentrateUnits() const { return _airConcUnits; }

protected:
    Terra_UnitsType _airConcUnits;
    inline TerraAirConcentrateUnitsInterfaceStorage(Terra_UnitsType airConcentrateUnits = Terra_UnitsType_Undefined) : _airConcUnits(airConcentrateUnits) { ; }
};

// Flow Rate Units Interface + Storage
class TerraFlowRateUnitsInterfaceStorage {
public:
    virtual void setFlowRateUnits(Terra_UnitsType flowRateUnits) = 0;
    inline Terra_UnitsType getFlowRateUnits() const { return _flowRateUnits; }
    inline Terra_UnitsType getVolumeUnits() const;

protected:
    Terra_UnitsType _flowRateUnits;
    inline TerraFlowRateUnitsInterfaceStorage(Terra_UnitsType flowRateUnits = Terra_UnitsType_Undefined) : _flowRateUnits(flowRateUnits) { ; }
};

// Measure Units Interface
class TerraMeasurementUnitsInterface {
public:
    virtual void setMeasurementUnits(Terra_UnitsType measurementUnits, uint8_t measurementRow = 0) = 0;
    virtual Terra_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const = 0;

    inline Terra_UnitsType getRateUnits(uint8_t measurementRow = 0) const;
    inline Terra_UnitsType getBaseUnits(uint8_t measurementRow = 0) const;
};

// Measure Units Storage
template <size_t N> class TerraMeasurementUnitsStorage {
protected:
    Terra_UnitsType _measurementUnits[N];
    inline TerraMeasurementUnitsStorage(Terra_UnitsType measurementUnits = Terra_UnitsType_Undefined) { for (size_t i = 0; i < N; ++i) { _measurementUnits[i] = measurementUnits; } }
};

// Single Measure Units Interface + Storage
class TerraMeasurementUnitsInterfaceStorageSingle : public TerraMeasurementUnitsInterface, public TerraMeasurementUnitsStorage<1> {
protected:
    inline TerraMeasurementUnitsInterfaceStorageSingle(Terra_UnitsType measurementUnits = Terra_UnitsType_Undefined) : TerraMeasurementUnitsStorage<1>(measurementUnits) { ; }
};

// Double Measure Units Interface + Storage
class TerraMeasurementUnitsInterfaceStorageDouble : public TerraMeasurementUnitsInterface, public TerraMeasurementUnitsStorage<2> {
protected:
    inline TerraMeasurementUnitsInterfaceStorageDouble(Terra_UnitsType measurementUnits = Terra_UnitsType_Undefined) : TerraMeasurementUnitsStorage<2>(measurementUnits) { ; }
};

// Triple Measure Units Interface + Storage
class TerraMeasurementUnitsInterfaceStorageTriple : public TerraMeasurementUnitsInterface, public TerraMeasurementUnitsStorage<3> {
protected:
    inline TerraMeasurementUnitsInterfaceStorageTriple(Terra_UnitsType measurementUnits = Terra_UnitsType_Undefined) : TerraMeasurementUnitsStorage<3>(measurementUnits) { ; }
};

// Power Units Interface + Storage
class TerraPowerUnitsInterfaceStorage {
public:
    virtual void setPowerUnits(Terra_UnitsType powerUnits) = 0;
    inline Terra_UnitsType getPowerUnits() const { return _powerUnits; }

protected:
    Terra_UnitsType _powerUnits;
    inline TerraPowerUnitsInterfaceStorage(Terra_UnitsType powerUnits = Terra_UnitsType_Undefined) : _powerUnits(powerUnits) { ; }
};

// Temperature Units Interface + Storage
class TerraTemperatureUnitsInterfaceStorage {
public:
    virtual void setTemperatureUnits(Terra_UnitsType temperatureUnits) = 0;
    inline Terra_UnitsType getTemperatureUnits() const { return _tempUnits; }

protected:
    Terra_UnitsType _tempUnits;
    inline TerraTemperatureUnitsInterfaceStorage(Terra_UnitsType temperatureUnits = Terra_UnitsType_Undefined) : _tempUnits(temperatureUnits) { ; }
};

// Volume Units Interface + Storage
class TerraVolumeUnitsInterfaceStorage {
public:
    virtual void setVolumeUnits(Terra_UnitsType volumeUnits) = 0;
    inline Terra_UnitsType getVolumeUnits() const { return _volumeUnits; }
    inline Terra_UnitsType getFlowRateUnits() const;

protected:
    Terra_UnitsType _volumeUnits;
    inline TerraVolumeUnitsInterfaceStorage(Terra_UnitsType volumeUnits = Terra_UnitsType_Undefined) : _volumeUnits(volumeUnits) { ; }
};

// Actuator Object Interface
class TerraActuatorObjectInterface {
public:
    virtual bool getCanEnable() = 0;
    virtual float getDriveIntensity() const = 0;
    virtual bool isEnabled(float tolerance = 0.0f) const = 0;

    virtual void setContinuousPowerUsage(TerraSingleMeasurement contPowerUsage) = 0;
    virtual const TerraSingleMeasurement &getContinuousPowerUsage() = 0;
    inline void setContinuousPowerUsage(float contPowerUsage, Terra_UnitsType contPowerUsageUnits = Terra_UnitsType_Undefined);

protected:
    virtual void _enableActuator(float intensity = 1.0) = 0;
    virtual void _disableActuator() = 0;
};

// Sensor Object Interface
class TerraSensorObjectInterface {
public:
    virtual bool takeMeasurement(bool force = false) = 0;
    virtual const TerraMeasurement *getMeasurement(bool poll = false) = 0;
    virtual bool isTakingMeasurement() const = 0;
    virtual bool needsPolling(tframe_t allowance = 0) const = 0;
};

// Reservoir Object Interface
class TerraReservoirObjectInterface {
public:
    virtual bool canActivate(TerraActuator *actuator) = 0;
    virtual float getLevel(bool poll = false) = 0;

    virtual Terra_ResourceState getState(bool poll = false) = 0;
    virtual bool isFilled(bool poll = false) = 0;
    virtual bool isHigh(bool poll = false) = 0;
    virtual bool isLow(bool poll = false) = 0;
    virtual bool isEmpty(bool poll = false) = 0;
};

// Rail Object Interface
class TerraRailObjectInterface {
public:
    virtual bool canActivate(TerraActuator *actuator) = 0;
    virtual float getCapacity(bool poll = false) = 0;
};

// Balancer Object Interface
class TerraBalancerObjectInterface {
public:
    virtual void setTargetSetpoint(float targetSetpoint) = 0;
    virtual Terra_BalancingState getBalancingState(bool poll = false) = 0;
    inline bool isBalanced(bool poll = false) { return getBalancingState(poll) == Terra_BalancingState_Balanced; }
};

// Trigger Object Interface
class TerraTriggerObjectInterface {
public:
    virtual Terra_TriggerState getTriggerState(bool poll = false) = 0;
    inline bool isTriggered(bool poll = false) { return getTriggerState(poll) == Terra_TriggerState_Triggered; }
};


// Pump Object Interface
class TerraPumpObjectInterface {
public:
    virtual bool canPump(float volume, Terra_UnitsType volumeUnits = Terra_UnitsType_Undefined) = 0;
    virtual TerraActivationHandle pump(float volume, Terra_UnitsType volumeUnits = Terra_UnitsType_Undefined) = 0;
    virtual bool canPump(millis_t time) = 0;
    virtual TerraActivationHandle pump(millis_t time) = 0;

    virtual TerraAttachment &getSourceReservoirAttachment() = 0;
    template<class U> inline void setSourceReservoir(U reservoir);
    template<class U = TerraReservoir> inline SharedPtr<U> getSourceReservoir();

    virtual TerraAttachment &getDestinationReservoirAttachment() = 0;
    template<class U> inline void setDestinationReservoir(U reservoir);
    template<class U = TerraReservoir> inline SharedPtr<U> getDestinationReservoir();

    virtual void setContinuousFlowRate(TerraSingleMeasurement contFlowRate) = 0;
    virtual const TerraSingleMeasurement &getContinuousFlowRate() = 0;
    inline void setContinuousFlowRate(float contFlowRate, Terra_UnitsType contFlowRateUnits = Terra_UnitsType_Undefined);

    inline bool isSourceReservoirEmpty(bool poll = false);
    inline bool isDestinationReservoirFilled(bool poll = false);

protected:
    virtual void handlePumpTime(millis_t time) = 0;
};


// Parent Actuator Attachment Interface
class TerraParentActuatorAttachmentInterface {
public:
    virtual TerraAttachment &getParentActuatorAttachment() = 0;

    template<class U> inline void setParentActuator(U actuator);
    template<class U = TerraActuator> inline SharedPtr<U> getParentActuator();
};

// Parent Sensor Attachment Interface
class TerraParentSensorAttachmentInterface {
public:
    virtual TerraAttachment &getParentSensorAttachment() = 0;

    template<class U> inline void setParentSensor(U sensor);
    template<class U = TerraSensor> inline SharedPtr<U> getParentSensor();
};

// Parent Reservoir Attachment Interface
class TerraParentReservoirAttachmentInterface {
public:
    virtual TerraAttachment &getParentReservoirAttachment() = 0;

    template<class U> inline void setParentReservoir(U reservoir);
    template<class U = TerraReservoir> inline SharedPtr<U> getParentReservoir();
};

// Parent Rail Attachment Interface
class TerraParentRailAttachmentInterface {
public:
    virtual TerraAttachment &getParentRailAttachment() = 0;

    template<class U> inline void setParentRail(U rail);
    template<class U = TerraRail> inline SharedPtr<U> getParentRail();
};


// Water Reservoir Attachment Interface
class TerraWaterReservoirAttachmentInterface {
public:
    virtual TerraAttachment &getWaterReservoirAttachment() = 0;

    template<class U> inline void setWaterReservoir(U reservoir);
    template<class U = TerraWaterReservoir> inline SharedPtr<U> getWaterReservoir(bool poll = false);
};

// Head Reservoir Attachment Interface
class TerraThermalReservoirAttachmentInterface {
public:
    virtual TerraAttachment &getThermalReservoirAttachment() = 0;

    template<class U> inline void setThermalReservoir(U reservoir);
    template<class U = TerraThermalReservoir> inline SharedPtr<U> getThermalReservoir(bool poll = false);
};


// Abstract Sensor Attachment Interface
class TerraSensorAttachmentInterface {
    virtual TerraSensorAttachment &getSensorAttachment() = 0;

    template<class U> inline void setSensor(U sensor);
    template<class U = TerraSensor> inline SharedPtr<U> getSensor(bool poll = false);
};

// Air CO2 Sensor Attachment Interface
class TerraAirCO2SensorAttachmentInterface {
public:
    virtual TerraSensorAttachment &getAirCO2SensorAttachment() = 0;

    template<class U> inline void setAirCO2Sensor(U sensor);
    template<class U = TerraSensor> inline SharedPtr<U> getAirCO2Sensor(bool poll = false);
};

// Air Temperature Sensor Attachment Interface
class TerraAirTemperatureSensorAttachmentInterface {
public:
    virtual TerraSensorAttachment &getAirTemperatureSensorAttachment() = 0;

    template<class U> inline void setAirTemperatureSensor(U sensor);
    template<class U = TerraSensor> inline SharedPtr<U> getAirTemperatureSensor(bool poll = false);
};

// Thermal Medium Temperature Sensor Attachment Interface
class TerraMediumTemperatureSensorAttachmentInterface {
public:
    virtual TerraSensorAttachment &getMediumTemperatureSensorAttachment() = 0;

    template<class U> inline void setMediumTemperatureSensor(U sensor);
    template<class U = TerraSensor> inline SharedPtr<U> getMediumTemperatureSensor(bool poll = false);
};

// Power Production Sensor Attachment Interface
class TerraPowerProductionSensorAttachmentInterface {
public:
    virtual TerraSensorAttachment &getPowerProductionSensorAttachment() = 0;

    template<class U> inline void setPowerProductionSensor(U sensor);
    template<class U = TerraSensor> inline SharedPtr<U> getPowerProductionSensor(bool poll = false);
};

// Power Usage Sensor Attachment Interface
class TerraPowerUsageSensorAttachmentInterface {
public:
    virtual TerraSensorAttachment &getPowerUsageSensorAttachment() = 0;

    template<class U> inline void setPowerUsageSensor(U sensor);
    template<class U = TerraSensor> inline SharedPtr<U> getPowerUsageSensor(bool poll = false);
};

// Liquid Flow Rate Sensor Attachment Interface
class TerraWaterFlowRateSensorAttachmentInterface {
public:
    virtual TerraSensorAttachment &getFlowRateSensorAttachment() = 0;

    template<class U> inline void setFlowRateSensor(U sensor);
    template<class U = TerraSensor> inline SharedPtr<U> getFlowRateSensor(bool poll = false);
};

// Water Temperature Sensor Attachment Interface
class TerraWaterTemperatureSensorAttachmentInterface {
public:
    virtual TerraSensorAttachment &getWaterTemperatureSensorAttachment() = 0;

    template<class U> inline void setWaterTemperatureSensor(U sensor);
    template<class U = TerraSensor> inline SharedPtr<U> getWaterTemperatureSensor(bool poll = false);
};

// Liquid Volume Sensor Attachment Interface
class TerraWaterVolumeSensorAttachmentInterface {
public:
    virtual TerraSensorAttachment &getWaterVolumeSensorAttachment() = 0;

    template<class U> inline void setWaterVolumeSensor(U sensor);
    template<class U = TerraSensor> inline SharedPtr<U> getWaterVolumeSensor(bool poll = false);
};


// Abstract Trigger Attachment Interface
class TerraTriggerAttachmentInterface {
    virtual TerraTriggerAttachment &getTriggerAttachment() = 0;

    template<class U> inline void setTrigger(U trigger);
    template<class U = TerraTrigger> inline SharedPtr<U> getTrigger(bool poll = false);
};

// Filled Trigger Attachment Interface
class TerraFilledTriggerAttachmentInterface {
public:
    virtual TerraTriggerAttachment &getFilledTriggerAttachment() = 0;

    template<class U> inline void setFilledTrigger(U trigger);
    template<class U = TerraTrigger> inline SharedPtr<U>getFilledTrigger(bool poll = false);
};

// High Trigger Attachment Interface
class TerraHighTriggerAttachmentInterface {
public:
    virtual TerraTriggerAttachment &getHighTriggerAttachment() = 0;

    template<class U> inline void setHighTrigger(U trigger);
    template<class U = TerraTrigger> inline SharedPtr<U>getHighTrigger(bool poll = false);
};

// Low Trigger Attachment Interface
class TerraLowTriggerAttachmentInterface {
public:
    virtual TerraTriggerAttachment &getLowTriggerAttachment() = 0;

    template<class U> inline void setLowTrigger(U trigger);
    template<class U = TerraTrigger> inline SharedPtr<U>getLowTrigger(bool poll = false);
};

// Empty Trigger Attachment Interface
class TerraEmptyTriggerAttachmentInterface {
public:
    virtual TerraTriggerAttachment &getEmptyTriggerAttachment() = 0;

    template<class U> inline void setEmptyTrigger(U trigger);
    template<class U = TerraTrigger> inline SharedPtr<U>getEmptyTrigger(bool poll = false);
};

// Limit Trigger Attachment Interface
class TerraLimitTriggerAttachmentInterface {
public:
    virtual TerraTriggerAttachment &getLimitTriggerAttachment() = 0;

    template<class U> inline void setLimitTrigger(U trigger);
    template<class U = TerraTrigger> inline SharedPtr<U>getLimitTrigger(bool poll = false);
};

#endif // /ifndef TerraInterfaces_H
