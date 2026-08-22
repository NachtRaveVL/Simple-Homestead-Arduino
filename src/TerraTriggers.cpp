/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Triggers
*/

#include "Terraduino.h"
#include "TerraUtils.h"
#include <string.h>

TerraTrigger *newTriggerObjectFromSubData(const TerraTriggerSubData *dataIn)
{
    if (!dataIn) { return nullptr; }
    switch (dataIn->type) {
        case TerraTrigger::MeasureValue: return new TerraMeasurementValueTrigger(dataIn);
        case TerraTrigger::MeasureRange: return new TerraMeasurementRangeTrigger(dataIn);
        default: return nullptr;
    }
}

TerraTrigger::TerraTrigger(TerraIdentity sensorId, uint8_t measurementRow,
                           float detriggerTol, millis_t detriggerDelay, int typeIn)
    : TerraSubObject(), TerraMeasurementUnitsInterfaceStorageSingle(Terra_Unit_Undefined),
      type(static_cast<decltype(MeasureValue)>(typeIn)), _sensor(this, measurementRow),
      _detriggerTol(detriggerTol), _detriggerDelay(detriggerDelay),
      _lastTrigger(millis_none), _triggerState(Terra_TriggerState_Disabled)
{
    _sensor.initObject(sensorId);
}

TerraTrigger::TerraTrigger(SharedPtr<TerraSensor> sensor, uint8_t measurementRow,
                           float detriggerTol, millis_t detriggerDelay, int typeIn)
    : TerraSubObject(), TerraMeasurementUnitsInterfaceStorageSingle(Terra_Unit_Undefined),
      type(static_cast<decltype(MeasureValue)>(typeIn)), _sensor(this, measurementRow),
      _detriggerTol(detriggerTol), _detriggerDelay(detriggerDelay),
      _lastTrigger(millis_none), _triggerState(Terra_TriggerState_Disabled)
{
    _sensor.initObject(sensor);
}

TerraTrigger::TerraTrigger(const TerraTriggerSubData *dataIn)
    : TerraSubObject(), TerraMeasurementUnitsInterfaceStorageSingle(dataIn ? dataIn->measurementUnits : Terra_Unit_Undefined),
      type(static_cast<decltype(MeasureValue)>(dataIn ? (int)dataIn->type : (int)Unknown)), _sensor(this, dataIn ? dataIn->measurementRow : 0),
      _detriggerTol(dataIn ? dataIn->detriggerTol : 0.0f),
      _detriggerDelay(dataIn ? dataIn->detriggerDelay : 0),
      _lastTrigger(millis_none), _triggerState(Terra_TriggerState_Disabled)
{
    if (dataIn && dataIn->sensorName[0]) { _sensor.initObject(dataIn->sensorName); }
    _sensor.setMeasurementUnits(getMeasurementUnits());
}

void TerraTrigger::saveToData(TerraTriggerSubData *dataOut) const
{
    if (!dataOut) { return; }
    dataOut->type = (tid_t)type;
    dataOut->measurementRow = (int8_t)_sensor.getMeasurementRow();
    dataOut->detriggerTol = _detriggerTol;
    dataOut->detriggerDelay = _detriggerDelay;
    dataOut->measurementUnits = getMeasurementUnits();
    if (_sensor.isSet()) {
        strncpy(dataOut->sensorName, _sensor.getKeyString().c_str(), TERRA_NAME_MAXSIZE - 1);
        dataOut->sensorName[TERRA_NAME_MAXSIZE - 1] = '\0';
    }
}

void TerraTrigger::update()
{
    _sensor.updateIfNeeded(true);
}

Terra_TriggerState TerraTrigger::getTriggerState(bool poll)
{
    _sensor.updateIfNeeded(poll);
    return _triggerState;
}

void TerraTrigger::setMeasurementUnits(Terra_Unit measurementUnits, uint8_t measurementRow)
{
    if (measurementRow) { return; }
    if (_measurementUnits[0] != measurementUnits) {
        _measurementUnits[0] = measurementUnits;
        _sensor.setMeasurementUnits(measurementUnits);
        bumpRevisionIfNeeded();
    }
}

Terra_Unit TerraTrigger::getMeasurementUnits(uint8_t measurementRow) const
{
    if (measurementRow) { return Terra_Unit_Undefined; }
    return _measurementUnits[0] != Terra_Unit_Undefined ? _measurementUnits[0] : _sensor.getMeasurementUnits();
}

Signal<Terra_TriggerState, TERRA_TRIGGER_SIGNAL_SLOTS> &TerraTrigger::getTriggerSignal()
{
    return _triggerSignal;
}

TerraMeasurementValueTrigger::TerraMeasurementValueTrigger(TerraIdentity sensorId, float triggerTol,
                                                           bool triggerBelow, uint8_t measurementRow,
                                                           float detriggerTol, millis_t detriggerDelay)
    : TerraTrigger(sensorId, measurementRow, detriggerTol, detriggerDelay, MeasureValue),
      _triggerTol(triggerTol), _triggerBelow(triggerBelow)
{
    _sensor.setHandleMethod(&TerraMeasurementValueTrigger::handleMeasurement, this);
}

TerraMeasurementValueTrigger::TerraMeasurementValueTrigger(SharedPtr<TerraSensor> sensor, float triggerTol,
                                                           bool triggerBelow, uint8_t measurementRow,
                                                           float detriggerTol, millis_t detriggerDelay)
    : TerraTrigger(sensor, measurementRow, detriggerTol, detriggerDelay, MeasureValue),
      _triggerTol(triggerTol), _triggerBelow(triggerBelow)
{
    _sensor.setHandleMethod(&TerraMeasurementValueTrigger::handleMeasurement, this);
}

TerraMeasurementValueTrigger::TerraMeasurementValueTrigger(const TerraTriggerSubData *dataIn)
    : TerraTrigger(dataIn),
      _triggerTol(dataIn ? dataIn->dataAs.measureValue.tolerance : 0.0f),
      _triggerBelow(dataIn ? dataIn->dataAs.measureValue.triggerBelow : true)
{
    _sensor.setHandleMethod(&TerraMeasurementValueTrigger::handleMeasurement, this);
}

void TerraMeasurementValueTrigger::saveToData(TerraTriggerSubData *dataOut) const
{
    TerraTrigger::saveToData(dataOut);
    dataOut->dataAs.measureValue.tolerance = _triggerTol;
    dataOut->dataAs.measureValue.triggerBelow = _triggerBelow;
}

void TerraMeasurementValueTrigger::setTriggerTolerance(float tolerance)
{
    if (!isFPEqual(_triggerTol, tolerance)) {
        _triggerTol = tolerance;
        _sensor.setNeedsMeasurement();
        bumpRevisionIfNeeded();
    }
}

void TerraMeasurementValueTrigger::handleMeasurement(const TerraMeasurement *measurement)
{
    if (!measurement || !measurement->isSet()) { return; }

    bool wasState = _triggerState == Terra_TriggerState_Triggered;
    TerraSingleMeasurement measure = getAsSingleMeasurement(measurement, getMeasurementRow());
    if (getMeasurementUnits() != Terra_Unit_Undefined && measure.units != getMeasurementUnits() && canConvertUnits(measure.units, getMeasurementUnits())) {
        measure.toUnits(getMeasurementUnits());
    }
    _sensor.setMeasurement(measure);

    float tolAdditive = wasState ? _detriggerTol : 0.0f;
    bool nextState = _triggerBelow ? measure.value <= _triggerTol + tolAdditive + FLT_EPSILON
                                   : measure.value >= _triggerTol - tolAdditive - FLT_EPSILON;

    millis_t now = millis();
    if (_lastTrigger && (millis_t)(now - _lastTrigger) >= _detriggerDelay) { _lastTrigger = millis_none; }

    if (_triggerState == Terra_TriggerState_Disabled ||
        (nextState != wasState && (nextState || !_lastTrigger))) {
        _triggerState = nextState ? Terra_TriggerState_Triggered : Terra_TriggerState_NotTriggered;
        _lastTrigger = nextState && _detriggerDelay ? now : millis_none;
        _triggerSignal.fire(_triggerState);
    }
}

TerraMeasurementRangeTrigger::TerraMeasurementRangeTrigger(TerraIdentity sensorId,
                                                           float toleranceLow, float toleranceHigh,
                                                           bool triggerOutside, uint8_t measurementRow,
                                                           float detriggerTol, millis_t detriggerDelay)
    : TerraTrigger(sensorId, measurementRow, detriggerTol, detriggerDelay, MeasureRange),
      _triggerTolLow(toleranceLow), _triggerTolHigh(toleranceHigh), _triggerOutside(triggerOutside)
{
    _sensor.setHandleMethod(&TerraMeasurementRangeTrigger::handleMeasurement, this);
}

TerraMeasurementRangeTrigger::TerraMeasurementRangeTrigger(SharedPtr<TerraSensor> sensor,
                                                           float toleranceLow, float toleranceHigh,
                                                           bool triggerOutside, uint8_t measurementRow,
                                                           float detriggerTol, millis_t detriggerDelay)
    : TerraTrigger(sensor, measurementRow, detriggerTol, detriggerDelay, MeasureRange),
      _triggerTolLow(toleranceLow), _triggerTolHigh(toleranceHigh), _triggerOutside(triggerOutside)
{
    _sensor.setHandleMethod(&TerraMeasurementRangeTrigger::handleMeasurement, this);
}

TerraMeasurementRangeTrigger::TerraMeasurementRangeTrigger(const TerraTriggerSubData *dataIn)
    : TerraTrigger(dataIn),
      _triggerTolLow(dataIn ? dataIn->dataAs.measureRange.toleranceLow : 0.0f),
      _triggerTolHigh(dataIn ? dataIn->dataAs.measureRange.toleranceHigh : 1.0f),
      _triggerOutside(dataIn ? dataIn->dataAs.measureRange.triggerOutside : true)
{
    _sensor.setHandleMethod(&TerraMeasurementRangeTrigger::handleMeasurement, this);
}

void TerraMeasurementRangeTrigger::saveToData(TerraTriggerSubData *dataOut) const
{
    TerraTrigger::saveToData(dataOut);
    dataOut->dataAs.measureRange.toleranceLow = _triggerTolLow;
    dataOut->dataAs.measureRange.toleranceHigh = _triggerTolHigh;
    dataOut->dataAs.measureRange.triggerOutside = _triggerOutside;
}

void TerraMeasurementRangeTrigger::setTriggerMidpoint(float toleranceMid)
{
    float halfRange = (_triggerTolHigh - _triggerTolLow) * 0.5f;
    if (!isFPEqual(_triggerTolLow, toleranceMid - halfRange)) {
        _triggerTolLow = toleranceMid - halfRange;
        _triggerTolHigh = toleranceMid + halfRange;
        _sensor.setNeedsMeasurement();
        bumpRevisionIfNeeded();
    }
}

void TerraMeasurementRangeTrigger::handleMeasurement(const TerraMeasurement *measurement)
{
    if (!measurement || !measurement->isSet()) { return; }

    bool wasState = _triggerState == Terra_TriggerState_Triggered;
    TerraSingleMeasurement measure = getAsSingleMeasurement(measurement, getMeasurementRow());
    if (getMeasurementUnits() != Terra_Unit_Undefined && measure.units != getMeasurementUnits() && canConvertUnits(measure.units, getMeasurementUnits())) {
        measure.toUnits(getMeasurementUnits());
    }
    _sensor.setMeasurement(measure);

    float tolAdditive = wasState ? _detriggerTol : 0.0f;
    bool nextState;
    if (_triggerOutside) {
        nextState = measure.value <= _triggerTolLow + tolAdditive + FLT_EPSILON ||
                    measure.value >= _triggerTolHigh - tolAdditive - FLT_EPSILON;
    } else {
        nextState = measure.value >= _triggerTolLow - tolAdditive - FLT_EPSILON &&
                    measure.value <= _triggerTolHigh + tolAdditive + FLT_EPSILON;
    }

    millis_t now = millis();
    if (_lastTrigger && (millis_t)(now - _lastTrigger) >= _detriggerDelay) { _lastTrigger = millis_none; }

    if (_triggerState == Terra_TriggerState_Disabled ||
        (nextState != wasState && (nextState || !_lastTrigger))) {
        _triggerState = nextState ? Terra_TriggerState_Triggered : Terra_TriggerState_NotTriggered;
        _lastTrigger = nextState && _detriggerDelay ? now : millis_none;
        _triggerSignal.fire(_triggerState);
    }
}

TerraTriggerSubData::TerraTriggerSubData()
    : TerraSubData(TerraTrigger::Unknown), sensorName{0}, measurementRow(0), dataAs(),
      detriggerTol(0.0f), detriggerDelay(0), measurementUnits(Terra_Unit_Undefined)
{ ; }

void TerraTriggerSubData::toJSONObject(JsonObject &objectOut) const
{
    TerraSubData::toJSONObject(objectOut);
    if (sensorName[0]) { objectOut["sensorName"] = sensorName; }
    if (measurementRow) { objectOut["measurementRow"] = measurementRow; }
    objectOut["detriggerTol"] = detriggerTol;
    objectOut["detriggerDelay"] = detriggerDelay;
    objectOut["measurementUnits"] = (int)measurementUnits;
    if (type == TerraTrigger::MeasureValue) {
        objectOut["tolerance"] = dataAs.measureValue.tolerance;
        objectOut["triggerBelow"] = dataAs.measureValue.triggerBelow;
    } else if (type == TerraTrigger::MeasureRange) {
        objectOut["toleranceLow"] = dataAs.measureRange.toleranceLow;
        objectOut["toleranceHigh"] = dataAs.measureRange.toleranceHigh;
        objectOut["triggerOutside"] = dataAs.measureRange.triggerOutside;
    }
}

void TerraTriggerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    TerraSubData::fromJSONObject(objectIn);
    const char *sensor = objectIn["sensorName"] | nullptr;
    if (sensor) {
        strncpy(sensorName, sensor, TERRA_NAME_MAXSIZE - 1);
        sensorName[TERRA_NAME_MAXSIZE - 1] = '\0';
    }
    measurementRow = objectIn["measurementRow"] | measurementRow;
    detriggerTol = objectIn["detriggerTol"] | detriggerTol;
    detriggerDelay = objectIn["detriggerDelay"] | detriggerDelay;
    measurementUnits = (Terra_Unit)(objectIn["measurementUnits"] | (int)measurementUnits);
    if (type == TerraTrigger::MeasureValue) {
        dataAs.measureValue.tolerance = objectIn["tolerance"] | dataAs.measureValue.tolerance;
        dataAs.measureValue.triggerBelow = objectIn["triggerBelow"] | dataAs.measureValue.triggerBelow;
    } else if (type == TerraTrigger::MeasureRange) {
        dataAs.measureRange.toleranceLow = objectIn["toleranceLow"] | dataAs.measureRange.toleranceLow;
        dataAs.measureRange.toleranceHigh = objectIn["toleranceHigh"] | dataAs.measureRange.toleranceHigh;
        dataAs.measureRange.triggerOutside = objectIn["triggerOutside"] | dataAs.measureRange.triggerOutside;
    }
}
