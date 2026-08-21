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
#include <ArxContainer.h>
#include <ArxSmartPtr.h>
typedef String TerraString;
inline uint32_t terraMillis() { return millis(); }
#else
#include <string>
#include <chrono>
#include <memory>
#include <map>
#ifndef PROGMEM
#define PROGMEM
#endif
typedef std::string TerraString;
inline uint32_t terraMillis() {
    using namespace std::chrono;
    return (uint32_t)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
#endif

#if defined(ARDUINO)
template<typename T> using SharedPtr = arx::stdx::shared_ptr<T>;
template<typename K, typename V, size_t N> using TerraMap = arx::map<K,V,N>;
template<class T, class U> inline SharedPtr<T> terraStaticPointerCast(const SharedPtr<U> &ptr) { return arx::stdx::static_pointer_cast<T>(ptr); }
template<class T, class U> inline SharedPtr<T> terraReinterpretPointerCast(const SharedPtr<U> &ptr) { return arx::stdx::reinterpret_pointer_cast<T>(ptr); }
#else
template<typename T> using SharedPtr = std::shared_ptr<T>;
template<typename K, typename V, size_t N> using TerraMap = std::map<K,V>;
template<class T, class U> inline SharedPtr<T> terraStaticPointerCast(const SharedPtr<U> &ptr) { return std::static_pointer_cast<T>(ptr); }
template<class T, class U> inline SharedPtr<T> terraReinterpretPointerCast(const SharedPtr<U> &ptr) { return ptr ? SharedPtr<T>(ptr, reinterpret_cast<T *>(ptr.get())) : SharedPtr<T>(); }
#endif

#endif
