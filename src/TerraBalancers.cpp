/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Balancers
*/

#include "TerraBalancers.h"

TerraTransferDecision TerraWaterBalancer::evaluate(TerraWaterRoute &route,
                                                    const TerraWaterSource &source,
                                                    const TerraWaterStorage &destination) const {
    return route.evaluate(source, destination);
}

const TerraWaterSource *TerraWaterBalancer::selectSource(const TerraWaterSource *const *sources,
                                                         uint8_t count) const {
    if (!sources || !count) return nullptr;
    const TerraWaterSource *selected = nullptr;
    for (uint8_t i = 0; i < count; ++i) {
        const TerraWaterSource *source = sources[i];
        if (!source || !source->isAvailable() || source->getLevel() <= source->getReserveLevel()) continue;
        if (!selected || source->getPriority() < selected->getPriority()) selected = source;
    }
    return selected;
}

TerraCistern *TerraWaterBalancer::selectFillCistern(TerraCistern *const *cisterns, uint8_t count) const {
    if (!cisterns || !count) return nullptr;
    TerraCistern *selected = nullptr;
    for (uint8_t i = 0; i < count; ++i) {
        TerraCistern *cistern = cisterns[i];
        if (!cistern || !cistern->needsFill(false)) continue;
        if (!selected || cistern->getLevel() < selected->getLevel()) selected = cistern;
    }
    return selected;
}

const TerraCistern *TerraWaterBalancer::selectSupplyCistern(const TerraCistern *const *cisterns, uint8_t count) const {
    if (!cisterns || !count) return nullptr;
    const TerraCistern *selected = nullptr;
    for (uint8_t i = 0; i < count; ++i) {
        const TerraCistern *cistern = cisterns[i];
        if (!cistern || !cistern->canSupplyWater()) continue;
        if (!selected || cistern->availableAboveReserveLiters() > selected->availableAboveReserveLiters()) selected = cistern;
    }
    return selected;
}

float TerraWaterBalancer::transferAllowance(const TerraCistern &source, const TerraCistern &destination, float requestedLiters) const {
    if (!source.canSupplyWater() || !destination.canAcceptWater()) return 0.0f;
    const float sourceReserve = source.getCapacityLiters() * (source.getReserveLevel() / 100.0f);
    const float destinationTarget = destination.getCapacityLiters() * (destination.getFillStopPercent() / 100.0f);
    return terraCisternTransferLiters(source.getStoredLiters(), sourceReserve, destination.getStoredLiters(), destinationTarget, requestedLiters);
}

bool TerraThermalBalancer::evaluate(TerraThermalLoop &loop, float sourceTempC, float storeTempC) const {
    bool run = loop.shouldCirculate(sourceTempC, storeTempC);
    loop.setRunning(run);
    return run;
}
