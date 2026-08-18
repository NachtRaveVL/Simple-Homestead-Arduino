/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Platform Compatibility
*/

#ifndef TerraPlatform_H
#define TerraPlatform_H

#include <stdint.h>
#include <stddef.h>
#include <math.h>

#if defined(ARDUINO)
#include <Arduino.h>
typedef String TerraString;
inline uint32_t terraMillis() { return millis(); }
#else
#include <string>
#include <chrono>
#ifndef PROGMEM
#define PROGMEM
#endif
typedef std::string TerraString;
inline uint32_t terraMillis() {
    using namespace std::chrono;
    return (uint32_t)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
#endif

#endif
