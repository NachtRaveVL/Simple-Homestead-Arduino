/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Environment
*/

#include "Terraduino.h"
#include <string.h>
#include "TerraUtils.h"

TerraEnvironment::TerraEnvironment(tposi_t environmentIndex, const TerraString &name)
    : TerraObject(TerraIdentity(Terra_ObjectType_Environment, environmentIndex), name),
      _airTemperature(this), _humidity(this), _pressure(this), _rainfall(this), _rainRate(this),
      _windSpeed(this), _windDirection(this), _solarRadiation(this)
{ ; }

TerraEnvironment::TerraEnvironment(const TerraEnvironmentData *dataIn)
    : TerraObject(dataIn), _airTemperature(this), _humidity(this), _pressure(this),
      _rainfall(this), _rainRate(this), _windSpeed(this), _windDirection(this), _solarRadiation(this)
{
    if (dataIn) {
        if (dataIn->airTemperatureSensor[0]) { _airTemperature.initObject(dataIn->airTemperatureSensor); }
        if (dataIn->humiditySensor[0]) { _humidity.initObject(dataIn->humiditySensor); }
        if (dataIn->pressureSensor[0]) { _pressure.initObject(dataIn->pressureSensor); }
        if (dataIn->rainfallSensor[0]) { _rainfall.initObject(dataIn->rainfallSensor); }
        if (dataIn->rainRateSensor[0]) { _rainRate.initObject(dataIn->rainRateSensor); }
        if (dataIn->windSpeedSensor[0]) { _windSpeed.initObject(dataIn->windSpeedSensor); }
        if (dataIn->windDirectionSensor[0]) { _windDirection.initObject(dataIn->windDirectionSensor); }
        if (dataIn->solarRadiationSensor[0]) { _solarRadiation.initObject(dataIn->solarRadiationSensor); }
    }
}

static float terraEnvironmentValue(const TerraSensorAttachment &attachment, Terra_Unit units)
{
    TerraSingleMeasurement measurement = attachment.getCachedMeasurement();
    if (!measurement.isSet()) { return NAN; }
    if (measurement.units != units) { measurement.toUnits(units); }
    return measurement.units == units ? measurement.value : NAN;
}

float TerraEnvironment::getAirTemperature() const
{
    return terraEnvironmentValue(_airTemperature, Terra_Unit_Celsius);
}

float TerraEnvironment::getRelativeHumidity() const
{
    return terraEnvironmentValue(_humidity, Terra_Unit_Percent);
}

float TerraEnvironment::getRainfall() const
{
    return terraEnvironmentValue(_rainfall, Terra_Unit_Millimeters);
}

float TerraEnvironment::getRainfallRate() const
{
    return terraEnvironmentValue(_rainRate, Terra_Unit_MillimetersPerHour);
}

float TerraEnvironment::getBarometricPressure() const
{
    return terraEnvironmentValue(_pressure, Terra_Unit_Hectopascals);
}

float TerraEnvironment::getWindSpeed() const
{
    return terraEnvironmentValue(_windSpeed, Terra_Unit_MetersPerSecond);
}

float TerraEnvironment::getWindDirection() const
{
    float direction = terraEnvironmentValue(_windDirection, Terra_Unit_Degrees);
    if (isnan(direction)) { return direction; }
    direction = fmodf(direction, 360.0f);
    return direction < 0.0f ? direction + 360.0f : direction;
}

float TerraEnvironment::getSolarRadiation() const
{
    return terraEnvironmentValue(_solarRadiation, Terra_Unit_WattsPerSquareMeter);
}

bool TerraEnvironment::isFreezing(float thresholdC) const
{
    float temperature = getAirTemperature();
    return !isnan(temperature) && temperature <= thresholdC;
}

float TerraEnvironment::dewPointC() const
{
    float temperature = getAirTemperature();
    float humidity = getRelativeHumidity();
    if (isnan(temperature) || isnan(humidity) || humidity <= 0.0f) { return NAN; }

    const float a = 17.62f;
    const float b = 243.12f;
    float gamma = logf(humidity / 100.0f) + a * temperature / (b + temperature);
    return b * gamma / (a - gamma);
}

void TerraEnvironment::update(uint32_t now)
{
    TerraObject::update(now);
    if (_airTemperature.isSet()) { _airTemperature.getMeasurement(now, true); }
    if (_humidity.isSet()) { _humidity.getMeasurement(now, true); }
    if (_pressure.isSet()) { _pressure.getMeasurement(now, true); }
    if (_rainfall.isSet()) { _rainfall.getMeasurement(now, true); }
    if (_rainRate.isSet()) { _rainRate.getMeasurement(now, true); }
    if (_windSpeed.isSet()) { _windSpeed.getMeasurement(now, true); }
    if (_windDirection.isSet()) { _windDirection.getMeasurement(now, true); }
    if (_solarRadiation.isSet()) { _solarRadiation.getMeasurement(now, true); }
}

void TerraEnvironment::unresolveAny(TerraObject *object)
{
    _airTemperature.unresolveAny(object);
    _humidity.unresolveAny(object);
    _pressure.unresolveAny(object);
    _rainfall.unresolveAny(object);
    _rainRate.unresolveAny(object);
    _windSpeed.unresolveAny(object);
    _windDirection.unresolveAny(object);
    _solarRadiation.unresolveAny(object);
    TerraObject::unresolveAny(object);
}

TerraData *TerraEnvironment::allocateData() const
{
    return new TerraEnvironmentData();
}

void TerraEnvironment::saveToData(TerraData *dataOut) const
{
    TerraObject::saveToData(dataOut);
    auto data = static_cast<TerraEnvironmentData *>(dataOut);
    auto copyAttachment = [](char *destination, const TerraAttachment &attachment) {
        if (attachment.isSet()) {
            strncpy(destination, attachment.getKeyString().c_str(), TERRA_NAME_MAXSIZE - 1);
            destination[TERRA_NAME_MAXSIZE - 1] = '\0';
        }
    };
    copyAttachment(data->airTemperatureSensor, _airTemperature);
    copyAttachment(data->humiditySensor, _humidity);
    copyAttachment(data->pressureSensor, _pressure);
    copyAttachment(data->rainfallSensor, _rainfall);
    copyAttachment(data->rainRateSensor, _rainRate);
    copyAttachment(data->windSpeedSensor, _windSpeed);
    copyAttachment(data->windDirectionSensor, _windDirection);
    copyAttachment(data->solarRadiationSensor, _solarRadiation);
}
