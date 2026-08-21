/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Attachments
*/

#ifndef TerraAttachments_H
#define TerraAttachments_H

class TerraObject;
class TerraSensor;
class TerraActuator;

#include "TerraObject.h"
#include "TerraActivation.h"
#include "TerraMeasurements.h"

// Returns a shared registered object by stable key, or an empty pointer when unresolved.
extern SharedPtr<TerraObject> terraObjectByKey(uint32_t key);

// Simple Attachment Point Base
// Registers the parent object with the linked object's linkages upon resolution and
// unregisters the parent object during destruction or reassignment.
template<class TObject>
class TerraAttachment {
public:
    TerraAttachment(TerraObject *parent = nullptr)
        : _parent(parent), _key(TERRA_INVALID_KEY), _object() { }
    virtual ~TerraAttachment() { detachObject(); }

    template<class U> inline void setObject(const SharedPtr<U> &object) {
        setObjectImpl(terraStaticPointerCast<TObject>(object));
    }
    void setObject(TObject *object);
    inline void initObject(uint32_t key)
    {
        detachObject();
        _key = key;
    }
    SharedPtr<TObject> getObject();
    inline TObject *get() { return getObject().get(); }

    inline bool isUnresolved() const { return !_object; }
    inline bool isResolved() const { return (bool)_object; }
    inline bool needsResolved() const { return isUnresolved() && isSet(); }
    inline bool resolve() { return isResolved() || (bool)getObject(); }
    inline bool isSet() const { return _key != TERRA_INVALID_KEY || (bool)_object; }
    inline uint32_t getKey() const { return _key; }

    void unresolve();
    void unresolveAny(TerraObject *object);
    void setParent(TerraObject *parent);

protected:
    TerraObject *_parent;                                   // Parent main object, not owned
    uint32_t _key;                                          // Stable registered object key
    SharedPtr<TObject> _object;                             // Resolved registered object

    void setObjectImpl(const SharedPtr<TObject> &object);
    void attachObject();
    void detachObject();
};

// Sensor Measurement Attachment Point
// Polls and caches the latest measurement from an attached registered sensor.
class TerraSensorAttachment : public TerraAttachment<TerraSensor> {
public:
    TerraSensorAttachment(TerraObject *parent = nullptr)
        : TerraAttachment<TerraSensor>(parent), _measurement() { }

    TerraMeasurement getMeasurement(uint32_t now = terraMillis(), bool poll = false);
    inline const TerraMeasurement &getCachedMeasurement() const { return _measurement; }

protected:
    TerraMeasurement _measurement;                          // Last attached measurement
};

// Actuator Attachment Point
// Keeps a resident activation handle so independently attached control processes can
// request actuator output without replacing one another's activation state.
class TerraActuatorAttachment : public TerraAttachment<TerraActuator> {
public:
    TerraActuatorAttachment(TerraObject *parent = nullptr);
    virtual ~TerraActuatorAttachment();

    void setupActivation(float intensity = 1.0f, uint32_t duration = (uint32_t)-1);
    void enableActivation();
    inline void disableActivation() { _actHandle.unset(); }
    inline bool isActivated() const { return _actHandle.isActive(); }
    inline uint32_t getTimeLeft() const { return _actHandle.getTimeLeft(); }
    inline uint32_t getTimeActive(uint32_t time = terraNZMillis()) const { return _actHandle.getTimeActive(time); }
    inline float getActiveDriveIntensity() const { return _actHandle.getDriveIntensity(); }
    inline float getSetupDriveIntensity() const { return _actSetup.getDriveIntensity(); }

protected:
    TerraActivationHandle _actHandle;                       // Resident actuator activation handle
    TerraActivation _actSetup;                              // Actuator activation setup
};

#include "TerraAttachments.hpp"

#endif
