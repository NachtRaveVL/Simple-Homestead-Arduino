/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Shared Pointer Compatibility
*/
#ifndef TerraSmartPtr_H
#define TerraSmartPtr_H
#if defined(ARDUINO)
#include <ArxSmartPtr.h>
template<typename T> using SharedPtr = arx::stdx::shared_ptr<T>;
template<class T, class U> inline SharedPtr<T> terraStaticPointerCast(const SharedPtr<U> &ptr) { return arx::stdx::static_pointer_cast<T>(ptr); }
#else
#include <memory>
template<typename T> using SharedPtr = std::shared_ptr<T>;
template<class T, class U> inline SharedPtr<T> terraStaticPointerCast(const SharedPtr<U> &ptr) { return std::static_pointer_cast<T>(ptr); }
#endif
#endif
