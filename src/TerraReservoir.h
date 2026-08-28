/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Reservoirs
*/

#ifndef TerraReservoir_H
#define TerraReservoir_H

class TerraReservoir;
class TerraWaterReservoir;
class TerraThermalReservoir;
class TerraInfiniteWaterReservoir;
class TerraInfiniteThermalReservoir;

struct TerraReservoirData;
struct TerraWaterReservoirData;
struct TerraThermalReservoirData;
struct TerraInfiniteWaterReservoirData;
struct TerraInfiniteThermalReservoirData;

#include "Terraduino.h"
#include "TerraTriggers.h"

// Creates reservoir object from passed reservoir data (return ownership transfer - user code *must* delete returned object)
extern TerraReservoir *newReservoirObjectFromData(const TerraReservoirData *dataIn);


// Reservoir Base
// This is the base class for all reservoirs, which defines how the reservoir is
// identified, where it lives, what's attached to it, if it is full or empty, and
// who can activate under it.
class TerraReservoir : public TerraObject,
                              TerraReservoirObjectInterface {
public:
    const enum : signed char { Water, Thermal, WaterPipe, ThermalPipe, Unknown = -1 } classType; // Reservoir class type (custom RTTI)
    inline bool isWaterClass() const { return classType == Water; }
    inline bool isThermalClass() const { return classType == Thermal; }
    inline bool isWaterPipeClass() const { return classType == WaterPipe; }
    inline bool isThermalPipeClass() const { return classType == ThermalPipe; }
    inline bool isAnyWaterClass() const { return isWaterClass() || isWaterPipeClass(); }
    inline bool isAnyThermalClass() const { return isThermalClass() || isThermalPipeClass(); }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    TerraReservoir(Terra_ReservoirType reservoirType,
                   tposi_t reservoirIndex,
                   int classType = Unknown);
    TerraReservoir(const TerraReservoirData *dataIn);

    inline Terra_ReservoirType getReservoirType() const { return _id.objTypeAs.reservoirType; }
    inline tposi_t getReservoirIndex() const { return _id.posIndex; }

    Signal<TerraReservoir *, TERRA_RESERVOIR_SIGNAL_SLOTS> &getFilledSignal();
    Signal<TerraReservoir *, TERRA_RESERVOIR_SIGNAL_SLOTS> &getHighSignal();
    Signal<TerraReservoir *, TERRA_RESERVOIR_SIGNAL_SLOTS> &getLowSignal();
    Signal<TerraReservoir *, TERRA_RESERVOIR_SIGNAL_SLOTS> &getEmptySignal();

protected:
    Terra_TriggerState _filledState;                        // Filled state (last handled)
    Terra_TriggerState _highState;                          // High state (last handled)
    Terra_TriggerState _lowState;                           // Low state (last handled)
    Terra_TriggerState _emptyState;                         // Empty state (last handled)

    Signal<TerraReservoir *, TERRA_RESERVOIR_SIGNAL_SLOTS> _filledSignal; // Filled state signal
    Signal<TerraReservoir *, TERRA_RESERVOIR_SIGNAL_SLOTS> _highSignal; // High state signal
    Signal<TerraReservoir *, TERRA_RESERVOIR_SIGNAL_SLOTS> _lowSignal; // Low state signal
    Signal<TerraReservoir *, TERRA_RESERVOIR_SIGNAL_SLOTS> _emptySignal; // Empty state signal

    virtual TerraData *allocateData() const override;
    virtual void saveToData(TerraData *dataOut) override;

    virtual void handleFilled(Terra_TriggerState filledState);
    virtual void handleHigh(Terra_TriggerState highState);
    virtual void handleLow(Terra_TriggerState lowState);
    virtual void handleEmpty(Terra_TriggerState emptyState);
    friend class TerraWaterReservoir;
    friend class TerraThermalReservoir;
};

// Simple Water Reservoir
// Basic water reservoir that contains a volume of water and the ability to track such.
// Crude, but effective.
class TerraWaterReservoir : public TerraReservoir,
                                   TerraVolumeUnitsInterfaceStorage,
                                   TerraWaterVolumeSensorAttachmentInterface,
                                   TerraWaterTemperatureSensorAttachmentInterface,
                                   TerraFilledTriggerAttachmentInterface,
                                   TerraHighTriggerAttachmentInterface,
                                   TerraLowTriggerAttachmentInterface,
                                   TerraEmptyTriggerAttachmentInterface {
public:
    TerraWaterReservoir(hposi_t reservoirIndex,
                        float maxVolume,
                        int classType = Water);
    TerraWaterReservoir(const TerraWaterReservoirData *dataIn);

    virtual void update() override;
    virtual SharedPtr<TerraObjInterface> getSharedPtrFor(const TerraObjInterface *obj) const override;

    virtual float getLevel(bool poll = false) override;

    virtual Terra_ResourceState getState(bool poll = false) override;
    virtual bool isFilled(bool poll = false) override;
    virtual bool isHigh(bool poll = false) override;
    virtual bool isLow(bool poll = false) override;
    virtual bool isEmpty(bool poll = false) override;

    virtual void setVolumeUnits(Terra_UnitsType volumeUnits) override;

    virtual TerraSensorAttachment &getWaterVolumeSensorAttachment() override;

    virtual TerraTriggerAttachment &getFilledTriggerAttachment() override;

    virtual TerraTriggerAttachment &getHighTriggerAttachment() override;

    virtual TerraTriggerAttachment &getLowTriggerAttachment() override;

    virtual TerraTriggerAttachment &getEmptyTriggerAttachment() override;

    inline float getMaxVolume() const { return _maxVolume; }

protected:
    float _maxVolume;                                       // Maximum volume
    TerraSensorAttachment _waterVolume;                     // Water volume sensor attachment
    TerraTriggerAttachment _filledTrigger;                  // Filled trigger attachment
    TerraTriggerAttachment _highTrigger;                    // High trigger attachment
    TerraTriggerAttachment _lowTrigger;                     // Low trigger attachment
    TerraTriggerAttachment _emptyTrigger;                   // Empty trigger attachment

    virtual void saveToData(TerraData *dataOut) override;

    virtual void handleFilled(Terra_TriggerState filledState) override;
    virtual void handleHigh(Terra_TriggerState filledState) override;
    virtual void handleLow(Terra_TriggerState filledState) override;
    virtual void handleEmpty(Terra_TriggerState emptyState) override;
};

// Simple Thermal Reservoir
// Basic thermal reservoir that contains a thermal mass temperature and the ability to track such.
// Crude, but effective.
class TerraThermalReservoir : public TerraReservoir,
                                     TerraTemperatureUnitsInterfaceStorage,
                                     TerraMediumTemperatureSensorAttachmentInterface,
                                     TerraFilledTriggerAttachmentInterface,
                                     TerraHighTriggerAttachmentInterface,
                                     TerraLowTriggerAttachmentInterface,
                                     TerraEmptyTriggerAttachmentInterface {
public:
    TerraThermalReservoir(hposi_t reservoirIndex,
                          float maxTemperature,
                          int classType = Thermal);
    TerraThermalReservoir(const TerraThermalReservoirData *dataIn);

    virtual void update() override;
    virtual SharedPtr<TerraObjInterface> getSharedPtrFor(const TerraObjInterface *obj) const override;

    virtual float getLevel(bool poll = false) override;

    virtual Terra_ResourceState getState(bool poll = false) override;
    virtual bool isFilled(bool poll = false) override;
    virtual bool isHigh(bool poll = false) override;
    virtual bool isLow(bool poll = false) override;
    virtual bool isEmpty(bool poll = false) override;

    virtual void setTemperatureUnits(Terra_UnitsType temperatureUnits) override;

    virtual TerraSensorAttachment &getThermalVolumeSensorAttachment() override;

    virtual TerraTriggerAttachment &getFilledTriggerAttachment() override;

    virtual TerraTriggerAttachment &getHighTriggerAttachment() override;

    virtual TerraTriggerAttachment &getLowTriggerAttachment() override;

    virtual TerraTriggerAttachment &getEmptyTriggerAttachment() override;

    inline float getMaxTemperature() const { return _maxTemperature; }

protected:
    float _maxTemperature;                                  // Maximum temperature
    TerraSensorAttachment _temperatureSensor;               // Temperature sensor attachment
    TerraTriggerAttachment _filledTrigger;                  // Filled trigger attachment
    TerraTriggerAttachment _highTrigger;                    // High trigger attachment
    TerraTriggerAttachment _lowTrigger;                     // Low trigger attachment
    TerraTriggerAttachment _emptyTrigger;                   // Empty trigger attachment

    virtual void saveToData(TerraData *dataOut) override;

    virtual void handleFilled(Terra_TriggerState filledState) override;
    virtual void handleHigh(Terra_TriggerState filledState) override;
    virtual void handleLow(Terra_TriggerState filledState) override;
    virtual void handleEmpty(Terra_TriggerState emptyState) override;
};


// Reservoir Serialization Data
struct TerraReservoirData : public TerraObjectData {
    TerraTriggerSubData filledTrigger;                      // Filled trigger
    TerraTriggerSubData highTrigger;                        // High trigger
    TerraTriggerSubData lowTrigger;                         // Low trigger
    TerraTriggerSubData emptyTrigger;                       // Empty trigger

    TerraReservoirData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Water Reservoir Serialization Data
struct TerraWaterReservoirData : public TerraReservoirData {
    Terra_UnitsType volumeUnits;                            // Volume units
    float maxVolume;                                        // Maximum volume
    char volumeSensor[TERRA_NAME_MAXSIZE];                  // Volume sensor

    TerraWaterReservoirData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Thermal Reservoir Serialization Data
struct TerraThermalReservoirData : public TerraReservoirData {
    Terra_UnitsType temperatureUnits;                       // Temperature units
    float maxTemperature;                                   // Maximum temperature
    char temperatureSensor[TERRA_NAME_MAXSIZE];             // Temperature sensor attachment

    TerraThermalReservoirData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef TerraReservoir_H
