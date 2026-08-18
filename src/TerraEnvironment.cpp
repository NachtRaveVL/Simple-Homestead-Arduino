/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Environment
*/

#include "TerraEnvironment.h"
#include "TerraCoreLogic.h"
#include "TerraUtils.h"

TerraWeatherSnapshot::TerraWeatherSnapshot()
    : airTemperatureC(0.0f), relativeHumidity(0.0f), barometricPressureHpa(0.0f), rainfallMm(0.0f),
      rainfallRateMmHr(0.0f), windSpeedMps(0.0f), windDirectionDeg(0.0f), solarRadiationWm2(0.0f),
      timestamp(0), validFields(0), valid(false) { }

TerraEnvironment::TerraEnvironment(uint32_t key, const TerraString &name)
    : TerraObject(Terra_ObjectType_Environment, key, name), _snapshot() { }

void TerraEnvironment::setSnapshot(const TerraWeatherSnapshot &snapshot) {
    _snapshot = snapshot;
    if (_snapshot.valid && !_snapshot.validFields) _snapshot.validFields = TERRA_WEATHER_ALL_FIELDS;
}

void TerraEnvironment::touch(uint32_t timestamp, uint16_t field) {
    _snapshot.timestamp = timestamp;
    _snapshot.validFields |= field;
    _snapshot.valid = _snapshot.validFields != 0;
}
void TerraEnvironment::setAirTemperature(float celsius, uint32_t timestamp) { _snapshot.airTemperatureC = celsius; touch(timestamp, TERRA_WEATHER_AIR_TEMPERATURE); }
void TerraEnvironment::setRelativeHumidity(float percent, uint32_t timestamp) { _snapshot.relativeHumidity = terraClamp(percent, 0.0f, 100.0f); touch(timestamp, TERRA_WEATHER_HUMIDITY); }
void TerraEnvironment::setRainfall(float millimeters, uint32_t timestamp) { _snapshot.rainfallMm = millimeters < 0.0f ? 0.0f : millimeters; touch(timestamp, TERRA_WEATHER_RAINFALL); }
void TerraEnvironment::setRainfallRate(float mmPerHour, uint32_t timestamp) { _snapshot.rainfallRateMmHr = mmPerHour < 0.0f ? 0.0f : mmPerHour; touch(timestamp, TERRA_WEATHER_RAIN_RATE); }
void TerraEnvironment::setBarometricPressure(float hPa, uint32_t timestamp) { _snapshot.barometricPressureHpa = hPa; touch(timestamp, TERRA_WEATHER_PRESSURE); }
void TerraEnvironment::setWind(float speedMps, float directionDeg, uint32_t timestamp) { _snapshot.windSpeedMps = speedMps < 0.0f ? 0.0f : speedMps; _snapshot.windDirectionDeg = fmodf(directionDeg, 360.0f); if (_snapshot.windDirectionDeg < 0.0f) _snapshot.windDirectionDeg += 360.0f; touch(timestamp, TERRA_WEATHER_WIND_SPEED | TERRA_WEATHER_WIND_DIRECTION); }
void TerraEnvironment::setSolarRadiation(float wattsPerSquareMeter, uint32_t timestamp) { _snapshot.solarRadiationWm2 = wattsPerSquareMeter < 0.0f ? 0.0f : wattsPerSquareMeter; touch(timestamp, TERRA_WEATHER_SOLAR_RADIATION); }
bool TerraEnvironment::isFreezing(float thresholdC) const { return hasField(TERRA_WEATHER_AIR_TEMPERATURE) && terraFreezeRisk(_snapshot.airTemperatureC, thresholdC); }

float TerraEnvironment::dewPointC() const {
    if (!hasField(TERRA_WEATHER_AIR_TEMPERATURE | TERRA_WEATHER_HUMIDITY) || _snapshot.relativeHumidity <= 0.0f) return NAN;
    const float a = 17.62f, b = 243.12f;
    float gamma = logf(_snapshot.relativeHumidity / 100.0f) + a * _snapshot.airTemperatureC / (b + _snapshot.airTemperatureC);
    return b * gamma / (a - gamma);
}
