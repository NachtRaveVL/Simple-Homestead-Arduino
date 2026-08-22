/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Logger Data
*/

#ifndef TerraLoggerSubData_H
#define TerraLoggerSubData_H

#include "TerraData.h"

// Logger Serialization Sub Data
// A part of TSYS system data.
struct TerraLoggerSubData : public TerraSubData {
    Terra_LogLevel logLevel;                                // Minimum logged event level
    char logFilePrefix[TERRA_PREFIX_MAXSIZE];               // Log file prefix
    bool logToSDCard;                                       // SD logging enabled
    bool logToWiFiStorage;                                  // WiFi storage logging enabled

    TerraLoggerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef TerraLoggerSubData_H
