/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Utilities
*/

#ifndef TerraUtils_H
#define TerraUtils_H

#include "TerraDefines.h"
#include <math.h>

// Debug assertion helpers used by TERRA_SOFT_ASSERT / TERRA_HARD_ASSERT when enabled.
extern void terraSoftAssert(bool condition, const TerraString &message, const char *file, const char *function, int line);
extern void terraHardAssert(bool condition, const TerraString &message, const char *file, const char *function, int line);

template <typename T>
inline T terraClamp(T value, T low, T high) {
    return value < low ? low : (value > high ? high : value);
}

template <typename T>
inline T terraMin(T lhs, T rhs) {
    return lhs < rhs ? lhs : rhs;
}

template <typename T>
inline T terraMax(T lhs, T rhs) {
    return lhs > rhs ? lhs : rhs;
}

// Returns if two single-precision floating point values are equal within the library epsilon.
inline bool isFPEqual(float lhs, float rhs) { return fabsf(rhs - lhs) <= TERRA_FLT_EPSILON; }
// Returns if two double-precision floating point values are equal within the library epsilon.
inline bool isFPEqual(double lhs, double rhs) { return fabs(rhs - lhs) <= TERRA_DBL_EPSILON; }

inline bool terraElapsed(uint32_t now, uint32_t then, uint32_t interval) {
    return (uint32_t)(now - then) >= interval;
}

float terraMapFloat(float value, float inMin, float inMax, float outMin, float outMax);
uint32_t terraHashString(const char *text);
bool terraStringEqualsIgnoreCase(const TerraString &a, const char *b);
bool terraStringEqualsIgnoreCase(const TerraString &a, const TerraString &b);
bool terraStringStartsWithIgnoreCase(const TerraString &text, const TerraString &prefix);
TerraString terraTrim(const TerraString &text);
TerraString terraSubstring(const TerraString &text, size_t offset);
bool terraParseLong(const TerraString &text, long &valueOut);
TerraString terraFloatToString(float value, uint8_t decimals = 2);

#include "TerraUtils.hpp"

#endif // /ifndef TerraUtils_H
