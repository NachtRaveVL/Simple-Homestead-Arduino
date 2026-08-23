/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Reservoirs
*/

#ifndef TerraReservoir_H
#define TerraReservoir_H

class TerraReservoir;

struct TerraReservoirData;
struct TerraWaterStorageData;
struct TerraCisternData;
struct TerraWaterSourceData;
struct TerraThermalStoreData;

#include "TerraObject.h"
#include "TerraAttachments.h"

// Creates reservoir object from passed reservoir data (return ownership transfer - user code *must* delete returned object)
extern TerraReservoir *newReservoirObjectFromData(const TerraReservoirData *dataIn);

// Reservoir Base
// Normalized stored-resource level with reserve, low, high, and fault states.
class TerraReservoir : public TerraObject {
public:
    const enum : signed char { Base, WaterStorage, Cistern, WaterSource, ThermalStore, Unknown = -1 } classType; // Reservoir class type (custom RTTI)

    TerraReservoir(Terra_ReservoirType type = Terra_ReservoirType_Undefined,
                   tposi_t reservoirIndex = TERRA_POS_SEARCH_FROMBEG,
                   const TerraString &name = TerraString(),
                   int classTypeIn = Base);
    TerraReservoir(const TerraReservoirData *dataIn);

    inline Terra_ReservoirType getReservoirType() const { return _id.objTypeAs.reservoirType; }
    inline tposi_t getReservoirIndex() const { return _id.posIndex; }
    Terra_ResourceState getState() const { return _state; }
    float getLevel() const { return _level; }
    float getReserveLevel() const { return _reserveLevel; }
    float getLowLevel() const { return _lowLevel; }
    float getHighLevel() const { return _highLevel; }

    bool setThresholds(float reserveLevel, float lowLevel, float highLevel);
    void setLevel(float level);
    void setFault(const TerraString &message = TerraString());
    void clearFault();

protected:
    Terra_ResourceState _state;                             // Current trigger/resource state
    float _level;                                           // Normalized level, percent
    float _reserveLevel;                                    // Protected reserve level, percent
    float _lowLevel;                                        // Low threshold, percent
    float _highLevel;                                       // High threshold, percent

    void updateState();
    virtual TerraData *allocateData() const override;
    virtual void saveToData(TerraData *dataOut) const override;
};

// Reservoir Serialization Data
struct TerraReservoirData : public TerraObjectData {
    float level;                                            // Normalized level, percent
    float reserveLevel;                                     // Protected reserve level, percent
    float lowLevel;                                         // Low threshold, percent
    float highLevel;                                        // High threshold, percent

    TerraReservoirData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Water Storage Serialization Data
struct TerraWaterStorageData : public TerraReservoirData {
    float capacityLiters;                                   // Storage capacity, liters
    char levelSensor[TERRA_NAME_MAXSIZE];                   // Level sensor attachment

    TerraWaterStorageData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Cistern Serialization Data
struct TerraCisternData : public TerraWaterStorageData {
    float fillStartPercent;                                 // Fill-start threshold, percent
    float fillStopPercent;                                  // Fill-stop threshold, percent
    float overflowPercent;                                  // Overflow threshold, percent

    TerraCisternData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};


// Water Source Serialization Data
struct TerraWaterSourceData : public TerraReservoirData {
    uint8_t priority;                                       // Source priority
    bool available;                                         // Configured availability
    float maximumFlowLpm;                                   // Maximum allowed flow
    char levelSensor[TERRA_NAME_MAXSIZE];                   // Level sensor attachment

    TerraWaterSourceData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Thermal Store Serialization Data
struct TerraThermalStoreData : public TerraReservoirData {
    float temperatureC;                                     // Store temperature, degrees Celsius
    float minimumTargetC;                                   // Minimum target temperature, degrees Celsius
    float maximumTargetC;                                   // Maximum target temperature, degrees Celsius
    float absoluteMaximumC;                                 // Absolute safety maximum, degrees Celsius
    char temperatureSensor[TERRA_NAME_MAXSIZE];             // Temperature sensor attachment

    TerraThermalStoreData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef TerraReservoir_H
