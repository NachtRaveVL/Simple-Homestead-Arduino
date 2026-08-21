/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Controller
*/

#include "Terraduino.h"
#include "TerraUtils.h"

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
    : TerraFactory(), TerraObjectRegistration(), scheduler(), logger(), publisher(), modules(), _data(),
      _rtcType(rtcType), _rtcSetup(rtcSetup),
#ifdef ARDUINO
      _rtc(nullptr),
#endif
      _rtcBegan(false), _rtcBattFail(false), _initialized(false), _running(false), _lastUpdateAt(0)
{
    _activeInstance = this;
}

Terraduino::~Terraduino()
{
    suspend();
    while (_objects.size()) { _objects.erase(_objects.begin()); }
    deallocateRTC();
    if (_activeInstance == this) _activeInstance = nullptr;
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

void Terraduino::init(const TerraSystemSetup &setup)
{
    _data.setup = setup;
    _initialized = true;
    _running = false;
    _lastUpdateAt = 0;
    logger.setMinimumLevel(_data.setup.loggerMinimumLevel);
    publisher.setInterval(_data.setup.publisherIntervalMs);
#ifdef ARDUINO
    if ((_rtcSyncProvider = getRTC())) setSyncProvider(rtcNow);
#endif
    logger.info("system", "initialized");
}

void Terraduino::launch()
{
    if (!_initialized) init();
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
    if (!_initialized || !_running || _data.setup.controlMode == Terra_ControlMode_Disabled) return;
    uint32_t current = terraMillis();
    if (_lastUpdateAt && !terraElapsed(current, _lastUpdateAt, _data.setup.updateIntervalMs)) return;
    _lastUpdateAt = current;
    updateObjects(current);
    scheduler.update();
    publisher.update(current);
}

void Terraduino::setTimeZoneOffset(int8_t hoursOffset)
{
    _data.setup.timeZoneOffset = hoursOffset;
}

time_t Terraduino::getTimeZoneOffset() const
{
    return (time_t)_data.setup.timeZoneOffset * 3600L;
}

#ifdef ARDUINO
TerraRTCInterface *Terraduino::getRTC(bool begin)
{
    if (!_rtc) allocateRTC();
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

bool Terraduino::importSystemJSON(const TerraString &json)
{
    TerraSystemData parsed = _data;
    if (!parsed.fromJSON(json)) return false;
    _data = parsed;
    logger.setMinimumLevel(_data.setup.loggerMinimumLevel);
    publisher.setInterval(_data.setup.publisherIntervalMs);
    return true;
}

bool Terraduino::importSystemBinary(const uint8_t *buffer, size_t length)
{
    TerraSystemData parsed = _data;
    if (!parsed.fromBinary(buffer, length)) return false;
    _data = parsed;
    logger.setMinimumLevel(_data.setup.loggerMinimumLevel);
    publisher.setInterval(_data.setup.publisherIntervalMs);
    return true;
}
