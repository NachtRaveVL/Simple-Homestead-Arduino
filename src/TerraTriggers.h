/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Triggers
*/

#ifndef TerraTriggers_H
#define TerraTriggers_H

class TerraTrigger;
class TerraMeasurementValueTrigger;
class TerraMeasurementRangeTrigger;

struct TerraTriggerSubData;

#include "TerraObject.h"
#include "TerraSensors.h"
#include "TerraAttachments.h"
#include "TerraData.h"

// Trigger Base
// Used to alert the system to a change in an attached sensor measurement.
class TerraTrigger : public TerraSubObject,
                     public TerraMeasurementUnitsInterfaceStorageSingle {
public:
    const enum : signed char { MeasureValue, MeasureRange, Unknown = -1 } type; // Trigger type

    TerraTrigger(TerraIdentity sensorId,
                 uint8_t measurementRow,
                 float detriggerTol,
                 millis_t detriggerDelay,
                 int typeIn = Unknown);
    TerraTrigger(SharedPtr<TerraSensor> sensor,
                 uint8_t measurementRow,
                 float detriggerTol,
                 millis_t detriggerDelay,
                 int typeIn = Unknown);
    TerraTrigger(const TerraTriggerSubData *dataIn);

    virtual void saveToData(TerraTriggerSubData *dataOut) const;
    virtual void update();

    virtual Terra_TriggerState getTriggerState(bool poll = false);
    virtual void setMeasurementUnits(Terra_Unit measurementUnits, uint8_t measurementRow = 0) override;
    virtual Terra_Unit getMeasurementUnits(uint8_t measurementRow = 0) const override;

    inline uint8_t getMeasurementRow() const { return _sensor.getMeasurementRow(); }
    inline float getMeasurementConvertParam() const { return _sensor.getMeasurementConvertParam(); }
    inline float getDetriggerTolerance() const { return _detriggerTol; }
    inline millis_t getDetriggerDelay() const { return _detriggerDelay; }
    inline bool isDetriggerDelayActive() const { return _lastTrigger != millis_none; }
    inline TerraSensorAttachment &getSensorAttachment() { return _sensor; }
    inline const TerraSensorAttachment &getSensorAttachment() const { return _sensor; }
    Signal<Terra_TriggerState, TERRA_TRIGGER_SIGNAL_SLOTS> &getTriggerSignal();

protected:
    TerraSensorAttachment _sensor;                          // Sensor attachment
    float _detriggerTol;                                    // De-trigger tolerance additive
    millis_t _detriggerDelay;                               // De-trigger delay milliseconds
    millis_t _lastTrigger;                                  // Last trigger timestamp / delay start
    Terra_TriggerState _triggerState;                       // Current trigger state
    Signal<Terra_TriggerState, TERRA_TRIGGER_SIGNAL_SLOTS> _triggerSignal; // Trigger state signal

    virtual void handleMeasurement(const TerraMeasurement *measurement) = 0;
};

// Sensor Data Measurement Value Trigger
class TerraMeasurementValueTrigger : public TerraTrigger {
public:
    TerraMeasurementValueTrigger(TerraIdentity sensorId,
                                 float triggerTol,
                                 bool triggerBelow = true,
                                 uint8_t measurementRow = 0,
                                 float detriggerTol = 0.0f,
                                 millis_t detriggerDelay = 0);
    TerraMeasurementValueTrigger(SharedPtr<TerraSensor> sensor,
                                 float triggerTol,
                                 bool triggerBelow = true,
                                 uint8_t measurementRow = 0,
                                 float detriggerTol = 0.0f,
                                 millis_t detriggerDelay = 0);
    TerraMeasurementValueTrigger(const TerraTriggerSubData *dataIn);

    virtual void saveToData(TerraTriggerSubData *dataOut) const override;

    void setTriggerTolerance(float tolerance);
    inline float getTriggerTolerance() const { return _triggerTol; }
    inline bool getTriggerBelow() const { return _triggerBelow; }

protected:
    float _triggerTol;                                      // Trigger tolerance limit
    bool _triggerBelow;                                     // Trigger below flag

    virtual void handleMeasurement(const TerraMeasurement *measurement) override;
};

// Sensor Data Measurement Range Trigger
class TerraMeasurementRangeTrigger : public TerraTrigger {
public:
    TerraMeasurementRangeTrigger(TerraIdentity sensorId,
                                 float toleranceLow,
                                 float toleranceHigh,
                                 bool triggerOutside = true,
                                 uint8_t measurementRow = 0,
                                 float detriggerTol = 0.0f,
                                 millis_t detriggerDelay = 0);
    TerraMeasurementRangeTrigger(SharedPtr<TerraSensor> sensor,
                                 float toleranceLow,
                                 float toleranceHigh,
                                 bool triggerOutside = true,
                                 uint8_t measurementRow = 0,
                                 float detriggerTol = 0.0f,
                                 millis_t detriggerDelay = 0);
    TerraMeasurementRangeTrigger(const TerraTriggerSubData *dataIn);

    virtual void saveToData(TerraTriggerSubData *dataOut) const override;

    void setTriggerMidpoint(float toleranceMid);
    inline float getTriggerToleranceLow() const { return _triggerTolLow; }
    inline float getTriggerToleranceHigh() const { return _triggerTolHigh; }
    inline bool getTriggerOutside() const { return _triggerOutside; }

protected:
    float _triggerTolLow;                                   // Low value tolerance
    float _triggerTolHigh;                                  // High value tolerance
    bool _triggerOutside;                                   // Trigger on outside flag

    virtual void handleMeasurement(const TerraMeasurement *measurement) override;
};

// Combined Trigger Serialization Sub Data
struct TerraTriggerSubData : public TerraSubData {
    char sensorName[TERRA_NAME_MAXSIZE];                    // Sensor identity string
    int8_t measurementRow;                                  // Measurement row
    union {
        struct {
            float tolerance;                                // Value tolerance
            bool triggerBelow;                              // Trigger below flag
        } measureValue;
        struct {
            float toleranceLow;                             // Low value tolerance
            float toleranceHigh;                            // High value tolerance
            bool triggerOutside;                            // Trigger outside flag
        } measureRange;
    } dataAs;
    float detriggerTol;                                     // De-trigger tolerance
    millis_t detriggerDelay;                                // De-trigger delay milliseconds
    Terra_Unit measurementUnits;                            // Measurement units

    TerraTriggerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

extern TerraTrigger *newTriggerObjectFromSubData(const TerraTriggerSubData *dataIn);

#endif
