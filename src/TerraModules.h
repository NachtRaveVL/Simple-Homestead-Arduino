/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Modules
*/

#ifndef TerraModules_H
#define TerraModules_H

#include "TerraDefines.h"
#include "TerraTypes.h"
#include "TerraSetup.h"

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
