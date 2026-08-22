/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Interfaces
*/

#ifndef TerraInterfaces_H
#define TerraInterfaces_H

class TerraObject;
class TerraSensor;
class TerraActuator;
class TerraRTCInterface;

struct TerraIdentity;
struct TerraMeasurement;
struct TerraSingleMeasurement;

#include "TerraDefines.h"

// JSON Serializable Interface
struct TerraJSONSerializableInterface {
    virtual void toJSONObject(JsonObject &objectOut) const = 0;
    virtual void fromJSONObject(JsonObjectConst &objectIn) = 0;
};

// Object Interface
class TerraObjInterface {
public:
    virtual ~TerraObjInterface() { ; }

    virtual void unresolveAny(TerraObject *object) = 0;

    virtual TerraIdentity getId() const = 0;
    virtual tkey_t getKey() const = 0;
    virtual TerraString getKeyString() const = 0;
    virtual SharedPtr<TerraObjInterface> getSharedPtr() const = 0;
    virtual SharedPtr<TerraObjInterface> getSharedPtrFor(const TerraObjInterface *object) const = 0;

    virtual bool isObject() const = 0;
    inline bool isSubObject() const { return !isObject(); }
};


// Measurement Units Interface
class TerraMeasurementUnitsInterface {
public:
    virtual ~TerraMeasurementUnitsInterface() { ; }
    virtual void setMeasurementUnits(Terra_Unit measurementUnits, uint8_t measurementRow = 0) = 0;
    virtual Terra_Unit getMeasurementUnits(uint8_t measurementRow = 0) const = 0;
};

// Measurement Units Storage
template <size_t N>
class TerraMeasurementUnitsStorage {
protected:
    Terra_Unit _measurementUnits[N];                        // Stored measurement units by row

    inline TerraMeasurementUnitsStorage(Terra_Unit measurementUnits = Terra_Unit_Undefined)
    {
        for (size_t index = 0; index < N; ++index) { _measurementUnits[index] = measurementUnits; }
    }
};

class TerraMeasurementUnitsInterfaceStorageSingle : public TerraMeasurementUnitsInterface,
                                                    public TerraMeasurementUnitsStorage<1> {
public:
    virtual void setMeasurementUnits(Terra_Unit measurementUnits, uint8_t measurementRow = 0) override
        { if (!measurementRow) { _measurementUnits[0] = measurementUnits; } }
    virtual Terra_Unit getMeasurementUnits(uint8_t measurementRow = 0) const override
        { return !measurementRow ? _measurementUnits[0] : Terra_Unit_Undefined; }

protected:
    inline TerraMeasurementUnitsInterfaceStorageSingle(Terra_Unit measurementUnits = Terra_Unit_Undefined)
        : TerraMeasurementUnitsStorage<1>(measurementUnits) { ; }
};

#ifdef ARDUINO
// RTC Interface
// Common interface used by supported RTClib real-time clock implementations.
class TerraRTCInterface {
public:
    virtual ~TerraRTCInterface() { ; }
    virtual bool begin(TwoWire *wire = TERRA_USE_WIRE) = 0;
    virtual void adjust(const DateTime &dt) = 0;
    virtual bool lostPower() = 0;
    virtual DateTime now() = 0;
};
#endif

#endif
