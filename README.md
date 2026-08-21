# Terraduino
Terraduino: Simple Homestead Automation Controller.

**Simple-Homestead-Arduino v0.7.2.0**

Simple automation controller for homestead resource and environmental systems.  
Licensed under the non-restrictive MIT license.

Created by NachtRaveVL, 2026.

This controller manages water sources, cisterns, pumps, valves, rainwater collection, stored heat, circulation, weather observations, sensors, scheduling, logging, and data collection for practical DIY homestead systems. It is intended for property infrastructure and self-sufficiency projects rather than generic smart-home automation.

Our Keep-It-Simple controller system:

* Can be used entirely offline for normal local control.
  * WiFi, Ethernet, MQTT, remote sensors, and other network transports are optional.
  * Loss of a network service does not have to stop unrelated local water, thermal, or environmental control.
* Supports water sources, storage, and routing.
  * Rainwater, wells, surface water, reclaimed water, and other sources can be represented without changing the routing logic.
  * Cisterns track capacity, protected reserve, fill bands, overflow state, inflow, outflow, and usable stored volume.
  * Source priority, transfer hysteresis, minimum/maximum flow, and flow-fault checks help avoid rapid cycling and bad transfer states.
  * `TerraSumpPump` adds high-level start, low-level stop hysteresis, high-water alarm state, invalid-level fail-safe shutdown, and maximum-runtime protection for sump applications.
* Supports rainwater harvesting as a first-class workflow.
  * Roof catchment, first-flush diversion, cistern fill limits, and overflow-aware storage can be modeled together.
* Supports stored heat and hydronic-style circulation.
  * Thermal stores can use target ranges, upper limits, and differential circulation rules.
  * The controller can coordinate pumps, circulators, heaters, and related actuators while leaving equipment safety to suitable external hardware.
* Supports weather and environmental observations.
  * Temperature, humidity, pressure, rainfall, rainfall rate, wind, solar irradiance, leak state, and other common homestead measurements can be represented.
  * Dew point and freezing-condition checks can inform local protection and routing decisions.
* Supports local and remote sensors through the same general sensor model.
  * Remote measurements track freshness and stale/offline state without making MQTT or any other transport part of the sensor abstraction.
* Supports familiar hobby electronics and Arduino-style I/O.
  * Digital and analog pins, active-low outputs, PWM, callbacks, sensors, actuators, activation requests, triggers, measurements, modules, and power rails are kept in the same general style as the sibling libraries.
* Supports interval and daily scheduling, system event logging, and data publishing.
  * Output sinks remain optional so local control does not require a server or broker.
* Supports JSON configuration and a versioned checksummed binary system record.
  * Controller preferences, object settings, pins, calibration, and object attachments can be persisted.
  * Enum import decoding uses a generated minimum-discriminator trie for compact and fast parsing.
* Includes the same `full`, `min`, `shared`, display/input, screen, and tcMenu source layout as the related controller libraries.
  * The shared tcMenu adapter layer is present.
  * Terraduino-specific menus, overview screens, and UI data are still TODO work while the homestead interface is designed.
* Keeps the controller on the supervisory side of the system.
  * Pressure relief, pump overload protection, dry-run protection, thermal cutoffs, combustion controls, mains isolation, backflow prevention, and similar safety functions still belong in suitable hardware.

Made primarily for Arduino microcontrollers / build environments, but should also fit PlatformIO, Espressif, Teensy, STM32, Pico/RP2040/RP2350, GIGA, Portenta, and similar MCU platforms. The practical size of a build depends heavily on how many sensors, actuators, stored objects, UI features, and network options are enabled.

*If you value the work that we do, our small team always appreciates a subscription to our [Patreon](www.patreon.com/nachtrave).*

## About

We want to make practical homestead automation more accessible to DIY'ers by using widely available low-cost microcontrollers, sensors, relays, motor controllers, plumbing hardware, and environmental instruments.

A lot of useful property automation is not really "smart home" work. Knowing how much water is in a cistern, deciding when a transfer pump should run, protecting a reserve, diverting first-flush rainwater, circulating stored heat, watching for freezing conditions, or logging weather data are infrastructure problems. They are also good jobs for small local controllers that can keep working without an internet connection.

Terraduino is meant to provide the common controller layer for those systems without dictating one physical build. A small rain barrel, a large cistern, a well transfer system, an outbuilding freeze monitor, a hydronic storage tank, or a full property weather station can all use the same resource, sensor, actuator, scheduler, logger, and persistence patterns.

## Controller Setup

### MCU Requirements

There is no single minimum MCU for every Terraduino build because total object count, UI support, logging, networking, and stored configuration all affect program and memory use.

As a practical planning target:

Minimum MCU: 256-512kB Flash, 16-24kB SRAM, 16MHz+  
Recommended: 512kB-1MB+ Flash, 24-32kB+ SRAM, 32-48MHz+

Modern 32-bit boards such as ESP32, RP2040/RP2350, Teensy 3.5+, STM32, GIGA, and Portenta-class devices are the easiest starting point for larger systems. Smaller supported MCUs may need GUI, networking, debug support, or object counts reduced.

Note: Pin-limited MCUs can use multiplexers, I2C expanders, remote sensor nodes, or other driver arrangements. The number and kind of real I/O channels required may become the limiting factor before CPU speed does.

Note: Pumps, valves, heaters, and other loads must use suitable external drivers. The MCU should provide control and instrumentation, not directly switch dangerous or high-power equipment.

### Installation

The easiest way to install this controller is to use the Arduino IDE library manager when available, or a package manager such as PlatformIO. Otherwise, download the controller and extract its files into a `Simple-Homestead-Arduino` folder in the Arduino custom libraries folder, typically found in `[My ]Documents\Arduino\libraries` on Windows or `~/Documents/Arduino/libraries/` on Linux/OSX.

From there, make a local copy of one of the example sketches based on the system being built. `BasicHomestead` shows the controller lifecycle, while `CisternManagement` is a good starting point for active water control.

Storage-constrained MCUs should start with a small object count and only the features required by the installation. Networking and GUI support can remain disabled for completely local systems.

### Water, Thermal, and Environment Data

Terraduino separates measurements from the control decisions that use them. Water level, flow, temperature, weather, and remote readings are represented as sensors or environment/resource state. Balancers and routes then make decisions from those values.

Cisterns are first-class storage objects rather than generic level readings. They keep capacity, fill thresholds, reserve levels, stored volume, and transfer state together. Rainwater catchment adds first-flush and overflow-aware collection on top of that storage model.

Sump control uses the same pump output model with dedicated level hysteresis. A valid rising level starts the pump at the configured start point, falling level stops it at the lower threshold, and bad level data shuts the pump down instead of allowing it to run blind.

Thermal stores follow the same pattern. A store represents the heat resource, while differential-control logic determines when circulation is useful.

### Host Tests

Core logic and source checks can be run without an Arduino connected:

```sh
./tests/run_tests.sh
```

The script verifies the generated enum trie and source/example checks before running the CMake host build and CTest suite.

The equivalent individual commands are:

```sh
python3 tests/generate_enum_trie.py --check
python3 tests/validate_source.py
python3 tests/validate_examples.py
cmake -S tests -B build-host
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

Host tests cover resource thresholds, cistern fill/overflow behavior, rain capture and first flush, transfer hysteresis, source priority, flow faults, thermal differential control, weather validity, local and remote sensors, actuator request aggregation, runtime faults, scheduling, logging, publishing, JSON/binary persistence, pin/calibration persistence, attachment reconstruction, factory reconstruction, units, and enum/string conversion.

### Setup

#### Header Defines

There are several defines inside of the controller's main `Terraduino.h` header file that allow for more fine-tuned control of the controller. They can be edited and uncommented directly, or supplied through custom build flags. Editing the installed header affects every sketch using that library installation, so custom build flags are preferred when practical.

From Terraduino.h:

```Arduino
// Uncomment or -D this define to completely disable multitasking commands and libraries.
//#define TERRA_DISABLE_MULTITASKING

// Uncomment or -D this define to disable tcMenu-based GUI control.
//#define TERRA_DISABLE_GUI

// Uncomment or -D this define to enable the platform WiFi library.
//#define TERRA_ENABLE_WIFI

// Uncomment or -D this define to enable serial AT-command WiFi support.
//#define TERRA_ENABLE_AT_WIFI

// Uncomment or -D this define to enable the platform Ethernet library.
//#define TERRA_ENABLE_ETHERNET

// Uncomment or -D this define to enable MQTT publishing support.
//#define TERRA_ENABLE_MQTT

// Uncomment or -D this define to enable GPS-based time/location support.
//#define TERRA_ENABLE_GPS

// Uncomment or -D this define to disable built-in Flash data and use external data storage.
//#define TERRA_DISABLE_BUILTIN_DATA

// Uncomment or -D this define to enable serial debug output.
//#define TERRA_ENABLE_DEBUG_OUTPUT

// Uncomment or -D this define to enable verbose debug output.
//#define TERRA_ENABLE_VERBOSE_DEBUG

// Uncomment or -D this define to enable debug assertions.
//#define TERRA_ENABLE_DEBUG_ASSERTIONS
```

Networking remains opt-in. A build with none of the network defines enabled is a normal supported configuration.

Terraduino-specific menu behavior is still TODO work, so the shared UI header does not yet expose the full set of project-specific UI defines found in the older controller libraries.

#### External Libraries

Terraduino keeps the core controller fairly dependency-light. Arduino builds use the following libraries depending on the enabled hardware and features:

* **TaskManagerIO** and **IoAbstraction** for multitasking and I/O support when multitasking is enabled.
* **Adafruit GPS** when GPS-derived time or location is enabled.
* **MQTT** when MQTT publishing is enabled.
* **SD** plus the platform SPI/Wire support for local storage and buses.
* **WiFi101**, **WiFiNINA_Generic**, **WiFiEspAT**, or **Ethernet** when the matching optional network path is enabled.

Networking is optional. Remote sensors are transport-neutral and do not require MQTT specifically. MQTT, ESP-NOW, LoRa, RS-485, CAN, serial radio, or another application transport can feed the same remote-sensor object.

#### External UI Libraries

The optional tcMenu UI layer can use the same display and input libraries across the controller family:

* **tcMenu** for the menu, remote-control, and display abstraction layer.
* **Adafruit GFX**, **Adafruit ILI9341**, and **Adafruit ST7735 and ST7789 Library** for supported color displays.
* **Adafruit FT6206**, **Adafruit TouchScreen**, and optional **XPT2046_Touchscreen** for touch input.
* **LiquidCrystalIO** for character LCD displays.
* **U8g2** for monochrome OLED and LCD displays.
* **TFT_eSPI** for supported advanced TFT configurations.
* **tcUnicodeHelper** for Unicode-capable tcMenu display paths.

* **U8g2** custom display setups use the selected U8g2 device class and are statically linked to that display configuration.
* **TFT_eSPI** uses its `TFT_eSPI\User_Setup.h` configuration and therefore requires a rebuild when that hardware setup changes.
* **BSP LCD / BSP Touch** support can use the included ChromaArt/BSP adapter layer on supported STM32/mbed targets. This is an advanced hardware-specific path.
* **ST7789 custom TFT / TFT_eSPI** setups use statically configured screen dimensions and require a rebuild when those values change.

Terraduino-specific menus and overview screens are still TODO work. The shared tcMenu adapter files are already present so the project-specific UI can use the same plumbing as the sibling libraries.

### Initialization

There are several initialization settings exposed through the controller for system name, control mode, measurement mode, timezone, logging, and publishing.

#### Class Instantiation

The main controller object is normally instantiated near the top of the sketch:

```Arduino
Terraduino terraController;
```

#### Controller Initialization

A call to `init()` is expected near the top of `setup()`. It accepts a `TerraSystemSetup` record so the base controller settings can be prepared together.

From Terraduino.h, in class Terraduino:

```Arduino
void init(const TerraSystemSetup &setup = TerraSystemSetup());
```

For example:

```Arduino
TerraSystemSetup setup;
setup.systemName = "Homestead";
setup.controlMode = Terra_ControlMode_Automatic;

terraController.init(setup);
terraController.launch();
```

The controller `update()` method advances registered objects, schedules, and publishing. Daily schedules use the controller's synchronized local clock internally, so application code does not need to calculate or supply wall-clock fields.

Terraduino also exposes JSON and binary helpers for controller-level persistence:

```Arduino
TerraString json = terraController.exportSystemJSON();

terraController.importSystemJSON(json);
terraController.exportSystemBinary(buffer, capacity);
terraController.importSystemBinary(buffer, length);
```

### Event Logging & Data Publishing

The controller exposes its scheduler, logger, publisher, and module registry as public subsystem instances:

```Arduino
terraController.scheduler;
terraController.logger;
terraController.publisher;
terraController.modules;
```

Logging and publishing are not tied to a network connection. Local storage, serial/application sinks, or network-backed transports can be attached according to the build.

## Hookup Callouts

Many homestead systems involve water, heat, motors, pressure, outdoor wiring, and equipment that can cause damage when controlled incorrectly. Terraduino is a supervisory controller and does not replace the hardware protections appropriate to the installation.

### General

* Check the logic voltage of the MCU, sensors, relay boards, motor controllers, and external modules before connecting them.
* Use isolation, fusing, overcurrent protection, and suitable drivers for pumps, valves, heaters, and other loads.
* Do not switch mains voltage or high-current loads directly from MCU pins.
* Outdoor and outbuilding wiring should be selected for the environment, cable length, moisture exposure, and electrical noise involved.

### Serial UART

UART can be useful for GPS modules, external motor controllers, ESP-AT modules, serial radio links, and other simple peripherals.

* `RX` and `TX` normally cross between the two devices.
* Ensure the signal levels are compatible before connecting 5v and 3.3v equipment.
* Long property runs may need RS-485, CAN, or another differential interface instead of raw TTL serial.

### SPI Bus

SPI is commonly useful for SD storage, displays, radios, and other higher-speed peripherals.

* Each normal SPI device needs its own `CS` line.
* Keep high-current pump and motor wiring away from high-speed data wiring where practical.
* Check the voltage and grounding requirements of remote or separately powered devices.

### I2C Bus

I2C is useful for RTC modules, EEPROM, environmental sensors, displays, and I/O expanders.

* Devices sharing the bus need compatible addresses.
* Pull-up voltage must be safe for every device on the bus.
* I2C is generally best kept local. Long property runs are better handled by a remote node or a bus designed for distance.

### Analog IO

Analog inputs can be used for pressure transducers, level sensors, current sensing, wind/light instruments, and other simple measurements.

* Never exceed the MCU's analog input voltage.
* Use correct scaling and calibration for the sensor's real electrical output.
* Outdoor analog signals may need filtering, shielding, surge protection, or local conversion to a digital/remote measurement.

### Pumps, Valves, Pressure, and Heat

* Pump motors need suitable relays, contactors, motor controllers, overload protection, and dry-run protection for the equipment involved.
* Pressurized water systems still need proper pressure switches, relief valves, expansion provisions, and backflow protection.
* Thermal systems still need independent high-limit protection and any required combustion or mains safety controls.
* A software timer, level reading, or temperature sensor should not be the only protection against a hazardous equipment failure.

### Sensors

Level, flow, pressure, rainfall, temperature, humidity, leak, wind, and solar sensors should be installed and calibrated for their actual environment.

Sensors that foul, drift, freeze, get wet, or age should be treated as measurements that can fail. Control rules should use reasonable limits and fail-safe hardware behavior where a bad reading could cause damage.

### Networking & Wireless

* Networking of any kind is 100% optional.
* Remote sensors can use MQTT, ESP-NOW, LoRa, RS-485, CAN, serial, or another transport selected by the application.
* The remote sensor layer tracks freshness so stale measurements can be distinguished from new values.
* Loss of connectivity should remove only the feature that depends on that link.
* Local pumps, freeze protection, and other important infrastructure should not depend on a cloud service being reachable.

## Memory Callouts

* Total object count, logging, publishing, GUI support, persistence buffers, and networking all affect Flash and SRAM use.
* Disabling GUI, networking, debug features, or other unused support allows the compiler to strip code that is not needed.
* Remote sensor nodes can move physical I/O away from the main controller without requiring the full control framework on every node.
* Large systems should prefer modern 32-bit MCUs with enough RAM for object state and configuration parsing.
* When planning a system, leave memory headroom for future sensors, calibration data, and logging rather than sizing the MCU to the smallest build that happens to compile.

## Example Usage

Below are several of the main examples of controller usage. The example sketches in the repository remain the source of truth.

### Cistern Management Example

`CisternManagement` demonstrates fill hysteresis, protected reserve, source limits, routing, and pump control.

```Arduino
// Simple-Homestead-Arduino Cistern Management Example
//
// Demonstrates fill hysteresis, protected reserve, and pump control for a primary cistern.
// The pump driver is intentionally generic so the real installation can use an isolated
// relay, motor controller, or another suitable low-voltage interface.

#include <Terraduino.h>

Terraduino terraController;
SharedPtr<TerraWaterSource> well;
SharedPtr<TerraCistern> cistern;
SharedPtr<TerraWaterRoute> fillRoute;
SharedPtr<TerraPump> fillPump;
SharedPtr<TerraCallbackOutputDriver> fillPumpDriver;

void driveFillPump(void *context, float output)
{
    (void)context;
    Serial.print(F("Fill pump output: "));
    Serial.println(output, 2);
    // Drive the real isolated pump-control interface here.
}

void setup()
{
    Serial.begin(115200);

    terraController.init();
    well = terraController.addWaterSource(Terra_WaterSourceType_Well, 0, 0, "Well");
    cistern = terraController.addCistern(5000.0f, 0, "Main Cistern");
    fillRoute = terraController.addWaterRoute(0, "Cistern Fill");
    fillPump = terraStaticPointerCast<TerraPump>(terraController.addActuator(Terra_ActuatorType_Pump, 0, "Fill Pump"));
    fillPumpDriver = SharedPtr<TerraCallbackOutputDriver>(new TerraCallbackOutputDriver(driveFillPump));

    well->setLevel(100.0f);
    well->setReserveLevel(10.0f);
    well->setMaximumFlowLpm(20.0f);

    cistern->setThresholds(15.0f, 30.0f, 95.0f);
    cistern->configureFillBand(30.0f, 90.0f, 99.0f);
    cistern->setLevel(28.0f);

    fillPump->setMaxContinuousRuntime(15UL * 60UL * 1000UL);
    fillPump->setDriver(fillPumpDriver);

    fillRoute->setSource(well);
    fillRoute->setDestination(cistern);
    fillRoute->setPump(fillPump);
    fillRoute->setDestinationBand(30.0f, 90.0f);
    fillRoute->setMaximumFlow(20.0f);

    terraController.launch();
}

void loop()
{
    // The route owns fill hysteresis and pump control during controller updates.
    terraController.update();
    delay(1000);
}
```

### Rainwater Collection Example

`RainwaterCollection` shows roof catchment, first-flush diversion, and storage into a cistern without allowing the modeled storage level to run past its overflow band.

```Arduino
// Simple-Homestead-Arduino Rainwater Collection Example
//
// Converts incremental roof rainfall into cistern inflow, applies a first-flush discard,
// and respects the configured cistern overflow band. A real rain gauge should supply the
// rainfall accumulated since the previous collection update.

#include <Terraduino.h>

Terraduino terraController;
SharedPtr<TerraRainCatchment> roofCatchment;
SharedPtr<TerraCistern> rainCistern;
TerraFirstFlushController firstFlush(20.0f);

void setup()
{
    Serial.begin(115200);

    terraController.init();
    roofCatchment = terraController.addRainCatchment(180.0f, 0.85f, 0, "Roof Catchment");
    rainCistern = terraController.addCistern(5000.0f, 0, "Rain Cistern");

    rainCistern->setThresholds(15.0f, 30.0f, 95.0f);
    rainCistern->configureFillBand(30.0f, 95.0f, 99.0f);
    rainCistern->setLevel(25.0f);
    terraController.launch();
}

void loop()
{
    // Replace with incremental rainfall from the installed rain gauge.
    const float rainfallMm = 0.0f;
    TerraRainCollectionResult result = roofCatchment->collectInto(*rainCistern, rainfallMm, &firstFlush);

    if (result.storedLiters > 0.0f) {
        Serial.print(F("Stored rainwater, L: "));
        Serial.println(result.storedLiters, 2);
    }

    terraController.update();
    delay(1000);
}
```

### Full System Example

`FullSystem` combines the main water, thermal, environmental, remote-sensor, scheduling, logging, and publishing pieces in one controller.

```Arduino
// Simple-Homestead-Arduino Full System Example
//
// Combines weather observations, cistern filling, thermal storage, a remote sensor,
// publishing, and normal controller updates. Hardware adapters remain application supplied
// so the example is not tied to one shield or board.

#include <Terraduino.h>

Terraduino terraController;
SharedPtr<TerraEnvironment> weather;
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
    rain = terraController.addWaterSource(Terra_WaterSourceType_Rainwater, 0, 0, "Rainwater");
    cistern = terraController.addCistern(4000.0f, 0, "Cistern");
    fillRoute = terraController.addWaterRoute(0, "Cistern Fill");
    transferPump = terraStaticPointerCast<TerraPump>(terraController.addActuator(Terra_ActuatorType_Pump, 0, "Transfer Pump"));
    collectorTemperature = terraController.addSensor(Terra_SensorType_Temperature, Terra_Unit_Celsius, 0, "Collector Temperature");
    storeTemperature = terraController.addSensor(Terra_SensorType_Temperature, Terra_Unit_Celsius, 0, "Store Temperature");
    thermalStore = terraController.addThermalStore(0, "Thermal Store");
    thermalLoop = terraController.addThermalLoop(0, "Thermal Loop");
    circulator = terraController.addActuator(Terra_ActuatorType_Circulator, 0, "Thermal Circulator");
    barnTemp = terraStaticPointerCast<TerraRemoteSensor>(terraController.addSensor(Terra_SensorType_Remote, Terra_Unit_Celsius, 0, "Barn Temp"));

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
    weather->setAirTemperature(8.0f);
    weather->setRelativeHumidity(78.0f);
    collectorTemperature->setMeasurement(70.0f, Terra_Unit_Celsius, terraMillis(), true);
    storeTemperature->setMeasurement(52.0f, Terra_Unit_Celsius, terraMillis(), true);

    terraController.update();
    delay(250);
}
```

### Data Writer Example

`DataWriter` exports persisted object data as JSON and is the main reference for external-data and storage workflows.

```Arduino
#include <Terraduino.h>

void setup() {
    Serial.begin(115200);
    while (!Serial) { }

    TerraCisternData cistern;
    cistern.key = 1001;
    cistern.name = "Main Cistern";
    cistern.capacityLiters = 5000.0f;
    cistern.level = 63.5f;
    cistern.reserveLevel = 15.0f;
    cistern.lowLevel = 30.0f;
    cistern.highLevel = 95.0f;
    cistern.fillStartPercent = 35.0f;
    cistern.fillStopPercent = 90.0f;
    cistern.overflowPercent = 99.0f;

    Serial.println(cistern.toJSON());

    TerraThermalStoreData thermal;
    thermal.key = 2001;
    thermal.name = "Thermal Store";
    thermal.level = 70.0f;
    thermal.temperatureC = 58.0f;
    thermal.minimumTargetC = 45.0f;
    thermal.maximumTargetC = 65.0f;
    thermal.absoluteMaximumC = 90.0f;

    Serial.println(thermal.toJSON());
}

void loop() { }
```

### Main System Examples

* **BasicHomestead** shows the basic controller lifecycle with homestead objects.
* **CisternManagement** shows fill hysteresis, reserve protection, source limits, and pump control.
* **RainwaterCollection** shows roof catchment, first flush, and overflow-aware cistern storage.
* **ThermalStorage** shows differential circulation into a stored-heat system.
* **WeatherStation** shows environmental observations for monitoring and control.
* **RemoteSensor** shows the transport-neutral remote measurement path.
* **FullSystem** combines the main controller subsystems.
* **UISetup** keeps the tcMenu setup pattern visible while project-specific menus remain TODO work.
* **DataWriter** exports persisted data for external-storage workflows.

The examples show controller patterns without requiring one particular brand of pump, valve, sensor, radio, or display.

## Homestead Callouts

### Water Sources and Cisterns

Water sources can be prioritized and limited independently from storage. A cistern can protect a reserve, request filling through a hysteresis band, stop before the overflow band, and keep track of stored volume.

This makes the same controller logic useful for rainwater storage, well transfer, intermediate tanks, livestock water, utility water, and other property systems without treating every source as identical.

### Rainwater Collection

Rain catchment combines rainfall with a collection area, first-flush handling, and the destination storage state. Overflow handling stays separate from the decision to accept more water so the controller can represent a full cistern without pretending the incoming rain stopped.

### Thermal Storage

Thermal stores model stored heat rather than a particular boiler, solar collector, or heating appliance. Differential circulation can move heat when the source is usefully warmer than the store and stop when the advantage disappears.

The actual heat source and safety controls remain external to Terraduino.

### Remote Sensors

A remote node should normally stay simple. It measures local sensors and reports values, while the main Terraduino controller keeps the resource and balancing logic.

The `TerraRemoteSensor` tracks the reported sensor type, last update, expected freshness, and stale/offline state. The transport remains an application choice.

### Safety Boundary

Terraduino can supervise infrastructure, but it cannot make an unsafe plumbing, electrical, pressure, or heating system safe. Code-required and manufacturer-required protective hardware should operate independently of the Arduino whenever a failure could create a dangerous condition.
