/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Environment
*/

#include "Terraduino.h"
#include <string.h>
#include "TerraUtils.h"

TerraEnvironment::TerraEnvironment(tposi_t environmentIndex, const TerraString &name)
    : TerraObject(TerraIdentity(Terra_ObjectType_Environment, environmentIndex), name), classType(Standard),
      _airTemperature(this), _humidity(this), _pressure(this), _rainfall(this), _rainRate(this),
      _windSpeed(this), _windDirection(this), _solarRadiation(this)
{ ; }

TerraEnvironment::TerraEnvironment(const TerraEnvironmentData *dataIn)
    : TerraObject(dataIn), classType(static_cast<decltype(Standard)>(dataIn ? (int)dataIn->id.object.classType : (int)Unknown)),
      _airTemperature(this), _humidity(this), _pressure(this),
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

static float terraEnvironmentValue(const TerraSensorAttachment &attachment, Terra_UnitsType units)
{
    TerraSingleMeasurement measurement = attachment.getCachedMeasurement();
    if (!measurement.isSet()) { return NAN; }
    if (measurement.units != units) { measurement.toUnits(units); }
    return measurement.units == units ? measurement.value : NAN;
}

float TerraEnvironment::getAirTemperature() const
{
    return terraEnvironmentValue(_airTemperature, Terra_UnitsType_Temperature_Celsius);
}

float TerraEnvironment::getRelativeHumidity() const
{
    return terraEnvironmentValue(_humidity, Terra_UnitsType_Percentile_100);
}

float TerraEnvironment::getRainfall() const
{
    return terraEnvironmentValue(_rainfall, Terra_UnitsType_Distance_Millimeters);
}

float TerraEnvironment::getRainfallRate() const
{
    return terraEnvironmentValue(_rainRate, Terra_UnitsType_Speed_MillimetersPerHour);
}

float TerraEnvironment::getBarometricPressure() const
{
    return terraEnvironmentValue(_pressure, Terra_UnitsType_Pressure_Hectopascals);
}

float TerraEnvironment::getWindSpeed() const
{
    return terraEnvironmentValue(_windSpeed, Terra_UnitsType_Speed_MetersPerSecond);
}

float TerraEnvironment::getWindDirection() const
{
    float direction = terraEnvironmentValue(_windDirection, Terra_UnitsType_Angle_Degrees_360);
    if (isnan(direction)) { return direction; }
    direction = fmodf(direction, 360.0f);
    return direction < 0.0f ? direction + 360.0f : direction;
}

float TerraEnvironment::getSolarRadiation() const
{
    return terraEnvironmentValue(_solarRadiation, Terra_UnitsType_Irradiance_WattsPerSquareMeter);
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
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void TerraEnvironment::saveToData(TerraData *dataOut) const
{
    TerraObject::saveToData(dataOut);
    dataOut->id.object.classType = (tid_t)classType;
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


TerraEnvironmentData::TerraEnvironmentData()
    : TerraObjectData(), airTemperatureSensor{0}, humiditySensor{0}, pressureSensor{0}, rainfallSensor{0},
      rainRateSensor{0}, windSpeedSensor{0}, windDirectionSensor{0}, solarRadiationSensor{0}
{
    _size = sizeof(*this);
    id.object.idType = (tid_t)Terra_ObjectType_Environment;
    id.object.objType = 0;
    id.object.posIndex = TERRA_POS_SEARCH_FROMBEG;
    id.object.classType = (tid_t)TerraEnvironment::Standard;
}

void TerraEnvironmentData::toJSONObject(JsonObject &objectOut) const
{
    TerraObjectData::toJSONObject(objectOut);
    if (airTemperatureSensor[0]) { objectOut[SFP(TStr_Key_AirTemperatureSensor)] = airTemperatureSensor; }
    if (humiditySensor[0]) { objectOut[SFP(TStr_Key_HumiditySensor)] = humiditySensor; }
    if (pressureSensor[0]) { objectOut[SFP(TStr_Key_PressureSensor)] = pressureSensor; }
    if (rainfallSensor[0]) { objectOut[SFP(TStr_Key_RainfallSensor)] = rainfallSensor; }
    if (rainRateSensor[0]) { objectOut[SFP(TStr_Key_RainRateSensor)] = rainRateSensor; }
    if (windSpeedSensor[0]) { objectOut[SFP(TStr_Key_WindSpeedSensor)] = windSpeedSensor; }
    if (windDirectionSensor[0]) { objectOut[SFP(TStr_Key_WindDirectionSensor)] = windDirectionSensor; }
    if (solarRadiationSensor[0]) { objectOut[SFP(TStr_Key_SolarRadiationSensor)] = solarRadiationSensor; }
}

void TerraEnvironmentData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraObjectData::fromJSONObject(objectIn);
    auto copyString = [](char *destinationOut, JsonVariantConst sourceIn) {
        const char *value = sourceIn | nullptr;
        if (value) {
            strncpy(destinationOut, value, TERRA_NAME_MAXSIZE - 1);
            destinationOut[TERRA_NAME_MAXSIZE - 1] = '\0';
        }
    };
    copyString(airTemperatureSensor, objectIn[SFP(TStr_Key_AirTemperatureSensor)]);
    copyString(humiditySensor, objectIn[SFP(TStr_Key_HumiditySensor)]);
    copyString(pressureSensor, objectIn[SFP(TStr_Key_PressureSensor)]);
    copyString(rainfallSensor, objectIn[SFP(TStr_Key_RainfallSensor)]);
    copyString(rainRateSensor, objectIn[SFP(TStr_Key_RainRateSensor)]);
    copyString(windSpeedSensor, objectIn[SFP(TStr_Key_WindSpeedSensor)]);
    copyString(windDirectionSensor, objectIn[SFP(TStr_Key_WindDirectionSensor)]);
    copyString(solarRadiationSensor, objectIn[SFP(TStr_Key_SolarRadiationSensor)]);
}
