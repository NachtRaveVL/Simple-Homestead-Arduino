/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Defines
*/

#ifndef TerraDefines_H
#define TerraDefines_H

#include <stddef.h>
#include <stdint.h>
#include <float.h>
#ifndef ARDUINO
#include <chrono>
#endif

#ifndef JOIN
#define JOIN_(X,Y) X##_##Y
#define JOIN(X,Y) JOIN_(X,Y)
#endif
#ifndef JOIN3
#define JOIN3_(X,Y,Z) X##_##Y##_##Z
#define JOIN3(X,Y,Z) JOIN3_(X,Y,Z)
#endif
#ifndef STR
#define STR_(X) #X
#define STR(X) STR_(X)
#endif

#ifndef FLT_EPSILON
#define FLT_EPSILON 0.00001f                         // Single-precision floating point error tolerance
#endif
#ifndef DBL_EPSILON
#define DBL_EPSILON 0.0000000000001                  // Double-precision floating point error tolerance
#endif
#ifndef FLT_UNDEF
#define FLT_UNDEF __FLT_MAX__
#endif
#ifndef DBL_UNDEF
#define DBL_UNDEF __DBL_MAX__
#endif

#ifdef ARDUINO
typedef typeof(millis()) millis_t;
#else
typedef uint32_t millis_t;
inline millis_t millis()
{
    using namespace std::chrono;
    return (millis_t)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
template<class T> inline T constrain(T value, T low, T high) { return value < low ? low : (value > high ? high : value); }
#ifndef PROGMEM
#define PROGMEM
#endif
#endif

typedef int8_t tposi_t;
typedef uint32_t tkey_t;
typedef int8_t tid_t;
typedef uint16_t tframe_t;
#define millis_none ((millis_t)0)
#define tposi_none ((tposi_t)-1)
#define tkey_none ((tkey_t)0)
#define tid_none ((tid_t)-1)
#define tframe_none ((tframe_t)0)

#ifndef TERRA_DEFAULT_MAXSIZE
#define TERRA_DEFAULT_MAXSIZE 8
#endif

#define TERRA_NAME_MAXSIZE                 32
#define TERRA_MAX_OBJECTS                  48
#define TERRA_MAX_ATTACHMENTS               8
#define TERRA_MAX_SCHEDULE_TASKS           16
#define TERRA_MAX_LOG_RECORDS              24
#define TERRA_MAX_PUBLISH_CHANNELS         24
#define TERRA_MAX_MODULES                  12
#define TERRA_INVALID_PIN                 255
#define TERRA_INVALID_KEY                   0UL
#define TERRA_DEFAULT_REMOTE_STALE_MS   300000UL
#define TERRA_DEFAULT_PUBLISH_INTERVAL   60000UL
#define TERRA_DEFAULT_LOG_INTERVAL       60000UL
#define TERRA_SUMP_STOP_LEVEL_PERCENT      20.0f       // Default sump pump stop level
#define TERRA_SUMP_START_LEVEL_PERCENT     70.0f       // Default sump pump start level
#define TERRA_SUMP_ALARM_LEVEL_PERCENT     90.0f       // Default sump high-water alarm level

// Networking is optional. Defining these enables transport-specific integration
// in user code without making networking a requirement for Terraduino itself.
//#define TERRA_ENABLE_WIFI
//#define TERRA_ENABLE_ETHERNET
//#define TERRA_ENABLE_MQTT

#endif
