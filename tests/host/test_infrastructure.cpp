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

int main()
{
    TerraSystemSetup setup;
    setup.systemName = "Homestead Test";
    setup.controlMode = Terra_ControlMode_Automatic;
    setup.measurementMode = Terra_MeasurementMode_Metric;
    setup.loggerMinimumLevel = Terra_LogLevel_Warning;
    setup.publisherIntervalMs = 5000;

    Terraduino controller;
    controller.init(setup);
    check(controller.isInitialized(), "controller initializes");
    check(getController() == &controller, "active controller helper resolves");
    check(getLogger() == &controller.logger, "active logger helper resolves");
    check(getPublisher() == &controller.publisher, "active publisher helper resolves");
    check(getScheduler() == &controller.scheduler, "active scheduler helper resolves");
    controller.launch();
    check(controller.isRunning(), "controller launches");
    controller.suspend();
    check(controller.isSuspended(), "controller suspends");

    TerraCistern cistern(2000.0f, 100, "Main Cistern");
    check(controller.registerObject(&cistern), "controller registers object");
    check(controller.findObjectByKey(100) == &cistern, "factory lookup by key");
    check(controller.findObjectByName("Main Cistern") == &cistern, "factory lookup by name");

    TerraRemoteSensor remote(Terra_SensorType_Temperature, Terra_Unit_Celsius, 101, "Barn Remote");
    remote.setStaleAfter(1000);
    remote.receiveReport(21.5f, Terra_Unit_Celsius, 100, true);
    check(remote.getMeasurement().valid, "remote report starts valid");
    remote.update(1200);
    check(remote.hasFault(), "stale remote sensor faults");
    check(!remote.getMeasurement().valid, "stale remote measurement becomes invalid");

    TerraPowerRail rail(12.0f, 102, "12V Rail", Terra_RailType_DC12V);
    OutputState railOutput;
    rail.setEnableCallback(writeOutput, &railOutput);
    rail.setEnabledState(true);
    check(rail.isRailEnabled() && isFPEqual(railOutput.value, 1.0f), "power rail enables output");
    rail.setEnabledState(false);
    check(!rail.isRailEnabled() && isFPEqual(railOutput.value, 0.0f), "power rail disables output");

    TerraSumpPump sump(103, "Utility Sump");
    check(sump.configureLevels(72.0f, 18.0f, 92.0f), "sump configuration accepted");
    sump.setMaxContinuousRuntime(45000);
    TerraObjectData *serialized = TerraFactory::newDataFromObject(&sump);
    check(serialized != nullptr && serialized->objectType == Terra_ObjectType_Actuator, "sump serializes as actuator data");
    TerraActuatorData *sumpData = static_cast<TerraActuatorData *>(serialized);
    check(sumpData->actuatorType == Terra_ActuatorType_SumpPump, "sump actuator type persists");
    check(isFPEqual(sumpData->sumpStartPercent, 72.0f), "sump start threshold persists");
    check(isFPEqual(sumpData->sumpStopPercent, 18.0f), "sump stop threshold persists");
    check(isFPEqual(sumpData->sumpAlarmPercent, 92.0f), "sump alarm threshold persists");
    check(sumpData->maxContinuousMs == 45000, "sump runtime limit persists");

    TerraString sumpJson = sumpData->toJSON();
    TerraActuatorData parsedSump;
    check(parsedSump.fromJSON(sumpJson), "sump actuator JSON parses");
    TerraObject *rebuilt = TerraFactory::newObjectFromData(&parsedSump);
    check(rebuilt != nullptr, "factory rebuilds sump from data");
    TerraActuator *rebuiltActuator = static_cast<TerraActuator *>(rebuilt);
    check(rebuiltActuator->getActuatorType() == Terra_ActuatorType_SumpPump, "rebuilt object remains a sump pump");
    TerraSumpPump *rebuiltSump = static_cast<TerraSumpPump *>(rebuiltActuator);
    check(isFPEqual(rebuiltSump->getStartLevelPercent(), 72.0f), "rebuilt sump start threshold restored");
    check(isFPEqual(rebuiltSump->getStopLevelPercent(), 18.0f), "rebuilt sump stop threshold restored");
    check(isFPEqual(rebuiltSump->getAlarmLevelPercent(), 92.0f), "rebuilt sump alarm threshold restored");
    check(rebuiltSump->getMaxContinuousRuntime() == 45000, "rebuilt sump runtime restored");
    delete rebuilt;
    delete serialized;

    TerraSystemData system;
    system.setup = setup;
    system.sequence = 42;
    uint8_t binary[256] = {0};
    size_t length = system.toBinary(binary, sizeof(binary));
    check(length > 0, "system binary export succeeds");
    TerraSystemData restored;
    check(restored.fromBinary(binary, length), "system binary import succeeds");
    check(restored.setup.systemName == setup.systemName, "system name round-trips");
    check(restored.setup.controlMode == setup.controlMode, "control mode round-trips");
    check(restored.sequence == 42, "system sequence round-trips");

    std::cout << "PASS Terraduino infrastructure" << std::endl;
    return 0;
}
