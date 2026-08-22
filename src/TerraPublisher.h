/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Publisher
*/

#ifndef TerraPublisher_H
#define TerraPublisher_H

class TerraPublisher;
struct TerraPublisherSubData;
struct TerraDataColumn;

#include "Astruino.h"
#include "TerraMeasurements.h"

// Data Publisher
// The Publisher allows for data collection and publishing capabilities. The data output
// is based on a simple table of time and measured value. Each time segment, called a
// polling frame (and controlled by the polling rate interval), collects data from all
// sensors into a data row, with the appropriate total number of columns. At time of
// either all sensors having reported in for their frame #, or the frame # proceeding
// to advance (in which case the existing value is recycled), the table's row is
// submitted to configured publishing services.
// Publishing to SD card .csv data files (via SPI card reader) is supported as is logging to
// WiFiStorage .csv data files (via OS/OTA filesystem / WiFiNINA_Generic only). MQTT is also
// supported but requires additional setup.
class TerraPublisher {
public:
    TerraPublisher();
    ~TerraPublisher();

    void update();
 
    bool beginPublishingToSDCard(String dataFilePrefix);
    inline bool isPublishingToSDCard() const;

#ifdef TERRA_USE_WIFI_STORAGE
    bool beginPublishingToWiFiStorage(String dataFilePrefix);
    inline bool isPublishingToWiFiStorage() const;
#endif

#ifdef TERRA_USE_MQTT
    bool beginPublishingToMQTTClient(MQTTClient &client);
    inline bool isPublishingToMQTTClient() const;
#endif

    void publishData(tposi_t columnIndex, TerraSingleMeasurement measurement);

    inline void setNeedsTabulation();
    inline bool needsTabulation() { return _needsTabulation; }

    inline bool isPublishingEnabled() const;
    tposi_t getColumnIndexStart(tkey_t sensorKey);

    Signal<Pair<uint8_t, const TerraDataColumn *>, TERRA_PUBLISH_SIGNAL_SLOTS> &getPublishSignal();

    void notifyDateChanged();

protected:
#if TERRA_SYS_LEAVE_FILES_OPEN
    File *_dataFileSD;                                      // SD card log file instance (owned)
#ifdef TERRA_USE_WIFI_STORAGE
    WiFiStorageFile *_dataFileWS;                           // WiFiStorageFile log file instance (owned)
#endif
#endif
#ifdef TERRA_USE_MQTT
    MQTTClient *_mqttClient;                                // MQTT client object (strong)
#endif
    String _dataFilename;                                   // Resolved data file name (based on day)
    tframe_t _pollingFrame;                                 // Polling frame that publishing is caught up to
    bool _needsTabulation;                                  // Needs tabulation tracking flag
    uint8_t _columnSize;                                    // Number of data columns
    TerraDataColumn *_dataColumns;                          // Data columns array (owned)

    Signal<Pair<uint8_t, const TerraDataColumn *>, TERRA_PUBLISH_SIGNAL_SLOTS> _publishSignal; // Data publishing signal

    friend class Terraduino;

    void advancePollingFrame();
    friend void dataLoop();

    void publishIfNeeded();
    void publish(time_t timestamp);

    void performTabulation();

public: // consider protected
    inline TerraPublisherSubData *publisherData() const;
    inline bool hasPublisherData() const;

    void resetDataFile();
    void cleanupOldestData(bool force = false);
};

// Publisher Data Column
// Data column worth of storage. Intended to be array allocated.
struct TerraDataColumn {
    tkey_t sensorKey;                                       // Key to sensor object
    TerraSingleMeasurement measurement;                     // Storage polling frame measurement
};


// Publisher Serialization Sub Data
// A part of TSYS system data.
struct TerraPublisherSubData : public TerraSubData {
    char dataFilePrefix[TERRA_PREFIX_MAXSIZE];              // Base data file name prefix / folder (default: "data/he")
    bool pubToSDCard;                                       // If publishing sensor data to SD card is enabled (default: false)
    bool pubToWiFiStorage;                                  // If publishing sensor data to WiFiStorage is enabled (default: false)

    TerraPublisherSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef TerraPublisher_H
