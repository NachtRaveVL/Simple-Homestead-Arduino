/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Logger
*/

#ifndef TerraLogger_H
#define TerraLogger_H

#include "TerraDefines.h"
#include "TerraLoggerSubData.h"
#include "TerraInterfaces.h"

class TerraLogger {
public:
    TerraLogger();

    void setSink(TerraTextSink *sink) { _sink = sink; }
    void setMinimumLevel(Terra_LogLevel level) { _minimumLevel = level; }
    Terra_LogLevel getMinimumLevel() const { return _minimumLevel; }
    void log(Terra_LogLevel level, const TerraString &category, const TerraString &message, uint32_t timestamp = terraMillis());
    void debug(const TerraString &category, const TerraString &message, uint32_t timestamp = terraMillis()) { log(Terra_LogLevel_Debug, category, message, timestamp); }
    void info(const TerraString &category, const TerraString &message, uint32_t timestamp = terraMillis()) { log(Terra_LogLevel_Info, category, message, timestamp); }
    void warning(const TerraString &category, const TerraString &message, uint32_t timestamp = terraMillis()) { log(Terra_LogLevel_Warning, category, message, timestamp); }
    void error(const TerraString &category, const TerraString &message, uint32_t timestamp = terraMillis()) { log(Terra_LogLevel_Error, category, message, timestamp); }

    uint8_t count() const { return _count; }
    const TerraLogRecord *at(uint8_t index) const;
    void clear();

protected:
    TerraString format(const TerraLogRecord &record) const;
    TerraLogRecord _records[TERRA_MAX_LOG_RECORDS];         // Records
    uint8_t _head;                                          // Oldest log record index
    uint8_t _count;                                         // Active entry count
    Terra_LogLevel _minimumLevel;                           // Minimum accepted log level
    TerraTextSink *_sink;
};

#endif
