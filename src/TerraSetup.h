/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Setup Data
*/

#ifndef TerraSetup_H
#define TerraSetup_H

#include "TerraDefines.h"
#include "TerraTypes.h"

// I2C Device Setup
// A quick and easy structure for storing I2C device connection settings.
struct TerraI2CDeviceSetup {
#ifdef ARDUINO
    TwoWire *wire;                                          // I2C wire instance
#else
    void *wire;                                             // Host I2C instance placeholder
#endif
    uint32_t speed;                                         // I2C max data speed, in Hz
    uint8_t address;                                        // I2C device address

#ifdef ARDUINO
    TerraI2CDeviceSetup(TwoWire *i2cWire = TERRA_USE_WIRE, uint32_t i2cSpeed = 100000U, uint8_t i2cAddress = 0)
#else
    TerraI2CDeviceSetup(void *i2cWire = nullptr, uint32_t i2cSpeed = 100000U, uint8_t i2cAddress = 0)
#endif
        : wire(i2cWire), speed(i2cSpeed), address(i2cAddress) { ; }
};

// SPI Device Setup
// A quick and easy structure for storing SPI device connection settings.
struct TerraSPIDeviceSetup {
#ifdef ARDUINO
    SPIClass *spi;                                          // SPI class instance
#else
    void *spi;                                              // Host SPI instance placeholder
#endif
    uint32_t speed;                                         // SPI max data speed, in Hz
    uint8_t chipSelectPin;                                  // SPI chip-select pin

#ifdef ARDUINO
    TerraSPIDeviceSetup(SPIClass *spiClass = TERRA_USE_SPI, uint32_t spiSpeed = 1000000U,
                        uint8_t spiCS = TERRA_INVALID_PIN)
#else
    TerraSPIDeviceSetup(void *spiClass = nullptr, uint32_t spiSpeed = 1000000U,
                        uint8_t spiCS = TERRA_INVALID_PIN)
#endif
        : spi(spiClass), speed(spiSpeed), chipSelectPin(spiCS) { ; }
};

// UART Device Setup
// A quick and easy structure for storing serial device connection settings.
struct TerraUARTDeviceSetup {
#ifdef ARDUINO
    SerialClass *serial;                                    // UART class instance
#else
    void *serial;                                           // Host UART instance placeholder
#endif
    uint32_t baudRate;                                      // UART baud rate, in bits per second

#ifdef ARDUINO
    TerraUARTDeviceSetup(SerialClass *serialClass = TERRA_USE_SERIAL1, uint32_t serialBaud = 9600U)
#else
    TerraUARTDeviceSetup(void *serialClass = nullptr, uint32_t serialBaud = 9600U)
#endif
        : serial(serialClass), baudRate(serialBaud) { ; }
};

// Combined Device Setup
// Tagged storage for the common I2C, SPI, and UART connection settings.
struct TerraDeviceSetup {
    enum : int8_t { None, I2CSetup, SPISetup, UARTSetup } cfgType; // Configuration type
    TerraI2CDeviceSetup i2c;                                // I2C configuration
    TerraSPIDeviceSetup spi;                                // SPI configuration
    TerraUARTDeviceSetup uart;                              // UART configuration

    TerraDeviceSetup() : cfgType(None), i2c(), spi(), uart() { ; }
    TerraDeviceSetup(const TerraI2CDeviceSetup &setup) : cfgType(I2CSetup), i2c(setup), spi(), uart() { ; }
    TerraDeviceSetup(const TerraSPIDeviceSetup &setup) : cfgType(SPISetup), i2c(), spi(setup), uart() { ; }
    TerraDeviceSetup(const TerraUARTDeviceSetup &setup) : cfgType(UARTSetup), i2c(), spi(), uart(setup) { ; }
};

struct TerraNetworkSetup {
    bool enabled;                                           // Enabled state
    uint16_t port;                                          // Network port
    TerraString host;                                       // Network host
    TerraString clientId;                                   // Network client ID

    TerraNetworkSetup() : enabled(false), port(0), host(), clientId() { ; }
};

#endif // /ifndef TerraSetup_H
