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

struct TerraObjectLink {
    TerraObject *object;                                    // Linked parent object
    uint8_t count;                                          // Number of active linkages

    TerraObjectLink(TerraObject *objectIn = nullptr, uint8_t countIn = 0)
        : object(objectIn), count(countIn) { }
};

class TerraObject : public TerraUpdatable {
public:
    TerraObject(Terra_ObjectType objectType = Terra_ObjectType_Undefined,
                uint32_t key = TERRA_INVALID_KEY,
                const TerraString &name = TerraString());
    virtual ~TerraObject() { }

    uint32_t getKey() const { return _key; }
    Terra_ObjectType getObjectType() const { return _objectType; }
    const TerraString &getName() const { return _name; }
    bool isEnabled() const { return _enabled; }
    bool hasFault() const { return _fault; }
    const TerraString &getFaultMessage() const { return _faultMessage; }

    void setKey(uint32_t key) { _key = key; }
    void setName(const TerraString &name) { _name = name; }
    virtual void setEnabled(bool enabled);
    void setFault(const TerraString &message = TerraString());
    void clearFault();

    void update(uint32_t now = millis()) override { (void)now; }

    // Adds a parent-object linkage, incrementing its reference count when already linked.
    bool addLinkage(TerraObject *object);
    // Removes one parent-object linkage reference.
    bool removeLinkage(TerraObject *object);
    // Returns if the passed parent object is currently linked.
    bool hasLinkage(TerraObject *object) const;
    // Unresolves this object from all linked parent attachment points.
    inline void unresolve() { unresolveAny(this); }
    // Notifies linked parent objects that a registered object is being unresolved.
    virtual void unresolveAny(TerraObject *object);

protected:
    Terra_ObjectType _objectType;                           // Runtime object type
    uint32_t _key;                                          // Stable object key
    TerraString _name;                                      // Object display name
    bool _enabled;                                          // Object enabled state
    bool _fault;                                            // Object fault state
    TerraString _faultMessage;                              // Object fault description
    TerraObjectLink _linkages[TERRA_MAX_ATTACHMENTS];       // Parent-object linkages
};

#endif
