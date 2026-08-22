/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Logger
*/

#include "Terraduino.h"

TerraLogEvent::TerraLogEvent(Terra_LogLevel levelIn, const String &prefixIn, const String &msgIn, const String &suffix1In, const String &suffix2In)
    : level(levelIn), timestamp(localNow().timestamp(DateTime::TIMESTAMP_FULL)), prefix(prefixIn), msg(msgIn), suffix1(suffix1In), suffix2(suffix2In)
{ ; }


TerraLogger::TerraLogger() :
#if TERRA_SYS_LEAVE_FILES_OPEN
    _logFileSD(nullptr),
#ifdef TERRA_USE_WIFI_STORAGE
    _logFileWS(nullptr),
#endif
#endif
    _logFilename(), _initTime(0), _lastSpaceCheck(0)
{ ; }

TerraLogger::~TerraLogger()
{
    flush();

    #if TERRA_SYS_LEAVE_FILES_OPEN
        if (_logFileSD) {
            _logFileSD->close();
            delete _logFileSD; _logFileSD = nullptr;
            Terraduino::_activeInstance->endSDCard();
        }
        #ifdef TERRA_USE_WIFI_STORAGE
            if (_logFileWS) {
                _logFileWS->close();
                delete _logFileWS; _logFileWS = nullptr;
            }
        #endif
    #endif
}

bool TerraLogger::beginLoggingToSDCard(String logFilePrefix)
{
    TERRA_SOFT_ASSERT(hasLoggerData(), SFP(HStr_Err_NotYetInitialized));

    if (hasLoggerData() && !loggerData()->logToSDCard) {
        auto sd = Terraduino::_activeInstance->getSDCard();

        if (sd) {
            String logFilename = getYYMMDDFilename(logFilePrefix, SFP(HStr_txt));
            createDirectoryFor(sd, logFilename);
            #if TERRA_SYS_LEAVE_FILES_OPEN
                auto &logFile = _logFileSD ? *_logFileSD : *(_logFileSD = new File(sd->open(logFilename.c_str(), FILE_WRITE)));
            #else
                auto logFile = sd->open(logFilename.c_str(), FILE_WRITE);
            #endif

            if (logFile) {
                #if !TERRA_SYS_LEAVE_FILES_OPEN
                    logFile.close();
                    Terraduino::_activeInstance->endSDCard(sd);
                #endif

                strncpy(loggerData()->logFilePrefix, logFilePrefix.c_str(), 16);
                loggerData()->logToSDCard = true;
                _logFilename = logFilename;
                Terraduino::_activeInstance->_systemData->bumpRevisionIfNeeded();

                return true;
            }
        }

        #if !TERRA_SYS_LEAVE_FILES_OPEN
            Terraduino::_activeInstance->endSDCard(sd);
        #endif
    }

    return false;
}

#ifdef TERRA_USE_WIFI_STORAGE

bool TerraLogger::beginLoggingToWiFiStorage(String logFilePrefix)
{
    TERRA_SOFT_ASSERT(hasLoggerData(), SFP(HStr_Err_NotYetInitialized));

    if (hasLoggerData() && !loggerData()->logToWiFiStorage) {
        String logFilename = getYYMMDDFilename(logFilePrefix, SFP(HStr_txt));
        #if TERRA_SYS_LEAVE_FILES_OPEN
            auto &logFile = _logFileWS ? *_logFileWS : *(_logFileWS = new WiFiStorageFile(WiFiStorage.open(logFilename.c_str())));
        #else
            auto logFile = WiFiStorage.open(logFilename.c_str());
        #endif

        if (logFile) {
            #if !TERRA_SYS_LEAVE_FILES_OPEN
                logFile.close();
            #endif

            strncpy(loggerData()->logFilePrefix, logFilePrefix.c_str(), 16);
            loggerData()->logToWiFiStorage = true;
            _logFilename = logFilename;
            Terraduino::_activeInstance->_systemData->bumpRevisionIfNeeded();

            return true;
        }
    }

    return false;
}

#endif

void TerraLogger::logSystemUptime()
{
    TimeSpan elapsed(getSystemUptime());
    if (elapsed.totalseconds()) {
        logMessage(SFP(HStr_Log_SystemUptime), timeSpanToString(elapsed));
    }
}

void TerraLogger::logMessage(const String &msg, const String &suffix1, const String &suffix2)
{
    if (!hasLoggerData() || (loggerData()->logLevel != Terra_LogLevel_None && loggerData()->logLevel <= Terra_LogLevel_All)) {
        log(TerraLogEvent(Terra_LogLevel_Info, SFP(HStr_Log_Prefix_Info), msg, suffix1, suffix2));
    }
}

void TerraLogger::logWarning(const String &warn, const String &suffix1, const String &suffix2)
{
    if (!hasLoggerData() || (loggerData()->logLevel != Terra_LogLevel_None && loggerData()->logLevel <= Terra_LogLevel_Warnings)) {
        log(TerraLogEvent(Terra_LogLevel_Warnings, SFP(HStr_Log_Prefix_Warning), warn, suffix1, suffix2));
    }
}

void TerraLogger::logError(const String &err, const String &suffix1, const String &suffix2)
{
    if (!hasLoggerData() || (loggerData()->logLevel != Terra_LogLevel_None && loggerData()->logLevel <= Terra_LogLevel_Errors)) {
        log(TerraLogEvent(Terra_LogLevel_Errors, SFP(HStr_Log_Prefix_Error), err, suffix1, suffix2));
    }
}

void TerraLogger::log(const TerraLogEvent &event)
{
    #ifdef TERRA_ENABLE_DEBUG_OUTPUT
        if (Serial) {
            Serial.print(event.timestamp);
            Serial.print(' ');
            Serial.print(event.prefix);
            Serial.print(event.msg);
            Serial.print(event.suffix1);
            Serial.println(event.suffix2);
        }
    #endif

    if (isLoggingToSDCard()) {
        auto sd = Terraduino::_activeInstance->getSDCard(TERRA_LOFS_BEGIN);

        if (sd) {
            #if TERRA_SYS_LEAVE_FILES_OPEN
                auto &logFile = _logFileSD ? *_logFileSD : *(_logFileSD = new File(sd->open(_logFilename.c_str(), FILE_WRITE)));
            #else
                createDirectoryFor(sd, _logFilename);
                auto logFile = sd->open(_logFilename.c_str(), FILE_WRITE);
            #endif

            if (logFile) {
                logFile.print(event.timestamp);
                logFile.print(' ');
                logFile.print(event.prefix);
                logFile.print(event.msg);
                logFile.print(event.suffix1);
                logFile.println(event.suffix2);

                #if !TERRA_SYS_LEAVE_FILES_OPEN
                    logFile.flush();
                    logFile.close();
                #endif
            }

            #if !TERRA_SYS_LEAVE_FILES_OPEN
                Terraduino::_activeInstance->endSDCard(sd);
            #endif
        }
    }

#ifdef TERRA_USE_WIFI_STORAGE

    if (isLoggingToWiFiStorage()) {
        #if TERRA_SYS_LEAVE_FILES_OPEN
            auto &logFile = _logFileWS ? *_logFileWS : *(_logFileWS = new WiFiStorageFile(WiFiStorage.open(_logFilename.c_str())));
        #else
            auto logFile = WiFiStorage.open(_logFilename.c_str());
        #endif

        if (logFile) {
            auto logFileStream = TerraWiFiStorageFileStream(logFile, logFile.size());

            logFileStream.print(event.timestamp);
            logFileStream.print(' ');
            logFileStream.print(event.prefix);
            logFileStream.print(event.msg);
            logFileStream.print(event.suffix1);
            logFileStream.println(event.suffix2);

            #if !TERRA_SYS_LEAVE_FILES_OPEN
                logFileStream.flush();
                logFile.close();
            #endif
        }
    }

#endif

    #ifdef TERRA_USE_MULTITASKING
        scheduleSignalFireOnce<const TerraLogEvent>(_logSignal, event);
    #else
        _logSignal.fire(event);
    #endif
}

void TerraLogger::flush()
{
    #ifdef TERRA_ENABLE_DEBUG_OUTPUT
        if (Serial) { Serial.flush(); }
    #endif
    #if TERRA_SYS_LEAVE_FILES_OPEN
        if(_logFileSD) { _logFileSD->flush(); }
    #endif
    yield();
}

void TerraLogger::setLogLevel(Terra_LogLevel logLevel)
{
    TERRA_SOFT_ASSERT(hasLoggerData(), SFP(HStr_Err_NotYetInitialized));
    if (hasLoggerData() && loggerData()->logLevel != logLevel) {
        loggerData()->logLevel = logLevel;
        Terraduino::_activeInstance->_systemData->bumpRevisionIfNeeded();
    }
}

Signal<const TerraLogEvent, TERRA_LOG_SIGNAL_SLOTS> &TerraLogger::getLogSignal()
{
    return _logSignal;
}

void TerraLogger::notifyDateChanged()
{
    if (isLoggingEnabled()) {
        _logFilename = getYYMMDDFilename(charsToString(loggerData()->logFilePrefix, 16), SFP(HStr_txt));
        cleanupOldestLogs();
    }
}

void TerraLogger::cleanupOldestLogs(bool force)
{
    // TODO: Old data cleanup. #17 in Hydruino.
}


TerraLoggerSubData::TerraLoggerSubData()
    : TerraSubData(0), logLevel(Terra_LogLevel_All), logFilePrefix{0}, logToSDCard(false), logToWiFiStorage(false)
{ ; }

void TerraLoggerSubData::toJSONObject(JsonObject &objectOut) const
{
    //TerraSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    if (logLevel != Terra_LogLevel_All) { objectOut[SFP(HStr_Key_LogLevel)] = logLevel; }
    if (logFilePrefix[0]) { objectOut[SFP(HStr_Key_LogFilePrefix)] = charsToString(logFilePrefix, 16); }
    if (logToSDCard != false) { objectOut[SFP(HStr_Key_LogToSDCard)] = logToSDCard; }
    if (logToWiFiStorage != false) { objectOut[SFP(HStr_Key_LogToWiFiStorage)] = logToWiFiStorage; }
}

void TerraLoggerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    //TerraSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    logLevel = objectIn[SFP(HStr_Key_LogLevel)] | logLevel;
    const char *logFilePrefixStr = objectIn[SFP(HStr_Key_LogFilePrefix)];
    if (logFilePrefixStr && logFilePrefixStr[0]) { strncpy(logFilePrefix, logFilePrefixStr, 16); }
    logToSDCard = objectIn[SFP(HStr_Key_LogToSDCard)] | logToSDCard;
    logToWiFiStorage = objectIn[SFP(HStr_Key_LogToWiFiStorage)] | logToWiFiStorage;
}
