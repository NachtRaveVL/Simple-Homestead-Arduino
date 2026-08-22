/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Object
*/

#ifndef TerraObject_H
#define TerraObject_H

#include "TerraDefines.h"
#include "TerraTypes.h"
#include "TerraInterfaces.h"

class TerraObject;
struct TerraData;
struct TerraObjectData;

// Shortcut to get shared pointer for object with built-in cast.
template<class T = TerraObjInterface> inline SharedPtr<T> getSharedPtr(const TerraObjInterface *object) { return object ? reinterpret_pointer_cast<T>(object->getSharedPtr()) : nullptr; }

// Object Identity
// Compact object identity used to generate stable object keys in the same manner as the
// sibling controller libraries. Display names are not part of object identity.
struct TerraIdentity {
    Terra_ObjectType type;                                  // Object type

    union {
        Terra_SensorType sensorType;                        // Sensor type
        Terra_ActuatorType actuatorType;                    // Actuator type
        Terra_ResourceType resourceType;                    // Resource type
        Terra_WaterStorageType waterStorageType;            // Water storage type
        Terra_WaterSourceType waterSourceType;              // Water source type
        Terra_RailType railType;                            // Rail type
        int16_t idType;                                     // Generic subtype
    } objTypeAs;                                            // Object subtype union

    tposi_t posIndex;                                       // Position index
    TerraString keyString;                                  // Human-readable object key
    tkey_t key;                                             // Hashed object key

    TerraIdentity(tkey_t keyIn = tkey_none);
    TerraIdentity(const char *keyStringIn);
    TerraIdentity(Terra_SensorType sensorTypeIn, tposi_t positionIndex = TERRA_POS_SEARCH_FROMBEG);
    TerraIdentity(Terra_ActuatorType actuatorTypeIn, tposi_t positionIndex = TERRA_POS_SEARCH_FROMBEG);
    TerraIdentity(Terra_ResourceType resourceTypeIn, tposi_t positionIndex = TERRA_POS_SEARCH_FROMBEG);
    TerraIdentity(Terra_WaterStorageType storageTypeIn, tposi_t positionIndex = TERRA_POS_SEARCH_FROMBEG);
    TerraIdentity(Terra_WaterSourceType sourceTypeIn, tposi_t positionIndex = TERRA_POS_SEARCH_FROMBEG);
    TerraIdentity(Terra_RailType railTypeIn, tposi_t positionIndex = TERRA_POS_SEARCH_FROMBEG);
    TerraIdentity(Terra_ObjectType objectTypeIn, tposi_t positionIndex = TERRA_POS_SEARCH_FROMBEG);
    TerraIdentity(int objectType, int16_t subType, tposi_t positionIndex);

    tkey_t regenKey();
    TerraString getDisplayString() const;

    inline operator bool() const { return key != tkey_none; }
    inline bool operator==(const TerraIdentity &other) const { return key == other.key; }
    inline bool operator!=(const TerraIdentity &other) const { return key != other.key; }
};

struct TerraObjectLink {
    TerraObject *object;                                    // Linked parent object
    int8_t count;                                           // Number of active linkages

    TerraObjectLink(TerraObject *objectIn = nullptr, int8_t countIn = 0)
        : object(objectIn), count(countIn) { }
};

class TerraObject : public TerraObjInterface, public TerraUpdatable {
public:
    TerraObject(TerraIdentity id = TerraIdentity(),
                const TerraString &name = TerraString());
    TerraObject(const TerraObjectData *dataIn);
    virtual ~TerraObject();

    virtual TerraIdentity getId() const override { return _id; }
    virtual tkey_t getKey() const override { return _id.key; }
    virtual TerraString getKeyString() const override { return _id.keyString; }
    virtual SharedPtr<TerraObjInterface> getSharedPtr() const override;
    virtual SharedPtr<TerraObjInterface> getSharedPtrFor(const TerraObjInterface *object) const override;
    virtual bool isObject() const override { return true; }

    Terra_ObjectType getObjectType() const { return _id.type; }
    const TerraString &getName() const { return _name; }
    bool isEnabled() const { return _enabled; }
    bool hasFault() const { return _fault; }
    const TerraString &getFaultMessage() const { return _faultMessage; }

    void setName(const TerraString &name) { _name = name; bumpRevisionIfNeeded(); }
    virtual void setEnabled(bool enabled);
    void setFault(const TerraString &message = TerraString());
    void clearFault();

    inline uint8_t getRevision() const { return _revision < 0 ? (uint8_t)-_revision : (uint8_t)_revision; }
    inline bool isModified() const { return _revision < 0; }
    inline void bumpRevisionIfNeeded() { if (!isModified()) { _revision = -(int8_t)(getRevision() + 1); } }
    inline void unsetModified() { if (_revision < 0) { _revision = -_revision; } }

    void update(uint32_t now = millis()) override { (void)now; }

    // (Re)allocates linkage list of specified size.
    void allocateLinkages(size_t size = 1);
    // Adds a parent-object linkage, incrementing its reference count when already linked.
    bool addLinkage(TerraObject *object);
    // Removes one parent-object linkage reference.
    bool removeLinkage(TerraObject *object);
    // Returns if the passed parent object is currently linked.
    bool hasLinkage(TerraObject *object) const;
    // Unresolves this object from all linked parent attachment points.
    inline void unresolve() { unresolveAny(this); }
    // Notifies linked parent objects that a registered object is being unresolved.
    virtual void unresolveAny(TerraObject *object) override;

    inline size_t getLinkagesSize() const { return _linksSize; }
    inline const TerraObjectLink *getLinkages() const { return _links; }

    TerraObjectData *newSaveData() const;

protected:
    TerraIdentity _id;                                      // Object identity
    TerraString _name;                                      // Object display name
    int8_t _revision;                                       // Stored object revision, negative when modified
    bool _enabled;                                          // Object enabled state
    bool _fault;                                            // Object fault state
    TerraString _faultMessage;                              // Object fault description
    size_t _linksSize;                                      // Allocated linkage slot count
    TerraObjectLink *_links;                                // Parent-object linkages, owned

    virtual TerraData *allocateData() const;
    virtual void saveToData(TerraData *dataOut) const;
};

// Sub Object Base
// Lightweight child object that borrows identity and revision state from a parent object.
class TerraSubObject : public TerraObjInterface {
public:
    TerraSubObject(TerraObjInterface *parent = nullptr) : _parent(parent) { ; }

    virtual void setParent(TerraObjInterface *parent) { _parent = parent; }
    inline TerraObjInterface *getParent() const { return _parent; }

    virtual void unresolveAny(TerraObject *object) override { (void)object; }
    virtual TerraIdentity getId() const override;
    virtual tkey_t getKey() const override;
    virtual TerraString getKeyString() const override;
    virtual SharedPtr<TerraObjInterface> getSharedPtr() const override;
    virtual SharedPtr<TerraObjInterface> getSharedPtrFor(const TerraObjInterface *object) const override;
    virtual bool isObject() const override { return false; }

    inline uint8_t getRevision() const { return _parent && _parent->isObject() ? static_cast<TerraObject *>(_parent)->getRevision() : 0; }
    inline bool isModified() const { return _parent && _parent->isObject() ? static_cast<TerraObject *>(_parent)->isModified() : false; }
    inline void bumpRevisionIfNeeded() { if (_parent && _parent->isObject()) { static_cast<TerraObject *>(_parent)->bumpRevisionIfNeeded(); } }
    inline void unsetModified() { ; }

protected:
    TerraObjInterface *_parent;                             // Parent, not owned
};

#endif
