/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Logger
*/

#include "Terraduino.h"
#include <stdio.h>
#include <string.h>

TerraLogger::TerraLogger()
    : _logLevel(Terra_LogLevel_Info), _data(nullptr), _logSignal()
{ ; }

void TerraLogger::setSubData(TerraLoggerSubData *data)
{
    _data = data;
}

void TerraLogger::setLogLevel(Terra_LogLevel logLevel)
{
    if (_data) { _data->logLevel = logLevel; }
    _logLevel = logLevel;
}

Terra_LogLevel TerraLogger::getLogLevel() const
{
    return _data ? _data->logLevel : _logLevel;
}

void TerraLogger::log(Terra_LogLevel level, uint32_t timestamp, const char *category, const char *message)
{
    Terra_LogLevel logLevel = getLogLevel();
    if (logLevel == Terra_LogLevel_Undefined || level < logLevel) { return; }

    TerraLogEvent event;
    event.level = level;
    event.timestamp = timestamp;
    snprintf(event.category, sizeof(event.category), "%s", category ? category : "");
    snprintf(event.message, sizeof(event.message), "%s", message ? message : "");
    _logSignal.fire(event);
}

void TerraLogger::logMessage(uint32_t timestamp, const char *category, const char *message)
{
    log(Terra_LogLevel_Info, timestamp, category, message);
}

void TerraLogger::logWarning(uint32_t timestamp, const char *category, const char *message)
{
    log(Terra_LogLevel_Warning, timestamp, category, message);
}

void TerraLogger::logError(uint32_t timestamp, const char *category, const char *message)
{
    log(Terra_LogLevel_Error, timestamp, category, message);
}

Signal<const TerraLogEvent, TERRA_DEFAULT_MAXSIZE> &TerraLogger::getLogSignal()
{
    return _logSignal;
}

TerraLoggerSubData::TerraLoggerSubData()
    : TerraSubData(0), logLevel(Terra_LogLevel_Info), logFilePrefix{0},
      logToSDCard(false), logToWiFiStorage(false)
{
    snprintf(logFilePrefix, sizeof(logFilePrefix), "logs/terra");
}

void TerraLoggerSubData::toJSONObject(JsonObject &objectOut) const
{
    TerraSubData::toJSONObject(objectOut);
    objectOut["logLevel"] = terraLogLevelToString(logLevel);
    objectOut["logFilePrefix"] = logFilePrefix;
    objectOut["logToSDCard"] = logToSDCard;
    objectOut["logToWiFiStorage"] = logToWiFiStorage;
}

void TerraLoggerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraSubData::fromJSONObject(objectIn);
    const char *logLevelString = objectIn["logLevel"] | nullptr;
    if (logLevelString) { logLevel = terraLogLevelFromString(TerraString(logLevelString)); }
    const char *prefix = objectIn["logFilePrefix"] | nullptr;
    if (prefix) {
        strncpy(logFilePrefix, prefix, TERRA_PREFIX_MAXSIZE - 1);
        logFilePrefix[TERRA_PREFIX_MAXSIZE - 1] = '\0';
    }
    logToSDCard = objectIn["logToSDCard"] | logToSDCard;
    logToWiFiStorage = objectIn["logToWiFiStorage"] | logToWiFiStorage;
}
