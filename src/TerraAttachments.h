/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Attachment Points
*/

#ifndef TerraAttachments_H
#define TerraAttachments_H

class TerraDLinkObject;
class TerraAttachment;
template<class ParameterType, int Slots> class TerraSignalAttachment;
class TerraActuatorAttachment;
class TerraSensorAttachment;
class TerraTriggerAttachment;

#include "Terraduino.h"
#include "TerraObject.h"
#include "TerraMeasurements.h"
#include "TerraActivation.h"

// Delay/Dynamic Loaded/Linked Object Reference
// Simple class for delay loading objects that get references to others during object
// load. T should be a derived class of TerraObjInterface, with getId() method.
class TerraDLinkObject {
public:
    TerraDLinkObject();
    TerraDLinkObject(const TerraDLinkObject &obj);
    TerraDLinkObject &operator=(const TerraDLinkObject &obj);
    virtual ~TerraDLinkObject();

    inline bool isUnresolved() const { return !_obj; }
    inline bool isResolved() const { return (bool)_obj; }
    inline bool needsResolved() const { return isUnresolved() && isSet(); }
    inline bool resolve() { return isResolved() || (bool)getObject(); }
    void unresolve();
    template<class U> inline void unresolveIf(U obj) { if (operator==(obj)) { unresolve(); } }

    template<class U> inline void setObject(U obj) { operator=(obj); }
    template<class U = TerraObjInterface> inline SharedPtr<U> getObject() { return reinterpret_pointer_cast<U>(resolveObject()); }
    template<class U = TerraObjInterface> inline U *get() { return getObject<U>().get(); }

    inline TerraIdentity getId() const { return _obj ? _obj->getId() : (_keyStr ? TerraIdentity(_keyStr) : TerraIdentity(_key)); }
    inline tkey_t getKey() const { return _key; }
    inline String getKeyString() const { return _keyStr ? String(_keyStr) : (_obj ? _obj->getKeyString() : addressToString((uintptr_t)_key)); }
    inline bool isSet() const { return _key != tkey_none; }

    inline operator bool() const { return isResolved(); }
    inline TerraObjInterface *operator->() { return get(); }

    inline TerraDLinkObject &operator=(TerraIdentity rhs);
    inline TerraDLinkObject &operator=(const char *rhs);
    template<class U> inline TerraDLinkObject &operator=(SharedPtr<U> &rhs);
    inline TerraDLinkObject &operator=(const TerraObjInterface *rhs);
    inline TerraDLinkObject &operator=(const TerraAttachment *rhs);
    inline TerraDLinkObject &operator=(nullptr_t) { return operator=((TerraObjInterface *)nullptr); }

    inline bool operator==(const TerraIdentity &rhs) const { return _key == rhs.key; }
    inline bool operator==(const char *rhs) const { return _key == stringHash(rhs); }
    template<class U> inline bool operator==(const SharedPtr<U> &rhs) const { return _key == (rhs ? rhs->getKey() : tkey_none); }
    inline bool operator==(const TerraObjInterface *rhs) const { return _key == (rhs ? rhs->getKey() : tkey_none); }
    inline bool operator==(nullptr_t) const { return _key == tkey_none; }

protected:
    tkey_t _key;                                            // Object key
    SharedPtr<TerraObjInterface> _obj;                      // Shared pointer to object
    const char *_keyStr;                                    // Copy of id.keyString (if not resolved, or unresolved)

private:
    SharedPtr<TerraObjInterface> resolveObject();
    friend class Terraduino;
    friend class TerraAttachment;
};

// Simple Attachment Point Base
// This attachment registers the parent object with the linked object's linkages upon
// dereference / unregisters the parent object at time of destruction or reassignment.
class TerraAttachment : public TerraSubObject {
public:
    TerraAttachment(TerraObjInterface *parent = nullptr, tposi_t subIndex = 0);
    TerraAttachment(const TerraAttachment &attachment);
    TerraAttachment &operator=(const TerraAttachment &attachment);
    virtual ~TerraAttachment();

    // Attaches object and any relevant signaling mechanisms. Derived classes should call base class's method first.
    virtual void attachObject();
    // Detaches object from any relevant signaling mechanism. Derived classes should call base class's method last.
    virtual void detachObject();

    // Attachment updater. Overridden by derived classes. May only update owned sub-objects (main objects are owned/updated by run system).
    virtual void updateIfNeeded(bool poll = false);

    inline bool isUnresolved() const { return !_obj; }
    inline bool isResolved() const { return (bool)_obj; }
    inline bool needsResolved() const { return _obj.needsResolved(); }
    inline bool resolve() { return isResolved() || (bool)getObject(); }
    inline void unresolve() { _obj.unresolve(); } 
    template<class U> inline void unresolveIf(U obj) { _obj.unresolveIf(obj); }

    template<class U> void setObject(U obj, bool modify = true);
    template<class U> inline void initObject(U obj) { setObject(obj, false); }
    template<class U = TerraObjInterface> SharedPtr<U> getObject();
    template<class U = TerraObjInterface> inline U *get() { return getObject<U>().get(); }

    virtual void setParent(TerraObjInterface *parent) override;
    inline void setParent(TerraObjInterface *parent, tposi_t subIndex) { setParent(parent); setParentSubIndex(subIndex); }
    inline void setParentSubIndex(tposi_t subIndex) { _subIndex = subIndex; }
    inline TerraObjInterface *getParent() { return _parent; }
    inline tposi_t getParentSubIndex() { return _subIndex; }

    virtual TerraIdentity getId() const override { return _obj.getId(); }
    virtual tkey_t getKey() const override { return _obj.getKey(); }
    virtual String getKeyString() const override { return _obj.getKeyString(); }
    inline bool isSet() const { return _obj.isSet(); }
    virtual SharedPtr<TerraObjInterface> getSharedPtrFor(const TerraObjInterface *obj) const override;

    inline operator bool() const { return isResolved(); }
    inline TerraObjInterface* operator->() { return get<TerraObjInterface>(); }

    inline TerraAttachment &operator=(const TerraIdentity &rhs) { setObject(rhs); return *this; }
    inline TerraAttachment &operator=(const char *rhs) { setObject(rhs); return *this; }
    template<class U> inline TerraAttachment &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline TerraAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }

    inline bool operator==(const TerraIdentity &rhs) const { return _obj == rhs; }
    inline bool operator==(const char *rhs) { return *this == TerraIdentity(rhs); }
    template<class U> inline bool operator==(const SharedPtr<U> &rhs) const { return _obj == rhs; }
    template<class U> inline bool operator==(const U *rhs) const { return _obj == rhs; }

protected:
    TerraDLinkObject _obj;                                  // Dynamic link object
    tposi_t _subIndex;                                      // Parent sub index, else 0
};


// Signal Attachment Point
// This attachment registers the parent object with a signal getter off the linked object
// upon resolvement / unregisters the parent object from the signal at time of destruction
// or reassignment.
template<class ParameterType, int Slots = 8>
class TerraSignalAttachment : public TerraAttachment {
public:
    typedef Signal<ParameterType,Slots> &(TerraObjInterface::*SignalGetterPtr)(void);

    template<class U> TerraSignalAttachment(TerraObjInterface *parent = nullptr, tposi_t subIndex = 0, Signal<ParameterType,Slots> &(U::*signalGetter)(void) = nullptr);
    TerraSignalAttachment(const TerraSignalAttachment<ParameterType,Slots> &attachment);
    TerraSignalAttachment<ParameterType,Slots> &operator=(const TerraSignalAttachment<ParameterType,Slots> &attachment);
    virtual ~TerraSignalAttachment();

    virtual void attachObject() override;
    virtual void detachObject() override;

    // Sets the signal handler getter method to use
    template<class U> void setSignalGetter(Signal<ParameterType,Slots> &(U::*signalGetter)(void));

    // Sets a handle slot to run when attached signal fires
    void setHandleSlot(const Slot<ParameterType> &handleSlot);
    inline void setHandleFunction(void (*handleFunctionPtr)(ParameterType)) { setHandleSlot(FunctionSlot<ParameterType>(handleFunctionPtr)); }
    template<class U, class V = U> inline void setHandleMethod(void (U::*handleMethodPtr)(ParameterType), V *handleClassInst = nullptr) { setHandleSlot(MethodSlot<V,ParameterType>(handleClassInst ? handleClassInst : static_cast<V *>(_parent), handleMethodPtr)); }

    inline TerraSignalAttachment<ParameterType,Slots> &operator=(const TerraIdentity &rhs) { setObject(rhs); return *this; }
    inline TerraSignalAttachment<ParameterType,Slots> &operator=(const char *rhs) { setObject(TerraIdentity(rhs)); return *this; }
    template<class U> inline TerraSignalAttachment<ParameterType,Slots> &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline TerraSignalAttachment<ParameterType,Slots> &operator=(const U *rhs) { setObject(rhs); return *this; }

protected:
    SignalGetterPtr _signalGetter;                          // Signal getter method ptr (weak)
    Slot<ParameterType> *_handleSlot;                       // Handler slot (owned)
};


// Actuator Attachment Point
// This attachment interfaces with actuator activation handles for actuator control, and
// registers the parent object with an actuator upon resolvement / unregisters the parent
// object from the actuator at time of destruction or reassignment.
class TerraActuatorAttachment : public TerraSignalAttachment<TerraActuator *, TERRA_ACTUATOR_SIGNAL_SLOTS> {
public:
    TerraActuatorAttachment(TerraObjInterface *parent = nullptr, tposi_t subIndex = 0);
    TerraActuatorAttachment(const TerraActuatorAttachment &attachment);
    TerraActuatorAttachment &operator=(const TerraActuatorAttachment &attachment);
    virtual ~TerraActuatorAttachment();

    // Updates with actuator activation handle. Does not call actuator's update() (handled by system).
    virtual void updateIfNeeded(bool poll = false) override;

    // A rate multiplier is used to adjust either the intensity or duration of activations,
    // which depends on whenever they operate in binary mode (on/off) or variably (ranged).
    inline void setRateMultiplier(float rateMultiplier) { if (!isFPEqual(_rateMultiplier, rateMultiplier)) { _rateMultiplier = rateMultiplier; applySetup(); } }
    inline float getRateMultiplier() const { return _rateMultiplier; }

    // Activations are set up first by calling one of these methods. This configures the
    // direction, intensity, duration, and any run flags that the actuator will operate
    // upon once enabled, pending any rate offsetting. These methods are re-entrant.
    // The most recently used setup values are used for repeat activations.
    inline void setupActivation(const TerraActivation &activation) { _actSetup = activation; applySetup(); }
    inline void setupActivation(const TerraActivationHandle &handle) { setupActivation(handle.activation); }
    inline void setupActivation(Terra_DirectionMode direction, float intensity = 1.0f, millis_t duration = -1, bool force = false) { setupActivation(TerraActivation(direction, intensity, duration, (force ? Terra_ActivationFlags_Forced : Terra_ActivationFlags_None))); }
    inline void setupActivation(millis_t duration = -1, bool force = false) { setupActivation(TerraActivation(Terra_DirectionMode_Forward, 1.0f, duration, (force ? Terra_ActivationFlags_Forced : Terra_ActivationFlags_None))); }
    // These activation methods take a variable value that gets transformed by any user
    // curvature calibration data before being used, assuming units to be the same. It is
    // otherwise assumed the value is a normalized driving intensity ([0,1] or [-1,1]).
    void setupActivation(float value, millis_t duration = -1, bool force = false);
    inline void setupActivation(const TerraSingleMeasurement &measurement, millis_t duration = -1, bool force = false) { setupActivation(measurement.value, duration, force); }

    // Gets what units are expected to be used in setupActivation() methods
    inline Terra_UnitsType getActivationUnits();

    // Enables activation handle with current setup, if not already active.
    // Repeat activations will reuse most recent setupActivation() values.
    void enableActivation();
    // Disables activation handle, if not already inactive.
    inline void disableActivation() { _actHandle.unset(); }

    // Activation status based on handle activation
    inline bool isActivated() const { return _actHandle.isActive(); }
    inline millis_t getTimeLeft() const { return _actHandle.getTimeLeft(); }
    inline millis_t getTimeActive(millis_t time = nzMillis()) const { return _actHandle.getTimeActive(time); }

    // Currently active driving intensity [-1.0,1.0] / calibrated value [calibMin,calibMax], from actuator
    inline float getActiveDriveIntensity();
    inline float getActiveCalibratedValue();

    // Currently setup driving intensity [-1.0,1.0] / calibrated value [calibMin,calibMax], from activation
    inline float getSetupDriveIntensity() const;
    inline float getSetupCalibratedValue();

    // Sets an update slot to run during execution of actuator that can further refine duration/intensity.
    // Useful for rate-based or variable activations. Slot receives actuator attachment pointer as parameter.
    // Guaranteed to be called with final finished activation.
    void setUpdateSlot(const Slot<TerraActuatorAttachment *> &updateSlot);
    inline void setUpdateFunction(void (*updateFunctionPtr)(TerraActuatorAttachment *)) { setUpdateSlot(FunctionSlot<TerraActuatorAttachment *>(updateFunctionPtr)); }
    template<class U> inline void setUpdateMethod(void (U::*updateMethodPtr)(TerraActivationHandle *), U *updateClassInst = nullptr) { setUpdateSlot(MethodSlot<U,TerraActuatorAttachment *>(updateClassInst ? updateClassInst : reinterpret_cast<U *>(_parent), updateMethodPtr)); }
    const Slot<TerraActuatorAttachment *> *getUpdateSlot() const { return _updateSlot; }

    inline const TerraActivationHandle &getActivationHandle() const { return _actHandle; }
    inline const TerraActivation &getActivationSetup() const { return _actSetup; }

    template<class U> inline void setObject(U obj, bool modify = false) { TerraAttachment::setObject(obj, modify); }
    inline SharedPtr<TerraActuator> getObject() { return TerraAttachment::getObject<TerraActuator>(); }
    inline TerraActuator *get() { return TerraAttachment::get<TerraActuator>(); }

    inline TerraActuator &operator*() { return *TerraAttachment::get<TerraActuator>(); }
    inline TerraActuator *operator->() { return TerraAttachment::get<TerraActuator>(); }

    inline TerraActuatorAttachment &operator=(const TerraIdentity &rhs) { setObject(rhs); return *this; }
    inline TerraActuatorAttachment &operator=(const char *rhs) { setObject(rhs); return *this; }
    template<class U> inline TerraActuatorAttachment &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline TerraActuatorAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }

protected:
    TerraActivationHandle _actHandle;                       // Actuator activation handle (double ref to object when active)
    TerraActivation _actSetup;                              // Actuator activation setup
    Slot<TerraActuatorAttachment *> *_updateSlot;           // Update slot (owned)
    float _rateMultiplier;                                  // Rate multiplier
    bool _calledLastUpdate;                                 // Last update call flag

    void applySetup();
};


// Sensor Measurement Attachment Point
// This attachment registers the parent object with a sensor's new measurement signal
// upon resolvement / unregisters the parent object from the sensor at time of destruction
// or reassignment.
// Custom handle method is responsible for calling setMeasurement() to update measurement.
class TerraSensorAttachment : public TerraSignalAttachment<const TerraMeasurement *, TERRA_SENSOR_SIGNAL_SLOTS> {
public:
    TerraSensorAttachment(TerraObjInterface *parent = nullptr, tposi_t subIndex = 0, uint8_t measurementRow = 0);
    TerraSensorAttachment(const TerraSensorAttachment &attachment);
    virtual ~TerraSensorAttachment();

    virtual void attachObject() override;
    virtual void detachObject() override;

    // Updates measurement attachment with sensor. Does not call sensor's update() (handled by system).
    virtual void updateIfNeeded(bool poll = false) override;

    // Sets the current measurement associated with this process. Required to be called by custom handlers.
    void setMeasurement(TerraSingleMeasurement measurement);
    inline void setMeasurement(float value, Terra_UnitsType units = Terra_UnitsType_Undefined) { setMeasurement(TerraSingleMeasurement(value, units)); }
    void setMeasurementRow(uint8_t measurementRow);
    void setMeasurementUnits(Terra_UnitsType units, float convertParam = FLT_UNDEF);

    inline const TerraSingleMeasurement &getMeasurement(bool poll = false) { updateIfNeeded(poll); return _measurement; }
    inline uint16_t getMeasurementFrame(bool poll = false) { updateIfNeeded(poll); return _measurement.frame; }
    inline float getMeasurementValue(bool poll = false) { updateIfNeeded(poll); return _measurement.value; }
    inline Terra_UnitsType getMeasurementUnits() const { return _measurement.units; }

    inline void setNeedsMeasurement() { _needsMeasurement = true; }
    inline bool needsMeasurement() { return _needsMeasurement; }

    inline uint8_t getMeasurementRow() const { return _measurementRow; }
    inline float getMeasurementConvertParam() const { return _convertParam; }

    inline SharedPtr<TerraSensor> getObject(bool poll = false) { updateIfNeeded(poll); return TerraAttachment::getObject<TerraSensor>(); }
    inline TerraSensor *get() { return TerraAttachment::get<TerraSensor>(); }

    inline TerraSensor &operator*() { return *TerraAttachment::get<TerraSensor>(); }
    inline TerraSensor *operator->() { return TerraAttachment::get<TerraSensor>(); }

    inline TerraSensorAttachment &operator=(const TerraIdentity &rhs) { setObject(rhs); return *this; }
    inline TerraSensorAttachment &operator=(const char *rhs) { setObject(rhs); return *this; }
    template<class U> inline TerraSensorAttachment &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline TerraSensorAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }

protected:
    TerraSingleMeasurement _measurement;                    // Local measurement (converted to measure units)
    uint8_t _measurementRow;                                // Measurement row
    float _convertParam;                                    // Convert param (default: FLT_UNDEF)
    bool _needsMeasurement;                                 // Stale measurement tracking flag

    void handleMeasurement(const TerraMeasurement *measurement);
};


// Trigger State Attachment Point
// This attachment registers the parent object with a triggers's trigger signal
// upon resolvement / unregisters the parent object from the trigger at time of
// destruction or reassignment.
class TerraTriggerAttachment  : public TerraSignalAttachment<Terra_TriggerState, TERRA_TRIGGER_SIGNAL_SLOTS> {
public:
    TerraTriggerAttachment(TerraObjInterface *parent = nullptr, tposi_t subIndex = 0);
    TerraTriggerAttachment(const TerraTriggerAttachment &attachment);
    virtual ~TerraTriggerAttachment();

    // Updates owned trigger attachment.
    virtual void updateIfNeeded(bool poll = false) override;

    inline Terra_TriggerState getTriggerState(bool poll = false);
    inline bool isTriggered(bool poll = false) { return getTriggerState(poll) == Terra_TriggerState_Triggered; }

    inline SharedPtr<TerraTrigger> getObject() { return TerraAttachment::getObject<TerraTrigger>(); }
    inline TerraTrigger *get() { return TerraAttachment::get<TerraTrigger>(); }

    inline TerraTrigger &operator*() { return *TerraAttachment::get<TerraTrigger>(); }
    inline TerraTrigger *operator->() { return TerraAttachment::get<TerraTrigger>(); }

    inline TerraTriggerAttachment &operator=(const TerraIdentity &rhs) { setObject(rhs); return *this; }
    inline TerraTriggerAttachment &operator=(const char *rhs) { setObject(rhs); return *this; }
    template<class U> inline TerraTriggerAttachment &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline TerraTriggerAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }
};
#endif // /ifndef TerraAttachments_H
