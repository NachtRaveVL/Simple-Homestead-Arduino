/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Core Logic
*/

#ifndef TerraCoreLogic_H
#define TerraCoreLogic_H

#include "TerraTypes.h"

inline Terra_ResourceState terraClassifyResourceState(float level, float reserveLevel, float lowLevel, float highLevel, bool fault = false) {
    if (fault) return Terra_ResourceState_Fault;
    if (level <= reserveLevel) return Terra_ResourceState_Reserve;
    if (level <= lowLevel) return Terra_ResourceState_Low;
    if (level >= highLevel) return Terra_ResourceState_High;
    return Terra_ResourceState_Normal;
}

inline float terraCisternTransferLiters(float sourceStoredLiters,
                                        float sourceReserveLiters,
                                        float destinationStoredLiters,
                                        float destinationTargetLiters,
                                        float requestedLiters) {
    if (requestedLiters <= 0.0f) return 0.0f;
    const float sourceAvailable = sourceStoredLiters > sourceReserveLiters ? sourceStoredLiters - sourceReserveLiters : 0.0f;  // Source volume available above reserve
    const float destinationRoom = destinationTargetLiters > destinationStoredLiters ? destinationTargetLiters - destinationStoredLiters : 0.0f;  // Destination room below target
    const float sourceLimited = sourceAvailable < requestedLiters ? sourceAvailable : requestedLiters;  // Source-limited transfer request
    return destinationRoom < sourceLimited ? destinationRoom : sourceLimited;
}

inline bool terraFreezeRisk(float temperatureC, float thresholdC = 0.0f) {
    return temperatureC <= thresholdC;
}

inline float terraResolveActuatorRequests(const float *requests, uint8_t count, Terra_EnableMode mode) {
    if (!requests || !count) return 0.0f;

    switch (mode) {
        case Terra_EnableMode_Highest: {
            float value = requests[0];                      // Intermediate aggregated value
            for (uint8_t i = 1; i < count; ++i) if (requests[i] > value) value = requests[i];
            return value;
        }
        case Terra_EnableMode_Lowest: {
            float value = requests[0];                      // Intermediate aggregated value
            for (uint8_t i = 1; i < count; ++i) if (requests[i] < value) value = requests[i];
            return value;
        }
        case Terra_EnableMode_Average: {
            float value = 0.0f;                             // Intermediate aggregated value
            for (uint8_t i = 0; i < count; ++i) value += requests[i];
            return value / (float)count;
        }
        case Terra_EnableMode_Multiply: {
            float value = requests[0];                      // Intermediate aggregated value
            for (uint8_t i = 1; i < count; ++i) value *= requests[i];
            return value;
        }
        case Terra_EnableMode_RevOrder:
            return requests[count - 1];
        case Terra_EnableMode_InOrder:
            return requests[0];
        case Terra_EnableMode_Undefined:
        case Terra_EnableMode_Count:
        default:
            return requests[0];
    }
}

inline bool terraFlowFault(bool commandedOn, float flowRate, float minimumFlow, float maximumFlow = 0.0f) {
    if (!commandedOn) return flowRate > minimumFlow;
    if (flowRate < minimumFlow) return true;
    return maximumFlow > 0.0f && flowRate > maximumFlow;
}

#endif
