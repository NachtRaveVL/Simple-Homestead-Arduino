/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Publisher
*/

#ifndef TerraPublisher_H
#define TerraPublisher_H

#include "TerraMeasurements.h"
#include "TerraData.h"

struct TerraPublisherSubData;

struct TerraDataColumn {
    tkey_t sensorKey;                                       // Sensor identity key
    TerraSingleMeasurement measurement;                     // Latest published measurement

    TerraDataColumn(tkey_t sensorKeyIn = tkey_none,
                    TerraSingleMeasurement measurementIn = TerraSingleMeasurement())
        : sensorKey(sensorKeyIn), measurement(measurementIn) { ; }
};

class TerraPublisher {
public:
    TerraPublisher();

    inline void setSubData(TerraPublisherSubData *data) { _data = data; }
    bool addColumn(tkey_t sensorKey);
    bool publishData(tkey_t sensorKey, const TerraSingleMeasurement &measurement);
    bool publishData(tkey_t sensorKey, float value, Terra_Unit units, tframe_t frame, uint32_t timestamp);
    void advancePollingFrame(tframe_t frame, uint32_t timestamp);
    tposi_t getColumnIndexStart(tkey_t sensorKey) const;

    inline uint8_t getColumnCount() const { return _columnCount; }
    inline const TerraDataColumn *getColumns() const { return _columns; }
    inline bool isPublishingEnabled() const { return _columnCount > 0; }

    Signal<Pair<uint8_t, const TerraDataColumn *>, TERRA_DEFAULT_MAXSIZE> &getPublishSignal();

protected:
    TerraDataColumn _columns[TERRA_MAX_PUBLISH_CHANNELS];    // Publisher data columns
    uint8_t _columnCount;                                   // Active column count
    tframe_t _publishedFrame;                               // Last published polling frame
    TerraPublisherSubData *_data;                           // Serialized publisher settings, not owned
    Signal<Pair<uint8_t, const TerraDataColumn *>, TERRA_DEFAULT_MAXSIZE> _publishSignal; // Publishing signal

    void publishIfReady(tframe_t frame, uint32_t timestamp);
};

struct TerraPublisherSubData : public TerraSubData {
    char dataFilePrefix[TERRA_PREFIX_MAXSIZE];               // Data file prefix
    bool pubToSDCard;                                       // SD publishing enabled
    bool pubToWiFiStorage;                                  // WiFi storage publishing enabled
    bool pubToMQTT;                                         // MQTT publishing enabled

    TerraPublisherSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef TerraPublisher_H
