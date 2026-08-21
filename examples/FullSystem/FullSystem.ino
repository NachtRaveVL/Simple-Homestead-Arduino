// Simple-Homestead-Arduino Full System Example
//
// Combines weather observations, cistern filling, thermal storage, a remote sensor,
// publishing, and normal controller updates. Hardware adapters remain application supplied
// so the example is not tied to one shield or board.

#include <Terraduino.h>

Terraduino terraController;
SharedPtr<TerraEnvironment> weather;
SharedPtr<TerraSensor> airTemperature;
SharedPtr<TerraSensor> relativeHumidity;
SharedPtr<TerraWaterSource> rain;
SharedPtr<TerraCistern> cistern;
SharedPtr<TerraWaterRoute> fillRoute;
SharedPtr<TerraPump> transferPump;
SharedPtr<TerraSensor> collectorTemperature;
SharedPtr<TerraSensor> storeTemperature;
SharedPtr<TerraThermalStore> thermalStore;
SharedPtr<TerraThermalLoop> thermalLoop;
SharedPtr<TerraActuator> circulator;
SharedPtr<TerraRemoteSensor> barnTemp;

void actuatorWrite(void *context, float value)
{
    (void)context;
    Serial.print(F("Actuator command: "));
    Serial.println(value, 2);
}

void publishValue(void *context, const char *channel, const TerraMeasurement &measurement)
{
    (void)context;
    Serial.print(channel);
    Serial.print('=');
    Serial.print(measurement.value);
    Serial.print(' ');
    Serial.println(terraUnitToString(measurement.unit));
}

void setup()
{
    Serial.begin(115200);

    TerraSystemSetup setup;
    setup.systemName = "Full Homestead";
    setup.updateIntervalMs = 250;
    terraController.init(setup);

    weather = terraController.addEnvironment(0, "Outside");
    airTemperature = terraController.addSensor(Terra_SensorType_Temperature, Terra_Unit_Celsius, 0, "Outside Temperature");
    relativeHumidity = terraController.addSensor(Terra_SensorType_Humidity, Terra_Unit_Percent, 0, "Outside Humidity");
    weather->setAirTemperatureSensor(airTemperature);
    weather->setHumiditySensor(relativeHumidity);
    rain = terraController.addWaterSource(Terra_WaterSourceType_Rainwater, 0, 0, "Rainwater");
    cistern = terraController.addCistern(4000.0f, 0, "Cistern");
    fillRoute = terraController.addWaterRoute(0, "Cistern Fill");
    transferPump = terraController.addPump(0, "Transfer Pump");
    collectorTemperature = terraController.addSensor(Terra_SensorType_Temperature, Terra_Unit_Celsius, 0, "Collector Temperature");
    storeTemperature = terraController.addSensor(Terra_SensorType_Temperature, Terra_Unit_Celsius, 0, "Store Temperature");
    thermalStore = terraController.addThermalStore(0, "Thermal Store");
    thermalLoop = terraController.addThermalLoop(0, "Thermal Loop");
    circulator = terraController.addCirculator(0, "Thermal Circulator");
    barnTemp = terraController.addRemoteSensor(Terra_SensorType_Temperature, Terra_Unit_Celsius, 0, "Barn Temp");

    cistern->setThresholds(15.0f, 30.0f, 95.0f);
    cistern->configureFillBand(30.0f, 95.0f, 99.0f);
    cistern->setLevel(25.0f);
    rain->setLevel(80.0f);

    transferPump->setMaxContinuousRuntime(10UL * 60UL * 1000UL);
    transferPump->setDriver(SharedPtr<TerraOutputDriver>(new TerraCallbackOutputDriver(actuatorWrite)));
    fillRoute->setSource(rain);
    fillRoute->setDestination(cistern);
    fillRoute->setPump(transferPump);
    fillRoute->setDestinationBand(30.0f, 95.0f);

    thermalStore->setTargetRange(45.0f, 65.0f);
    thermalStore->setAbsoluteMaximum(90.0f);
    thermalStore->setTemperatureSensor(storeTemperature);
    circulator->setDriver(SharedPtr<TerraOutputDriver>(new TerraCallbackOutputDriver(actuatorWrite)));
    thermalLoop->configure(8.0f, 3.0f, 80.0f);
    thermalLoop->setSourceTemperatureSensor(collectorTemperature);
    thermalLoop->setThermalStore(thermalStore);
    thermalLoop->setCirculator(circulator);

    barnTemp->setStaleAfter(5UL * 60UL * 1000UL);
    terraController.publisher.setCallback(publishValue);
    terraController.publisher.addChannel("barn-temp", barnTemp.get(), Terra_Unit_Celsius);
    terraController.launch();
}

void loop()
{
    airTemperature->setMeasurement(8.0f, Terra_Unit_Celsius, terraMillis(), true);
    relativeHumidity->setMeasurement(78.0f, Terra_Unit_Percent, terraMillis(), true);
    collectorTemperature->setMeasurement(70.0f, Terra_Unit_Celsius, terraMillis(), true);
    storeTemperature->setMeasurement(52.0f, Terra_Unit_Celsius, terraMillis(), true);

    terraController.update();
    delay(250);
}
