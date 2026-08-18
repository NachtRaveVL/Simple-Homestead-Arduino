/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Balancers
*/

#ifndef TerraBalancers_H
#define TerraBalancers_H

#include "TerraWater.h"
#include "TerraThermal.h"

// Water Balancer
// Selects sources/storage and evaluates safe water-transfer decisions.
class TerraWaterBalancer {
public:
    TerraTransferDecision evaluate(TerraWaterRoute &route,
                                   const TerraWaterSource &source,
                                   const TerraWaterStorage &destination) const;

    // Selects the usable source with the lowest numeric priority value.
    // Sources at or below their reserve level are not considered usable.
    const TerraWaterSource *selectSource(const TerraWaterSource *const *sources,
                                         uint8_t count) const;

    TerraCistern *selectFillCistern(TerraCistern *const *cisterns, uint8_t count) const;
    const TerraCistern *selectSupplyCistern(const TerraCistern *const *cisterns, uint8_t count) const;
    float transferAllowance(const TerraCistern &source, const TerraCistern &destination, float requestedLiters) const;
};

// Thermal Balancer
// Evaluates differential-temperature circulation for a thermal loop.
class TerraThermalBalancer {
public:
    bool evaluate(TerraThermalLoop &loop, float sourceTempC, float storeTempC) const;
};

#endif
