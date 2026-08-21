/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Controller Modules
*/

#ifndef TerraModules_H
#define TerraModules_H

#include "TerraDefines.h"
#include "TerraTypes.h"
#include "TerraInlines.hh"
#include "TerraObject.h"

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


// Object Registration Storage
// Stores objects in main system store, which is used for SharedPtr<> lookups as well as
// notifying appropriate modules upon entry-to/exit-from the system.
class TerraObjectRegistration {
public:
    TerraObjectRegistration();

    // Adds object to system, returning success.
    bool registerObject(SharedPtr<TerraObject> object);
    // Removes object from system, returning success.
    bool unregisterObject(SharedPtr<TerraObject> object);

    // Returns shared object by stable key, or empty pointer when not found.
    SharedPtr<TerraObject> sharedObjectByKey(uint32_t key) const;
    // Returns object by stable key, or nullptr when not found.
    inline TerraObject *findObjectByKey(uint32_t key) const { return sharedObjectByKey(key).get(); }
    // Returns object by display name, or nullptr when not found.
    TerraObject *findObjectByName(const TerraString &name) const;
    // Returns first object matching runtime type, or nullptr when not found.
    TerraObject *findFirstByType(Terra_ObjectType type) const;
    // Copies matching objects into output array, returning count copied.
    uint8_t findByType(Terra_ObjectType type, TerraObject **output, uint8_t capacity) const;
    // Returns object at collection index, or nullptr when out of range.
    TerraObject *objectAt(uint8_t index) const;
    // Returns number of registered objects.
    inline uint8_t objectCount() const { return (uint8_t)_objects.size(); }
    // Allocates a stable key, preferring name hash when available.
    uint32_t allocateKey(const TerraString &name = TerraString());
    // Updates all enabled registered objects.
    void updateObjects(uint32_t now = terraMillis());

protected:
    TerraMap<uint32_t, SharedPtr<TerraObject>, TERRA_MAX_OBJECTS> _objects; // Shared object collection, keyed by stable object key
    uint32_t _nextKey;                                      // Next automatically assigned object key
};

#endif
