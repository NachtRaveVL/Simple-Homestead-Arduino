/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Streams
*/

#include "Terraduino.h"

TerraEEPROMStream::TerraEEPROMStream()
    : Stream(), _eeprom(nullptr), _readAddress(0), _writeAddress(0), _endAddress(0)
{
    if (getController() && (_eeprom = getController()->getEEPROM())) {
        _endAddress = _eeprom->getDeviceSize();
    }
    TERRA_HARD_ASSERT(_eeprom, SFP(TStr_Err_UnsupportedOperation));
}

TerraEEPROMStream::TerraEEPROMStream(uint16_t dataAddress, size_t dataSize)
      : Stream(), _eeprom(nullptr), _readAddress(dataAddress), _writeAddress(dataAddress), _endAddress(dataAddress + dataSize)
{
    if (getController()) {
        _eeprom = getController()->getEEPROM();
    }
    TERRA_HARD_ASSERT(_eeprom, SFP(TStr_Err_UnsupportedOperation));
}

int TerraEEPROMStream::available()
{
    return _eeprom ? ((int)_endAddress - _readAddress) : 0;
}

int TerraEEPROMStream::read()
{
    if (!_eeprom || _readAddress >= _endAddress) { return -1; }
    return (int)_eeprom->readByte(_readAddress++);
}

size_t TerraEEPROMStream::readBytes(char *buffer, size_t length)
{
    if (!_eeprom || _readAddress >= _endAddress) { return -1; }
    size_t retVal = _eeprom->readBlock(_readAddress, (uint8_t *)buffer, length);
    _readAddress += retVal;
    return retVal;
}

int TerraEEPROMStream::peek()
{
    if (!_eeprom || _readAddress >= _endAddress) { return -1; }
    return (int)_eeprom->readByte(_readAddress);
}

void TerraEEPROMStream::flush()
{
    //_eeprom->commit();
}

size_t TerraEEPROMStream::write(const uint8_t *buffer, size_t size)
{
    if (!_eeprom || _writeAddress >= _endAddress) { return 0; }
    size_t remaining = _endAddress - _writeAddress;
    if (size > remaining) { size = remaining; }
    if (_eeprom->updateBlockVerify(_writeAddress, buffer, size)) {
        _writeAddress += size;
        return size;
    } else {
        TERRA_SOFT_ASSERT(false, SFP(TStr_Err_OperationFailure));
        return 0;
    }
}

size_t TerraEEPROMStream::write(uint8_t data)
{
    if (!_eeprom || _writeAddress >= _endAddress) { return 0; }
    if (_eeprom->updateByteVerify(_writeAddress, data)) {
        _writeAddress += 1;
        return 1;
    } else {
        TERRA_SOFT_ASSERT(false, SFP(TStr_Err_OperationFailure));
        return 0;
    }
}

int TerraEEPROMStream::availableForWrite()
{
    return _eeprom ? ((int)_endAddress - _writeAddress) : 0;
}


TerraPROGMEMStream::TerraPROGMEMStream()
    : Stream(), _readAddress(0), _writeAddress(0), _endAddress(UINTPTR_MAX)
{ ; }

TerraPROGMEMStream::TerraPROGMEMStream(uintptr_t dataAddress)
    : Stream(), _readAddress(dataAddress), _writeAddress(dataAddress), _endAddress(dataAddress + strlen_P((const char *)dataAddress))
{ ; }

TerraPROGMEMStream::TerraPROGMEMStream(uintptr_t dataAddress, size_t dataSize)
    : Stream(), _readAddress(dataAddress), _writeAddress(dataAddress), _endAddress(dataAddress + dataSize)
{ ; }

int TerraPROGMEMStream::available()
{
    return _endAddress - _readAddress;
}

int TerraPROGMEMStream::read()
{
    if (_readAddress >= _endAddress) { return -1; }
    #ifdef ESP8266
        return pgm_read_byte((const void *)(_readAddress++));
    #else
        return pgm_read_byte(_readAddress++);
    #endif
}

int TerraPROGMEMStream::peek()
{
    if (_readAddress >= _endAddress) { return -1; }
    #ifdef ESP8266
        return pgm_read_byte((const void *)(_readAddress));
    #else
        return pgm_read_byte(_readAddress);
    #endif
}

void TerraPROGMEMStream::flush()
{ ; }

size_t TerraPROGMEMStream::write(const uint8_t *buffer, size_t size)
{
    TERRA_SOFT_ASSERT(false, SFP(TStr_Err_OperationFailure));
    return 0;
}

size_t TerraPROGMEMStream::write(uint8_t data)
{
    TERRA_SOFT_ASSERT(false, SFP(TStr_Err_OperationFailure));
    return 0;
}


#ifdef TERRA_USE_WIFI_STORAGE

TerraWiFiStorageFileStream::TerraWiFiStorageFileStream(WiFiStorageFile file, uintptr_t seekPos)
    : Stream(), _file(file), _buffer{0}, _bufferOffset(0), _bufferFileOffset(-1), _bufferDirection(None), _readOffset(0), _writeOffset(0), _endOffset(0)
{
    if (_file) {
        _endOffset = _file.size();
        _readOffset = _writeOffset = seekPos;
    }
}

TerraWiFiStorageFileStream::~TerraWiFiStorageFileStream()
{
    if (_file) {
        if (_bufferDirection == WriteBuffer && _bufferOffset > 0) {
            _file.seek(_bufferFileOffset);
            _file.write((const void*)_buffer, _bufferOffset); _bufferOffset = 0;
        }
    }
}

int TerraWiFiStorageFileStream::available()
{
    return _file ? _endOffset - _readOffset : 0;
}

int TerraWiFiStorageFileStream::read()
{
    if (!_file || _readOffset >= _endOffset) { return -1; }
    prepareReadBuffer();
    _readOffset++;
    return _buffer[_bufferOffset++];
}

size_t TerraWiFiStorageFileStream::readBytes(char *buffer, size_t length)
{
    if (!_file || _readOffset >= _endOffset) { return -1; }
    while (length && _readOffset < _endOffset) {
        prepareReadBuffer();
        size_t howMany = min(length, _endOffset - _readOffset);
        howMany = min(howMany, TERRA_WIFISTREAM_BUFFER_SIZE - _bufferOffset);
        memcpy(buffer, &_buffer[_bufferOffset], howMany);
        _readOffset += howMany;
        _bufferOffset += howMany;
        buffer += howMany;
        length -= howMany;
    }
}

int TerraWiFiStorageFileStream::peek()
{
    if (!_file || _readOffset >= _endOffset) { return -1; }
    prepareReadBuffer();
    return _buffer[_bufferOffset];
}

void TerraWiFiStorageFileStream::flush()
{
    if (_bufferDirection == WriteBuffer && _bufferOffset > 0) {
        _file.seek(_bufferFileOffset);
        _file.write((const void*)_buffer, _bufferOffset); _bufferOffset = 0;
    }
}

size_t TerraWiFiStorageFileStream::write(const uint8_t *buffer, size_t size)
{
    if (!_file || _writeOffset >= _endOffset) { return -1; }
    while (size) {
        prepareWriteBuffer();
        size_t howMany = min(size, TERRA_WIFISTREAM_BUFFER_SIZE - _bufferOffset);
        memcpy(&_buffer[_bufferOffset], buffer, howMany);
        _writeOffset += howMany;
        _bufferOffset += howMany;
        buffer += howMany;
        size -= howMany;
    }
}

size_t TerraWiFiStorageFileStream::write(uint8_t data)
{
    if (!_file || _writeOffset >= _endOffset) { return -1; }
    prepareWriteBuffer();
    _buffer[_bufferOffset++] = data;
    _writeOffset++;
    return 1;
}

int TerraWiFiStorageFileStream::availableForWrite() 
{
    return _file ? _endOffset - _writeOffset : 0;
}

void TerraWiFiStorageFileStream::prepareReadBuffer()
{
    if (_bufferDirection != ReadBuffer || _bufferFileOffset == -1 || _readOffset < _bufferFileOffset || _readOffset >= _bufferFileOffset + TERRA_WIFISTREAM_BUFFER_SIZE) {
        if (_bufferDirection == WriteBuffer && _bufferOffset > 0) {
            _file.seek(_bufferFileOffset);
            _file.write((const void*)_buffer, _bufferOffset); //_bufferOffset = 0;
        }
        _bufferDirection = ReadBuffer;
        _bufferFileOffset = _readOffset;
        _bufferOffset = 0;

        _file.seek(_bufferFileOffset);
        _file.read((void *)_buffer, TERRA_WIFISTREAM_BUFFER_SIZE);
    }
}

void TerraWiFiStorageFileStream::prepareWriteBuffer()
{
    if (_bufferDirection != WriteBuffer || _bufferFileOffset == -1 || _writeOffset < _bufferFileOffset || _writeOffset >= _bufferFileOffset + TERRA_WIFISTREAM_BUFFER_SIZE) {
        if (_bufferDirection == WriteBuffer && _bufferOffset > 0) {
            _file.seek(_bufferFileOffset);
            _file.write((const void*)_buffer, _bufferOffset); //_bufferOffset = 0;
        }
        _bufferDirection = WriteBuffer;
        _bufferFileOffset = _writeOffset;
        _bufferOffset = 0;
    }
}

#endif
