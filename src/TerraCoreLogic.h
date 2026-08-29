/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Core Logic
*/

#ifndef TerraCoreLogic_H
#define TerraCoreLogic_H

#include "Terraduino.h"

// Returns elapsed unsigned time while remaining safe across 32-bit timer rollover.
inline uint32_t terraElapsedTime(uint32_t now, uint32_t start)
{
    return (uint32_t)(now - start);
}

// Returns true once the requested unsigned duration has elapsed.
inline bool terraHasElapsed(uint32_t now, uint32_t start, uint32_t duration)
{
    return terraElapsedTime(now, start) >= duration;
}

// Converts a signed actuator value into reverse, stopped, or forward direction.
inline int terraDirectionForValue(float value, float epsilon = FLT_EPSILON)
{
    return fabsf(value) <= epsilon ? 0 : value > 0.0f ? 1 : -1;
}

// Applies a minimum stable time before accepting a changed binary sensor state.
inline bool terraUpdateStableBinaryState(bool acceptedState, bool sampledState, uint32_t now,
                                         uint16_t stableTimeMillis, bool &pendingState,
                                         bool &hasPendingState, uint32_t &pendingStateStart)
{
    if (sampledState == acceptedState) {
        hasPendingState = false;
    } else if (!stableTimeMillis) {
        hasPendingState = false;
        return sampledState;
    } else if (!hasPendingState || pendingState != sampledState) {
        pendingState = sampledState;
        pendingStateStart = now;
        hasPendingState = true;
    } else if (terraHasElapsed(now, pendingStateStart, stableTimeMillis)) {
        hasPendingState = false;
        return sampledState;
    }

    return acceptedState;
}

// Classifies a normalized resource level against reserve/low/high thresholds.
inline Terra_ResourceState terraClassifyResourceState(float level, float reserveLevel,
                                                       float lowLevel, float highLevel,
                                                       bool fault = false)
{
    if (fault) { return Terra_ResourceState_Fault; }
    if (level <= reserveLevel) { return Terra_ResourceState_Reserve; }
    if (level <= lowLevel) { return Terra_ResourceState_Low; }
    if (level >= highLevel) { return Terra_ResourceState_High; }
    return Terra_ResourceState_Normal;
}

// Returns the amount that can be transferred while respecting source reserve and destination target.
inline float terraCisternTransferLiters(float sourceStoredLiters,
                                        float sourceReserveLiters,
                                        float destinationStoredLiters,
                                        float destinationTargetLiters,
                                        float requestedLiters)
{
    if (requestedLiters <= 0.0f) { return 0.0f; }
    const float sourceAvailable = sourceStoredLiters > sourceReserveLiters ? sourceStoredLiters - sourceReserveLiters : 0.0f;
    const float destinationRoom = destinationTargetLiters > destinationStoredLiters ? destinationTargetLiters - destinationStoredLiters : 0.0f;
    const float sourceLimited = sourceAvailable < requestedLiters ? sourceAvailable : requestedLiters;
    return destinationRoom < sourceLimited ? destinationRoom : sourceLimited;
}

inline bool terraFreezeRisk(float temperatureC, float thresholdC = 0.0f)
{
    return temperatureC <= thresholdC;
}

inline bool terraFlowFault(bool commandedOn, float flowRate, float minimumFlow, float maximumFlow = 0.0f)
{
    if (!commandedOn) { return flowRate > minimumFlow; }
    if (flowRate < minimumFlow) { return true; }
    return maximumFlow > 0.0f && flowRate > maximumFlow;
}

// Binary record copy/skip plan used for append-only serialized data migrations.
struct TerraBinaryDataReadPlan
{
    size_t copyBytes;                                       // Bytes that can be copied into the current data structure
    size_t skipBytes;                                       // Unknown trailing bytes that must be skipped in the serialized record
};

// Builds a safe copy plan for older, current, or newer append-only binary records.
inline TerraBinaryDataReadPlan terraBinaryDataReadPlan(size_t serializedSize, size_t currentSize, size_t baseSize)
{
    if (serializedSize < baseSize || currentSize < baseSize) { return {0, 0}; }

    const size_t serializedRemaining = serializedSize - baseSize;
    const size_t currentRemaining = currentSize - baseSize;
    const size_t copyBytes = serializedRemaining < currentRemaining ? serializedRemaining : currentRemaining;
    return {copyBytes, serializedRemaining - copyBytes};
}

#endif // /ifndef TerraCoreLogic_H
