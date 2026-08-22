/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Logger
*/

#ifndef TerraLogger_H
#define TerraLogger_H

#include "TerraData.h"
#include "TerraLoggerSubData.h"


struct TerraLogEvent {
    Terra_LogLevel level;                                   // Logging level
    uint32_t timestamp;                                     // Event timestamp
    char category[TERRA_PREFIX_MAXSIZE];                    // Event category
    char message[TERRA_LOG_MESSAGE_MAXSIZE];                // Event message
};

class TerraLogger {
public:
    TerraLogger();

    void setSubData(TerraLoggerSubData *data);
    void setLogLevel(Terra_LogLevel logLevel);
    Terra_LogLevel getLogLevel() const;
    inline bool isLoggingEnabled() const { return getLogLevel() != Terra_LogLevel_Undefined; }

    void logMessage(uint32_t timestamp, const char *category, const char *message);
    void logWarning(uint32_t timestamp, const char *category, const char *message);
    void logError(uint32_t timestamp, const char *category, const char *message);

    inline void info(const TerraString &category, const TerraString &message, uint32_t timestamp = millis())
        { logMessage(timestamp, category.c_str(), message.c_str()); }
    inline void warning(const TerraString &category, const TerraString &message, uint32_t timestamp = millis())
        { logWarning(timestamp, category.c_str(), message.c_str()); }
    inline void error(const TerraString &category, const TerraString &message, uint32_t timestamp = millis())
        { logError(timestamp, category.c_str(), message.c_str()); }

    Signal<const TerraLogEvent, TERRA_DEFAULT_MAXSIZE> &getLogSignal();

protected:
    Terra_LogLevel _logLevel;                               // Active log level
    TerraLoggerSubData *_data;                              // Serialized logger settings, not owned
    Signal<const TerraLogEvent, TERRA_DEFAULT_MAXSIZE> _logSignal; // Logging signal

    void log(Terra_LogLevel level, uint32_t timestamp, const char *category, const char *message);
};


#endif // /ifndef TerraLogger_H
