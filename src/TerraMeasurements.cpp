/*  Terraduino: Simple automation controller for solar tracking systems.
    Copyright (C) 2023 NachtRaveVL          <nachtravevl@gmail.com>
    Terraduino Sensor Measurements
*/

#include "Terraduino.h"

TerraMeasurement *newMeasurementObjectFromSubData(const TerraMeasurementData *dataIn)
{
    if (!dataIn || !isValidType(dataIn->type)) return nullptr;
    TERRA_SOFT_ASSERT(dataIn && isValidType(dataIn->type), SFP(HStr_Err_InvalidParameter));

    if (dataIn) {
        switch (dataIn->type) {
            case (tid_t)TerraMeasurement::Binary:
                return new TerraBinaryMeasurement(dataIn);
            case (tid_t)TerraMeasurement::Single:
                return new TerraSingleMeasurement(dataIn);
            case (tid_t)TerraMeasurement::Double:
                return new TerraDoubleMeasurement(dataIn);
            case (tid_t)TerraMeasurement::Triple:
                return new TerraTripleMeasurement(dataIn);
            default: break;
        }
    }

    return nullptr;
}

float getMeasurementValue(const TerraMeasurement *measurement, uint8_t measurementRow, float binScale)
{
    if (measurement) {
        switch (measurement->type) {
            case TerraMeasurement::Binary:
                return ((TerraBinaryMeasurement *)measurement)->state ? binScale : 0.0f;
            case TerraMeasurement::Single:
                return ((TerraSingleMeasurement *)measurement)->value;
            case TerraMeasurement::Double:
                return ((TerraDoubleMeasurement *)measurement)->value[measurementRow];
            case TerraMeasurement::Triple:
                return ((TerraTripleMeasurement *)measurement)->value[measurementRow];
            default: break;
        }
    }
    return 0.0f;
}

Terra_UnitsType getMeasurementUnits(const TerraMeasurement *measurement, uint8_t measurementRow, Terra_UnitsType binUnits)
{
    if (measurement) {
        switch (measurement->type) {
            case TerraMeasurement::Binary:
                return binUnits;
            case TerraMeasurement::Single:
                return ((TerraSingleMeasurement *)measurement)->units;
            case TerraMeasurement::Double:
                return ((TerraDoubleMeasurement *)measurement)->units[measurementRow];
            case TerraMeasurement::Triple:
                return ((TerraTripleMeasurement *)measurement)->units[measurementRow];
            default: break;
        }
    }
    return Terra_UnitsType_Undefined;
}

uint8_t getMeasurementRowCount(const TerraMeasurement *measurement)
{
    return measurement ? max(1, (int)(measurement->type)) : 0;
}

TerraSingleMeasurement getAsSingleMeasurement(const TerraMeasurement *measurement, uint8_t measurementRow, float binScale, Terra_UnitsType binUnits)
{
    if (measurement) {
        switch (measurement->type) {
            case TerraMeasurement::Binary:
                return ((TerraBinaryMeasurement *)measurement)->getAsSingleMeasurement(binScale, binUnits);
            case TerraMeasurement::Single:
                return *((const TerraSingleMeasurement *)measurement);
            case TerraMeasurement::Double:
                return ((TerraDoubleMeasurement *)measurement)->getAsSingleMeasurement(measurementRow);
            case TerraMeasurement::Triple:
                return ((TerraTripleMeasurement *)measurement)->getAsSingleMeasurement(measurementRow);
            default: break;
        }
    }
    TerraSingleMeasurement retVal;
    retVal.frame = tframe_none; // meant to fail frame checks
    return retVal;
}


TerraMeasurement::TerraMeasurement(int classType, time_t timestampIn)
    : type((typeof(type))classType), timestamp(timestampIn)
{
    updateFrame();
}

TerraMeasurement::TerraMeasurement(const TerraMeasurementData *dataIn)
    : type((typeof(type))(dataIn->type)), timestamp(dataIn->timestamp)
{
    updateFrame(1);
}

void TerraMeasurement::saveToData(TerraMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    dataOut->type = (int8_t)type;
    dataOut->measurementRow = measurementRow;
    dataOut->timestamp = timestamp;
}

void TerraMeasurement::updateFrame(tframe_t minFrame)
{
    frame = max(minFrame, getController() ? getController()->getPollingFrame() : 0);
}


TerraBinaryMeasurement::TerraBinaryMeasurement()
    : TerraMeasurement(), state(false)
{ ; }

TerraBinaryMeasurement::TerraBinaryMeasurement(bool stateIn, time_t timestamp)
    : TerraMeasurement((int)Binary, timestamp), state(stateIn)
{ ; }

TerraBinaryMeasurement::TerraBinaryMeasurement(bool stateIn, time_t timestamp, tframe_t frame)
    : TerraMeasurement((int)Binary, timestamp, frame), state(stateIn)
{ ; }

TerraBinaryMeasurement::TerraBinaryMeasurement(const TerraMeasurementData *dataIn)
    : TerraMeasurement(dataIn),
      state(dataIn->measurementRow == 0 && dataIn->value >= 0.5f - FLT_EPSILON)
{ ; }

void TerraBinaryMeasurement::saveToData(TerraMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    TerraMeasurement::saveToData(dataOut, measurementRow, additionalDecPlaces);

    dataOut->value = measurementRow == 0 && state ? 1.0f : 0.0f;
    dataOut->units = measurementRow == 0 ? Terra_UnitsType_Raw_1 : Terra_UnitsType_Undefined;
}


TerraSingleMeasurement::TerraSingleMeasurement()
    : TerraMeasurement((int)Single), value(0.0f), units(Terra_UnitsType_Undefined)
{ ; }

TerraSingleMeasurement::TerraSingleMeasurement(float valueIn, Terra_UnitsType unitsIn, time_t timestamp)
    : TerraMeasurement((int)Single, timestamp), value(valueIn), units(unitsIn)
{ ; }

TerraSingleMeasurement::TerraSingleMeasurement(float valueIn, Terra_UnitsType unitsIn, time_t timestamp, tframe_t frame)
    : TerraMeasurement((int)Single, timestamp, frame), value(valueIn), units(unitsIn)
{ ; }

TerraSingleMeasurement::TerraSingleMeasurement(const TerraMeasurementData *dataIn)
    : TerraMeasurement(dataIn),
      value(dataIn->measurementRow == 0 ? dataIn->value : 0.0f),
      units(dataIn->measurementRow == 0 ? dataIn->units : Terra_UnitsType_Undefined)
{ ; }

void TerraSingleMeasurement::saveToData(TerraMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    TerraMeasurement::saveToData(dataOut, measurementRow, additionalDecPlaces);

    dataOut->value = measurementRow == 0 ? roundForExport(value, additionalDecPlaces) : 0.0f;
    dataOut->units = measurementRow == 0 ? units : Terra_UnitsType_Undefined;
}


TerraDoubleMeasurement::TerraDoubleMeasurement()
    : TerraMeasurement((int)Double), value{0}, units{Terra_UnitsType_Undefined,Terra_UnitsType_Undefined}
{ ; }

TerraDoubleMeasurement::TerraDoubleMeasurement(float value1, Terra_UnitsType units1,
                                               float value2, Terra_UnitsType units2,
                                               time_t timestamp)
    : TerraMeasurement((int)Double, timestamp), value{value1,value2}, units{units1,units2}
{ ; }

TerraDoubleMeasurement::TerraDoubleMeasurement(float value1, Terra_UnitsType units1,
                                               float value2, Terra_UnitsType units2,
                                               time_t timestamp, tframe_t frame)
    : TerraMeasurement((int)Double, timestamp, frame), value{value1,value2}, units{units1,units2}
{ ; }

TerraDoubleMeasurement::TerraDoubleMeasurement(const TerraMeasurementData *dataIn)
    : TerraMeasurement(dataIn),
      value{dataIn->measurementRow == 0 ? dataIn->value : 0.0f,
            dataIn->measurementRow == 1 ? dataIn->value : 0.0f
      },
      units{dataIn->measurementRow == 0 ? dataIn->units : Terra_UnitsType_Undefined,
            dataIn->measurementRow == 1 ? dataIn->units : Terra_UnitsType_Undefined
      }
{ ; }

void TerraDoubleMeasurement::saveToData(TerraMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    TerraMeasurement::saveToData(dataOut, measurementRow, additionalDecPlaces);

    dataOut->value = measurementRow >= 0 && measurementRow < 2 ? roundForExport(value[measurementRow], additionalDecPlaces) : 0.0f;
    dataOut->units = measurementRow >= 0 && measurementRow < 2 ? units[measurementRow] : Terra_UnitsType_Undefined;
}


TerraTripleMeasurement::TerraTripleMeasurement()
    : TerraMeasurement((int)Triple), value{0}, units{Terra_UnitsType_Undefined,Terra_UnitsType_Undefined,Terra_UnitsType_Undefined}
{ ; }

TerraTripleMeasurement::TerraTripleMeasurement(float value1, Terra_UnitsType units1,
                                               float value2, Terra_UnitsType units2,
                                               float value3, Terra_UnitsType units3,
                                               time_t timestamp)
    : TerraMeasurement((int)Triple, timestamp), value{value1,value2,value3}, units{units1,units2,units3}
{ ; }

TerraTripleMeasurement::TerraTripleMeasurement(float value1, Terra_UnitsType units1,
                                               float value2, Terra_UnitsType units2,
                                               float value3, Terra_UnitsType units3,
                                               time_t timestamp, tframe_t frame)
    : TerraMeasurement((int)Triple, timestamp, frame), value{value1,value2,value3}, units{units1,units2,units3}
{ ; }

TerraTripleMeasurement::TerraTripleMeasurement(const TerraMeasurementData *dataIn)
    : TerraMeasurement(dataIn),
      value{dataIn->measurementRow == 0 ? dataIn->value : 0.0f,
            dataIn->measurementRow == 1 ? dataIn->value : 0.0f,
            dataIn->measurementRow == 2 ? dataIn->value : 0.0f,
      },
      units{dataIn->measurementRow == 0 ? dataIn->units : Terra_UnitsType_Undefined,
            dataIn->measurementRow == 1 ? dataIn->units : Terra_UnitsType_Undefined,
            dataIn->measurementRow == 2 ? dataIn->units : Terra_UnitsType_Undefined,
      }
{ ; }

void TerraTripleMeasurement::saveToData(TerraMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    TerraMeasurement::saveToData(dataOut, measurementRow, additionalDecPlaces);

    dataOut->value = measurementRow >= 0 && measurementRow < 3 ? roundForExport(value[measurementRow], additionalDecPlaces) : 0.0f;
    dataOut->units = measurementRow >= 0 && measurementRow < 3 ? units[measurementRow] : Terra_UnitsType_Undefined;
}


TerraMeasurementData::TerraMeasurementData()
    : TerraSubData(), measurementRow(0), value(0.0f), units(Terra_UnitsType_Undefined), timestamp(0)
{
    type = 0; // no type differentiation
}

void TerraMeasurementData::toJSONObject(JsonObject &objectOut) const
{
    //TerraSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    objectOut[SFP(HStr_Key_MeasurementRow)] = measurementRow;
    objectOut[SFP(HStr_Key_Value)] = value;
    objectOut[SFP(HStr_Key_Units)] = unitsTypeToSymbol(units);
    objectOut[SFP(HStr_Key_Timestamp)] = timestamp;
}

void TerraMeasurementData::fromJSONObject(JsonObjectConst &objectIn)
{
    //TerraSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    measurementRow = objectIn[SFP(HStr_Key_MeasurementRow)] | measurementRow;
    value = objectIn[SFP(HStr_Key_Value)] | value;
    units = unitsTypeFromSymbol(objectIn[SFP(HStr_Key_Units)]);
    timestamp = objectIn[SFP(HStr_Key_Timestamp)] | timestamp;
}

void TerraMeasurementData::fromJSONVariant(JsonVariantConst &variantIn)
{
    if (variantIn.is<JsonObjectConst>()) {
        JsonObjectConst variantObj = variantIn;
        fromJSONObject(variantObj);
    } else if (variantIn.is<float>() || variantIn.is<int>()) {
        value = variantIn.as<float>();
    } else {
        TERRA_SOFT_ASSERT(false, SFP(HStr_Err_UnsupportedOperation));
    }
}
