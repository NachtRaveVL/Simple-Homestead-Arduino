/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Attachment Points
*/

#ifndef TerraAttachments_H
#define TerraAttachments_H

class TerraObject;
class TerraSensor;
class TerraActuator;

#include "TerraObject.h"
#include "TerraActivation.h"
#include "TerraMeasurements.h"

// Serialized Attachment Data
// Stable object identity and relationship role used by persistence. Runtime relationships
// are represented by the attachment-point classes below rather than role/key bags.
struct TerraAttachmentData {
    uint32_t objectKey;                                     // Attached object key
    Terra_AttachmentRole role;                              // Attachment relationship role

    TerraAttachmentData(uint32_t key = TERRA_INVALID_KEY,
                        Terra_AttachmentRole roleIn = Terra_AttachmentRole_Undefined)
        : objectKey(key), role(roleIn) { }
};

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
    TerraAttachment(const TerraAttachment<TObject> &attachment)
        : _parent(attachment._parent), _key(attachment._key), _object(attachment._object) { attachObject(); }
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

    void setOutput(float intensity, uint32_t durationMs = 0, uint32_t now = terraMillis());
    void off();
    inline bool isActive() const { return _activation.isActive(); }

protected:
    TerraActivationHandle _activation;                      // Resident actuator request
};

#include "TerraAttachments.hpp"

#endif
