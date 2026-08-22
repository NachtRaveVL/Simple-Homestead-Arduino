/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Power Rails
*/

#ifndef TerraRails_H
#define TerraRails_H

struct TerraPowerRailData;

#include "TerraObject.h"
#include "TerraDatas.h"

class TerraPowerRail : public TerraObject {
public:
    TerraPowerRail(float nominalVoltage = 0.0f,
                   tposi_t railIndex = TERRA_POS_SEARCH_FROMBEG,
                   const TerraString &name = TerraString(),
                   Terra_RailType railType = Terra_RailType_Custom);  // Power rail type
    TerraPowerRail(const TerraPowerRailData *dataIn);

    void setEnabledState(bool enabled);
    bool isRailEnabled() const { return _railEnabled; }
    float getNominalVoltage() const { return _nominalVoltage; }
    Terra_RailType getRailType() const { return _id.objTypeAs.railType; }
    void setMeasuredVoltage(float voltage) { _measuredVoltage = voltage; }
    float getMeasuredVoltage() const { return _measuredVoltage; }

protected:
    float _nominalVoltage;                                  // Nominal rail voltage
    float _measuredVoltage;                                 // Latest measured rail voltage
    bool _railEnabled;                                      // Rail enabled state

    virtual TerraData *allocateData() const override;
    virtual void saveToData(TerraData *dataOut) const override;
};

#endif
