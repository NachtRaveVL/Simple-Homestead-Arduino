/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Power Rails
*/

#ifndef TerraRails_H
#define TerraRails_H

class TerraRail;
class TerraSimpleRail;
class TerraRegulatedRail;

struct TerraRailData;
struct TerraSimpleRailData;
struct TerraRegulatedRailData;

#include "Terraduino.h"

// Creates rail object from passed rail data (return ownership transfer - user code *must* delete returned object)
extern TerraRail *newRailObjectFromData(const TerraRailData *dataIn);


// Power Rail Base
// This is the base class for all power rails, which defines how the rail is identified,
// where it lives, what's attached to it, and who can activate under it.
class TerraRail : public TerraObject,
                  public TerraRailObjectInterface,
                  public TerraPowerUnitsInterfaceStorage {
public:
    const enum : signed char { Simple, Regulated, Unknown = -1 } classType; // Power rail class (custom RTTI)
    inline bool isSimpleClass() const { return classType == Simple; }
    inline bool isRegulatedClass() const { return classType == Regulated; }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    TerraRail(Terra_RailType railType,
              tposi_t railIndex,
              int classType = Unknown);
    TerraRail(const TerraRailData *dataIn);
    virtual ~TerraRail();

    virtual void update() override;

    virtual bool addLinkage(TerraObject *obj) override;
    virtual bool removeLinkage(TerraObject *obj) override;

    inline Terra_RailType getRailType() const { return _id.objTypeAs.railType; }
    inline tposi_t getRailIndex() const { return _id.posIndex; }
    inline float getRailVoltage() const { return getRailVoltageFromType(getRailType()); }

    Signal<TerraRail *, TERRA_RAIL_SIGNAL_SLOTS> &getCapacitySignal();

protected:
    Terra_TriggerState _limitState;                         // Limit state (last handled)

    Signal<TerraRail *, TERRA_RAIL_SIGNAL_SLOTS> _capacitySignal; // Capacity changed signal

    virtual TerraData *allocateData() const override;
    virtual void saveToData(TerraData *dataOut) override;

    void handleLimit(Terra_TriggerState limitState);
    friend class TerraRegulatedRail;
};

// Simple Power Rail
// Basic power rail that tracks # of devices turned on, with a limit to how many
// can be on at the same time. Crude, but effective, especially when all devices
// along the rail will use about the same amount of power anyways.
class TerraSimpleRail : public TerraRail {
public:
    TerraSimpleRail(Terra_RailType railType,
                    tposi_t railIndex,
                    int maxActiveAtOnce = 2,
                    int classType = Simple);
    TerraSimpleRail(const TerraSimpleRailData *dataIn);

    virtual bool canActivate(TerraActuator *actuator) override;
    virtual float getCapacity(bool poll = false) override;

    virtual void setPowerUnits(Terra_UnitsType powerUnits) override;

    inline int getActiveCount() { return _activeCount; }

protected:
    int _activeCount;                                       // Current active count
    int _maxActiveAtOnce;                                   // Max active count

    virtual void saveToData(TerraData *dataOut) override;

    void handleActivation(TerraActuator *actuator);
    friend class TerraRail;
};

// Regulated Power Rail
// Power rail that has a max power rating and power sensor that can track power
// usage, with limit trigger for over-power state limiting actuator activation.
class TerraRegulatedRail : public TerraRail,
                           public TerraPowerUsageSensorAttachmentInterface,
                           public TerraLimitTriggerAttachmentInterface {
public:
    TerraRegulatedRail(Terra_RailType railType,
                       tposi_t railIndex,
                       float maxPower,
                       int classType = Regulated);
    TerraRegulatedRail(const TerraRegulatedRailData *dataIn);

    virtual void update() override;
    virtual SharedPtr<TerraObjInterface> getSharedPtrFor(const TerraObjInterface *obj) const override;

    virtual bool canActivate(TerraActuator *actuator) override;
    virtual float getCapacity(bool poll = false) override;

    virtual void setPowerUnits(Terra_UnitsType powerUnits) override;

    virtual TerraSensorAttachment &getPowerUsageSensorAttachment() override;

    virtual TerraTriggerAttachment &getLimitTriggerAttachment() override;

    inline float getMaxPower() const { return _maxPower; }

protected:
    float _maxPower;                                        // Maximum power
    TerraSensorAttachment _powerUsage;                      // Power usage sensor attachment
    TerraTriggerAttachment _limitTrigger;                   // Power limit trigger attachment

    virtual void saveToData(TerraData *dataOut) override;

    void handleActivation(TerraActuator *actuator);
    friend class TerraRail;

    void handlePower(const TerraMeasurement *measurement);
};


// Rail Serialization Data
struct TerraRailData : public TerraObjectData
{
    Terra_UnitsType powerUnits;                             // Power units

    TerraRailData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Simple Rail Serialization Data
struct TerraSimpleRailData : public TerraRailData
{
    int maxActiveAtOnce;                                    // Max active count

    TerraSimpleRailData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Regulated Rail Serialization Data
struct TerraRegulatedRailData : public TerraRailData
{
    float maxPower;                                         // Maximum power
    char powerUsageSensor[TERRA_NAME_MAXSIZE];              // Power usage sensor
    TerraTriggerSubData limitTrigger;                       // Power limit trigger

    TerraRegulatedRailData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef TerraRails_H
