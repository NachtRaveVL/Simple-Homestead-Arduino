// Simple-Homestead-Arduino Full System Example
//
// Combines water storage and transfer, thermal storage, remote measurements, scheduler
// updates, and logging in one system. This is also the example intended for console
// debugging: enable TERRA_ENABLE_DEBUG_OUTPUT through Terraduino.h or a build flag to
// receive the library's normal logger output over Serial.

#include <Terraduino.h>

Terraduino terraController;
SharedPtr<TerraInfiniteWaterReservoir> waterSource;
SharedPtr<TerraWaterReservoir> waterStorage;
SharedPtr<TerraRemoteSensor> waterVolume;
SharedPtr<TerraRelayPumpActuator> fillPump;
SharedPtr<TerraThermalReservoir> thermalStorage;
SharedPtr<TerraRemoteSensor> thermalTemperature;
SharedPtr<TerraRemoteSensor> remoteTemperature;
TerraActivationHandle fillRequest;

void setup()
{
    Serial.begin(115200);

    terraController.init(Terra_SystemMode_Automatic, Terra_MeasurementMode_Metric);
    terraController.setSystemName("Full Homestead");

    waterSource = terraController.addInfiniteWaterReservoir(true);
    waterStorage = terraController.addWaterReservoir(4000.0f);
    waterVolume = terraController.addRemoteSensor(Terra_SensorType_Level,
                                                   Terra_UnitsType_LiqVolume_Liters);
    fillPump = terraController.addPumpRelay(5, false);

    waterStorage->getWaterVolumeSensorAttachment().setObject(waterVolume);
    fillPump->getSourceReservoirAttachment().setObject(waterSource);
    fillPump->getDestinationReservoirAttachment().setObject(waterStorage);
    fillPump->setEnableMode(Terra_EnableMode_Highest);

    thermalStorage = terraController.addThermalReservoir(90.0f);
    thermalTemperature = terraController.addRemoteSensor(Terra_SensorType_Temperature,
                                                          Terra_UnitsType_Temperature_Celsius);
    thermalStorage->getMediumTemperatureSensorAttachment().setObject(thermalTemperature);

    remoteTemperature = terraController.addRemoteSensor(Terra_SensorType_Temperature,
                                                         Terra_UnitsType_Temperature_Celsius);
    remoteTemperature->setStaleAfter(5UL * 60UL * 1000UL);

    terraController.logger.logMessage(F("Full system starting"));
    terraController.launch();
}

void loop()
{
    // Replace these sample reports with installed or remote sensor data.
    waterVolume->receiveReport(1400.0f, Terra_UnitsType_LiqVolume_Liters);
    thermalTemperature->receiveReport(54.0f, Terra_UnitsType_Temperature_Celsius);
    remoteTemperature->receiveReport(11.0f, Terra_UnitsType_Temperature_Celsius);

    float waterLevel = waterStorage->getLevel(true);
    if (waterLevel < 30.0f && !fillRequest.isValid()) {
        terraController.logger.logMessage(F("Requesting water storage fill"));
        fillRequest = fillPump->enableActuator();
    } else if (waterLevel >= 90.0f && fillRequest.isValid()) {
        terraController.logger.logMessage(F("Water storage fill complete"));
        fillRequest.unset();
    }

    terraController.update();
    delay(1000);
}
