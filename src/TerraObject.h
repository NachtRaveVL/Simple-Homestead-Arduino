/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Object
*/

#ifndef TerraObject_H
#define TerraObject_H

#include "TerraDefines.h"
#include "TerraTypes.h"
#include "TerraInterfaces.h"

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

    void update(uint32_t now = terraMillis()) override { (void)now; }

protected:
    Terra_ObjectType _objectType;                           // Runtime object type
    uint32_t _key;                                          // Stable object key
    TerraString _name;                                      // Object display name
    bool _enabled;                                          // Object enabled state
    bool _fault;                                            // Object fault state
    TerraString _faultMessage;                              // Object fault description
};

#endif
