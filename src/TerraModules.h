/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Controller Modules
*/

#ifndef TerraModules_H
#define TerraModules_H

class TerraCalibrations;
class TerraObjectRegistration;
class TerraObject;
struct TerraCalibrationData;

#include "TerraDefines.h"
#include "TerraObject.h"

// Calibrations Storage
// Stores user calibration data, which calibrates sensor output into usable values.
class TerraCalibrations {
public:
    virtual ~TerraCalibrations();
    void clearUserCalibrations();

    // Adds/updates user calibration data to the store, returning success flag.
    bool setUserCalibrationData(const TerraCalibrationData *calibrationData);

    // Drops/removes user calibration data from the store, returning success flag.
    bool dropUserCalibrationData(const TerraCalibrationData *calibrationData);

    // Returns user calibration data instance in store.
    const TerraCalibrationData *getUserCalibrationData(tkey_t key) const;

    // Returns if there are user calibrations in the store.
    inline bool hasUserCalibrations() const { return _calibrationData.size(); };

protected:
    Map<tkey_t, TerraCalibrationData *, TERRA_MAX_OBJECTS> _calibrationData; // Loaded user calibration data
};

// Object Registration Storage
// Stores objects in the main system store, which is used for SharedPtr<> lookups and
// stable attachment resolution in the same manner as the sibling controller libraries.
class TerraObjectRegistration {
public:
    void clearObjects();

    // Adds object to system, returning success.
    bool registerObject(SharedPtr<TerraObject> object);
    // Removes object from system, returning success.
    bool unregisterObject(SharedPtr<TerraObject> object);

    // Searches for object by identity key.
    SharedPtr<TerraObject> objectById(TerraIdentity id) const;

    // Finds first position either open or taken, given the identity type.
    tposi_t firstPosition(TerraIdentity id, bool taken) const;
    inline tposi_t firstPositionTaken(TerraIdentity id) const { return firstPosition(id, true); }
    inline tposi_t firstPositionOpen(TerraIdentity id) const { return firstPosition(id, false); }

    // Updates registered system objects.
    void updateObjects(uint32_t now = millis());

    // Enumeration helpers retained for domain queries, not identity.
    TerraObject *findFirstByType(Terra_ObjectType type) const;
    uint8_t findByType(Terra_ObjectType type, TerraObject **output, uint8_t capacity) const;
    TerraObject *objectAt(uint8_t index) const;
    inline uint8_t objectCount() const { return (uint8_t)_objects.size(); }

protected:
    Map<tkey_t, SharedPtr<TerraObject>, TERRA_MAX_OBJECTS> _objects; // Shared object collection, keyed by TerraIdentity

    SharedPtr<TerraObject> objectById_Col(const TerraIdentity &id) const;
};

#endif
