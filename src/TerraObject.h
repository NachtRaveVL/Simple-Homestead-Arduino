/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Object
*/

#ifndef TerraObject_H
#define TerraObject_H

struct TerraIdentity;
class TerraObject;
class TerraSubObject;

struct TerraObjectData;

#include "Terraduino.h"
#include "TerraData.h"

// Creates object from passed object data (return ownership transfer - user code *must* delete returned object)
extern TerraObject *newObjectFromData(const TerraData *dataIn);

// Shortcut to get shared pointer for object with built-in cast.
template<class T = TerraObjInterface> inline SharedPtr<T> getSharedPtr(const TerraObjInterface *obj) { return obj ? reinterpret_pointer_cast<T>(obj->getSharedPtr()) : nullptr; }


// Object Identity
// Simple class for referencing an object in the Terra system.
// This class is mainly used to simplify object key generation, which is used when we
// want to uniquely refer to objects in the Terra system.
struct TerraIdentity {
    enum : signed char { Actuator, Sensor, Reservoir, Rail, Unknown = -1 } type; // Object type (custom RTTI)
    inline bool isActuatorType() const { return type == Actuator; }
    inline bool isSensorType() const { return type == Sensor; }
    inline bool isReservoirType() const { return type == Reservoir; }
    inline bool isRailType() const { return type == Rail; }
    inline bool isUnknownType() const { return type <= Unknown; }

    union {
        Terra_ActuatorType actuatorType;                    // As actuator type enumeration
        Terra_SensorType sensorType;                        // As sensor type enumeration
        Terra_ReservoirType reservoirType;                  // As reservoir type enumeration
        Terra_RailType railType;                            // As rail type enumeration
        tid_t idType;                                       // As standard id type enumeration
    } objTypeAs;                                            // Object type union
    tposi_t posIndex;                                       // Position index
    String keyString;                                       // String key
    tkey_t key;                                             // UInt Key

    // Default/copy key (incomplete id)
    inline TerraIdentity(tkey_t key = -1) : type(Unknown), objTypeAs{.idType=Unknown}, posIndex(-1), keyString(), key(key) { ; }
    // Copy into keyStr (incomplete id)
    inline TerraIdentity(const char *idKeyStr) : type(Unknown), objTypeAs{.idType=Unknown}, posIndex(-1), keyString(idKeyStr), key(stringHash(idKeyStr)) { ; }
    // Copy into keyStr (incomplete id)
    inline TerraIdentity(String idKey) : type(Unknown), objTypeAs{.idType=Unknown}, posIndex(-1), keyString(idKey), key(stringHash(idKey.c_str())) { ; }

    // Copy id with new position index
    inline TerraIdentity(const TerraIdentity &id, tposi_t positionIndex) : type(id.type), objTypeAs{.idType=id.objTypeAs.idType}, posIndex(positionIndex), keyString(), key(tkey_none) { regenKey(); }

    // Actuator id constructor
    inline TerraIdentity(Terra_ActuatorType actuatorTypeIn,
                         tposi_t positionIndex = TERRA_POS_SEARCH_FROMBEG) : type(Actuator), objTypeAs{.actuatorType=actuatorTypeIn}, posIndex(positionIndex), keyString(), key(tkey_none) { regenKey(); }
    // Sensor id constructor
    inline TerraIdentity(Terra_SensorType sensorTypeIn,
                         tposi_t positionIndex = TERRA_POS_SEARCH_FROMBEG) : type(Sensor), objTypeAs{.sensorType=sensorTypeIn}, posIndex(positionIndex), keyString(), key(tkey_none) { regenKey(); }
    // Reservoir id constructor
    inline TerraIdentity(Terra_ReservoirType reservoirTypeIn,
                         tposi_t positionIndex = TERRA_POS_SEARCH_FROMBEG) : type(Reservoir), objTypeAs{.reservoirType=reservoirTypeIn}, posIndex(positionIndex), keyString(), key(tkey_none) { regenKey(); }
    // Rail id constructor
    inline TerraIdentity(Terra_RailType railTypeIn,
                         tposi_t positionIndex = TERRA_POS_SEARCH_FROMBEG) : type(Rail), objTypeAs{.railType=railTypeIn}, posIndex(positionIndex), keyString(), key(tkey_none) { regenKey(); }

    // Data constructor
    inline TerraIdentity(const TerraData *dataIn) : type((typeof(type))(dataIn->id.object.idType)), objTypeAs{.idType=dataIn->id.object.objType}, posIndex(dataIn->id.object.posIndex), keyString(), key(tkey_none) { regenKey(); }

    // Used to update key value after modification, returning new key by convenience
    tkey_t regenKey();

    // Returns displayable string associated with this identity (type + name)
    String getDisplayString();

    inline operator bool() const { return key != tkey_none; }
    inline bool operator==(const TerraIdentity &otherId) const { return key == otherId.key; }
    inline bool operator!=(const TerraIdentity &otherId) const { return key != otherId.key; }
};


// Object Base
// A simple base class for referring to objects in the Terra system.
class TerraObject : public TerraObjInterface {
public:
    inline bool isActuatorType() const { return _id.isActuatorType(); }
    inline bool isSensorType() const { return _id.isSensorType(); }
    inline bool isReservoirType() const { return _id.isReservoirType(); }
    inline bool isRailType() const { return _id.isRailType(); }
    inline bool isUnknownType() const { return _id.isUnknownType(); }

    inline TerraObject(TerraIdentity id) : _id(id), _revision(-1), _linksSize(0), _links(nullptr) { ; }
    inline TerraObject(const TerraData *data) : _id(data), _revision(data->_revision), _linksSize(0), _links(nullptr) { ; }
    virtual ~TerraObject();

    // Called over intervals of time by runloop
    virtual void update();
    // Called upon low memory condition to try and free memory up
    virtual void handleLowMemory();

    // Saves object state to proper backing data
    TerraData *newSaveData();

    // (Re)allocates linkage list of specified size
    void allocateLinkages(size_t size = 1);
    // Adds linkage to this object, returns true upon initial add
    virtual bool addLinkage(TerraObject *obj);
    // Removes linkage from this object, returns true upon last remove
    virtual bool removeLinkage(TerraObject *obj);
    // Checks object linkage to this object
    bool hasLinkage(TerraObject *obj) const;

    // Returns the linkages this object contains, along with refcount for how many times it has registered itself as linked (via attachment points).
    // Objects are considered strong pointers, since existence -> SharedPtr ref to this instance exists.
    inline Pair<uint8_t, Pair<TerraObject *, int8_t> *> getLinkages() const { return make_pair(_linksSize, _links); }

    // Unresolves any dlinks to obj prior to caching
    virtual void unresolveAny(TerraObject *obj) override;
    // Unresolves this instance from any dlinks
    inline void unresolve() { unresolveAny(this); }

    // Returns the unique Identity of the object
    virtual TerraIdentity getId() const override;
    // Returns the unique key of the object
    virtual tkey_t getKey() const override;
    // Returns the key string of the object
    virtual String getKeyString() const override;
    // Returns the SharedPtr instance for this object
    virtual SharedPtr<TerraObjInterface> getSharedPtr() const override;
    // Returns the SharedPtr instance for passed object
    virtual SharedPtr<TerraObjInterface> getSharedPtrFor(const TerraObjInterface *obj) const override;
    // Returns true for object
    virtual bool isObject() const override;

    // Returns revision #
    inline uint8_t getRevision() const { return abs(_revision); }
    // If revision has been modified since last saved
    inline bool isModified() const { return _revision < 0; }
    // Bumps revision # if not already modified, and sets modified flag (called after modifying data)
    inline void bumpRevisionIfNeeded() { if (!isModified()) { _revision = -(abs(_revision) + 1); } }
    // Unsets modified flag from revision (called after save-out)
    inline void unsetModified() { _revision = abs(_revision); }

protected:
    TerraIdentity _id;                                      // Object id
    int8_t _revision;                                       // Revision # of stored data (uses -vals for modified flag)
    uint8_t _linksSize;                                     // Number of object linkages
    Pair<TerraObject *, int8_t> *_links;                    // Object linkages array (owned, lazily allocated/grown/shrunk)

    virtual TerraData *allocateData() const;                // Only up to base type classes (sensor, reservoir, etc.) does this need overriden
    virtual void saveToData(TerraData *dataOut);            // *ALL* derived classes must override and implement

private:
    // Private constructor to disable derived/public access
    inline TerraObject() : _id(), _revision(-1), _linksSize(0), _links(nullptr) { ; }
};


// Sub Object Base
// A base class for sub objects that are typically found embedded in bigger main objects,
// but want to replicate some of the same functionality. Not required to be inherited from.
class TerraSubObject : public TerraObjInterface {
public:
    inline TerraSubObject(TerraObjInterface *parent = nullptr) : _parent(parent) { ; }

    virtual void setParent(TerraObjInterface *parent);
    inline TerraObjInterface *getParent() const { return _parent; }

    virtual void unresolveAny(TerraObject *obj) override;

    virtual TerraIdentity getId() const override;
    virtual tkey_t getKey() const override;
    virtual String getKeyString() const override;
    virtual SharedPtr<TerraObjInterface> getSharedPtr() const override;
    virtual SharedPtr<TerraObjInterface> getSharedPtrFor(const TerraObjInterface *obj) const override;
    virtual bool isObject() const override;

    inline uint8_t getRevision() const { return _parent && _parent->isObject() ? ((TerraObject *)_parent)->getRevision() : 0; }
    inline bool isModified() const { return _parent && _parent->isObject() ? ((TerraObject *)_parent)->isModified() : false; }
    inline void bumpRevisionIfNeeded() { if (_parent && _parent->isObject()) { ((TerraObject *)_parent)->bumpRevisionIfNeeded(); } }
    inline void unsetModified() { ; }

protected:
    TerraObjInterface *_parent;                             // Parent object pointer (reverse ownership)
};


// Object Serialization Data Intermediate
// Intermediate data class for object data.
struct TerraObjectData : public TerraData {
    char name[TERRA_NAME_MAXSIZE];

    TerraObjectData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef TerraObject_H