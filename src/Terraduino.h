/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    Simple-Homestead-Arduino - Version 0.7.1.0
*/

#ifndef Terraduino_H
#define Terraduino_H

// Library Setup
// NOTE: It is recommended to use custom build flags instead of editing this file directly.

// Uncomment or -D this define to completely disable multitasking commands and libraries.
//#define TERRA_DISABLE_MULTITASKING

// Uncomment or -D this define to disable tcMenu-based GUI control.
//#define TERRA_DISABLE_GUI

// Uncomment or -D this define to enable the platform WiFi library.
//#define TERRA_ENABLE_WIFI

// Uncomment or -D this define to enable serial AT-command WiFi support.
//#define TERRA_ENABLE_AT_WIFI

// Uncomment or -D this define to enable the platform Ethernet library.
//#define TERRA_ENABLE_ETHERNET

// Uncomment or -D this define to enable MQTT publishing support.
//#define TERRA_ENABLE_MQTT

// Uncomment or -D this define to enable GPS-based time/location support.
//#define TERRA_ENABLE_GPS

// Uncomment or -D this define to disable built-in Flash data and use external data storage.
//#define TERRA_DISABLE_BUILTIN_DATA

// Uncomment or -D this define to enable serial debug output.
//#define TERRA_ENABLE_DEBUG_OUTPUT

// Uncomment or -D this define to enable verbose debug output.
//#define TERRA_ENABLE_VERBOSE_DEBUG

// Uncomment or -D this define to enable debug assertions.
//#define TERRA_ENABLE_DEBUG_ASSERTIONS

#ifdef ARDUINO
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

#ifndef USE_SW_SERIAL
typedef HardwareSerial SerialClass;
#else
#include <SoftwareSerial.h>
#define TERRA_USE_SOFTWARE_SERIAL
typedef SoftwareSerial SerialClass;
#endif

#if defined(TERRA_ENABLE_WIFI)
#if defined(ARDUINO_SAMD_MKR1000)
#include <WiFi101.h>
#else
#include <WiFiNINA_Generic.h>
#define TERRA_USE_WIFI_STORAGE
#endif
#define TERRA_USE_WIFI
#define TERRA_USE_NET
#elif defined(TERRA_ENABLE_AT_WIFI)
#include <WiFiEspAT.h>
#define TERRA_USE_AT_WIFI
#define TERRA_USE_WIFI
#define TERRA_USE_NET
#elif defined(TERRA_ENABLE_ETHERNET)
#include <Ethernet.h>
#define TERRA_USE_ETHERNET
#define TERRA_USE_NET
#endif

#ifndef TERRA_DISABLE_MULTITASKING
#include <TaskManagerIO.h>
#include <IoAbstraction.h>
#define TERRA_USE_MULTITASKING
#else
#ifndef TERRA_DISABLE_GUI
#define TERRA_DISABLE_GUI
#endif
#define secondsToMillis(val) ((val)*1000U)
#if defined(ARDUINO_ARCH_MBED)
typedef uint32_t pintype_t;
#else
typedef uint8_t pintype_t;
#endif
#endif

#ifdef TERRA_ENABLE_GPS
#include <Adafruit_GPS.h>
#define TERRA_USE_GPS
typedef Adafruit_GPS GPSClass;
#endif

#include <RTClib.h>
#include <TimeLib.h>
#ifdef TERRA_ENABLE_MQTT
#include <MQTT.h>
#define TERRA_USE_MQTT
#endif

#ifndef TERRA_DISABLE_GUI
#include <tcMenu.h>
#define TERRA_USE_GUI
#endif

#if !(defined(NO_GLOBAL_INSTANCES) || defined(NO_GLOBAL_SPI))
#define TERRA_USE_SPI &SPI
#else
#define TERRA_USE_SPI nullptr
#endif
#if !(defined(NO_GLOBAL_INSTANCES) || defined(NO_GLOBAL_TWOWIRE))
#define TERRA_USE_WIRE &Wire
#else
#define TERRA_USE_WIRE nullptr
#endif
#if !(defined(NO_GLOBAL_INSTANCES) || defined(NO_GLOBAL_SERIAL1))
#define TERRA_USE_SERIAL1 &Serial1
#else
#define TERRA_USE_SERIAL1 nullptr
#endif
#endif // /ifdef ARDUINO

#ifdef NDEBUG
#ifdef TERRA_ENABLE_DEBUG_OUTPUT
#undef TERRA_ENABLE_DEBUG_OUTPUT
#endif
#ifdef TERRA_ENABLE_VERBOSE_DEBUG
#undef TERRA_ENABLE_VERBOSE_DEBUG
#endif
#ifdef TERRA_ENABLE_DEBUG_ASSERTIONS
#undef TERRA_ENABLE_DEBUG_ASSERTIONS
#endif
#endif // /ifdef NDEBUG

#if defined(TERRA_ENABLE_DEBUG_OUTPUT) && defined(TERRA_ENABLE_VERBOSE_DEBUG)
#define TERRA_USE_VERBOSE_OUTPUT
#endif
#if defined(TERRA_ENABLE_DEBUG_OUTPUT) && defined(TERRA_ENABLE_DEBUG_ASSERTIONS)
#define TERRA_SOFT_ASSERT(cond,msg)     terraSoftAssert((bool)(cond), TerraString((msg)), __FILE__, __func__, __LINE__)
#define TERRA_HARD_ASSERT(cond,msg)     terraHardAssert((bool)(cond), TerraString((msg)), __FILE__, __func__, __LINE__)
#define TERRA_USE_DEBUG_ASSERTIONS
#else
#define TERRA_SOFT_ASSERT(cond,msg)     ((void)0)
#define TERRA_HARD_ASSERT(cond,msg)     ((void)0)
#endif

#include "TerraDefines.h"
#include <ArduinoJson.h>

#ifdef ARDUINO

#include <ArxContainer.h>
#include <ArxSmartPtr.h>
typedef String TerraString;

#if ARX_HAVE_LIBSTDCPLUSPLUS >= 201103L
#include "ArxSmartPtr/shared_ptr.h"
using namespace std;
template<typename T, size_t N = TERRA_DEFAULT_MAXSIZE> using Vector = std::vector<T>;
template<class T1, class T2> using Pair = std::pair<T1,T2>;
template<typename K, typename V, size_t N = TERRA_DEFAULT_MAXSIZE> using Map = std::map<K,V>;
#else
using namespace arx;
template<typename T, size_t N = ARX_VECTOR_DEFAULT_SIZE> using Vector = arx::vector<T,N>;
template<class T1, class T2> using Pair = arx::pair<T1,T2>;
template<typename K, typename V, size_t N = ARX_MAP_DEFAULT_SIZE> using Map = arx::map<K,V,N>;
#endif
using namespace arx::stdx;
template <typename T> using SharedPtr = arx::stdx::shared_ptr<T>;

#else

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace std;
typedef std::string TerraString;
template<typename T, size_t N = TERRA_DEFAULT_MAXSIZE> using Vector = std::vector<T>;
template<class T1, class T2> using Pair = std::pair<T1,T2>;
template<typename K, typename V, size_t N = TERRA_DEFAULT_MAXSIZE> using Map = std::map<K,V>;
template<typename T> using SharedPtr = std::shared_ptr<T>;
#if __cplusplus < 201703L
template<class T, class U> inline SharedPtr<T> reinterpret_pointer_cast(const SharedPtr<U> &ptr) { return ptr ? SharedPtr<T>(ptr, reinterpret_cast<T *>(ptr.get())) : SharedPtr<T>(); }
#endif

#endif

#include "TerraSetup.h"
#include "TerraStrings.h"
#include "TerraInlines.hh"
#include "TerraCallback.hh"
#include "TerraCoreLogic.h"
#include "TerraInterfaces.h"
#include "TerraActivation.h"
#include "TerraAttachments.h"
#include "TerraData.h"
#include "TerraObject.h"
#include "TerraMeasurements.h"
#include "TerraPins.h"
#include "TerraUtils.h"
#include "TerraDatas.h"
#include "TerraStreams.h"
#include "TerraTriggers.h"
#include "TerraDrivers.h"
#include "TerraActuators.h"
#include "TerraSensors.h"
#include "TerraReservoir.h"
#include "TerraEnvironment.h"
#include "TerraWater.h"
#include "TerraThermal.h"
#include "TerraBalancers.h"
#include "TerraRails.h"
#include "TerraModules.h"
#include "TerraScheduler.h"
#include "TerraLogger.h"
#include "TerraPublisher.h"
#include "TerraFactory.h"
#include "TerraInterfaces.hpp"

// Terraduino Controller
// Main controller interface for homestead resource and environmental systems. Networking,
// displays, remote transports, and external services remain optional so normal water,
// thermal, environmental, scheduling, logging, and control behavior can remain local.
class Terraduino : public TerraFactory, public TerraCalibrations, public TerraObjectRegistration {
public:
    TerraScheduler scheduler;                              // Scheduler public instance
    TerraLogger logger;                                    // Logger public instance
    TerraPublisher publisher;                              // Publisher public instance

    // Controller constructor. Typically called during class instantiation before setup().
    Terraduino(Terra_RTCType rtcType = Terra_RTCType_None,
               TerraDeviceSetup rtcSetup = TerraDeviceSetup());
    ~Terraduino();

    // Initializes the controller with the requested operating and measurement modes.
    void init(Terra_ControlMode controlMode = Terra_ControlMode_Automatic,
              Terra_MeasurementMode measurementMode = Terra_MeasurementMode_Metric);

    // Launches the controller into operational mode.
    void launch();
    // Suspends operational updates without discarding configured state.
    void suspend();
    // Updates registered objects, scheduling, and publishing from the controller clock.
    void update();

    inline bool isInitialized() const { return _initialized; }
    inline bool isRunning() const { return _running; }
    inline bool isSuspended() const { return !_running; }

    // System Settings.
    void setSystemName(const TerraString &name);
    // Sets system time zone offset from UTC, in whole hours.
    void setTimeZoneOffset(int8_t hoursOffset);
    // System time zone offset from UTC, in total offset seconds.
    time_t getTimeZoneOffset() const;
    inline void setControlMode(Terra_ControlMode mode) { _data.controlMode = mode; }
    inline Terra_ControlMode getControlMode() const { return _data.controlMode; }
    inline void setMeasurementMode(Terra_MeasurementMode mode) { _data.measurementMode = mode; }
    inline Terra_MeasurementMode getMeasurementMode() const { return _data.measurementMode; }
    inline void setLoggerMinimumLevel(Terra_LogLevel level) { logger.setLogLevel(level); }
    inline void setPollingInterval(uint16_t pollingInterval) { _data.pollingInterval = pollingInterval; }
    inline uint16_t getPollingInterval() const { return _data.pollingInterval; }

    // Core subsystem accessors kept alongside the public instances for family parity.
    inline TerraScheduler &getScheduler() { return scheduler; }
    inline TerraLogger &getLogger() { return logger; }
    inline TerraPublisher &getPublisher() { return publisher; }
    inline TerraSystemData &getSystemData() { return _data; }
    inline const TerraSystemData &getSystemData() const { return _data; }
    inline TerraSystemData &systemData() { return _data; }
    inline const TerraSystemData &systemData() const { return _data; }

    inline tframe_t getPollingFrame() const { return _pollingFrame; }
    inline bool isPollingFrameOld(tframe_t frame, tframe_t allowance = 0) const
        { return !frame || (tframe_t)(_pollingFrame - frame) > allowance; }

#ifdef ARDUINO
    // Real time clock instance (lazily instantiated, nullptr return -> failure/no device).
    TerraRTCInterface *getRTC(bool begin = true);
    // Sets the RTC's time to the passed local time, with respect to set timezone.
    void setRTCTime(DateTime time);
#endif
    inline Terra_RTCType getRTCType() const { return _rtcType; }
    inline const TerraDeviceSetup &getRTCSetup() const { return _rtcSetup; }
    // Whenever the system booted up with the RTC battery failure flag set.
    inline bool getRTCBatteryFailure() const { return _rtcBattFail; }

    // Persistence helpers.
    TerraString exportSystemJSON() const;
    bool importSystemJSON(const TerraString &json);

    // Returns the currently active Terraduino controller instance, if any.
    static inline Terraduino *getActiveInstance() { return _activeInstance; }

protected:
    static Terraduino *_activeInstance;                    // Active controller instance

    TerraSystemData _data;                                 // Serialized controller setup data
    const Terra_RTCType _rtcType;                          // RTC device type
    const TerraDeviceSetup _rtcSetup;                      // RTC device setup
#ifdef ARDUINO
    TerraRTCInterface *_rtc;                               // Real time clock instance (owned, lazy)
#endif
    bool _rtcBegan;                                        // Status of RTC begin() call
    bool _rtcBattFail;                                     // Status of RTC battery failure flag
    bool _initialized;                                     // Initialization state flag
    bool _running;                                         // Operational state flag
    uint32_t _lastControlAt;                               // Last control update timestamp
    uint32_t _lastPollAt;                                  // Last sensor polling timestamp
    tframe_t _pollingFrame;                                // Current sensor polling frame

    void allocateRTC();
    void deallocateRTC();

    friend class TerraDLinkObject;
};

// Returns the currently active controller instance.
extern Terraduino *getController();
// Returns the active system logger, when a controller exists.
extern TerraLogger *getLogger();
// Returns the active data publisher, when a controller exists.
extern TerraPublisher *getPublisher();
// Returns the active scheduler, when a controller exists.
extern TerraScheduler *getScheduler();

#include "Terraduino.hpp"

#endif // /ifndef Terraduino_H
