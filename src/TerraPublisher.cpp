/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Publisher
*/

#include "Terraduino.h"
#include <stdio.h>
#include <string.h>

TerraPublisher::TerraPublisher()
    : _columns(), _columnCount(0), _publishedFrame(tframe_none), _data(nullptr), _publishSignal()
{ ; }

bool TerraPublisher::addColumn(tkey_t sensorKey)
{
    if (!sensorKey || _columnCount >= TERRA_MAX_PUBLISH_CHANNELS) { return false; }
    if (getColumnIndexStart(sensorKey) >= 0) { return true; }
    _columns[_columnCount++] = TerraDataColumn(sensorKey);
    return true;
}

bool TerraPublisher::publishData(tkey_t sensorKey, const TerraSingleMeasurement &measurement)
{
    tposi_t index = getColumnIndexStart(sensorKey);
    if (index < 0) { return false; }
    _columns[index].measurement = measurement;
    publishIfReady(measurement.frame, measurement.timestamp);
    return true;
}

bool TerraPublisher::publishData(tkey_t sensorKey, float value, Terra_Unit units,
                                 tframe_t frame, uint32_t timestamp)
{
    return publishData(sensorKey, TerraSingleMeasurement(value, units, timestamp, frame));
}

void TerraPublisher::advancePollingFrame(tframe_t frame, uint32_t timestamp)
{
    publishIfReady(frame, timestamp);
}

tposi_t TerraPublisher::getColumnIndexStart(tkey_t sensorKey) const
{
    for (uint8_t index = 0; index < _columnCount; ++index) {
        if (_columns[index].sensorKey == sensorKey) { return (tposi_t)index; }
    }
    return tposi_none;
}

void TerraPublisher::publishIfReady(tframe_t frame, uint32_t timestamp)
{
    (void)timestamp;
    if (!_columnCount || frame == tframe_none || frame == _publishedFrame) { return; }
    for (uint8_t index = 0; index < _columnCount; ++index) {
        if (_columns[index].measurement.frame != frame) { return; }
    }
    _publishedFrame = frame;
    _publishSignal.fire(Pair<uint8_t, const TerraDataColumn *>(_columnCount, _columns));
}

Signal<Pair<uint8_t, const TerraDataColumn *>, TERRA_DEFAULT_MAXSIZE> &TerraPublisher::getPublishSignal()
{
    return _publishSignal;
}

TerraPublisherSubData::TerraPublisherSubData()
    : TerraSubData(0), dataFilePrefix{0}, pubToSDCard(false),
      pubToWiFiStorage(false), pubToMQTT(false)
{
    snprintf(dataFilePrefix, sizeof(dataFilePrefix), "data/terra");
}

void TerraPublisherSubData::toJSONObject(JsonObject &objectOut) const
{
    TerraSubData::toJSONObject(objectOut);
    objectOut["dataFilePrefix"] = dataFilePrefix;
    objectOut["pubToSDCard"] = pubToSDCard;
    objectOut["pubToWiFiStorage"] = pubToWiFiStorage;
    objectOut["pubToMQTT"] = pubToMQTT;
}

void TerraPublisherSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraSubData::fromJSONObject(objectIn);
    const char *prefix = objectIn["dataFilePrefix"] | nullptr;
    if (prefix) {
        strncpy(dataFilePrefix, prefix, TERRA_PREFIX_MAXSIZE - 1);
        dataFilePrefix[TERRA_PREFIX_MAXSIZE - 1] = '\0';
    }
    pubToSDCard = objectIn["pubToSDCard"] | pubToSDCard;
    pubToWiFiStorage = objectIn["pubToWiFiStorage"] | pubToWiFiStorage;
    pubToMQTT = objectIn["pubToMQTT"] | pubToMQTT;
}
