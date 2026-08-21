/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Modules
*/

#ifndef TerraModules_H
#define TerraModules_H

#include "TerraDefines.h"
#include "TerraTypes.h"
#include "TerraSetup.h"


class TerraObject;

// Object Registration Storage
// Stores registered main objects in a shared keyed collection used by factory-created
// objects and attachment-point resolution, matching the sibling controller libraries.
class TerraObjectRegistration {
public:
    TerraObjectRegistration();

    // Adds object to system, returning success.
    bool registerObject(SharedPtr<TerraObject> object);
    // Removes object from system, returning success.
    bool unregisterObject(SharedPtr<TerraObject> object);

    SharedPtr<TerraObject> sharedObjectByKey(uint32_t key) const;
    TerraObject *findObjectByKey(uint32_t key) const;
    TerraObject *findObjectByName(const TerraString &name) const;
    TerraObject *findFirstByType(Terra_ObjectType type) const;
    uint8_t findByType(Terra_ObjectType type, TerraObject **output, uint8_t capacity) const;
    TerraObject *objectAt(uint8_t index) const;
    uint8_t objectCount() const { return (uint8_t)_objects.size(); }
    uint32_t allocateKey(const TerraString &name = TerraString());
    void updateObjects(uint32_t now = terraMillis());

protected:
    TerraMap<uint32_t, SharedPtr<TerraObject>, TERRA_MAX_OBJECTS> _objects; // Shared object collection, keyed by stable object key
    uint32_t _nextKey;                                      // Next automatically assigned object key
};

struct TerraModule {
    Terra_ModuleType type;                                  // Object/subsystem type
    TerraString name;                                       // Display name
    TerraDeviceSetup setup;                                 // Saved setup data
    bool enabled;                                           // Enabled state
    bool online;                                            // Module online state

    TerraModule() : type(Terra_ModuleType_Undefined), name(), setup(), enabled(false), online(false) { }
};

class TerraModuleRegistry {
public:
    TerraModuleRegistry();
    int8_t add(Terra_ModuleType type, const TerraString &name, const TerraDeviceSetup &setup = TerraDeviceSetup());
    bool remove(uint8_t index);
    TerraModule *at(uint8_t index);
    const TerraModule *at(uint8_t index) const;
    TerraModule *find(Terra_ModuleType type);
    uint8_t count() const { return _count; }

protected:
    TerraModule _modules[TERRA_MAX_MODULES];                // Modules
    uint8_t _count;                                         // Active entry count
};

#endif
