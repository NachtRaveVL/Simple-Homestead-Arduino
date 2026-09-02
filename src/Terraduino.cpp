/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Controller
*/

#include "Terraduino.h"
#include "shared/TerraduinoUI.h"

static TerraRTCInterface *_rtcSyncProvider = nullptr;
time_t rtcNow() {
    return _rtcSyncProvider ? _rtcSyncProvider->now().unixtime() : 0;
}

void handleInterrupt(pintype_t pin)
{
    if (Terraduino::_activeInstance) {
        for (auto iter = Terraduino::_activeInstance->_objects.begin(); iter != Terraduino::_activeInstance->_objects.end(); ++iter) {
            if (iter->second->isSensorType()) {
                auto sensor = static_pointer_cast<TerraSensor>(iter->second);
                if (sensor->isBinaryClass()) {
                    auto binarySensor = static_pointer_cast<TerraBinarySensor>(sensor);
                    if (binarySensor && binarySensor->getInputPin().pin == pin) { binarySensor->notifyISRTriggered(); }
                }
            }
        }

        if (pin < tpin_virtual) {
            for (auto iter = Terraduino::_activeInstance->_pinMuxers.begin(); iter != Terraduino::_activeInstance->_pinMuxers.end(); ++iter) {
                if (iter->second->getInterruptPin().pin == pin) {
                    handleInterrupt(iter->second->getSignalPin().pin);
                }
            }
            #ifdef TERRA_USE_MULTITASKING
                for (auto iter = Terraduino::_activeInstance->_pinExpanders.begin(); iter != Terraduino::_activeInstance->_pinExpanders.end(); ++iter) {
                    if (iter->second->getInterruptPin().pin == pin) {
                        iter->second->trySyncChannel();
                        for (pintype_t virtPin = tpin_virtual + (16 * iter->second->getExpanderPos());
                            virtPin < tpin_virtual + (16 * (iter->second->getExpanderPos() + 1));
                            ++virtPin) {
                            handleInterrupt(virtPin);
                        }
                    }
                }
            #endif
        }
    }
}


Terraduino *Terraduino::_activeInstance = nullptr;

Terraduino::Terraduino(pintype_t piezoBuzzerPin,
                       Terra_EEPROMType eepromType, DeviceSetup eepromSetup,
                       Terra_RTCType rtcType, DeviceSetup rtcSetup,
                       DeviceSetup sdSetup,
                       DeviceSetup netSetup,
                       DeviceSetup gpsSetup,
                       pintype_t *ctrlInputPins,
                       DeviceSetup displaySetup)
    : _piezoBuzzerPin(piezoBuzzerPin),
      _eepromType(eepromType), _eepromSetup(eepromSetup), _eeprom(nullptr), _eepromBegan(false),
      _rtcType(rtcType), _rtcSetup(rtcSetup), _rtc(nullptr), _rtcBegan(false), _rtcBattFail(false),
      _sdSetup(sdSetup), _sd(nullptr), _sdBegan(false), _sdOut(0),
#ifdef TERRA_USE_NET
      _netSetup(netSetup), _netBegan(false),
#endif
#ifdef TERRA_USE_GPS
      _gpsSetup(gpsSetup), _gps(nullptr), _gpsBegan(false),
#endif
#ifdef TERRA_USE_GUI
      _activeUIInstance(nullptr), _uiData(nullptr), _ctrlInputPins(ctrlInputPins), _displaySetup(displaySetup),
#endif
#ifdef TERRA_USE_MULTITASKING
      _controlTaskId(TASKMGR_INVALIDID), _dataTaskId(TASKMGR_INVALIDID), _miscTaskId(TASKMGR_INVALIDID),
#endif
      _systemData(nullptr), _suspend(true), _pollingFrame(0), _lastSpaceCheck(0), _lastAutosave(0),
      _sysConfigFilename(SFP(TStr_Default_ConfigFilename)), _sysDataAddress(-1)
{
    _activeInstance = this;
}

Terraduino::~Terraduino()
{
    suspend();
#ifdef TERRA_USE_GUI
    if (_activeUIInstance) { delete _activeUIInstance; _activeUIInstance = nullptr; }
    if (_uiData) { delete _uiData; _uiData = nullptr; }
#endif
    deactivatePinMuxers();
    while (_objects.size()) { _objects.erase(_objects.begin()); }
    while (_pinOneWire.size()) { dropOneWireForPin(_pinOneWire.begin()->first); }
    while (_pinMuxers.size()) { _pinMuxers.erase(_pinMuxers.begin()); }
#ifdef TERRA_USE_MULTITASKING
    while (_pinExpanders.size()) { _pinExpanders.erase(_pinExpanders.begin()); }
#endif
    deallocateEEPROM();
    deallocateRTC();
    deallocateSD();
#ifdef TERRA_USE_GPS
    deallocateGPS();
#endif
    if (this == _activeInstance) { _activeInstance = nullptr; }
    if (_systemData) { delete _systemData; _systemData = nullptr; }
}

void Terraduino::allocateEEPROM()
{
    if (!_eeprom && _eepromType != Terra_EEPROMType_None && _eepromSetup.cfgType == DeviceSetup::I2CSetup) {
        _eeprom = new I2C_eeprom(TERRA_SYS_I2CEEPROM_BASEADDR | _eepromSetup.cfgAs.i2c.address,
                                 getEEPROMSize(), _eepromSetup.cfgAs.i2c.wire);
        _eepromBegan = false;
        TERRA_SOFT_ASSERT(_eeprom, SFP(TStr_Err_AllocationFailure));
    }
}

void Terraduino::deallocateEEPROM()
{
    if (_eeprom) {
        delete _eeprom; _eeprom = nullptr;
        _eepromBegan = false;
    }
}

void Terraduino::allocateRTC()
{
    if (!_rtc && _rtcType != Terra_RTCType_None && _rtcSetup.cfgType == DeviceSetup::I2CSetup) {
        switch (_rtcType) {
            case Terra_RTCType_DS1307:
                _rtc = new TerraRTCWrapper<RTC_DS1307>();
                break;
            case Terra_RTCType_DS3231:
                _rtc = new TerraRTCWrapper<RTC_DS3231>();
                break;
            case Terra_RTCType_PCF8523:
                _rtc = new TerraRTCWrapper<RTC_PCF8523>();
                break;
            case Terra_RTCType_PCF8563:
                _rtc = new TerraRTCWrapper<RTC_PCF8563>();
                break;
            default: break;
        }
        _rtcBegan = false;
        TERRA_SOFT_ASSERT(_rtc, SFP(TStr_Err_AllocationFailure));
        TERRA_HARD_ASSERT(_rtcSetup.cfgAs.i2c.address == 0b000, F("RTClib does not support i2c multi-addressing, only i2c address 0b000 may be used"));
    }
}

void Terraduino::deallocateRTC()
{
    if (_rtc) {
        if (_rtcSyncProvider == _rtc) { setSyncProvider(nullptr); _rtcSyncProvider = nullptr; }
        delete _rtc; _rtc = nullptr;
        _rtcBegan = false;
    }
}

void Terraduino::allocateSD()
{
    if (!_sd && _sdSetup.cfgType == DeviceSetup::SPISetup) {
        #if !(defined(NO_GLOBAL_INSTANCES) || defined(NO_GLOBAL_SD))
            _sd = &SD;
        #else
            _sd = new SDClass();
        #endif
        _sdBegan = false;
        TERRA_SOFT_ASSERT(_sd, SFP(TStr_Err_AllocationFailure));
    }
}

void Terraduino::deallocateSD()
{
    if (_sd) {
        #if !(defined(NO_GLOBAL_INSTANCES) || defined(NO_GLOBAL_SD))
            _sd = nullptr;
        #else
            delete _sd; _sd = nullptr;
        #endif
        _sdBegan = false;
    }
}

#ifdef TERRA_USE_GPS

void Terraduino::allocateGPS()
{
    if (!_gps && _gpsSetup.cfgType != DeviceSetup::None) {
        switch (_gpsSetup.cfgType) {
            case DeviceSetup::UARTSetup:
                _gps = new GPSClass(_gpsSetup.cfgAs.uart.serial);
                break;
            case DeviceSetup::I2CSetup:
                _gps = new GPSClass(_gpsSetup.cfgAs.i2c.wire);
                break;
            case DeviceSetup::SPISetup:
                _gps = new GPSClass(_gpsSetup.cfgAs.spi.spi, _gpsSetup.cfgAs.spi.cs);
                break;
            default: break;
        }
        _gpsBegan = false;
        TERRA_SOFT_ASSERT(_gps, SFP(TStr_Err_AllocationFailure));
    }
}

void Terraduino::deallocateGPS()
{
    if (_gps) {
        delete _gps; _gps = nullptr;
        _gpsBegan = false;
    }
}

#endif

void Terraduino::init(Terra_SystemMode systemMode,
                      Terra_MeasurementMode measureMode,
                      Terra_DisplayOutputMode dispOutMode,
                      Terra_ControlInputMode ctrlInMode)
{
    TERRA_HARD_ASSERT(!_systemData, SFP(TStr_Err_AlreadyInitialized));

    if (!_systemData) {
        commonPreInit();

        TERRA_SOFT_ASSERT((int)systemMode >= 0 && systemMode < Terra_SystemMode_Count, SFP(TStr_Err_InvalidParameter));
        TERRA_SOFT_ASSERT((int)measureMode >= 0 && measureMode < Terra_MeasurementMode_Count, SFP(TStr_Err_InvalidParameter));
        #ifdef TERRA_USE_GUI
            TERRA_SOFT_ASSERT((int)dispOutMode >= 0 && dispOutMode < Terra_DisplayOutputMode_Count, SFP(TStr_Err_InvalidParameter));
            TERRA_SOFT_ASSERT((int)ctrlInMode >= 0 && ctrlInMode < Terra_ControlInputMode_Count, SFP(TStr_Err_InvalidParameter));
        #endif

        _systemData = new TerraSystemData();
        TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_AllocationFailure));

        if (_systemData) {
            _systemData->systemMode = systemMode;
            _systemData->measureMode = measureMode;
            #ifdef TERRA_USE_GUI
                _systemData->dispOutMode = dispOutMode;
                _systemData->ctrlInMode = ctrlInMode;
            #else
                _systemData->dispOutMode = Terra_DisplayOutputMode_Disabled;
                _systemData->ctrlInMode = Terra_ControlInputMode_Disabled;
            #endif

            commonPostInit();
        }
    }  
}

bool Terraduino::initFromEEPROM(bool jsonFormat)
{
    TERRA_HARD_ASSERT(!_systemData, SFP(TStr_Err_AlreadyInitialized));

    if (!_systemData) {
        commonPreInit();

        if (getEEPROM() && _eepromBegan && _sysDataAddress != -1) {
            TerraEEPROMStream eepromStream(_sysDataAddress, getEEPROMSize() - _sysDataAddress);
            return jsonFormat ? initFromJSONStream(&eepromStream) : initFromBinaryStream(&eepromStream);
        }
    }

    return false;
}

bool Terraduino::saveToEEPROM(bool jsonFormat)
{
    TERRA_HARD_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));

    if (_systemData) {
        if (getEEPROM() && _eepromBegan && _sysDataAddress != -1) {
            TerraEEPROMStream eepromStream(_sysDataAddress, getEEPROMSize() - _sysDataAddress);
            return jsonFormat ? saveToJSONStream(&eepromStream) : saveToBinaryStream(&eepromStream);
        }
    }

    return false;
}

bool Terraduino::initFromSDCard(bool jsonFormat)
{
    TERRA_HARD_ASSERT(!_systemData, SFP(TStr_Err_AlreadyInitialized));

    if (!_systemData) {
        commonPreInit();
        auto sd = getSDCard();

        if (sd) {
            bool retVal = false;
            auto configFile = sd->open(_sysConfigFilename.c_str(), FILE_READ);

            if (configFile) {
                retVal = jsonFormat ? initFromJSONStream(&configFile) : initFromBinaryStream(&configFile);

                configFile.close();
            }

            endSDCard(sd);
            return retVal;
        }
    }

    return false;
}

bool Terraduino::saveToSDCard(bool jsonFormat)
{
    TERRA_HARD_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));

    if (_systemData) {
        auto sd = getSDCard();

        if (sd) {
            bool retVal = false;
            if (sd->exists(_sysConfigFilename.c_str())) {
                sd->remove(_sysConfigFilename.c_str());
            }
            auto configFile = sd->open(_sysConfigFilename.c_str(), FILE_WRITE);

            if (configFile) {
                retVal = jsonFormat ? saveToJSONStream(&configFile, false) : saveToBinaryStream(&configFile);

                configFile.flush();
                configFile.close();
            }

            endSDCard(sd);
            return retVal;
        }
    }

    return false;
}

#ifdef TERRA_USE_WIFI_STORAGE

bool Terraduino::initFromWiFiStorage(bool jsonFormat)
{
    TERRA_HARD_ASSERT(!_systemData, SFP(TStr_Err_AlreadyInitialized));

    if (!_systemData) {
        commonPreInit();

        auto configFile = WiFiStorage.open(_sysConfigFilename.c_str());

        if (configFile) {
            bool retVal = false;
            {
                auto configFileStream = TerraWiFiStorageFileStream(configFile);
                retVal = jsonFormat ? initFromJSONStream(&configFileStream) : initFromBinaryStream(&configFileStream);
            }

            configFile.close();
            return retVal;
        }
    }

    return false;
}

bool Terraduino::saveToWiFiStorage(bool jsonFormat)
{
    TERRA_HARD_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));

    if (_systemData) {
        if (WiFiStorage.exists(_sysConfigFilename.c_str())) {
            WiFiStorage.remove(_sysConfigFilename.c_str());
        }
        auto configFile = WiFiStorage.open(_sysConfigFilename.c_str());

        bool retVal = false;
        {
            auto configFileStream = TerraWiFiStorageFileStream(configFile);
            retVal = jsonFormat ? saveToJSONStream(&configFileStream, false) : saveToBinaryStream(&configFileStream);
            configFileStream.flush();
        }

        retVal = retVal && configFile;
        configFile.close();
        return retVal;
    }

    return false;
}

#endif

bool Terraduino::initFromJSONStream(Stream *streamIn)
{
    TERRA_HARD_ASSERT(!_systemData, SFP(TStr_Err_AlreadyInitialized));
    TERRA_SOFT_ASSERT(streamIn && streamIn->available(), SFP(TStr_Err_InvalidParameter));

    if (!_systemData && streamIn && streamIn->available()) {
        commonPreInit();

        {   StaticJsonDocument<TERRA_JSON_DOC_SYSSIZE> doc;
            deserializeJson(doc, *streamIn);
            JsonObjectConst systemDataObj = doc.as<JsonObjectConst>();
            TerraSystemData *systemData = (TerraSystemData *)newDataFromJSONObject(systemDataObj);

            TERRA_SOFT_ASSERT(systemData && systemData->isSystemData(), SFP(TStr_Err_ImportFailure));
            if (systemData && systemData->isSystemData()) {
                _systemData = systemData;
            } else if (systemData) {
                delete systemData;
            }
        }

        if (_systemData) {
            while (streamIn->available()) {
                StaticJsonDocument<TERRA_JSON_DOC_DEFSIZE> doc;
                deserializeJson(doc, *streamIn);
                JsonObjectConst dataObj = doc.as<JsonObjectConst>();
                TerraData *data = newDataFromJSONObject(dataObj);

                TERRA_SOFT_ASSERT(data && (data->isStandardData() || data->isObjectData()), SFP(TStr_Err_ImportFailure));
                if (data && data->isStandardData()) {
                    if (data->isCalibrationData()) {
                        setUserCalibrationData((TerraCalibrationData *)data);
                    } else if (data->isUIData()) {
                        if (_uiData) { delete _uiData; }
                        _uiData = (TerraUIData *)data; data = nullptr;
                    }
                    if (data) { delete data; data = nullptr; }
                } else if (data && data->isObjectData()) {
                    TerraObject *obj = newObjectFromData(data);
                    delete data; data = nullptr;

                    if (obj && !obj->isUnknownType()) {
                        _objects[obj->getKey()] = SharedPtr<TerraObject>(obj);
                    } else {
                        TERRA_SOFT_ASSERT(false, SFP(TStr_Err_ImportFailure));
                        if (obj) { delete obj; }
                        delete _systemData; _systemData = nullptr;
                        break;
                    }
                } else {
                    if (data) { delete data; data = nullptr; }
                    delete _systemData; _systemData = nullptr;
                    break;
                }
            }
        }

        TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_InitializationFailure));
        if (_systemData) { commonPostInit(); }
        return _systemData;
    }

    return false;
}

bool Terraduino::saveToJSONStream(Stream *streamOut, bool compact)
{
    TERRA_HARD_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    TERRA_SOFT_ASSERT(streamOut, SFP(TStr_Err_InvalidParameter));

    if (_systemData && streamOut) {
        {   StaticJsonDocument<TERRA_JSON_DOC_SYSSIZE> doc;

            JsonObject systemDataObj = doc.to<JsonObject>();
            _systemData->toJSONObject(systemDataObj);

            if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                TERRA_SOFT_ASSERT(false, SFP(TStr_Err_ExportFailure));
                return false;
            }
        }

        if (hasUserCalibrations()) {
            for (auto iter = _calibrationData.begin(); iter != _calibrationData.end(); ++iter) {
                StaticJsonDocument<TERRA_JSON_DOC_DEFSIZE> doc;

                JsonObject calibDataObj = doc.to<JsonObject>();
                iter->second->toJSONObject(calibDataObj);

                if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                    TERRA_SOFT_ASSERT(false, SFP(TStr_Err_ExportFailure));
                    return false;
                }
            }
        }

        if (_uiData) {
            StaticJsonDocument<TERRA_JSON_DOC_DEFSIZE> doc;

            JsonObject uiDataObj = doc.to<JsonObject>();
            _uiData->toJSONObject(uiDataObj);

            if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                TERRA_SOFT_ASSERT(false, SFP(TStr_Err_ExportFailure));
                return false;
            }
        }

        if (_objects.size()) {
            for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
                TerraData *data = iter->second->newSaveData();

                TERRA_SOFT_ASSERT(data && data->isObjectData(), SFP(TStr_Err_AllocationFailure));
                if (data && data->isObjectData()) {
                    StaticJsonDocument<TERRA_JSON_DOC_DEFSIZE> doc;

                    JsonObject objectDataObj = doc.to<JsonObject>();
                    data->toJSONObject(objectDataObj);
                    delete data; data = nullptr;

                    if (!(compact ? serializeJson(doc, *streamOut) : serializeJsonPretty(doc, *streamOut))) {
                        TERRA_SOFT_ASSERT(false, SFP(TStr_Err_ExportFailure));
                        return false;
                    }
                } else {
                    if (data) { delete data; data = nullptr; }
                    return false;
                }
            }
        }

        commonPostSave();
        return true;
    }

    return false;
}

bool Terraduino::initFromBinaryStream(Stream *streamIn)
{
    TERRA_HARD_ASSERT(!_systemData, SFP(TStr_Err_AlreadyInitialized));
    TERRA_SOFT_ASSERT(streamIn && streamIn->available(), SFP(TStr_Err_InvalidParameter));

    if (!_systemData && streamIn && streamIn->available()) {
        commonPreInit();

        {   TerraSystemData *systemData = (TerraSystemData *)newDataFromBinaryStream(streamIn);

            TERRA_SOFT_ASSERT(systemData && systemData->isSystemData(), SFP(TStr_Err_ImportFailure));
            if (systemData && systemData->isSystemData()) {
                _systemData = systemData;
            } else if (systemData) {
                delete systemData;
            }
        }

        if (_systemData) {
            while (streamIn->available()) {
                TerraData *data = newDataFromBinaryStream(streamIn);

                TERRA_SOFT_ASSERT(data && (data->isStandardData() || data->isObjectData()), SFP(TStr_Err_AllocationFailure));
                if (data && data->isStandardData()) {
                    if (data->isCalibrationData()) {
                        setUserCalibrationData((TerraCalibrationData *)data);
                    } else if (data->isUIData()) {
                        if (_uiData) { delete _uiData; }
                        _uiData = (TerraUIData *)data; data = nullptr;
                    }
                    if (data) { delete data; data = nullptr; }
                } else if (data && data->isObjectData()) {
                    TerraObject *obj = newObjectFromData(data);
                    delete data; data = nullptr;

                    if (obj && !obj->isUnknownType()) {
                        _objects[obj->getKey()] = SharedPtr<TerraObject>(obj);
                    } else {
                        TERRA_SOFT_ASSERT(false, SFP(TStr_Err_ImportFailure));
                        if (obj) { delete obj; }
                        delete _systemData; _systemData = nullptr;
                        break;
                    }
                } else {
                    if (data) { delete data; data = nullptr; }    
                    delete _systemData; _systemData = nullptr;
                    break;
                }
            }
        }

        TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_InitializationFailure));
        if (_systemData) { commonPostInit(); }
        return _systemData;
    }

    return false;
}

bool Terraduino::saveToBinaryStream(Stream *streamOut)
{
    TERRA_HARD_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    TERRA_SOFT_ASSERT(streamOut, SFP(TStr_Err_InvalidParameter));

    if (_systemData && streamOut) {
        {   size_t bytesWritten = serializeDataToBinaryStream(_systemData, streamOut);

            TERRA_SOFT_ASSERT(bytesWritten, SFP(TStr_Err_ExportFailure));
            if (!bytesWritten) { return false; }
        }

        if (hasUserCalibrations()) {
            size_t bytesWritten = 0;

            for (auto iter = _calibrationData.begin(); iter != _calibrationData.end(); ++iter) {
                bytesWritten += serializeDataToBinaryStream(iter->second, streamOut);
            }

            TERRA_SOFT_ASSERT(bytesWritten, SFP(TStr_Err_ExportFailure));
            if (!bytesWritten) { return false; }
        }

        if (_uiData) {
            size_t bytesWritten = serializeDataToBinaryStream(_uiData, streamOut);

            TERRA_SOFT_ASSERT(bytesWritten, SFP(TStr_Err_ExportFailure));
            if (!bytesWritten) { return false; }
        }

        if (_objects.size()) {
            for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
                TerraData *data = iter->second->newSaveData();

                TERRA_SOFT_ASSERT(data && data->isObjectData(), SFP(TStr_Err_AllocationFailure));
                if (data && data->isObjectData()) {
                    size_t bytesWritten = serializeDataToBinaryStream(data, streamOut);
                    delete data; data = nullptr;

                    TERRA_SOFT_ASSERT(bytesWritten, SFP(TStr_Err_ExportFailure));
                    if (!bytesWritten) { return false; }
                } else {
                    if (data) { delete data; data = nullptr; }
                    return false;
                }
            }
        }

        commonPostSave();
        return true;
    }

    return false;
}

void Terraduino::commonPreInit()
{
    Map<uintptr_t,uint32_t> began;

    #ifdef TERRA_USE_MULTITASKING
        taskManager.setInterruptCallback(&handleInterrupt);

        for (auto iter = _pinExpanders.begin(); iter != _pinExpanders.end(); ++iter) {
            iter->second->init();
            iter->second->tryRegisterISR();
        }
    #endif
    for (auto iter = _pinMuxers.begin(); iter != _pinMuxers.end(); ++iter) {
        iter->second->init();
        iter->second->tryRegisterISR();
    }

    if (rtcNow() == 0) { setTime(12,0,0,1,1,2000); }

    if (isValidPin(_piezoBuzzerPin)) {
        pinMode(_piezoBuzzerPin, OUTPUT);
        #ifdef ESP32
            ledcSetup(0, 0, 10);
            ledcAttachPin(_piezoBuzzerPin, 0);
        #elif !defined(ARDUINO_SAM_DUE)
            noTone(_piezoBuzzerPin);
        #else
            digitalWrite(_piezoBuzzerPin, 0);
        #endif
    }
    if (_eepromType != Terra_EEPROMType_None && _eepromSetup.cfgType == DeviceSetup::I2CSetup) {
        if (began.find((uintptr_t)_eepromSetup.cfgAs.i2c.wire) == began.end() || _eepromSetup.cfgAs.i2c.speed < began[(uintptr_t)_eepromSetup.cfgAs.i2c.wire]) {
            _eepromSetup.cfgAs.i2c.wire->begin();
            _eepromSetup.cfgAs.i2c.wire->setClock((began[(uintptr_t)_eepromSetup.cfgAs.i2c.wire] = _eepromSetup.cfgAs.i2c.speed));
        }
    }
    if (_rtcType != Terra_RTCType_None && _rtcSetup.cfgType == DeviceSetup::I2CSetup) {
        if (began.find((uintptr_t)_rtcSetup.cfgAs.i2c.wire) == began.end() || _rtcSetup.cfgAs.i2c.speed < began[(uintptr_t)_rtcSetup.cfgAs.i2c.wire]) {
            _rtcSetup.cfgAs.i2c.wire->begin();
            _rtcSetup.cfgAs.i2c.wire->setClock((began[(uintptr_t)_rtcSetup.cfgAs.i2c.wire] = _rtcSetup.cfgAs.i2c.speed));
        }
    }
    #ifdef TERRA_USE_GUI
        if (getDisplayOutputMode() != Terra_DisplayOutputMode_Disabled && _displaySetup.cfgType == DeviceSetup::I2CSetup) {
            if (began.find((uintptr_t)_displaySetup.cfgAs.i2c.wire) == began.end() || _displaySetup.cfgAs.i2c.speed < began[(uintptr_t)_displaySetup.cfgAs.i2c.wire]) {
                _displaySetup.cfgAs.i2c.wire->begin();
                _displaySetup.cfgAs.i2c.wire->setClock((began[(uintptr_t)_displaySetup.cfgAs.i2c.wire] = _displaySetup.cfgAs.i2c.speed));
            }
        }
    #endif
    if (_sdSetup.cfgType == DeviceSetup::SPISetup && isValidPin(_sdSetup.cfgAs.spi.cs)) {
        if (began.find((uintptr_t)_sdSetup.cfgAs.spi.spi) == began.end()) {
            _sdSetup.cfgAs.spi.spi->begin();
            began[(uintptr_t)_sdSetup.cfgAs.spi.spi] = 0;
        }
        pinMode(_sdSetup.cfgAs.spi.cs, OUTPUT);
        digitalWrite(_sdSetup.cfgAs.spi.cs, HIGH);
    }
    #ifdef TERRA_USE_NET
        if (_netSetup.cfgType == DeviceSetup::SPISetup && isValidPin(_netSetup.cfgAs.spi.cs)) {
            if (began.find((uintptr_t)_netSetup.cfgAs.spi.spi) == began.end()) {
                _netSetup.cfgAs.spi.spi->begin();
                began[(uintptr_t)_netSetup.cfgAs.spi.spi] = 0;
            }
            pinMode(_netSetup.cfgAs.spi.cs, OUTPUT);
            digitalWrite(_netSetup.cfgAs.spi.cs, HIGH);
            #ifdef TERRA_USE_ETHERNET
                Ethernet.init(_netSetup.cfgAs.spi.cs);
            #endif
        } else if (_netSetup.cfgType == DeviceSetup::UARTSetup) {
            if (began.find((uintptr_t)_netSetup.cfgAs.uart.serial) == began.end() || _netSetup.cfgAs.uart.baud < began[(uintptr_t)_netSetup.cfgAs.uart.serial]) {
                _netSetup.cfgAs.uart.serial->begin((began[(uintptr_t)_netSetup.cfgAs.uart.serial] = _netSetup.cfgAs.uart.baud), (uartmode_t)TERRA_SYS_ATWIFI_SERIALMODE);
            }
            #ifdef TERRA_USE_AT_WIFI
                WiFi.init(_netSetup.cfgAs.uart.serial);
            #endif
        }
    #endif
    #ifdef TERRA_USE_WIFI_STORAGE
        //WiFiStorage.begin();
    #endif
}

#ifdef TERRA_USE_VERBOSE_OUTPUT
static void printDeviceSetup(String prefix, const DeviceSetup &devSetup)
{
    switch(devSetup.cfgType) {
        case DeviceSetup::I2CSetup:
            Serial.print(','); Serial.print(' '); Serial.print(prefix); Serial.print(F("I2CAddress: 0x"));
            Serial.print(devSetup.cfgAs.i2c.address, HEX);
            Serial.print(','); Serial.print(' '); Serial.print(prefix); Serial.print(F("I2CSpeed: "));
            Serial.print(roundf(devSetup.cfgAs.i2c.speed / 1000.0f)); Serial.print(F("kHz"));
            break;

        case DeviceSetup::SPISetup:
            Serial.print(','); Serial.print(' '); Serial.print(prefix); Serial.print(F("SPICSPin: "));
            if (isValidPin(devSetup.cfgAs.spi.cs)) { Serial.print(devSetup.cfgAs.spi.cs); }
            else { Serial.print(SFP(TStr_Disabled)); }
            Serial.print(','); Serial.print(' '); Serial.print(prefix); Serial.print(F("SPISpeed: "));
            Serial.print(roundf(devSetup.cfgAs.spi.speed / 1000000.0f)); Serial.print(F("MHz"));
            break;

        case DeviceSetup::UARTSetup:
            Serial.print(','); Serial.print(' '); Serial.print(prefix); Serial.print(F("UARTBaud: "));
            Serial.print(devSetup.cfgAs.uart.baud); Serial.print(F("bps"));
            break;

        default:
            Serial.print(','); Serial.print(' '); Serial.print(prefix); Serial.print(':'); Serial.print(' ');
            Serial.print(SFP(TStr_Disabled));
            break;
    }
}
#endif

void Terraduino::commonPostInit()
{
    if ((_rtcSyncProvider = getRTC())) {
        setSyncProvider(rtcNow);
    }

    scheduler.updateDayTracking(); // also calls setNeedsScheduling & setNeedsRedraw
    logger.updateInitTracking();
    setNeedsTabulation();

    #ifdef TERRA_USE_WIFI
        if (!_systemData->wifiPasswordSeed && _systemData->wifiPassword[0]) {
            setWiFiConnection(getWiFiSSID(), getWiFiPassword()); // sets seed and encrypts
        }
    #endif

    #ifdef TERRA_USE_VERBOSE_OUTPUT
        #if 1 // set to 0 if you just want this gone
            Serial.print(F("Terraduino::commonPostInit piezoBuzzerPin: "));
            if (isValidPin(_piezoBuzzerPin)) { Serial.print(_piezoBuzzerPin); }
            else { Serial.print(SFP(TStr_Disabled)); }
            Serial.print(F(", eepromSize: "));
            if (getEEPROMSize()) { Serial.print(getEEPROMSize()); }
            else { Serial.print(SFP(TStr_Disabled)); }
            printDeviceSetup(F("eeprom"), _eepromSetup);
            Serial.print(F(", rtcType: "));
            if (_rtcType != Terra_RTCType_None) { Serial.print(_rtcType); }
            else { Serial.print(SFP(TStr_Disabled)); }
            printDeviceSetup(F("rtc"), _rtcSetup);
            printDeviceSetup(F("sd"), _sdSetup);
            #ifdef TERRA_USE_NET
                printDeviceSetup(F("net"), _netSetup);
            #endif
            #ifdef TERRA_USE_GUI
                Serial.print(F(", controlInputPins: "));
                if (getControlInputPins().first && _ctrlInputPins && isValidPin(_ctrlInputPins[0])) {
                    Serial.print('{');
                    for (int i = 0; i < getControlInputPins().first; ++i) {
                        if (i) { Serial.print(','); }
                        Serial.print(_ctrlInputPins[i]);
                    }
                    Serial.print('}');
                }
                else { Serial.print(SFP(TStr_Disabled)); }
                printDeviceSetup(F("displaySetup"), _displaySetup);
            #endif
            Serial.print(F(", systemMode: "));
            Serial.print(systemModeToString(getSystemMode()));
            Serial.print(F(", measureMode: "));
            Serial.print(measurementModeToString(getMeasurementMode()));
            Serial.print(F(", dispOutMode: "));
            Serial.print(displayOutputModeToString(getDisplayOutputMode()));
            Serial.print(F(", ctrlInMode: "));
            Serial.print(controlInputModeToString(getControlInputMode()));
            Serial.println(); flushYield();
        #endif
    #endif // /ifdef TERRA_USE_VERBOSE_OUTPUT
}

void Terraduino::commonPostSave()
{
    logger.logSystemSave();

    if (_systemData) {
        _systemData->unsetModified();
    }
    #ifdef TERRA_USE_GUI
        if (_uiData) {
            _uiData->unsetModified();
        }
    #endif

    if (hasUserCalibrations()) {
        for (auto iter = _calibrationData.begin(); iter != _calibrationData.end(); ++iter) {
            iter->second->unsetModified();
        }
    }

    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        iter->second->unsetModified();
    }
}

// Runloops

// Tight updates (buzzer/etc) that need to be ran often
inline void tightUpdates()
{
    // TODO: put in link to buzzer update here. #5 in Hydruino.
}

// Loose updates (gps/etc) that need ran every so often
inline void looseUpdates()
{
    #ifdef TERRA_USE_GPS
        if (Terraduino::_activeInstance->_gps) { while(Terraduino::_activeInstance->_gps->available()) { Terraduino::_activeInstance->_gps->read(); } }
    #endif
    #ifdef TERRA_USE_MQTT
        if (publisher._mqttClient) { publisher._mqttClient->loop(); }
    #endif
}

// Yields upon time limit exceed
inline void yieldIfNeeded(millis_t &lastYield)
{
    tightUpdates();
    millis_t time = millis();
    if (time - lastYield >= TERRA_SYS_YIELD_AFTERMILLIS) {
        looseUpdates();
        lastYield = time; yield();
    }
}

void controlLoop()
{
    if (Terraduino::_activeInstance && !Terraduino::_activeInstance->_suspend) {
        #ifdef TERRA_USE_VERBOSE_OUTPUT
            Serial.println(F("controlLoop")); flushYield();
        #endif
        millis_t lastYield = millis();

        for (auto iter = Terraduino::_activeInstance->_objects.begin(); iter != Terraduino::_activeInstance->_objects.end(); ++iter) {
            iter->second->update();

            yieldIfNeeded(lastYield);
        }

        Terraduino::_activeInstance->scheduler.update();

        #ifdef TERRA_USE_VERBOSE_OUTPUT
            Serial.println(F("~controlLoop")); flushYield();
        #endif
    }

    tightUpdates();
}

void dataLoop()
{
    if (Terraduino::_activeInstance && !Terraduino::_activeInstance->_suspend) {
        #ifdef TERRA_USE_VERBOSE_OUTPUT
            Serial.println(F("dataLoop")); flushYield();
        #endif
        millis_t lastYield = millis();

        Terraduino::_activeInstance->publisher.advancePollingFrame();

        for (auto iter = Terraduino::_activeInstance->_objects.begin(); iter != Terraduino::_activeInstance->_objects.end(); ++iter) {
            if (iter->second->isSensorType()) {
                auto sensor = static_pointer_cast<TerraSensor>(iter->second);
                if (sensor->needsPolling()) {
                    sensor->takeMeasurement(); // no force if already current for this frame #, we're just ensuring data for publisher
                }
            }

            yieldIfNeeded(lastYield);
        }

        #ifdef TERRA_USE_VERBOSE_OUTPUT
            Serial.println(F("~dataLoop")); flushYield();
        #endif
    }

    tightUpdates();
}

void miscLoop()
{
    if (Terraduino::_activeInstance && !Terraduino::_activeInstance->_suspend) {
        #ifdef TERRA_USE_VERBOSE_OUTPUT
            Serial.println(F("miscLoop")); flushYield();
        #endif
        millis_t lastYield = millis();

        #if TERRA_SYS_MEM_LOGGING_ENABLE
        {   static time_t _lastMemLog = unixNow();
            if (unixNow() >= _lastMemLog + 15) {
                _lastMemLog = unixNow();
                Terraduino::_activeInstance->logger.logMessage(String(F("Free memory: ")), String(freeMemory()));
            }
        }
        #endif
        Terraduino::_activeInstance->checkFreeMemory();

        yieldIfNeeded(lastYield);

        Terraduino::_activeInstance->checkFreeSpace();

        yieldIfNeeded(lastYield);

        Terraduino::_activeInstance->checkAutosave();

        yieldIfNeeded(lastYield);

        Terraduino::_activeInstance->publisher.update();

        #ifdef TERRA_USE_GPS
            yieldIfNeeded(lastYield);

            if (Terraduino::_activeInstance->_gps && Terraduino::_activeInstance->_gps->newNMEAreceived()) {
                Terraduino::_activeInstance->_gps->parse(Terraduino::_activeInstance->_gps->lastNMEA());
                if (Terraduino::_activeInstance->_gps->fix) {
                    Terraduino::_activeInstance->setSystemLocation(Terraduino::_activeInstance->_gps->lat, Terraduino::_activeInstance->_gps->lon, Terraduino::_activeInstance->_gps->altitude);
                }
            }
        #endif

        #ifdef TERRA_USE_VERBOSE_OUTPUT
            Serial.println(F("~miscLoop")); flushYield();
        #endif
    }

    tightUpdates();
}

void Terraduino::launch()
{
    // Forces all sensors to get a new measurement
    publisher.advancePollingFrame();

    // Create/enable main runloops
    _suspend = false;
    #ifdef TERRA_USE_MULTITASKING
        if (!isValidTask(_controlTaskId)) {
            _controlTaskId = taskManager.scheduleFixedRate(TERRA_CONTROL_LOOP_INTERVAL, controlLoop);
        } else {
            taskManager.setTaskEnabled(_controlTaskId, true);
        }
        if (!isValidTask(_dataTaskId)) {
            _dataTaskId = taskManager.scheduleFixedRate(getPollingInterval(), dataLoop);
        } else {
            taskManager.setTaskEnabled(_dataTaskId, true);
        }
        if (!isValidTask(_miscTaskId)) {
            _miscTaskId = taskManager.scheduleFixedRate(TERRA_MISC_LOOP_INTERVAL, miscLoop);
        } else {
            taskManager.setTaskEnabled(_miscTaskId, true);
        }
    #endif

    #ifdef TERRA_USE_VERBOSE_OUTPUT
        Serial.println(F("Terraduino::launch System launched!")); flushYield();
    #endif
}

void Terraduino::suspend()
{
    _suspend = true;
    #ifdef TERRA_USE_MULTITASKING
        if (isValidTask(_controlTaskId)) {
            taskManager.setTaskEnabled(_controlTaskId, false);
        }
        if (isValidTask(_dataTaskId)) {
            taskManager.setTaskEnabled(_dataTaskId, false);
        }
        if (isValidTask(_miscTaskId)) {
            taskManager.setTaskEnabled(_miscTaskId, false);
        }
    #endif

    #ifdef TERRA_USE_VERBOSE_OUTPUT
        Serial.println(F("Terraduino::suspend System suspended!")); flushYield();
    #endif
}

void Terraduino::update()
{
    #ifdef TERRA_USE_MULTITASKING
        taskManager.runLoop(); // tcMenu also uses this system to run its UI
    #else
        controlLoop();
        dataLoop();
        miscLoop();
    #endif

    looseUpdates();
    tightUpdates();
}

void Terraduino::setSystemName(String systemName)
{
    TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    if (_systemData && !systemName.equals(getSystemName())) {
        strncpy(_systemData->systemName, systemName.c_str(), TERRA_NAME_MAXSIZE);

        setNeedsRedraw();
        _systemData->bumpRevisionIfNeeded();
    }
}

void Terraduino::setTimeZoneOffset(float hoursOffset)
{
    TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    if (_systemData && !isFPEqual(_systemData->timeZoneOffset, hoursOffset)) {
        _systemData->timeZoneOffset = hoursOffset;

        setNeedsRedraw();
        _systemData->bumpRevisionIfNeeded();
    }
}

void Terraduino::setPollingInterval(uint16_t pollingInterval)
{
    TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    if (_systemData && _systemData->pollingInterval != pollingInterval) {
        _systemData->pollingInterval = pollingInterval;
        _systemData->bumpRevisionIfNeeded();

        #ifdef TERRA_USE_MULTITASKING
            if (isValidTask(_dataTaskId)) {
                auto dataTask = taskManager.getTask(_dataTaskId);
                if (dataTask) {
                    bool enabled = dataTask->isEnabled();
                    auto next = dataTask->getNext();
                    dataTask->handleScheduling(getPollingInterval(), TIME_MILLIS, true);
                    dataTask->setNext(next);
                    dataTask->setEnabled(enabled);
                }
            }
        #endif
    }
}

void Terraduino::setAutosaveEnabled(Terra_Autosave autosaveEnabled, Terra_Autosave autosaveFallback, uint16_t autosaveInterval)
{
    TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    if (_systemData && (_systemData->autosaveEnabled != autosaveEnabled || _systemData->autosaveFallback != autosaveFallback || _systemData->autosaveInterval != autosaveInterval)) {
        _systemData->autosaveEnabled = autosaveEnabled;
        _systemData->autosaveFallback = autosaveFallback;
        _systemData->autosaveInterval = autosaveInterval;
        _systemData->bumpRevisionIfNeeded();
    }
}

void Terraduino::setRTCTime(DateTime time)
{
    _setUnixTime(DateTime((uint32_t)unixTime(time)), true);
}

#ifdef TERRA_USE_WIFI

void Terraduino::setWiFiConnection(String ssid, String pass)
{
    TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    if (_systemData) {
        bool ssidChanged = !ssid.equals(getWiFiSSID());
        bool passChanged = !pass.equals(getWiFiPassword());

        if (ssidChanged || passChanged || (pass.length() && !_systemData->wifiPasswordSeed)) {
            if (ssid.length()) {
                strncpy(_systemData->wifiSSID, ssid.c_str(), TERRA_NAME_MAXSIZE);
            } else {
                memset(_systemData->wifiSSID, '\000', TERRA_NAME_MAXSIZE);
            }

            if (pass.length()) {
                randomSeed(unixNow());
                _systemData->wifiPasswordSeed = random(1, RANDOM_MAX);

                randomSeed(_systemData->wifiPasswordSeed);
                for (int charIndex = 0; charIndex < TERRA_NAME_MAXSIZE; ++charIndex) {
                    _systemData->wifiPassword[charIndex] = (uint8_t)(charIndex < pass.length() ? pass[charIndex] : '\000') ^ (uint8_t)random(256);
                }
            } else {
                _systemData->wifiPasswordSeed = 0;
                memset(_systemData->wifiPassword, '\000', TERRA_NAME_MAXSIZE);
            }

            _systemData->bumpRevisionIfNeeded();

            if (_netBegan && (ssidChanged || passChanged)) { WiFi.disconnect(); _netBegan = false; } // forces re-connect on next getWiFi
        }
    }
}

#endif
#ifdef TERRA_USE_ETHERNET

void Terraduino::setEthernetConnection(const uint8_t *macAddress)
{
    TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    if (_systemData) {
        bool macChanged = memcmp(macAddress, getMACAddress(), sizeof(uint8_t[6])) != 0;

        if (macChanged) {
            memcpy(_systemData->macAddress, macAddress, sizeof(uint8_t[6]));
            _systemData->bumpRevisionIfNeeded();

            if (_netBegan) { Ethernet.setMACAddress(macAddress); }
        }
    }
}

#endif

void Terraduino::setSystemLocation(double latitude, double longitude, double altitude, bool isSigChange)
{
    TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    if (_systemData && (!isFPEqual(_systemData->latitude, latitude) || !isFPEqual(_systemData->longitude, longitude) || !isFPEqual(_systemData->altitude, altitude))) {
        isSigChange = isSigChange || ((latitude - _systemData->latitude) * (latitude - _systemData->latitude)) +
                                     ((longitude - _systemData->longitude) * (longitude - _systemData->longitude)) >= TERRA_SYS_LATLONG_DISTSQRDTOL ||
                                     fabs(altitude - _systemData->altitude) >= TERRA_SYS_ALTITUDE_DISTTOL;
        _systemData->latitude = latitude;
        _systemData->longitude = longitude;
        _systemData->altitude = altitude;
        if (isSigChange) { notifySignificantLocation(*((Location *)&_systemData->latitude)); }
    }
}

#ifdef TERRA_USE_GUI

Pair<uint8_t, const pintype_t *> Terraduino::getControlInputPins() const
{
    if (_ctrlInputPins) {
        switch (getControlInputMode()) {
            case Terra_ControlInputMode_RotaryEncoderOk:
            case Terra_ControlInputMode_UpDownButtonsOk:
            case Terra_ControlInputMode_UpDownESP32TouchOk:
            case Terra_ControlInputMode_AnalogJoystickOk:
                return make_pair((uint8_t)3, (const pintype_t *)_ctrlInputPins);
            case Terra_ControlInputMode_RotaryEncoderOkLR:
            case Terra_ControlInputMode_UpDownButtonsOkLR:
            case Terra_ControlInputMode_UpDownESP32TouchOkLR:
                return make_pair((uint8_t)5, (const pintype_t *)_ctrlInputPins);
            case Terra_ControlInputMode_Matrix3x4Keyboard_OptRotEncOk:  
                return make_pair((uint8_t)10, (const pintype_t *)_ctrlInputPins);
            case Terra_ControlInputMode_Matrix3x4Keyboard_OptRotEncOkLR:
                return make_pair((uint8_t)12, (const pintype_t *)_ctrlInputPins);
            case Terra_ControlInputMode_Matrix4x4Keyboard_OptRotEncOk:
                return make_pair((uint8_t)11, (const pintype_t *)_ctrlInputPins);
            case Terra_ControlInputMode_Matrix4x4Keyboard_OptRotEncOkLR:
                return make_pair((uint8_t)13, (const pintype_t *)_ctrlInputPins);
            case Terra_ControlInputMode_Matrix2x2UpDownButtonsOkL:
            case Terra_ControlInputMode_ResistiveTouch:
                return make_pair((uint8_t)4, (const pintype_t *)_ctrlInputPins);
            #ifdef TERRA_UI_ENABLE_XPT2046TS
                case Terra_ControlInputMode_TouchScreen:
            #endif
            case Terra_ControlInputMode_TFTTouch:
                return make_pair((uint8_t)2, (const pintype_t *)_ctrlInputPins);
            default: break;
        }
    }
    return make_pair((uint8_t)0, (const pintype_t *)nullptr);
}

#endif

I2C_eeprom *Terraduino::getEEPROM(bool begin)
{
    if (!_eeprom) { allocateEEPROM(); }

    if (_eeprom && begin && !_eepromBegan) {
        _eepromBegan = _eeprom->begin();

        if (!_eepromBegan) { deallocateEEPROM(); }
    }

    return (!begin || _eepromBegan) ? _eeprom : nullptr;
}

TerraRTCInterface *Terraduino::getRTC(bool begin)
{
    if (!_rtc) { allocateRTC(); }

    if (_rtc && begin && !_rtcBegan) {
        _rtcBegan = _rtc->begin(_rtcSetup.cfgAs.i2c.wire);

        if (_rtcBegan) {
            bool rtcBattFailBefore = _rtcBattFail;
            _rtcBattFail = _rtc->lostPower();
            if (_rtcBattFail && !rtcBattFailBefore) {
                logger.logWarning(SFP(TStr_Log_RTCBatteryFailure));
            }
        } else { deallocateRTC(); }
    }

    return (!begin || _rtcBegan) ? _rtc : nullptr;
}

SDClass *Terraduino::getSDCard(bool begin)
{
    if (!_sd) { allocateSD(); }

    if (_sd && begin) {
        if (!_sdBegan) {
            #if defined(ESP32)
                _sdBegan = _sd->begin(_sdSetup.cfgAs.spi.cs, *_sdSetup.cfgAs.spi.spi, _sdSetup.cfgAs.spi.speed);
            #elif defined(CORE_TEENSY)
                _sdBegan = _sd->begin(_sdSetup.cfgAs.spi.cs); // card speed not possible to set on teensy
            #else
                _sdBegan = _sd->begin(_sdSetup.cfgAs.spi.speed, _sdSetup.cfgAs.spi.cs);
            #endif
        }

        if (!_sdBegan && _sdOut == 0) { deallocateSD(); }

        if (_sd && _sdBegan) { _sdOut++; }
    }

    return (!begin || _sdBegan) ? _sd : nullptr;
}

void Terraduino::endSDCard(SDClass *sd)
{
    #if defined(CORE_TEENSY)
        --_sdOut; // no delayed write on teensy's SD impl
    #else
        if (--_sdOut == 0 && _sd) {
            _sd->end();
        }
    #endif
}

#ifdef TERRA_USE_WIFI

WiFiClass *Terraduino::getWiFi(String ssid, String pass, bool begin)
{
    int status = WiFi.status();

    if (begin && (!_netBegan || status != WL_CONNECTED)) {
        if (status == WL_CONNECTED) {
            _netBegan = true;
        } else if (status == WL_NO_SHIELD) {
            _netBegan = false;
        } else { // attempt connection
            #ifdef TERRA_USE_AT_WIFI
                status = WiFi.begin(ssid.c_str(), pass.c_str());
            #else
                status = pass.length() ? WiFi.begin(const_cast<char *>(ssid.c_str()), pass.c_str())
                                       : WiFi.begin(const_cast<char *>(ssid.c_str()));
            #endif

            _netBegan = (status == WL_CONNECTED);
        }
    }

    return (!begin || _netBegan) ? &WiFi : nullptr;
}

#endif
#ifdef TERRA_USE_ETHERNET

EthernetClass *Terraduino::getEthernet(const uint8_t *macAddress, bool begin)
{
    int status = Ethernet.linkStatus();

    if (begin && (!_netBegan || status != LinkON)) {
        if (status == LinkON) {
            _netBegan = true;
        } else if (Ethernet.hardwareStatus() == EthernetNoHardware) {
            _netBegan = false;
        } else { // attempt connection
            status = Ethernet.begin(const_cast<uint8_t *>(getMACAddress()));

            _netBegan = (status == LinkON);
        }
    }

    return (!begin || _netBegan) ? &Ethernet : nullptr;
}

#endif
#ifdef TERRA_USE_GPS

GPSClass *Terraduino::getGPS(bool begin)
{
    if (!_gps) { allocateGPS(); }

    if (_gps && begin && !_gpsBegan) {
        switch (_gpsSetup.cfgType) {
            case DeviceSetup::UARTSetup:
                _gpsBegan = _gps->begin(_gpsSetup.cfgAs.uart.baud);
                break;
            case DeviceSetup::I2CSetup:
                _gpsBegan = _gps->begin(GPS_DEFAULT_I2C_ADDR | _gpsSetup.cfgAs.i2c.address);
                break;
            case DeviceSetup::SPISetup:
                _gpsBegan = _gps->begin(_gpsSetup.cfgAs.spi.speed);
                break;
            default: break;
        }
        if (!_gpsBegan) { deallocateGPS(); }
    }

    return (!begin || _gpsBegan) ? _gps : nullptr;
}

#endif

Terra_SystemMode Terraduino::getSystemMode() const
{
    return _systemData ? _systemData->systemMode : Terra_SystemMode_Undefined;
}

Terra_MeasurementMode Terraduino::getMeasurementMode() const
{
    return _systemData ? _systemData->measureMode : Terra_MeasurementMode_Undefined;
}

Terra_DisplayOutputMode Terraduino::getDisplayOutputMode() const
{
    return _systemData ? _systemData->dispOutMode : Terra_DisplayOutputMode_Undefined;
}

Terra_ControlInputMode Terraduino::getControlInputMode() const
{
    return _systemData ? _systemData->ctrlInMode : Terra_ControlInputMode_Undefined;
}

String Terraduino::getSystemName() const
{
    TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    return _systemData ? String(_systemData->systemName) : String();
}

time_t Terraduino::getTimeZoneOffset() const
{
    TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    return _systemData ? _systemData->timeZoneOffset * SECS_PER_HOUR : 0;
}

uint16_t Terraduino::getPollingInterval() const
{
    TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    return _systemData ? _systemData->pollingInterval : 0;
}

bool Terraduino::isPollingFrameOld(tframe_t frame, tframe_t allowance) const
{
    return _pollingFrame - frame > allowance;
}

bool Terraduino::isAutosaveEnabled() const
{
    TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    return _systemData ? _systemData->autosaveEnabled != Terra_Autosave_Disabled : false;
}

bool Terraduino::isAutosaveFallbackEnabled() const
{
    TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    return _systemData ? _systemData->autosaveFallback != Terra_Autosave_Disabled : false;
}

#ifdef TERRA_USE_WIFI

String Terraduino::getWiFiSSID() const
{
    TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    return _systemData ? String(_systemData->wifiSSID) : String();
}

String Terraduino::getWiFiPassword() const
{
    TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    if (_systemData) {
        char wifiPassword[TERRA_NAME_MAXSIZE] = {0};

        if (_systemData->wifiPasswordSeed) {
            randomSeed(_systemData->wifiPasswordSeed);
            for (int charIndex = 0; charIndex < TERRA_NAME_MAXSIZE; ++charIndex) {
                wifiPassword[charIndex] = (char)(_systemData->wifiPassword[charIndex] ^ (uint8_t)random(256));
            }
        } else {
            strncpy(wifiPassword, (const char *)(_systemData->wifiPassword), TERRA_NAME_MAXSIZE);
        }

        return String(wifiPassword);
    }
    return String();
}

#endif
#ifdef TERRA_USE_ETHERNET

const uint8_t *Terraduino::getMACAddress() const
{
    TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    return _systemData ? &_systemData->macAddress[0] : nullptr;
}

#endif

Location Terraduino::getSystemLocation() const
{
    TERRA_SOFT_ASSERT(_systemData, SFP(TStr_Err_NotYetInitialized));
    return _systemData ? Location(_systemData->latitude, _systemData->longitude, _systemData->altitude) : Location();
}

void Terraduino::checkFreeMemory()
{
    auto memLeft = freeMemory();
    if (memLeft != -1 && memLeft < TERRA_SYS_FREERAM_LOWBYTES) {
        broadcastLowMemory();
    }
}

static uint64_t getSDCardFreeSpace()
{
    uint64_t retVal = TERRA_SYS_FREESPACE_LOWSPACE;
    #if defined(CORE_TEENSY)
        auto sd = getController()->getSDCard();
        if (sd) {
            retVal = sd->totalSize() - sd->usedSize();
            getController()->endSDCard(sd);
        }
    #endif
    return retVal;
}

void Terraduino::checkFreeSpace()
{
    if ((logger.isLoggingEnabled() || publisher.isPublishingEnabled()) &&
        (!_lastSpaceCheck || unixNow() >= _lastSpaceCheck + (TERRA_SYS_FREESPACE_INTERVAL * SECS_PER_MIN))) {
        if (logger.isLoggingToSDCard() || publisher.isPublishingToSDCard()) {
            uint32_t freeKB = getSDCardFreeSpace();
            while (freeKB < TERRA_SYS_FREESPACE_LOWSPACE) {
                logger.cleanupOldestLogs(true);
                publisher.cleanupOldestData(true);
                freeKB = getSDCardFreeSpace();
            }
        }
        // TODO: URL free space
        _lastSpaceCheck = unixNow();
    }
}

void Terraduino::checkAutosave()
{
    if (isAutosaveEnabled() && unixNow() >= _lastAutosave + (_systemData->autosaveInterval * SECS_PER_MIN)) {
        performAutosave();
    }
}
