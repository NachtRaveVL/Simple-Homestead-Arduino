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
    while (_calibrationData.size()) {
        auto iter = _calibrationData.begin();
        tkey_t key = iter->first;

        if (iter->second && getController()) {
            TerraIdentity ownerId(iter->second->ownerName);
            ownerId.posIndex = 0;
            auto owner = getController()->objectById(ownerId);

            if (owner) {
                if (owner->isSensorType()) {
                    static_pointer_cast<TerraSensor>(owner)->setUserCalibrationData(nullptr);
                } else if (owner->isActuatorType()) {
                    static_pointer_cast<TerraActuator>(owner)->setUserCalibrationData(nullptr);
                }
            }
        }

        iter = _calibrationData.find(key);
        if (iter != _calibrationData.end()) {
            if (iter->second) { delete iter->second; }
            _calibrationData.erase(iter);
        }
    }
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
        tkey_t key = stringHash(calibrationData->ownerName);
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

    tkey_t key = stringHash(calibrationData->ownerName);
    auto iter = _calibrationData.find(key);
    if (iter != _calibrationData.end()) {
        if (iter->second) { delete iter->second; }
        _calibrationData.erase(iter);
        return true;
    }

    return false;
}


bool TerraObjectRegistration::registerObject(SharedPtr<TerraObject> object)
{
    TERRA_SOFT_ASSERT(object && object->getId().posIndex >= 0 && object->getId().posIndex < TERRA_POS_MAXSIZE, SFP(TStr_Err_InvalidParameter));
    if (object && _objects.find(object->getKey()) == _objects.end()) {
        _objects[object->getKey()] = object;

        if (object->isActuatorType() || object->isReservoirType()) {
            if (getScheduler()) {
                getScheduler()->setNeedsScheduling();
            }
        }
        if (object->isSensorType()) {
            if (getPublisher()) {
                getPublisher()->setNeedsTabulation();
            }
        }

        return true;
    }
    return false;
}

bool TerraObjectRegistration::unregisterObject(SharedPtr<TerraObject> object)
{
    TERRA_SOFT_ASSERT(object, SFP(TStr_Err_InvalidParameter));
    if (!object) { return false; }

    auto iter = _objects.find(object->getKey());
    if (iter != _objects.end()) {
        _objects.erase(iter);

        if (object->isActuatorType() || object->isReservoirType()) {
            if (getScheduler()) {
                getScheduler()->setNeedsScheduling();
            }
        }
        if (object->isSensorType()) {
            if (getPublisher()) {
                getPublisher()->setNeedsTabulation();
            }
        }

        return true;
    }
    return false;
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
            if (id.keyString == iter->second->getKeyString()) {
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
    TERRA_SOFT_ASSERT(false, F("Hashing collision"));

    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (id.keyString == iter->second->getKeyString()) {
            return iter->second;
        }
    }

    return nullptr;
}

tposi_t TerraObjectRegistration::firstPosition(TerraIdentity id, bool taken)
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

    return -1;
}


bool TerraPinHandlers::tryGetPinLock(pintype_t pin, millis_t wait)
{
    millis_t start = millis();
    while (1) {
        auto iter = _pinLocks.find(pin);
        if (iter == _pinLocks.end()) {
            _pinLocks[pin] = true;
            return (_pinLocks.find(pin) != _pinLocks.end());
        }
        else if (millis() - start >= wait) { return false; }
        else { yield(); }
    }
}

void TerraPinHandlers::deactivatePinMuxers()
{
    for (auto iter = _pinMuxers.begin(); iter != _pinMuxers.end(); ++iter) {
        iter->second->deactivate();
    }
}

OneWire *TerraPinHandlers::getOneWireForPin(pintype_t pin)
{
    auto wireIter = _pinOneWire.find(pin);
    if (wireIter != _pinOneWire.end()) {
        return wireIter->second;
    } else {
        OneWire *oneWire = new OneWire(pin);
        if (oneWire) {
            _pinOneWire[pin] = oneWire;
            if (_pinOneWire.find(pin) != _pinOneWire.end()) { return oneWire; }
            else if (oneWire) { delete oneWire; }
        } else if (oneWire) { delete oneWire; }
    }
    return nullptr;
}

void TerraPinHandlers::dropOneWireForPin(pintype_t pin)
{
    auto wireIter = _pinOneWire.find(pin);
    if (wireIter != _pinOneWire.end()) {
        if (wireIter->second) {
            wireIter->second->depower();
            delete wireIter->second;
        }
        _pinOneWire.erase(wireIter);
    }
}
