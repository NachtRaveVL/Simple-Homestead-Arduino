/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Controller Modules
*/

#include "Terraduino.h"
#include "TerraObject.h"
#include "TerraDatas.h"


TerraCalibrations::~TerraCalibrations()
{
    clearUserCalibrations();
}

void TerraCalibrations::clearUserCalibrations()
{
    for (auto iter = _calibrationData.begin(); iter != _calibrationData.end(); ++iter) {
        if (iter->second) { delete iter->second; }
    }
    _calibrationData.clear();
}

const TerraCalibrationData *TerraCalibrations::getUserCalibrationData(tkey_t key) const
{
    auto iter = _calibrationData.find(key);
    if (iter != _calibrationData.end()) {
        return iter->second;
    }
    return nullptr;
}

bool TerraCalibrations::setUserCalibrationData(const TerraCalibrationData *calibrationData)
{
    TERRA_SOFT_ASSERT(calibrationData, SFP(TStr_Err_InvalidParameter));

    if (calibrationData && calibrationData->ownerName[0]) {
        tkey_t key = terraHashString(calibrationData->ownerName);
        auto iter = _calibrationData.find(key);
        bool retVal = false;

        if (iter == _calibrationData.end()) {
            auto calibData = new TerraCalibrationData();

            TERRA_SOFT_ASSERT(calibData, SFP(TStr_Err_AllocationFailure));
            if (calibData) {
                *calibData = *calibrationData;
                _calibrationData[key] = calibData;
                retVal = (_calibrationData.find(key) != _calibrationData.end());
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
    TERRA_HARD_ASSERT(calibrationData, SFP(TStr_Err_InvalidParameter));
    if (!calibrationData) { return false; }

    tkey_t key = terraHashString(calibrationData->ownerName);
    auto iter = _calibrationData.find(key);
    if (iter != _calibrationData.end()) {
        if (iter->second) { delete iter->second; }
        _calibrationData.erase(iter);
        return true;
    }

    return false;
}


void TerraObjectRegistration::clearObjects()
{
    while (_objects.size()) {
        auto iter = _objects.begin();
        if (iter->second) { iter->second->unresolve(); }
        _objects.erase(iter);
    }
}

bool TerraObjectRegistration::registerObject(SharedPtr<TerraObject> object)
{
    if (!object || object->getKey() == tkey_none || _objects.find(object->getKey()) != _objects.end()) { return false; }
    _objects[object->getKey()] = object;
    if (getScheduler()) { getScheduler()->setNeedsScheduling(); }
    return true;
}

bool TerraObjectRegistration::unregisterObject(SharedPtr<TerraObject> object)
{
    if (!object) { return false; }
    auto iter = _objects.find(object->getKey());
    if (iter == _objects.end() || iter->second.get() != object.get()) { return false; }

    object->unresolve();
    _objects.erase(iter);
    if (getScheduler()) { getScheduler()->setNeedsScheduling(); }
    return true;
}

SharedPtr<TerraObject> TerraObjectRegistration::objectById(TerraIdentity id) const
{
    if (id.posIndex == TERRA_POS_SEARCH_FROMBEG) {
        while (++id.posIndex < TERRA_POS_MAXSIZE) {
            auto iter = _objects.find(id.regenKey());
            if (iter != _objects.end()) {
                if (id.keyString == iter->second->getKeyString()) {
                    return iter->second;
                } else {
                    return objectById_Col(id);
                }
            }
        }
    } else if (id.posIndex == TERRA_POS_SEARCH_FROMEND) {
        while (--id.posIndex >= 0) {
            auto iter = _objects.find(id.regenKey());
            if (iter != _objects.end()) {
                if (id.keyString == iter->second->getKeyString()) {
                    return iter->second;
                } else {
                    return objectById_Col(id);
                }
            }
        }
    } else {
        auto iter = _objects.find(id.key);
        if (iter != _objects.end()) {
            if (id.type == Terra_ObjectType_Undefined || !id.keyString.length() || id.keyString == iter->second->getKeyString()) {
                return iter->second;
            } else {
                return objectById_Col(id);
            }
        }
    }

    return nullptr;
}

SharedPtr<TerraObject> TerraObjectRegistration::objectById_Col(const TerraIdentity &id) const
{
    TERRA_SOFT_ASSERT(false, SFP(TStr_Err_HashingCollision));

    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (id.keyString == iter->second->getKeyString()) {
            return iter->second;
        }
    }

    return nullptr;
}

tposi_t TerraObjectRegistration::firstPosition(TerraIdentity id, bool taken) const
{
    if (id.posIndex != TERRA_POS_SEARCH_FROMEND) {
        id.posIndex = TERRA_POS_SEARCH_FROMBEG;
        while (++id.posIndex < TERRA_POS_MAXSIZE) {
            auto iter = _objects.find(id.regenKey());
            if (taken == (iter != _objects.end())) {
                return id.posIndex;
            }
        }
    } else {
        id.posIndex = TERRA_POS_SEARCH_FROMEND;
        while (--id.posIndex >= 0) {
            auto iter = _objects.find(id.regenKey());
            if (taken == (iter != _objects.end())) {
                return id.posIndex;
            }
        }
    }

    return tposi_none;
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

void TerraObjectRegistration::updateObjects(uint32_t now)
{
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (iter->second && iter->second->isEnabled()) { iter->second->update(now); }
    }
}
