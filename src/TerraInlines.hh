/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Common Inlines
*/

#ifndef TerraInlines_HH
#define TerraInlines_HH

struct TerraI2CDeviceSetup;
struct TerraSPIDeviceSetup;
struct TerraUARTDeviceSetup;
struct TerraDeviceSetup;

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
    inline TerraI2CDeviceSetup(TwoWire *i2cWire = TERRA_USE_WIRE, uint32_t i2cSpeed = 100000U, uint8_t i2cAddress = 0)
#else
    inline TerraI2CDeviceSetup(void *i2cWire = nullptr, uint32_t i2cSpeed = 100000U, uint8_t i2cAddress = 0)
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
    uint8_t cs;                                             // SPI chip-select pin

#ifdef ARDUINO
    inline TerraSPIDeviceSetup(SPIClass *spiClass = TERRA_USE_SPI, uint32_t spiSpeed = 1000000U, uint8_t spiCS = TERRA_INVALID_PIN)
#else
    inline TerraSPIDeviceSetup(void *spiClass = nullptr, uint32_t spiSpeed = 1000000U, uint8_t spiCS = TERRA_INVALID_PIN)
#endif
        : spi(spiClass), speed(spiSpeed), cs(spiCS) { ; }
};

// UART Device Setup
// A quick and easy structure for storing serial device connection settings.
struct TerraUARTDeviceSetup {
#ifdef ARDUINO
    SerialClass *serial;                                    // UART class instance
#else
    void *serial;                                           // Host UART instance placeholder
#endif
    uint32_t baud;                                          // UART baud rate, in bits per second

#ifdef ARDUINO
    inline TerraUARTDeviceSetup(SerialClass *serialClass = TERRA_USE_SERIAL1, uint32_t serialBaud = 9600U)
#else
    inline TerraUARTDeviceSetup(void *serialClass = nullptr, uint32_t serialBaud = 9600U)
#endif
        : serial(serialClass), baud(serialBaud) { ; }
};

// Combined Device Setup
// Tagged storage for the common I2C, SPI, and UART connection settings.
struct TerraDeviceSetup {
    enum : signed char { None, I2CSetup, SPISetup, UARTSetup } cfgType; // Configuration type
    TerraI2CDeviceSetup i2c;                                // I2C configuration
    TerraSPIDeviceSetup spi;                                // SPI configuration
    TerraUARTDeviceSetup uart;                              // UART configuration

    inline TerraDeviceSetup() : cfgType(None), i2c(), spi(), uart() { ; }
    inline TerraDeviceSetup(const TerraI2CDeviceSetup &setup) : cfgType(I2CSetup), i2c(setup), spi(), uart() { ; }
    inline TerraDeviceSetup(const TerraSPIDeviceSetup &setup) : cfgType(SPISetup), i2c(), spi(setup), uart() { ; }
    inline TerraDeviceSetup(const TerraUARTDeviceSetup &setup) : cfgType(UARTSetup), i2c(), spi(), uart(setup) { ; }
};

#endif // /ifndef TerraInlines_HH
