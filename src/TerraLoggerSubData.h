/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Logger Data
*/

#ifndef TerraLoggerSubData_H
#define TerraLoggerSubData_H

#include "TerraTypes.h"

struct TerraLogRecord {
    uint32_t timestamp;                                     // Measurement timestamp
    Terra_LogLevel level;                                   // Normalized resource level, percent
    TerraString category;                                   // Log category
    TerraString message;                                    // Log message

    TerraLogRecord(uint32_t timestampIn = 0,
                   Terra_LogLevel levelIn = Terra_LogLevel_Info,
                   const TerraString &categoryIn = TerraString(),
                   const TerraString &messageIn = TerraString())
        : timestamp(timestampIn), level(levelIn), category(categoryIn), message(messageIn) { }
};

#endif
