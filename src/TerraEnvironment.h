/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Environment
*/

#ifndef TerraEnvironment_H
#define TerraEnvironment_H

#include "TerraObject.h"
#include "TerraAttachments.h"

// Environment
// Provides a single attachment point for each local weather measurement used by
// homestead monitoring and control logic. Measurement state remains owned by sensors.
class TerraEnvironment : public TerraObject {
public:
    TerraEnvironment(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString("Environment"));

    float getAirTemperature() const;
    float getRelativeHumidity() const;
    float getRainfall() const;
    float getRainfallRate() const;
    float getBarometricPressure() const;
    float getWindSpeed() const;
    float getWindDirection() const;
    float getSolarRadiation() const;

    // Environmental Sensor Attachment Points
    template<class T> inline void setAirTemperatureSensor(const SharedPtr<T> &sensor) { _airTemperature.setObject(sensor); }
    template<class T> inline void setHumiditySensor(const SharedPtr<T> &sensor) { _humidity.setObject(sensor); }
    template<class T> inline void setPressureSensor(const SharedPtr<T> &sensor) { _pressure.setObject(sensor); }
    template<class T> inline void setRainfallSensor(const SharedPtr<T> &sensor) { _rainfall.setObject(sensor); }
    template<class T> inline void setRainRateSensor(const SharedPtr<T> &sensor) { _rainRate.setObject(sensor); }
    template<class T> inline void setWindSpeedSensor(const SharedPtr<T> &sensor) { _windSpeed.setObject(sensor); }
    template<class T> inline void setWindDirectionSensor(const SharedPtr<T> &sensor) { _windDirection.setObject(sensor); }
    template<class T> inline void setSolarRadiationSensor(const SharedPtr<T> &sensor) { _solarRadiation.setObject(sensor); }

    inline TerraSensorAttachment &getAirTemperatureSensorAttachment() { return _airTemperature; }
    inline TerraSensorAttachment &getHumiditySensorAttachment() { return _humidity; }
    inline TerraSensorAttachment &getPressureSensorAttachment() { return _pressure; }
    inline TerraSensorAttachment &getRainfallSensorAttachment() { return _rainfall; }
    inline TerraSensorAttachment &getRainRateSensorAttachment() { return _rainRate; }
    inline TerraSensorAttachment &getWindSpeedSensorAttachment() { return _windSpeed; }
    inline TerraSensorAttachment &getWindDirectionSensorAttachment() { return _windDirection; }
    inline TerraSensorAttachment &getSolarRadiationSensorAttachment() { return _solarRadiation; }
    inline const TerraSensorAttachment &getAirTemperatureSensorAttachment() const { return _airTemperature; }
    inline const TerraSensorAttachment &getHumiditySensorAttachment() const { return _humidity; }
    inline const TerraSensorAttachment &getPressureSensorAttachment() const { return _pressure; }
    inline const TerraSensorAttachment &getRainfallSensorAttachment() const { return _rainfall; }
    inline const TerraSensorAttachment &getRainRateSensorAttachment() const { return _rainRate; }
    inline const TerraSensorAttachment &getWindSpeedSensorAttachment() const { return _windSpeed; }
    inline const TerraSensorAttachment &getWindDirectionSensorAttachment() const { return _windDirection; }
    inline const TerraSensorAttachment &getSolarRadiationSensorAttachment() const { return _solarRadiation; }

    bool isFreezing(float thresholdC = 0.0f) const;
    float dewPointC() const;

    virtual void update(uint32_t now = terraMillis()) override;
    virtual void unresolveAny(TerraObject *object) override;

protected:
    TerraSensorAttachment _airTemperature;                  // Air temperature sensor attachment point
    TerraSensorAttachment _humidity;                        // Humidity sensor attachment point
    TerraSensorAttachment _pressure;                        // Barometric pressure sensor attachment point
    TerraSensorAttachment _rainfall;                        // Accumulated rainfall sensor attachment point
    TerraSensorAttachment _rainRate;                        // Rainfall-rate sensor attachment point
    TerraSensorAttachment _windSpeed;                       // Wind-speed sensor attachment point
    TerraSensorAttachment _windDirection;                   // Wind-direction sensor attachment point
    TerraSensorAttachment _solarRadiation;                  // Solar-radiation sensor attachment point

    void initAttachmentKey(Terra_AttachmentRole role, uint32_t key);

    friend class TerraFactory;
};

#endif
