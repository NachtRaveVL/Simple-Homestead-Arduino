/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Publisher
*/

#include "Terraduino.h"

TerraPublisher::TerraPublisher()
    : _dataFilename(), _needsTabulation(false), _pollingFrame(0), _dataColumns(nullptr), _columnSize(0)
#if TERRA_SYS_LEAVE_FILES_OPEN
      , _dataFileSD(nullptr)
#ifdef TERRA_USE_WIFI_STORAGE
      , _dataFileWS(nullptr)
#endif
#endif
#ifdef TERRA_USE_MQTT
    , _mqttClient(nullptr)
#endif
{ ; }

TerraPublisher::~TerraPublisher()
{
    if (_dataColumns) { delete [] _dataColumns; _dataColumns = nullptr; }
    #if TERRA_SYS_LEAVE_FILES_OPEN
        if (_dataFileSD) { _dataFileSD->flush(); _dataFileSD->close(); delete _dataFileSD; _dataFileSD = nullptr; }
        #ifdef TERRA_USE_WIFI_STORAGE
            if (_dataFileWS) { _dataFileWS->close(); delete _dataFileWS; _dataFileWS = nullptr; }
        #endif
    #endif
    #ifdef TERRA_USE_MQTT
        if (_mqttClient) {
            if (_mqttClient->connected()) { _mqttClient->disconnect(); }
            delete _mqttClient; _mqttClient = nullptr;
        }
    #endif
}

void TerraPublisher::update()
{
    if (hasPublisherData()) {
        if (_needsTabulation) { performTabulation(); }

        publishIfNeeded();
    }
}

bool TerraPublisher::beginPublishingToSDCard(String dataFilePrefix)
{
    TERRA_SOFT_ASSERT(hasPublisherData(), SFP(TStr_Err_NotYetInitialized));

    if (hasPublisherData() && !publisherData()->pubToSDCard) {
        auto sd = Terraduino::_activeInstance->getSDCard();

        if (sd) {
            String dataFilename = getYYMMDDFilename(dataFilePrefix, SFP(TStr_csv));
            createDirectoryFor(sd, dataFilename);
            #if TERRA_SYS_LEAVE_FILES_OPEN
                auto &dataFile = _dataFileSD ? *_dataFileSD : *(_dataFileSD = new File(sd->open(dataFilename.c_str(), FILE_WRITE)));
            #else
                auto dataFile = sd->open(dataFilename.c_str(), FILE_WRITE);
            #endif

            if (dataFile) {
                #if !TERRA_SYS_LEAVE_FILES_OPEN
                    dataFile.close();
                    Terraduino::_activeInstance->endSDCard(sd);
                #endif

                strncpy(publisherData()->dataFilePrefix, dataFilePrefix.c_str(), 16);
                publisherData()->pubToSDCard = true;
                _dataFilename = dataFilename;
                
                setNeedsTabulation();
                Terraduino::_activeInstance->_systemData->bumpRevisionIfNeeded();

                return true;
            }

            #if !TERRA_SYS_LEAVE_FILES_OPEN
                Terraduino::_activeInstance->endSDCard(sd);
            #endif
        }
    }

    return false;
}

#ifdef TERRA_USE_WIFI_STORAGE

bool TerraPublisher::beginPublishingToWiFiStorage(String dataFilePrefix)
{
    TERRA_SOFT_ASSERT(hasPublisherData(), SFP(TStr_Err_NotYetInitialized));

    if (hasPublisherData() && !publisherData()->pubToWiFiStorage) {
        String dataFilename = getYYMMDDFilename(dataFilePrefix, SFP(TStr_csv));
        #if TERRA_SYS_LEAVE_FILES_OPEN
            auto &dataFile = _dataFileWS ? *_dataFileWS : *(_dataFileWS = new WiFiStorageFile(WiFiStorage.open(dataFilename.c_str())));
        #else
            auto dataFile = WiFiStorage.open(dataFilename.c_str());
        #endif

        if (dataFile) {
            #if !TERRA_SYS_LEAVE_FILES_OPEN
                dataFile.close();
            #endif

            strncpy(publisherData()->dataFilePrefix, dataFilePrefix.c_str(), 16);
            publisherData()->pubToWiFiStorage = true;
            _dataFilename = dataFilename;

            setNeedsTabulation();
            Terraduino::_activeInstance->_systemData->bumpRevisionIfNeeded();

            return true;
        }
    }

    return false;
}

#endif
#ifdef TERRA_USE_MQTT

static uint32_t mqttNow()
{
    return unixNow();
}

bool TerraPublisher::beginPublishingToMQTTClient(MQTTClient &client)
{
    TERRA_SOFT_ASSERT(hasPublisherData(), SFP(TStr_Err_NotYetInitialized));

    if (hasPublisherData() && !_mqttClient) {
        _mqttClient = &client;
        _mqttClient->setClockSource(&mqttNow);
        if (!_mqttClient->connected()) {
            String unPw = String(F("public"));
            _mqttClient->connect(Terraduino::_activeInstance->getSystemName().c_str(),
                                 unPw.c_str(), unPw.c_str());
        }

        setNeedsTabulation();

        return true;
    }

    return false;
}

#endif

void TerraPublisher::publishData(tposi_t columnIndex, TerraSingleMeasurement measurement)
{
    TERRA_SOFT_ASSERT(hasPublisherData() && _dataColumns && _columnSize, SFP(TStr_Err_NotYetInitialized));
    if (_dataColumns && _columnSize && columnIndex >= 0 && columnIndex < _columnSize) {
        _dataColumns[columnIndex].measurement = measurement;
        publishIfNeeded();
    }
}

tposi_t TerraPublisher::getColumnIndexStart(tkey_t sensorKey)
{
    TERRA_SOFT_ASSERT(hasPublisherData() && _dataColumns && _columnSize, SFP(TStr_Err_NotYetInitialized));
    if (_dataColumns && _columnSize) {
        for (int columnIndex = 0; columnIndex < _columnSize; ++columnIndex) {
            if (_dataColumns[columnIndex].sensorKey == sensorKey) {
                return (tposi_t)columnIndex;
            }
        }
    }
    return (tposi_t)-1;
}

Signal<Pair<uint8_t, const TerraDataColumn *>, TERRA_PUBLISH_SIGNAL_SLOTS> &TerraPublisher::getPublishSignal()
{
    return _publishSignal;
}

void TerraPublisher::notifyDateChanged()
{
    if (isPublishingEnabled()) {
        _dataFilename = getYYMMDDFilename(charsToString(publisherData()->dataFilePrefix, 16), SFP(TStr_csv));
        cleanupOldestData();
    }
}

void TerraPublisher::advancePollingFrame()
{
    TERRA_HARD_ASSERT(hasPublisherData(), SFP(TStr_Err_NotYetInitialized));

    auto pollingFrame = Terraduino::_activeInstance->getPollingFrame();

    if (pollingFrame && _pollingFrame != pollingFrame) {
        time_t timestamp = unixNow();
        _pollingFrame = pollingFrame;

        if (Terraduino::_activeInstance->inOperationalMode()) {
            #ifdef TERRA_USE_MULTITASKING
                scheduleObjectMethodCallOnce<TerraPublisher>(this, &TerraPublisher::publish, timestamp);
            #else
                publish(timestamp);
            #endif
        }
    }

    if (++pollingFrame == 0) { pollingFrame = 1; } // use only valid frame #

    Terraduino::_activeInstance->_pollingFrame = pollingFrame;
}

void TerraPublisher::publishIfNeeded()
{
    if (_dataColumns && _columnSize && Terraduino::_activeInstance->isPollingFrameOld(_pollingFrame)) {
        bool allCurrent = true;

        for (int columnIndex = 0; columnIndex < _columnSize; ++columnIndex) {
            if (Terraduino::_activeInstance->isPollingFrameOld(_dataColumns[columnIndex].measurement.frame)) {
                allCurrent = false;
                break;
            }
        }

        if (allCurrent) {
            time_t timestamp = unixNow();
            _pollingFrame = Terraduino::_activeInstance->getPollingFrame();

            if (Terraduino::_activeInstance->inOperationalMode()) {
                #ifdef TERRA_USE_MULTITASKING
                    scheduleObjectMethodCallOnce<TerraPublisher>(this, &TerraPublisher::publish, timestamp);
                #else
                    publish(timestamp);
                #endif
            }
        }
    }
}

void TerraPublisher::publish(time_t timestamp)
{
    if (isPublishingToSDCard()) {
        auto sd = Terraduino::_activeInstance->getSDCard(TERRA_LOFS_BEGIN);

        if (sd) {
            #if TERRA_SYS_LEAVE_FILES_OPEN
                auto &dataFile = _dataFileSD ? *_dataFileSD : *(_dataFileSD = new File(sd->open(_dataFilename.c_str(), FILE_WRITE)));
            #else
                createDirectoryFor(sd, _dataFilename);
                auto dataFile = sd->open(_dataFilename.c_str(), FILE_WRITE);
            #endif

            if (dataFile) {
                dataFile.print(timestamp);

                for (int columnIndex = 0; columnIndex < _columnSize; ++columnIndex) {
                    dataFile.print(',');
                    dataFile.print(_dataColumns[columnIndex].measurement.value);
                }

                dataFile.println();

                #if !TERRA_SYS_LEAVE_FILES_OPEN
                    dataFile.flush();
                    dataFile.close();
                #endif
            }

            #if !TERRA_SYS_LEAVE_FILES_OPEN
                Terraduino::_activeInstance->endSDCard(sd);
            #endif
        }
    }

#ifdef TERRA_USE_WIFI_STORAGE

    if (isPublishingToWiFiStorage()) {
        #if TERRA_SYS_LEAVE_FILES_OPEN
            auto &dataFile = _dataFileWS ? *_dataFileWS : *(_dataFileWS = new WiFiStorageFile(WiFiStorage.open(_dataFilename.c_str())));
        #else
            auto dataFile = WiFiStorage.open(_dataFilename.c_str());
        #endif

        if (dataFile) {
            auto dataFileStream = TerraWiFiStorageFileStream(dataFile, dataFile.size());
            dataFileStream.print(timestamp);

            for (int columnIndex = 0; columnIndex < _columnSize; ++columnIndex) {
                dataFileStream.print(',');
                dataFileStream.print(_dataColumns[columnIndex].measurement.value);
            }

            dataFileStream.println();
            #if !TERRA_SYS_LEAVE_FILES_OPEN
                dataFile.close();
            #endif
        }
    }

#endif
#ifdef TERRA_USE_MQTT

    if (isPublishingToMQTTClient()) {
        String systemName = Terraduino::_activeInstance->getSystemName();
        for (int columnIndex = 0; columnIndex < _columnSize; ++columnIndex) {
            auto sensor = (TerraSensor *)(Terraduino::_activeInstance->_objects[_dataColumns[columnIndex].sensorKey].get());
            if (sensor) {
                String topic; topic.reserve(systemName.length() + 1 + sensor->getKeyString().length() + 1);
                topic.concat(systemName);
                topic.concat('/');
                topic.concat(sensor->getKeyString());
                String payload = String(_dataColumns[columnIndex].measurement.value, 6); // skipping units/rounding/etc to allow MQTT broker full value data
                _mqttClient->publish(topic.c_str(), payload.c_str());
            }
        }
    }

#endif

    #ifdef TERRA_USE_MULTITASKING
        scheduleSignalFireOnce<Pair<uint8_t, const TerraDataColumn *>>(_publishSignal, make_pair(_columnSize, (const TerraDataColumn *)_dataColumns));
    #else
        _publishSignal.fire(make_pair(_columnSize, (const TerraDataColumn *)_dataColumns));
    #endif
}

void TerraPublisher::performTabulation()
{
    TERRA_SOFT_ASSERT(hasPublisherData(), SFP(TStr_Err_NotYetInitialized));

    bool sameOrder = _dataColumns && _columnSize ? true : false;
    int columnSize = 0;

    for (auto iter = Terraduino::_activeInstance->_objects.begin(); iter != Terraduino::_activeInstance->_objects.end(); ++iter) {
        if (iter->second->isSensorType()) {
            auto sensor = static_pointer_cast<TerraSensor>(iter->second);
            auto rowCount = getMeasurementRowCount(sensor->getMeasurement());

            for (int rowIndex = 0; sameOrder && rowIndex < rowCount; ++rowIndex) {
                sameOrder = sameOrder && (columnSize + rowIndex + 1 <= _columnSize) &&
                            (_dataColumns[columnSize + rowIndex].sensorKey == sensor->getKey());
            }

            columnSize += rowCount;
        }
    }
    sameOrder = sameOrder && (columnSize == _columnSize);

    if (!sameOrder) {
        if (_dataColumns && _columnSize != columnSize) { delete [] _dataColumns; _dataColumns = nullptr; }
        _columnSize = columnSize;

        if (_columnSize) {
            if (!_dataColumns) {
                _dataColumns = new TerraDataColumn[_columnSize];
                TERRA_SOFT_ASSERT(_dataColumns, SFP(TStr_Err_AllocationFailure));
            }
            if (_dataColumns) {
                int columnIndex = 0;

                for (auto iter = Terraduino::_activeInstance->_objects.begin(); iter != Terraduino::_activeInstance->_objects.end(); ++iter) {
                    if (iter->second->isSensorType()) {
                        auto sensor = static_pointer_cast<TerraSensor>(iter->second);
                        auto measurement = sensor->getMeasurement();
                        auto rowCount = getMeasurementRowCount(measurement);

                        for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
                            TERRA_HARD_ASSERT(columnIndex < _columnSize, SFP(TStr_Err_OperationFailure));
                            _dataColumns[columnIndex].measurement = getAsSingleMeasurement(measurement, rowIndex);
                            _dataColumns[columnIndex].sensorKey = sensor->getKey();
                            columnIndex++;
                        }
                    }
                }
            }
        }

        resetDataFile();
    }

    _needsTabulation = false;
}

void TerraPublisher::resetDataFile()
{
    if (isPublishingToSDCard()) {
        auto sd = Terraduino::_activeInstance->getSDCard(TERRA_LOFS_BEGIN);

        if (sd) {
            #if TERRA_SYS_LEAVE_FILES_OPEN
                if (_dataFileSD) { _dataFileSD->flush(); _dataFileSD->close(); delete _dataFileSD; _dataFileSD = nullptr; }
            #endif
            if (sd->exists(_dataFilename.c_str())) {
                sd->remove(_dataFilename.c_str());
            }
            #if TERRA_SYS_LEAVE_FILES_OPEN
                auto &dataFile = _dataFileSD ? *_dataFileSD : *(_dataFileSD = new File(sd->open(_dataFilename.c_str(), FILE_WRITE)));
            #else
                createDirectoryFor(sd, _dataFilename);
                auto dataFile = sd->open(_dataFilename.c_str(), FILE_WRITE);
            #endif

            if (dataFile) {
                TerraSensor *lastSensor = nullptr;
                uint8_t measurementRow = 0;

                dataFile.print(SFP(TStr_Key_Timestamp));

                for (int columnIndex = 0; columnIndex < _columnSize; ++columnIndex) {
                    dataFile.print(',');

                    auto sensor = (TerraSensor *)(Terraduino::_activeInstance->_objects[_dataColumns[columnIndex].sensorKey].get());
                    if (sensor && sensor == lastSensor) { ++measurementRow; }
                    else { measurementRow = 0; lastSensor = sensor; }

                    if (sensor) {
                        dataFile.print(sensor->getKeyString());
                        dataFile.print('_');
                        dataFile.print(unitsCategoryToString(defaultCategoryForSensor(sensor->getSensorType(), measurementRow)));
                        dataFile.print('_');
                        dataFile.print(unitsTypeToSymbol(getMeasurementUnits(sensor->getMeasurement(), measurementRow)));
                    } else {
                        TERRA_SOFT_ASSERT(false, SFP(TStr_Err_OperationFailure));
                        dataFile.print(SFP(TStr_Undefined));
                    }
                }

                dataFile.println();

                #if !TERRA_SYS_LEAVE_FILES_OPEN
                    dataFile.flush();
                    dataFile.close();
                #endif
            }

            #if !TERRA_SYS_LEAVE_FILES_OPEN
                Terraduino::_activeInstance->endSDCard(sd);
            #endif
        }
    }

#ifdef TERRA_USE_WIFI_STORAGE

    if (isPublishingToWiFiStorage()) {
        #if TERRA_SYS_LEAVE_FILES_OPEN
            if (_dataFileWS) { _dataFileWS->close(); delete _dataFileWS; _dataFileWS = nullptr; }
        #endif
        if (WiFiStorage.exists(_dataFilename.c_str())) {
            WiFiStorage.remove(_dataFilename.c_str());
        }
        #if TERRA_SYS_LEAVE_FILES_OPEN
            auto &dataFile = _dataFileWS ? *_dataFileWS : *(_dataFileWS = new WiFiStorageFile(WiFiStorage.open(_dataFilename.c_str())));
        #else
            auto dataFile = WiFiStorage.open(_dataFilename.c_str());
        #endif

        if (dataFile) {
            auto dataFileStream = TerraWiFiStorageFileStream(dataFile);
            TerraSensor *lastSensor = nullptr;
            uint8_t measurementRow = 0;

            dataFileStream.print(SFP(TStr_Key_Timestamp));

            for (int columnIndex = 0; columnIndex < _columnSize; ++columnIndex) {
                dataFileStream.print(',');

                auto sensor = (TerraSensor *)(Terraduino::_activeInstance->_objects[_dataColumns[columnIndex].sensorKey].get());
                if (sensor && sensor == lastSensor) { ++measurementRow; }
                else { measurementRow = 0; lastSensor = sensor; }

                if (sensor) {
                    dataFileStream.print(sensor->getKeyString());
                    dataFileStream.print('_');
                    dataFileStream.print(unitsCategoryToString(defaultCategoryForSensor(sensor->getSensorType(), measurementRow)));
                    dataFileStream.print('_');
                    dataFileStream.print(unitsTypeToSymbol(getMeasurementUnits(sensor->getMeasurement(), measurementRow)));
                } else {
                    TERRA_SOFT_ASSERT(false, SFP(TStr_Err_OperationFailure));
                    dataFileStream.print(SFP(TStr_Undefined));
                }
            }

            dataFileStream.println();
        }
    }

#endif
}

void TerraPublisher::cleanupOldestData(bool force)
{
    // TODO: Old data cleanup. #17 in Terraduino.
}


TerraPublisherSubData::TerraPublisherSubData()
    : TerraSubData(0), dataFilePrefix{0}, pubToSDCard(false), pubToWiFiStorage(false)
{ ; }

void TerraPublisherSubData::toJSONObject(JsonObject &objectOut) const
{
    //TerraSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    if (dataFilePrefix[0]) { objectOut[SFP(TStr_Key_DataFilePrefix)] = charsToString(dataFilePrefix, 16); }
    if (pubToSDCard != false) { objectOut[SFP(TStr_Key_PublishToSDCard)] = pubToSDCard; }
    if (pubToWiFiStorage != false) { objectOut[SFP(TStr_Key_PublishToWiFiStorage)] = pubToWiFiStorage; }
}

void TerraPublisherSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    //TerraSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    const char *dataFilePrefixStr = objectIn[SFP(TStr_Key_DataFilePrefix)];
    if (dataFilePrefixStr && dataFilePrefixStr[0]) { strncpy(dataFilePrefix, dataFilePrefixStr, 16); }
    pubToSDCard = objectIn[SFP(TStr_Key_PublishToSDCard)] | pubToSDCard;
    pubToWiFiStorage = objectIn[SFP(TStr_Key_PublishToWiFiStorage)] | pubToWiFiStorage;
}
