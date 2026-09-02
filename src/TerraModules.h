/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Controller Modules
*/

#ifndef TerraModules_H
#define TerraModules_H

class TerraCalibrations;
class TerraObjectRegistration;
class TerraPinHandlers;
class TerraObject;
struct TerraCalibrationData;

#include "TerraDefines.h"
#include "TerraObject.h"
#include "TerraPins.h"

// Calibrations Storage
// Stores user calibration data, which calibrates sensor output into usable values.
class TerraCalibrations {
public:
    ~TerraCalibrations();
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
    Map<tkey_t, TerraCalibrationData *, TERRA_CAL_CALIBS_MAXSIZE> _calibrationData; // Loaded user calibration data
};

// Object Registration Storage
// Stores objects in the main system store, which is used for SharedPtr<> lookups and
// stable attachment resolution in the same manner as the sibling controller libraries.
class TerraObjectRegistration {
public:
    // Adds object to system, returning success.
    bool registerObject(SharedPtr<TerraObject> object);
    // Removes object from system, returning success.
    bool unregisterObject(SharedPtr<TerraObject> object);

    // Searches for object by identity key.
    SharedPtr<TerraObject> objectById(TerraIdentity id) const;

    // Finds first position either open or taken, given the identity type.
    tposi_t firstPosition(TerraIdentity id, bool taken);
    inline tposi_t firstPositionTaken(TerraIdentity id) { return firstPosition(id, true); }
    inline tposi_t firstPositionOpen(TerraIdentity id) { return firstPosition(id, false); }

protected:
    Map<tkey_t, SharedPtr<TerraObject>, TERRA_SYS_OBJECTS_MAXSIZE> _objects; // Shared object collection, keyed by TerraIdentity

    SharedPtr<TerraObject> objectById_Col(const TerraIdentity &id) const;
};


// Pin Handlers Storage
// Stores various pin-related system data on a shared pin # basis. Covers:
// - Pin locks: used for async shared resource management
// - Pin muxers: used for i/o pin multiplexing across a shared address bus
// - Pin expanders: used for i/o virtual pin expanding across an i2c interface
// - Pin OneWire: used for digital sensor pin's OneWire owner
class TerraPinHandlers {
public:
    // Attempts to get a lock on pin #, to prevent multi-device comm overlap (e.g. for OneWire comms).
    bool tryGetPinLock(pintype_t pin, millis_t wait = 150);
    // Returns a locked pin lock for the given pin. Only call if pin lock was successfully locked.
    inline void returnPinLock(pintype_t pin) { _pinLocks.erase(pin); }

    // Sets pin muxer for pin #.
    inline void setPinMuxer(pintype_t pin, SharedPtr<TerraPinMuxer> pinMuxer) { _pinMuxers[pin] = pinMuxer; }
    // Returns pin muxer for pin #.
    inline SharedPtr<TerraPinMuxer> getPinMuxer(pintype_t pin) { return _pinMuxers[pin]; }
    // Deactivates all pin muxers. Called before selecting another channel if pin muxers are assumed
    // to have a shared address bus (based on TERRA_MUXERS_SHARED_ADDR_BUS setting).
    void deactivatePinMuxers();

#ifdef TERRA_USE_MULTITASKING

    // Sets pin expander for index.
    inline void setPinExpander(tposi_t index, SharedPtr<TerraPinExpander> pinExpander) { _pinExpanders[index] = pinExpander; }
    // Returns expander for index.
    inline SharedPtr<TerraPinExpander> getPinExpander(tposi_t index) { return _pinExpanders[index]; }

#endif // /ifdef TERRA_USE_MULTITASKING

    // OneWire instance for given pin (lazily instantiated)
    OneWire *getOneWireForPin(pintype_t pin);
    // Drops OneWire instance for given pin (if created)
    void dropOneWireForPin(pintype_t pin);

protected:
    Map<pintype_t, OneWire *, TERRA_SYS_ONEWIRES_MAXSIZE> _pinOneWire; // Pin OneWire mapping
    Map<pintype_t, pintype_t, TERRA_SYS_PINLOCKS_MAXSIZE> _pinLocks; // Pin locks mapping (existence = locked)
    Map<pintype_t, SharedPtr<TerraPinMuxer>, TERRA_SYS_PINMUXERS_MAXSIZE> _pinMuxers; // Pin muxers mapping
#ifdef TERRA_USE_MULTITASKING
    Map<tposi_t, SharedPtr<TerraPinExpander>, TERRA_SYS_PINEXPANDERS_MAXSIZE> _pinExpanders; // Pin expanders mapping
#endif
};

#endif
