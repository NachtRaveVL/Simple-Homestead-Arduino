/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Streams
*/

#ifndef TerraStreams_H
#define TerraStreams_H

class TerraEEPROMStream;
class TerraPROGMEMStream;

#include "Terraduino.h"

#ifdef ARDUINO_ARCH_SAM // Stream doesn't have availableForWrite
#define TERRA_STREAM_AVAIL4WRT_OVERRIDE
#else
#define TERRA_STREAM_AVAIL4WRT_OVERRIDE override
#endif

// EEPROM Stream
// Stream class for working with I2C_EEPROM data.
class TerraEEPROMStream : public Stream {
public:
    TerraEEPROMStream();
    TerraEEPROMStream(uint16_t dataAddress, size_t dataSize);

    virtual int available() override;
    virtual int read() override;
    size_t readBytes(char *buffer, size_t length);
    virtual int peek() override;
    virtual void flush() override;
    virtual size_t write(const uint8_t *buffer, size_t size) override;
    virtual size_t write(uint8_t data) override;
    virtual int availableForWrite() TERRA_STREAM_AVAIL4WRT_OVERRIDE;

protected:
    I2C_eeprom *_eeprom;
    uint16_t _readAddress, _writeAddress, _endAddress;
};


// PROGMEM Stream
// Stream class for working with PROGMEM data.
class TerraPROGMEMStream : public Stream {
public:
    TerraPROGMEMStream();
    TerraPROGMEMStream(uintptr_t dataAddress);
    TerraPROGMEMStream(uintptr_t dataAddress, size_t dataSize);

    virtual int available() override;
    virtual int read() override;
    virtual int peek() override;
    virtual void flush() override;
    virtual size_t write(const uint8_t *buffer, size_t size) override;
    virtual size_t write(uint8_t data) override;

protected:
    uintptr_t _readAddress, _writeAddress, _endAddress;
};

#ifdef TERRA_USE_WIFI_STORAGE

class TerraWiFiStorageFileStream : public Stream {
public:
    TerraWiFiStorageFileStream(WiFiStorageFile file, uintptr_t seekPos = 0);
    virtual ~TerraWiFiStorageFileStream();

    virtual int available() override;
    virtual int read() override;
    size_t readBytes(char *buffer, size_t length);
    virtual int peek() override;
    virtual void flush() override;
    virtual size_t write(const uint8_t *buffer, size_t size) override;
    virtual size_t write(uint8_t data) override;
    virtual int availableForWrite() TERRA_STREAM_AVAIL4WRT_OVERRIDE;

protected:
    enum WiFiStorageFileDirection : signed char { ReadBuffer, WriteBuffer, None = -1 };

    WiFiStorageFile _file;
    uint8_t _buffer[TERRA_WIFISTREAM_BUFFER_SIZE];
    size_t _bufferOffset;
    uintptr_t _bufferFileOffset;
    WiFiStorageFileDirection _bufferDirection;
    uintptr_t _readOffset, _writeOffset, _endOffset;

    void prepareReadBuffer();
    void prepareWriteBuffer();
};

#endif

#endif // /ifndef TerraStreams_H
