/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Environment
*/

#include "Terraduino.h"
#include "TerraUtils.h"

TerraEnvironment::TerraEnvironment(uint32_t key, const TerraString &name)
    : TerraObject(Terra_ObjectType_Environment, key, name),
      _airTemperature(this), _humidity(this), _pressure(this), _rainfall(this), _rainRate(this),
      _windSpeed(this), _windDirection(this), _solarRadiation(this)
{ ; }

static float terraEnvironmentValue(const TerraSensorAttachment &attachment, Terra_Unit units)
{
    TerraMeasurement measurement = attachment.getCachedMeasurement();
    if (!measurement.valid) { return NAN; }
    if (measurement.unit != units) { measurement = terraConvertMeasurement(measurement, units); }
    return measurement.valid ? measurement.value : NAN;
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

void TerraEnvironment::initAttachmentKey(Terra_AttachmentRole role, uint32_t key)
{
    switch (role) {
        case Terra_AttachmentRole_TemperatureSensor: _airTemperature.initObject(key); break;
        case Terra_AttachmentRole_HumiditySensor: _humidity.initObject(key); break;
        case Terra_AttachmentRole_PressureSensor: _pressure.initObject(key); break;
        case Terra_AttachmentRole_RainfallSensor: _rainfall.initObject(key); break;
        case Terra_AttachmentRole_RainRateSensor: _rainRate.initObject(key); break;
        case Terra_AttachmentRole_WindSpeedSensor: _windSpeed.initObject(key); break;
        case Terra_AttachmentRole_WindDirectionSensor: _windDirection.initObject(key); break;
        case Terra_AttachmentRole_SolarRadiationSensor: _solarRadiation.initObject(key); break;
        default: break;
    }
}
