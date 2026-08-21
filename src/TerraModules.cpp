/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Controller Modules
*/

#include "Terraduino.h"
#include "TerraObject.h"
#include "TerraUtils.h"

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

const TerraCalibrationData *TerraCalibrations::getUserCalibrationData(uint32_t key) const
{
    auto iter = _calibrationData.find(key);
    if (iter != _calibrationData.end()) {
        return iter->second;
    }
    return nullptr;
}

bool TerraCalibrations::setUserCalibrationData(const TerraCalibrationData *calibrationData)
{
    TERRA_SOFT_ASSERT(calibrationData, TerraString("Invalid calibration data"));

    if (calibrationData && calibrationData->ownerKey != TERRA_INVALID_KEY) {
        auto iter = _calibrationData.find(calibrationData->ownerKey);
        bool retVal = false;

        if (iter == _calibrationData.end()) {
            auto calibData = new TerraCalibrationData();

            TERRA_SOFT_ASSERT(calibData, TerraString("Calibration allocation failure"));
            if (calibData) {
                *calibData = *calibrationData;
                _calibrationData[calibrationData->ownerKey] = calibData;
                retVal = (_calibrationData.find(calibrationData->ownerKey) != _calibrationData.end());
            }
        } else {
            *(iter->second) = *calibrationData;
            retVal = true;
        }

        return retVal;
    }
    return false;
}

bool TerraCalibrations::dropUserCalibrationData(const TerraCalibrationData *calibrationData)
{
    TERRA_HARD_ASSERT(calibrationData, TerraString("Invalid calibration data"));
    if (!calibrationData) return false;

    auto iter = _calibrationData.find(calibrationData->ownerKey);
    if (iter != _calibrationData.end()) {
        if (iter->second) { delete iter->second; }
        _calibrationData.erase(iter);
        return true;
    }

    return false;
}

TerraObjectRegistration::TerraObjectRegistration()
    : _objects(), _nextKey(1)
{ ; }

bool TerraObjectRegistration::registerObject(SharedPtr<TerraObject> object)
{
    if (!object || _objects.size() >= TERRA_MAX_OBJECTS) { return false; }
    if (object->getKey() == TERRA_INVALID_KEY) { object->setKey(allocateKey(object->getName())); }
    if (_objects.find(object->getKey()) != _objects.end()) { return false; }
    _objects[object->getKey()] = object;
    return true;
}

bool TerraObjectRegistration::unregisterObject(SharedPtr<TerraObject> object)
{
    if (!object) { return false; }
    auto iter = _objects.find(object->getKey());
    if (iter == _objects.end() || iter->second.get() != object.get()) { return false; }
    object->unresolve();
    _objects.erase(iter);
    return true;
}

SharedPtr<TerraObject> TerraObjectRegistration::sharedObjectByKey(uint32_t key) const
{
    auto iter = _objects.find(key);
    return iter != _objects.end() ? iter->second : SharedPtr<TerraObject>();
}

TerraObject *TerraObjectRegistration::findObjectByName(const TerraString &name) const
{
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (iter->second && iter->second->getName() == name) { return iter->second.get(); }
    }
    return nullptr;
}

TerraObject *TerraObjectRegistration::findFirstByType(Terra_ObjectType type) const
{
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (iter->second && iter->second->getObjectType() == type) { return iter->second.get(); }
    }
    return nullptr;
}

uint8_t TerraObjectRegistration::findByType(Terra_ObjectType type, TerraObject **output, uint8_t capacity) const
{
    if (!output || !capacity) { return 0; }
    uint8_t found = 0;
    for (auto iter = _objects.begin(); iter != _objects.end() && found < capacity; ++iter) {
        if (iter->second && iter->second->getObjectType() == type) { output[found++] = iter->second.get(); }
    }
    return found;
}

TerraObject *TerraObjectRegistration::objectAt(uint8_t index) const
{
    if (index >= _objects.size()) { return nullptr; }
    auto iter = _objects.begin();
    while (index--) { ++iter; }
    return iter->second.get();
}

uint32_t TerraObjectRegistration::allocateKey(const TerraString &name)
{
    uint32_t candidate = 0;
#if defined(ARDUINO)
    if (name.length()) { candidate = terraHashString(name.c_str()); }
#else
    if (!name.empty()) { candidate = terraHashString(name.c_str()); }
#endif
    if (candidate && _objects.find(candidate) == _objects.end()) { return candidate; }
    while (!_nextKey || _objects.find(_nextKey) != _objects.end()) { ++_nextKey; }
    return _nextKey++;
}

void TerraObjectRegistration::updateObjects(uint32_t now)
{
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (iter->second && iter->second->isEnabled()) { iter->second->update(now); }
    }
}
