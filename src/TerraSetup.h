/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Setup Data
*/

#ifndef TerraSetup_H
#define TerraSetup_H

#include "TerraDefines.h"
#include "TerraTypes.h"

struct TerraDeviceSetup {
    uint8_t address;                                        // Device address
    uint8_t bus;                                            // Bus index
    uint8_t chipSelectPin;                                  // SPI chip-select pin
    uint32_t baudRate;                                      // UART baud rate

    TerraDeviceSetup(uint8_t addressIn = 0, uint8_t busIn = 0,
                     uint8_t chipSelectPinIn = TERRA_INVALID_PIN, uint32_t baudRateIn = 0)
        : address(addressIn), bus(busIn), chipSelectPin(chipSelectPinIn), baudRate(baudRateIn) { }
};

struct TerraNetworkSetup {
    bool enabled;                                           // Enabled state
    uint16_t port;                                          // Network port
    TerraString host;                                       // Network host
    TerraString clientId;                                   // Network client ID

    TerraNetworkSetup() : enabled(false), port(0), host(), clientId() { }
};

struct TerraSystemSetup {
    TerraString systemName;                                 // System display name
    int8_t timeZoneHours;                                   // Time-zone hour offset
    int8_t timeZoneMinutes;                                 // Time-zone minute offset
    Terra_ControlMode controlMode;                          // Controller operating mode
    Terra_MeasurementMode measurementMode;                  // Preferred measurement mode
    uint32_t updateIntervalMs;                              // Controller update interval, milliseconds
    Terra_LogLevel loggerMinimumLevel;                      // Minimum stored log level
    uint32_t publisherIntervalMs;                           // Publisher interval, milliseconds

    TerraSystemSetup()
        : systemName("Terraduino"), timeZoneHours(0), timeZoneMinutes(0),
          controlMode(Terra_ControlMode_Automatic), measurementMode(Terra_MeasurementMode_Metric),
          updateIntervalMs(250), loggerMinimumLevel(Terra_LogLevel_Info), publisherIntervalMs(10000) { }
};

#endif
