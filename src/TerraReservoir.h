/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Reservoirs
*/

#ifndef TerraReservoir_H
#define TerraReservoir_H

struct TerraReservoirData;

#include "TerraObject.h"
#include "TerraDatas.h"

// Resource Base
// Normalized resource level with reserve, low, high, and fault states.
class TerraReservoir : public TerraObject {
public:
    TerraReservoir(Terra_ReservoirType type = Terra_ReservoirType_Undefined,
                  tposi_t resourceIndex = TERRA_POS_SEARCH_FROMBEG,
                  const TerraString &name = TerraString());
    TerraReservoir(const TerraReservoirData *dataIn);

protected:
    TerraReservoir(TerraIdentity id, const TerraString &name = TerraString());

public:

    Terra_ReservoirType getType() const { return _id.objTypeAs.ReservoirType; }
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

#endif
