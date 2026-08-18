/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Logger
*/

#include "TerraLogger.h"
#include "TerraStrings.h"

TerraLogger::TerraLogger() : _head(0), _count(0), _minimumLevel(Terra_LogLevel_Info), _sink(nullptr) { }

TerraString TerraLogger::format(const TerraLogRecord &record) const {
#if defined(ARDUINO)
    TerraString text = String(record.timestamp) + "," + terraLogLevelToString(record.level) + "," + record.category + "," + record.message + "\n";
#else
    TerraString text = std::to_string(record.timestamp) + "," + terraLogLevelToString(record.level) + "," + record.category + "," + record.message + "\n";
#endif
    return text;
}

void TerraLogger::log(Terra_LogLevel level, const TerraString &category, const TerraString &message, uint32_t timestamp) {
    if (level < _minimumLevel) return;
    TerraLogRecord record(timestamp, level, category, message);
    uint8_t index = (_head + _count) % TERRA_MAX_LOG_RECORDS;
    if (_count == TERRA_MAX_LOG_RECORDS) {
        _records[_head] = record;
        _head = (_head + 1) % TERRA_MAX_LOG_RECORDS;
    } else {
        _records[index] = record;
        ++_count;
    }
    if (_sink) _sink->write(format(record));
}

const TerraLogRecord *TerraLogger::at(uint8_t index) const {
    if (index >= _count) return nullptr;
    return &_records[(_head + index) % TERRA_MAX_LOG_RECORDS];
}

void TerraLogger::clear() { _head = 0; _count = 0; }
