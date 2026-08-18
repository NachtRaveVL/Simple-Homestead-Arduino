/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Modules
*/

#include "TerraModules.h"

TerraModuleRegistry::TerraModuleRegistry() : _count(0) { }

int8_t TerraModuleRegistry::add(Terra_ModuleType type, const TerraString &name, const TerraDeviceSetup &setup) {
    if (type == Terra_ModuleType_Undefined || _count >= TERRA_MAX_MODULES) return -1;
    TerraModule &module = _modules[_count];
    module.type = type; module.name = name; module.setup = setup; module.enabled = true; module.online = false;
    return (int8_t)_count++;
}

bool TerraModuleRegistry::remove(uint8_t index) {
    if (index >= _count) return false;
    for (uint8_t i = index + 1; i < _count; ++i) _modules[i - 1] = _modules[i];
    --_count;
    return true;
}

TerraModule *TerraModuleRegistry::at(uint8_t index) { return index < _count ? &_modules[index] : nullptr; }
const TerraModule *TerraModuleRegistry::at(uint8_t index) const { return index < _count ? &_modules[index] : nullptr; }
TerraModule *TerraModuleRegistry::find(Terra_ModuleType type) { for (uint8_t i = 0; i < _count; ++i) if (_modules[i].type == type) return &_modules[i]; return nullptr; }
