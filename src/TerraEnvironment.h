/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Environment
*/

#ifndef TerraEnvironment_H
#define TerraEnvironment_H

#include "TerraObject.h"

static const uint16_t TERRA_WEATHER_AIR_TEMPERATURE = 0x0001;
static const uint16_t TERRA_WEATHER_HUMIDITY        = 0x0002;
static const uint16_t TERRA_WEATHER_PRESSURE        = 0x0004;
static const uint16_t TERRA_WEATHER_RAINFALL        = 0x0008;
static const uint16_t TERRA_WEATHER_RAIN_RATE       = 0x0010;
static const uint16_t TERRA_WEATHER_WIND_SPEED      = 0x0020;
static const uint16_t TERRA_WEATHER_WIND_DIRECTION  = 0x0040;
static const uint16_t TERRA_WEATHER_SOLAR_RADIATION = 0x0080;
static const uint16_t TERRA_WEATHER_ALL_FIELDS      = 0x00ff;

// Weather Snapshot
// Stores the latest local environmental observations with per-field validity.
struct TerraWeatherSnapshot {
    float airTemperatureC;                                  // Air temperature, degrees Celsius
    float relativeHumidity;                                 // Relative humidity, percent
    float barometricPressureHpa;                            // Barometric pressure, hPa
    float rainfallMm;                                       // Accumulated rainfall, mm
    float rainfallRateMmHr;                                 // Rainfall rate, mm/hour
    float windSpeedMps;                                     // Wind speed, meters per second
    float windDirectionDeg;                                 // Wind direction, degrees
    float solarRadiationWm2;                                // Solar irradiance, W/m2
    uint32_t timestamp;                                     // Measurement timestamp
    uint16_t validFields;                                   // Valid weather-field bit mask
    bool valid;                                             // Measurement validity flag

    TerraWeatherSnapshot();
};

// Environment
// Maintains local weather conditions used by homestead monitoring and control logic.
class TerraEnvironment : public TerraObject {
public:
    TerraEnvironment(uint32_t key = TERRA_INVALID_KEY, const TerraString &name = TerraString("Environment"));

    void setSnapshot(const TerraWeatherSnapshot &snapshot);
    const TerraWeatherSnapshot &getSnapshot() const { return _snapshot; }

    void setAirTemperature(float celsius, uint32_t timestamp = terraMillis());
    void setRelativeHumidity(float percent, uint32_t timestamp = terraMillis());
    void setRainfall(float millimeters, uint32_t timestamp = terraMillis());
    void setRainfallRate(float mmPerHour, uint32_t timestamp = terraMillis());
    void setBarometricPressure(float hPa, uint32_t timestamp = terraMillis());
    void setWind(float speedMps, float directionDeg, uint32_t timestamp = terraMillis());
    void setSolarRadiation(float wattsPerSquareMeter, uint32_t timestamp = terraMillis());

    float getAirTemperature() const { return _snapshot.airTemperatureC; }
    float getRelativeHumidity() const { return _snapshot.relativeHumidity; }
    float getRainfall() const { return _snapshot.rainfallMm; }
    float getRainfallRate() const { return _snapshot.rainfallRateMmHr; }
    float getBarometricPressure() const { return _snapshot.barometricPressureHpa; }
    float getWindSpeed() const { return _snapshot.windSpeedMps; }
    float getWindDirection() const { return _snapshot.windDirectionDeg; }
    float getSolarRadiation() const { return _snapshot.solarRadiationWm2; }
    bool hasField(uint16_t field) const { return _snapshot.valid && (_snapshot.validFields & field) == field; }

    bool isFreezing(float thresholdC = 0.0f) const;
    float dewPointC() const;

protected:
    void touch(uint32_t timestamp, uint16_t field);
    TerraWeatherSnapshot _snapshot;                         // Snapshot
};

#endif
