/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Logger
*/

#ifndef TerraLogger_H
#define TerraLogger_H

class TerraLogger;
struct TerraLoggerSubData;

#include "Terraduino.h"

// Logging Level
// Log levels that can be filtered upon if desired.
enum Terra_LogLevel : signed char {
    Terra_LogLevel_All,                                     // All (info, warn, err)
    Terra_LogLevel_Warnings,                                // Warnings & errors (warn, err)
    Terra_LogLevel_Errors,                                  // Just errors (err)
    Terra_LogLevel_None = -1,                               // None / disabled
    Terra_LogLevel_Info = Terra_LogLevel_All                // Info alias
};

// Logging Events
// Logging event structure that is used in signaling.
struct TerraLogEvent {
    Terra_LogLevel level;                                   // Log level
    String timestamp;                                       // Timestamp (generated)
    String prefix;                                          // Prefix
    String msg;                                             // Message
    String suffix1;                                         // Suffix1 (optional)
    String suffix2;                                         // Suffix2 (optional)

    TerraLogEvent(Terra_LogLevel levelIn,
                  const String &prefixIn,
                  const String &msgIn,
                  const String &suffix1In = String(),
                  const String &suffix2In = String());
};

// Data Logger
// The Logger acts as the system's event monitor that collects and reports on the various
// processes of interest inside of the system. It allows for different log levels to be
// used that can help filter out unwanted noise, as well as attempts to be more optimized
// for embedded systems by spreading string data out over multiple call parameters to
// avoid large string concatenations that can overstress and crash constrained devices.
// Logging to SD card .txt log files (via SPI card reader) is supported as is logging to
// WiFiStorage .txt log files (via OS/OTA filesystem / WiFiNINA_Generic only).
class TerraLogger {
public:
    TerraLogger();
    ~TerraLogger();

    bool beginLoggingToSDCard(String logFilePrefix);
    inline bool isLoggingToSDCard() const;

#ifdef TERRA_USE_WIFI_STORAGE
    bool beginLoggingToWiFiStorage(String logFilePrefix);
    inline bool isLoggingToWiFiStorage() const;
#endif

    inline void logActivation(const TerraActuator *actuator);
    inline void logDeactivation(const TerraActuator *actuator);
    inline void logProcess(const TerraObjInterface *obj, const String &processString = String(), const String &statusString = String());
    inline void logStatus(const TerraObjInterface *obj, const String &statusString = String());

    void logSystemUptime();
    inline void logSystemSave() { logMessage(SFP(TStr_Log_SystemDataSaved)); }

    void logMessage(const String &msg, const String &suffix1 = String(), const String &suffix2 = String());
    void logWarning(const String &warn, const String &suffix1 = String(), const String &suffix2 = String());
    void logError(const String &err, const String &suffix1 = String(), const String &suffix2 = String());
    void flush();

    void setLogLevel(Terra_LogLevel logLevel);
    inline Terra_LogLevel getLogLevel() const;

    inline bool isLoggingEnabled() const;
    inline time_t getSystemInit() const { return _initTime; }
    inline time_t getSystemUptime() const { return unixNow() - (_initTime ?: SECS_YR_2000); }

    Signal<const TerraLogEvent, TERRA_LOG_SIGNAL_SLOTS> &getLogSignal();

    void notifyDateChanged();

protected:
#if TERRA_SYS_LEAVE_FILES_OPEN
    File *_logFileSD;                                       // SD card log file instance (owned)
#ifdef TERRA_USE_WIFI_STORAGE
    WiFiStorageFile *_logFileWS;                            // WiFiStorageFile log file instance (owned)
#endif
#endif
    String _logFilename;                                    // Resolved log file name (based on day)
    time_t _initTime;                                       // Time of init, for uptime (UTC)
    time_t _lastSpaceCheck;                                 // Last time enough space was checked (UTC)

    Signal<const TerraLogEvent, TERRA_LOG_SIGNAL_SLOTS> _logSignal; // Logging signal

    friend class Terraduino;
    
    void log(const TerraLogEvent &event);

public: // consider protected
    inline TerraLoggerSubData *loggerData() const;
    inline bool hasLoggerData() const;

    inline void updateInitTracking(time_t time = unixNow()) { _initTime = time; }
    void cleanupOldestLogs(bool force = false);
};

// Logger Serialization Sub Data
// A part of TSYS system data.
struct TerraLoggerSubData : public TerraSubData {
    Terra_LogLevel logLevel;                                // Log level filter (default: All)
    char logFilePrefix[TERRA_PREFIX_MAXSIZE];               // Base log file name prefix / folder (default: "logs/he")
    bool logToSDCard;                                       // If system logging to SD card is enabled (default: false)
    bool logToWiFiStorage;                                  // If system logging to WiFiStorage is enabled (default: false)

    TerraLoggerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef TerraLogger_H
