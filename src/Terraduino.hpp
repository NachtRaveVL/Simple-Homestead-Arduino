/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Controller
*/

#ifndef Terraduino_HPP
#define Terraduino_HPP

#include "Terraduino.h"

inline bool Twilight::isDaytime(time_t unixTime) const {
    DateTime time = isUTC ? DateTime((uint32_t)unixTime) : localTime(unixTime);
    double hour = time.hour() + (time.minute() / 60.0) + (time.second() / 3600.0);
    return sunrise <= sunset ? hour >= sunrise && hour <= sunset
                             : hour >= sunrise || hour <= sunset;
}

inline bool Twilight::isDaytime(DateTime localTime) const
{
    DateTime time = isUTC ? DateTime((uint32_t)unixTime(localTime)) : localTime;
    double hour = time.hour() + (time.minute() / 60.0) + (time.second() / 3600.0);
    return sunrise <= sunset ? hour >= sunrise && hour <= sunset
                             : hour >= sunrise || hour <= sunset;
}

inline time_t Twilight::hourToUnixTime(double hour, bool isUTC)
{
    return isUTC ? unixDayStart() + (time_t)(hour * SECS_PER_HOUR)
                 : unixTime(localDayStart() + TimeSpan(hour * SECS_PER_HOUR));
}

inline DateTime Twilight::hourToLocalTime(double hour, bool isUTC)
{
    return isUTC ? localTime(unixDayStart() + (time_t)(hour * SECS_PER_HOUR))
                 : localDayStart() + TimeSpan(hour * SECS_PER_HOUR);
}


#ifdef TERRA_USE_WIFI

inline WiFiClass *Terraduino::getWiFi(bool begin)
{
    return getWiFi(getWiFiSSID(), getWiFiPassword(), begin);
}

#endif
#ifdef TERRA_USE_ETHERNET

inline EthernetClass *Terraduino::getEthernet(bool begin)
{
    return getEthernet(getMACAddress(), begin);
}

#endif

inline void Terraduino::broadcastLowMemory()
{
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        iter->second->handleLowMemory();
    }
}

inline void Terraduino::performAutosave()
{
    for (int autosave = 0; autosave < 2; ++autosave) {
        switch (autosave == 0 ? _systemData->autosaveEnabled : _systemData->autosaveFallback) {
            case Terra_Autosave_EnabledToSDCardJson:
                saveToSDCard(JSON);
                break;
            case Terra_Autosave_EnabledToSDCardRaw:
                saveToSDCard(RAW);
                break;
            case Terra_Autosave_EnabledToEEPROMJson:
                saveToEEPROM(JSON);
                break;
            case Terra_Autosave_EnabledToEEPROMRaw:
                saveToEEPROM(RAW);
                break;
            case Terra_Autosave_EnabledToWiFiStorageJson:
                #ifdef TERRA_USE_WIFI_STORAGE
                    saveToWiFiStorage(JSON);
                #endif
                break;
            case Terra_Autosave_EnabledToWiFiStorageRaw:
                #ifdef TERRA_USE_WIFI_STORAGE
                    saveToWiFiStorage(RAW);
                #endif
                break;
            case Terra_Autosave_Disabled:
                break;
        }
    }

    _lastAutosave = unixNow();
}

inline void Terraduino::notifyRTCTimeUpdated()
{
    _rtcBattFail = false;
}

inline void Terraduino::broadcastDateChanged()
{
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (iter->second->isReservoirType()) {
            auto reservoir = static_pointer_cast<TerraReservoir>(iter->second);

            if (reservoir) { reservoir->notifyDateChanged(); }
        }
    }
}

inline void Terraduino::notifySignificantTime(time_t time)
{
    logger.updateInitTracking(time);
    _lastAutosave = isAutosaveEnabled() ? time : 0;
}

inline void Terraduino::notifySignificantLocation(Location loc)
{
    if (_systemData) { _systemData->bumpRevisionIfNeeded(); }
}


inline TerraLoggerSubData *TerraLogger::loggerData() const
{
    return &Terraduino::_activeInstance->_systemData->logger;
}

inline bool TerraLogger::hasLoggerData() const
{
    return Terraduino::_activeInstance && Terraduino::_activeInstance->_systemData;
}

inline bool TerraLogger::isLoggingToSDCard() const
{
    return hasLoggerData() && loggerData()->logLevel != Terra_LogLevel_None && loggerData()->logToSDCard;
}

#ifdef TERRA_USE_WIFI_STORAGE

inline bool TerraLogger::isLoggingToWiFiStorage() const
{
    return hasLoggerData() && loggerData()->logLevel != Terra_LogLevel_None && loggerData()->logToWiFiStorage;
}

#endif

inline void TerraLogger::logActivation(const TerraActuator *actuator)
{
    if (actuator) { logMessage(actuator->getId().getDisplayString(), SFP(TStr_Log_HasEnabled)); }
}

inline void TerraLogger::logDeactivation(const TerraActuator *actuator)
{
    if (actuator) { logMessage(actuator->getId().getDisplayString(), SFP(TStr_Log_HasDisabled)); }
}

inline void TerraLogger::logProcess(const TerraObjInterface *obj, const String &processString, const String &statusString)
{
    if (obj) { logMessage(obj->getId().getDisplayString(), processString, statusString); }
}

inline void TerraLogger::logStatus(const TerraObjInterface *obj, const String &statusString)
{
    if (obj) { logMessage(obj->getId().getDisplayString(), statusString); }
}

inline Terra_LogLevel TerraLogger::getLogLevel() const
{
    return hasLoggerData() ? loggerData()->logLevel : Terra_LogLevel_None;
}

inline bool TerraLogger::isLoggingEnabled() const
{
    return hasLoggerData() && loggerData()->logLevel != Terra_LogLevel_None && (loggerData()->logToSDCard || loggerData()->logToWiFiStorage);
}


inline TerraPublisherSubData *TerraPublisher::publisherData() const
{
    return &Terraduino::_activeInstance->_systemData->publisher;
}

inline bool TerraPublisher::hasPublisherData() const
{
    return Terraduino::_activeInstance && Terraduino::_activeInstance->_systemData;
}

inline bool TerraPublisher::isPublishingToSDCard() const
{
    return hasPublisherData() && publisherData()->pubToSDCard;
}

#ifdef TERRA_USE_WIFI_STORAGE

inline bool TerraPublisher::isPublishingToWiFiStorage() const
{
    return hasPublisherData() && publisherData()->pubToWiFiStorage;
}

#endif
#ifdef TERRA_USE_MQTT

inline bool TerraPublisher::isPublishingToMQTTClient() const
{
    return hasPublisherData() && _mqttClient;
}

#endif

inline bool TerraPublisher::isPublishingEnabled() const
{
    return hasPublisherData() && (publisherData()->pubToSDCard || publisherData()->pubToWiFiStorage
        #ifdef TERRA_USE_MQTT
            || _mqttClient
        #endif
        );
}

inline void TerraPublisher::setNeedsTabulation()
{
    _needsTabulation = hasPublisherData();
}


inline TerraSchedulerSubData *TerraScheduler::schedulerData() const
{
    return &Terraduino::_activeInstance->_systemData->scheduler;
}

inline bool TerraScheduler::hasSchedulerData() const
{
    return Terraduino::_activeInstance && Terraduino::_activeInstance->_systemData;
}

#endif
