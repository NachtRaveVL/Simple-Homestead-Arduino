/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Attachment Points
*/

#ifndef TerraAttachments_H
#define TerraAttachments_H

class TerraDLinkObject;
class TerraAttachment;
template<class ParameterType, int Slots> class SignalAttachment;
class TerraActuatorAttachment;
class TerraSensorAttachment;

class TerraObject;
class TerraSensor;
class TerraActuator;

#include "TerraObject.h"
#include "TerraActivation.h"

extern tkey_t terraHashString(const char *text);
#include "TerraMeasurements.h"

// Delay/Dynamic Loaded/Linked Object Reference
// Simple class for delay loading objects that get references to others during object load.
class TerraDLinkObject {
public:
    TerraDLinkObject();
    TerraDLinkObject(const TerraDLinkObject &object);
    virtual ~TerraDLinkObject() { ; }

    inline bool isUnresolved() const { return !_obj; }
    inline bool isResolved() const { return (bool)_obj; }
    inline bool needsResolved() const { return isUnresolved() && isSet(); }
    inline bool resolve() { return isResolved() || (bool)getObject(); }
    void unresolve();
    template<class U> inline void unresolveIf(U object) { if (operator==(object)) { unresolve(); } }

    template<class U> inline void setObject(U object) { operator=(object); }
    template<class U = TerraObjInterface> inline SharedPtr<U> getObject() { return reinterpret_pointer_cast<U>(resolveObject()); }
    template<class U = TerraObjInterface> inline U *get() { return getObject<U>().get(); }

    TerraIdentity getId() const;
    inline tkey_t getKey() const { return _key; }
    TerraString getKeyString() const;
    inline bool isSet() const { return _key != tkey_none; }

    inline operator bool() const { return isResolved(); }
    inline TerraObjInterface *operator->() { return get(); }

    TerraDLinkObject &operator=(const TerraDLinkObject &rhs);
    inline TerraDLinkObject &operator=(TerraIdentity rhs);
    inline TerraDLinkObject &operator=(const char *rhs);
    template<class U> inline TerraDLinkObject &operator=(SharedPtr<U> rhs);
    inline TerraDLinkObject &operator=(const TerraObjInterface *rhs);
    inline TerraDLinkObject &operator=(const TerraAttachment *rhs);
    inline TerraDLinkObject &operator=(nullptr_t) { return operator=((TerraObjInterface *)nullptr); }

    inline bool operator==(const TerraIdentity &rhs) const { return _key == rhs.key; }
    inline bool operator==(const char *rhs) const { return _key == (rhs ? terraHashString(rhs) : tkey_none); }
    template<class U> inline bool operator==(const SharedPtr<U> &rhs) const { return _key == (rhs ? rhs->getKey() : tkey_none); }
    inline bool operator==(const TerraObjInterface *rhs) const { return _key == (rhs ? rhs->getKey() : tkey_none); }
    inline bool operator==(nullptr_t) const { return _key == tkey_none; }

protected:
    tkey_t _key;                                            // Object key
    SharedPtr<TerraObjInterface> _obj;                      // Shared pointer to object
    TerraString _keyString;                                 // Object key string while unresolved

private:
    SharedPtr<TerraObjInterface> resolveObject();
    friend class TerraAttachment;
};

// Simple Attachment Point Base
// This attachment registers the parent object with the linked object's linkages upon
// dereference / unregisters the parent object at time of destruction or reassignment.
class TerraAttachment : public TerraSubObject {
public:
    TerraAttachment(TerraObjInterface *parent = nullptr);
    TerraAttachment(const TerraAttachment &attachment);
    virtual ~TerraAttachment();

    virtual void attachObject();
    virtual void detachObject();
    virtual void updateIfNeeded(bool poll = false);

    inline bool isUnresolved() const { return !_obj; }
    inline bool isResolved() const { return (bool)_obj; }
    inline bool needsResolved() const { return _obj.needsResolved(); }
    inline bool resolve() { return isResolved() || (bool)getObject(); }
    void unresolve();
    virtual void unresolveAny(TerraObject *object) override { unresolveIf(object); }
    template<class U> inline void unresolveIf(U object) { if (_obj == object) { unresolve(); } }

    template<class U> void setObject(U object, bool modify = true);
    template<class U> inline void initObject(U object) { setObject(object, false); }
    template<class U = TerraObjInterface> SharedPtr<U> getObject();
    template<class U = TerraObjInterface> inline U *get() { return getObject<U>().get(); }

    virtual void setParent(TerraObjInterface *parent) override;

    virtual TerraIdentity getId() const override { return _obj.getId(); }
    virtual tkey_t getKey() const override { return _obj.getKey(); }
    virtual TerraString getKeyString() const override { return _obj.getKeyString(); }
    inline bool isSet() const { return _obj.isSet(); }
    virtual SharedPtr<TerraObjInterface> getSharedPtrFor(const TerraObjInterface *object) const override;

    inline operator bool() const { return isResolved(); }
    inline TerraObjInterface *operator->() { return get<TerraObjInterface>(); }

    inline TerraAttachment &operator=(const TerraIdentity &rhs) { setObject(rhs); return *this; }
    inline TerraAttachment &operator=(const char *rhs) { setObject(rhs); return *this; }
    template<class U> inline TerraAttachment &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline TerraAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }

protected:
    TerraDLinkObject _obj;                                  // Dynamic link object
};


// Signal Attachment Point
// Registers a handler with a linked object's signal during attachment lifetime.
template<class ParameterType, int Slots = 8>
class SignalAttachment : public TerraAttachment {
public:
    typedef Signal<ParameterType, Slots> &(TerraObjInterface::*SignalGetterPtr)(void);

    template<class U> SignalAttachment(TerraObjInterface *parent = nullptr, Signal<ParameterType, Slots> &(U::*signalGetter)(void) = nullptr);
    SignalAttachment(const SignalAttachment<ParameterType, Slots> &attachment);
    virtual ~SignalAttachment();

    virtual void attachObject() override;
    virtual void detachObject() override;

    template<class U> void setSignalGetter(Signal<ParameterType, Slots> &(U::*signalGetter)(void));
    void setHandleSlot(const Slot<ParameterType> &handleSlot);
    inline void setHandleFunction(void (*handleFunctionPtr)(ParameterType)) { setHandleSlot(FunctionSlot<ParameterType>(handleFunctionPtr)); }
    template<class U, class V = U> inline void setHandleMethod(void (U::*handleMethodPtr)(ParameterType), V *handleClassInst = nullptr) { setHandleSlot(MethodSlot<V, ParameterType>(handleClassInst ? handleClassInst : static_cast<V *>(_parent), handleMethodPtr)); }

protected:
    SignalGetterPtr _signalGetter;                           // Signal getter method pointer
    Slot<ParameterType> *_handleSlot;                  // Signal handler slot (owned)
};

// Sensor Measurement Attachment Point
class TerraSensorAttachment : public SignalAttachment<const TerraMeasurement *, TERRA_SENSOR_SIGNAL_SLOTS> {
public:
    TerraSensorAttachment(TerraObjInterface *parent = nullptr, uint8_t measurementRow = 0);
    TerraSensorAttachment(const TerraSensorAttachment &attachment);
    virtual ~TerraSensorAttachment() { ; }

    virtual void attachObject() override;
    virtual void detachObject() override;
    virtual void updateIfNeeded(bool poll = false) override;

    void setMeasurement(TerraSingleMeasurement measurement);
    void setMeasurementRow(uint8_t measurementRow);
    void setMeasurementUnits(Terra_Unit units, float convertParam = 0.0f);

    TerraSingleMeasurement getMeasurement(uint32_t now = millis(), bool poll = false);
    inline const TerraSingleMeasurement &getCachedMeasurement() const { return _measurement; }
    inline uint8_t getMeasurementRow() const { return _measurementRow; }
    inline Terra_Unit getMeasurementUnits() const { return _measurement.units; }
    inline float getMeasurementConvertParam() const { return _convertParam; }
    inline void setNeedsMeasurement() { _needsMeasurement = true; }

    template<class U> inline void setObject(U object, bool modify = false) { TerraAttachment::setObject(object, modify); }
    inline SharedPtr<TerraSensor> getObject() { return TerraAttachment::getObject<TerraSensor>(); }
    inline TerraSensor *get() { return TerraAttachment::get<TerraSensor>(); }

protected:
    TerraSingleMeasurement _measurement;                    // Last attached measurement
    uint8_t _measurementRow;                                // Measurement row
    float _convertParam;                                    // Measurement conversion parameter
    bool _needsMeasurement;                                 // Measurement update flag

    void handleMeasurement(const TerraMeasurement *measurement);
};

// Actuator Attachment Point
class TerraActuatorAttachment : public TerraAttachment {
public:
    TerraActuatorAttachment(TerraObjInterface *parent = nullptr);
    TerraActuatorAttachment(const TerraActuatorAttachment &attachment);
    virtual ~TerraActuatorAttachment() { ; }

    virtual void updateIfNeeded(bool poll = false) override;

    void setOutput(float intensity, millis_t duration = (millis_t)-1, uint32_t now = millis());
    void off();
    inline bool isActive() const { return _activation.isActive(); }

    template<class U> inline void setObject(U object, bool modify = false) { TerraAttachment::setObject(object, modify); }
    inline SharedPtr<TerraActuator> getObject() { return TerraAttachment::getObject<TerraActuator>(); }
    inline TerraActuator *get() { return TerraAttachment::get<TerraActuator>(); }

protected:
    TerraActivationHandle _activation;                      // Resident actuator request
};

#include "TerraAttachments.hpp"

#endif // /ifndef TerraAttachments_H
