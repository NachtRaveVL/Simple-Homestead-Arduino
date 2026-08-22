/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Controller
*/

#include "Terraduino.h"
#include "TerraUtils.h"
#include <string.h>

#ifdef ARDUINO
static TerraRTCInterface *_rtcSyncProvider = nullptr;
static time_t rtcNow()
{
    return _rtcSyncProvider ? (time_t)_rtcSyncProvider->now().unixtime() : 0;
}
#endif

Terraduino *Terraduino::_activeInstance = nullptr;
Terraduino *getController()
{
    return Terraduino::getActiveInstance();
}
TerraLogger *getLogger()
{
    return getController() ? &getController()->logger : nullptr;
}
TerraPublisher *getPublisher()
{
    return getController() ? &getController()->publisher : nullptr;
}
TerraScheduler *getScheduler()
{
    return getController() ? &getController()->scheduler : nullptr;
}

time_t terraUnixNow()
{
#ifdef ARDUINO
    return now();
#else
    return time(nullptr);
#endif
}

#ifdef ARDUINO
time_t terraUnixTime(DateTime localTime)
{
    return (time_t)localTime.unixtime() - (getController() ? getController()->getTimeZoneOffset() : 0);
}

DateTime terraLocalTime(time_t unixTime)
{
    return DateTime((uint32_t)(unixTime + (getController() ? getController()->getTimeZoneOffset() : 0)));
}

DateTime terraLocalNow()
{
    return terraLocalTime(terraUnixNow());
}
#else
time_t terraLocalNow()
{
    return terraUnixNow() + (getController() ? getController()->getTimeZoneOffset() : 0);
}
#endif

Terraduino::Terraduino(Terra_RTCType rtcType, TerraDeviceSetup rtcSetup)
    : TerraFactory(), TerraCalibrations(), TerraObjectRegistration(), scheduler(), logger(), publisher(), _data(),
      _rtcType(rtcType), _rtcSetup(rtcSetup),
#ifdef ARDUINO
      _rtc(nullptr),
#endif
      _rtcBegan(false), _rtcBattFail(false), _initialized(false), _running(false),
      _lastControlAt(0), _lastPollAt(0), _pollingFrame(1)
{
    _activeInstance = this;
}

Terraduino::~Terraduino()
{
    suspend();
    deallocateRTC();
    if (_activeInstance == this) { _activeInstance = nullptr; }
}

void Terraduino::allocateRTC()
{
#ifdef ARDUINO
    if (!_rtc && _rtcType != Terra_RTCType_None && _rtcSetup.cfgType == TerraDeviceSetup::I2CSetup) {
        switch (_rtcType) {
            case Terra_RTCType_DS1307: _rtc = new TerraRTCWrapper<RTC_DS1307>(); break;
            case Terra_RTCType_DS3231: _rtc = new TerraRTCWrapper<RTC_DS3231>(); break;
            case Terra_RTCType_PCF8523: _rtc = new TerraRTCWrapper<RTC_PCF8523>(); break;
            case Terra_RTCType_PCF8563: _rtc = new TerraRTCWrapper<RTC_PCF8563>(); break;
            default: break;
        }
        _rtcBegan = false;
        TERRA_SOFT_ASSERT(_rtc, "RTC allocation failed");
        TERRA_HARD_ASSERT(_rtcSetup.i2c.address == 0, "RTClib only supports the default I2C RTC address");
    }
#endif
}

void Terraduino::deallocateRTC()
{
#ifdef ARDUINO
    if (_rtc) {
        if (_rtcSyncProvider == _rtc) {
            setSyncProvider(nullptr);
            _rtcSyncProvider = nullptr;
        }
        delete _rtc;
        _rtc = nullptr;
        _rtcBegan = false;
    }
#endif
}

void Terraduino::init(Terra_ControlMode controlMode, Terra_MeasurementMode measurementMode)
{
    _data.controlMode = controlMode;
    _data.measurementMode = measurementMode;
    _initialized = true;
    _running = false;
    _lastControlAt = 0;
    _lastPollAt = 0;
    _pollingFrame = 1;
    logger.setSubData(&_data.logger);
    publisher.setSubData(&_data.publisher);
#ifdef ARDUINO
    if ((_rtcSyncProvider = getRTC())) { setSyncProvider(rtcNow); }
#endif
    logger.info("system", "initialized");
}

void Terraduino::launch()
{
    if (!_initialized) { init(); }
    _running = true;
    logger.info("system", "launched");
}

void Terraduino::suspend()
{
    _running = false;
    logger.info("system", "suspended");
}

void Terraduino::update()
{
    if (!_initialized || !_running || _data.controlMode == Terra_ControlMode_Disabled) { return; }

    uint32_t current = millis();
    bool pollSensors = !_lastPollAt || (uint32_t)(current - _lastPollAt) >= _data.pollingInterval;
    bool runControl = !_lastControlAt || (uint32_t)(current - _lastControlAt) >= TERRA_CONTROL_LOOP_INTERVAL;

    if (pollSensors) {
        _lastPollAt = current;
        if (++_pollingFrame == tframe_none) { ++_pollingFrame; }
    }

    if (runControl) {
        _lastControlAt = current;
        updateObjects(current);
        scheduler.update();
    }

    if (pollSensors) { publisher.advancePollingFrame(_pollingFrame, current); }
}

void Terraduino::setSystemName(const TerraString &name)
{
    strncpy(_data.systemName, name.c_str(), TERRA_NAME_MAXSIZE - 1);
    _data.systemName[TERRA_NAME_MAXSIZE - 1] = '\0';
}

void Terraduino::setTimeZoneOffset(int8_t hoursOffset)
{
    _data.timeZoneOffset = hoursOffset;
}

time_t Terraduino::getTimeZoneOffset() const
{
    return (time_t)_data.timeZoneOffset * 3600L;
}

#ifdef ARDUINO
TerraRTCInterface *Terraduino::getRTC(bool begin)
{
    if (!_rtc) { allocateRTC(); }
    if (_rtc && begin && !_rtcBegan) {
        _rtcBegan = _rtc->begin(_rtcSetup.i2c.wire);
        if (_rtcBegan) {
            bool rtcBattFailBefore = _rtcBattFail;
            _rtcBattFail = _rtc->lostPower();
            if (_rtcBattFail && !rtcBattFailBefore) { logger.warning("rtc", "RTC battery failure"); }
        } else {
            deallocateRTC();
        }
    }
    return (!begin || _rtcBegan) ? _rtc : nullptr;
}

void Terraduino::setRTCTime(DateTime time)
{
    TerraRTCInterface *rtc = getRTC();
    if (rtc) {
        rtc->adjust(DateTime((uint32_t)terraUnixTime(time)));
        _rtcBattFail = false;
    }
}
#endif

TerraString Terraduino::exportSystemJSON() const
{
    DynamicJsonDocument doc(TERRA_JSON_DOC_EXPORTSIZE);
    JsonArray records = doc.to<JsonArray>();

    JsonObject systemObject = records.createNestedObject();
    _data.toJSONObject(systemObject);

    for (auto iter = _calibrationData.begin(); iter != _calibrationData.end(); ++iter) {
        if (!iter->second) { continue; }
        JsonObject calibrationObject = records.createNestedObject();
        iter->second->toJSONObject(calibrationObject);
    }

    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (!iter->second) { continue; }
        TerraObjectData *data = iter->second->newSaveData();
        if (!data) { continue; }
        JsonObject object = records.createNestedObject();
        data->toJSONObject(object);
        delete data;
    }

    TerraString output;
    serializeJson(doc, output);
    return output;
}

bool Terraduino::importSystemJSON(const TerraString &json)
{
    DynamicJsonDocument doc(TERRA_JSON_DOC_EXPORTSIZE);
    if (deserializeJson(doc, json.c_str())) { return false; }
    JsonArrayConst records = doc.as<JsonArrayConst>();
    if (records.isNull() || !records.size()) { return false; }

    TerraSystemData systemData;
    bool haveSystemData = false;
    Vector<TerraData *> decoded;

    for (JsonObjectConst object : records) {
        TerraData *data = newDataFromJSONObject(object);
        if (!data) {
            for (auto decodedData : decoded) { delete decodedData; }
            return false;
        }
        if (data->isSystemData()) {
            systemData = *static_cast<TerraSystemData *>(data);
            haveSystemData = true;
            delete data;
        } else {
            decoded.push_back(data);
        }
    }

    if (!haveSystemData) {
        for (auto decodedData : decoded) { delete decodedData; }
        return false;
    }

    clearObjects();
    clearUserCalibrations();
    _data = systemData;

    bool success = true;
    for (auto data : decoded) {
        if (data->isCalibrationData()) {
            success = setUserCalibrationData(static_cast<TerraCalibrationData *>(data)) && success;
        } else if (data->isObjectData()) {
            TerraObject *object = TerraFactory::newObjectFromData(static_cast<TerraObjectData *>(data));
            if (object) {
                SharedPtr<TerraObject> sharedObject(object);
                bool registered = registerObject(sharedObject);
                success = registered && success;
            } else {
                success = false;
            }
        }
        delete data;
    }

    logger.setSubData(&_data.logger);
    publisher.setSubData(&_data.publisher);
    _initialized = true;
    _running = false;
    _pollingFrame = 1;
    _lastControlAt = 0;
    _lastPollAt = 0;
    return success;
}
