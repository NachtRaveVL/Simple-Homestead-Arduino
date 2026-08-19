#include "Terraduino.h"
#include <cstdlib>
#include <iostream>

static void check(bool condition, const char *message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << std::endl;
        std::exit(1);
    }
}

struct OutputState {
    float value = 0.0f;
    int writes = 0;
};

static void writeOutput(void *context, float value)
{
    OutputState *state = static_cast<OutputState *>(context);
    state->value = value;
    ++state->writes;
}

static int dailyRuns = 0;
static void dailyTask(void *) { ++dailyRuns; }

int main()
{
    TerraCistern cistern(1000.0f, 1, "Main Cistern");
    check(cistern.setThresholds(20.0f, 30.0f, 90.0f), "cistern thresholds configure");
    check(cistern.configureFillBand(40.0f, 90.0f, 99.0f), "cistern fill band configures");
    check(!cistern.configureFillBand(40.0f, 99.0f, 99.0f), "cistern rejects fill stop at overflow limit");
    cistern.setStoredLiters(980.0f);
    check(isFPEqual(cistern.receiveWater(50.0f), 10.0f), "cistern accepts only safe fill capacity");
    check(isFPEqual(cistern.getStoredLiters(), 990.0f), "cistern stops at overflow safety band");
    check(isFPEqual(cistern.getOverflowLiters(), 40.0f), "cistern accounts rejected overflow water");

    TerraWaterSource rain(Terra_WaterSourceType_Rainwater, 0, 2, "Rainwater");
    TerraWaterSource well(Terra_WaterSourceType_Well, 1, 3, "Well");
    rain.setLevel(8.0f);
    rain.setReserveLevel(10.0f);
    well.setLevel(75.0f);
    well.setReserveLevel(20.0f);
    const TerraWaterSource *sources[] = { &rain, &well };
    TerraWaterBalancer waterBalancer;
    check(waterBalancer.selectSource(sources, 2) == &well, "water balancer skips source at reserve");

    TerraWaterRoute route(4, "Fill Route");
    route.configure(well.getKey(), cistern.getKey(), 40.0f, 90.0f);
    cistern.setLevel(35.0f);
    TerraTransferDecision decision = route.evaluate(well, cistern);
    check(decision.shouldRun && decision.state == Terra_RouteState_Requested, "route requests fill below start level");
    cistern.setLevel(91.0f);
    decision = route.evaluate(well, cistern);
    check(!decision.shouldRun && decision.state == Terra_RouteState_Complete, "route stops above fill target");

    TerraSumpPump sump(5, "Basement Sump");
    OutputState sumpOutput;
    sump.setWriteCallback(writeOutput, &sumpOutput);
    sump.setMaxContinuousRuntime(1000);
    check(sump.configureLevels(70.0f, 20.0f, 90.0f), "sump levels configure");
    check(!sump.configureLevels(20.0f, 70.0f, 90.0f), "sump rejects reversed hysteresis");
    check(sump.updateLevel(75.0f, true, 100), "sump starts at high level");
    check(sump.isActive() && isFPEqual(sumpOutput.value, 1.0f), "sump output turns on");
    check(sump.updateLevel(50.0f, true, 500), "sump stays on through hysteresis band");
    check(!sump.updateLevel(15.0f, true, 600), "sump stops at low level");
    check(!sump.isActive() && isFPEqual(sumpOutput.value, 0.0f), "sump output turns off");
    check(sump.updateLevel(95.0f, true, 700), "sump runs at high-water level");
    check(sump.hasHighWaterAlarm(), "sump raises high-water alarm");
    check(!sump.updateLevel(95.0f, false, 800), "invalid sump level stops pump");
    check(sump.hasFault() && !sump.hasValidLevel() && !sump.isActive(), "invalid sump level fails safe");
    check(sump.updateLevel(75.0f, true, 900), "valid sump level recovers invalid-level fault");
    sump.update(2000);
    check(sump.hasFault() && !sump.isActive(), "sump maximum runtime stops pump");

    TerraThermalStore thermal(6, "Thermal Store");
    check(thermal.setAbsoluteMaximum(90.0f), "thermal absolute maximum configures");
    check(thermal.setTargetRange(45.0f, 65.0f), "thermal target range configures");
    check(!thermal.setTargetRange(45.0f, 95.0f), "normal target cannot raise safety ceiling");
    thermal.setTemperature(90.0f);
    check(thermal.isSafetyLimitExceeded(), "thermal safety ceiling trips");

    TerraThermalLoop loop(7, "Collector Loop");
    check(loop.configure(8.0f, 3.0f, 80.0f), "thermal loop configures");
    TerraThermalBalancer thermalBalancer;
    check(thermalBalancer.evaluate(loop, 70.0f, 50.0f), "thermal loop starts on useful differential");
    check(!thermalBalancer.evaluate(loop, 85.0f, 80.0f), "thermal loop stops at storage maximum");

    TerraScheduler scheduler;
    check(scheduler.addDailyTask(dailyTask, nullptr, 360) >= 0, "daily task added");
    scheduler.update(0, 359, 10);
    check(dailyRuns == 0, "daily task waits for scheduled minute");
    scheduler.update(0, 360, 10);
    check(dailyRuns == 1, "daily task runs at scheduled minute");
    scheduler.update(0, 500, 10);
    check(dailyRuns == 1, "daily task runs once per day");
    scheduler.update(0, 360, 11);
    check(dailyRuns == 2, "daily task runs again next day");

    std::cout << "PASS Terraduino automation" << std::endl;
    return 0;
}
