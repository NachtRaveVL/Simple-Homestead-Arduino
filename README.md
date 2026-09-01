# Terraduino
Terraduino: Simple Homestead Automation Controller.

**Simple-Homestead-Arduino v0.7.2.0**

Simple automation controller for homestead resource and environmental systems.  
Licensed under the non-restrictive MIT license.

Created by NachtRaveVL, 2026.

Terraduino provides the common controller layer for water storage and transfer, thermal storage, environmental sensing, pumps, valves, heaters, shared power rails, scheduling, logging, and data publishing on Arduino-compatible microcontrollers.

Our Keep-It-Simple controller system:

* Can operate entirely offline for normal local control.
  * WiFi, Ethernet, MQTT, GPS, remote sensors, and GUI support are optional build features.
  * Network loss does not need to interrupt unrelated reservoir, actuator, or sensor control.
* Models the system with four main registered object families:
  * Sensors provide measurements.
  * Actuators perform work.
  * Reservoirs represent stored or effectively infinite resources.
  * Rails limit shared electrical capacity.
* Uses attachments rather than parallel route/source objects to describe relationships.
  * A relay pump, valve, or circulator can attach to source and destination reservoirs.
  * Reservoirs attach to their sensors and filled/high/low/empty triggers.
  * Actuators can attach to a parent reservoir and power rail.
* Provides water and thermal reservoirs.
  * `TerraWaterReservoir` tracks volume through an attached sensor.
  * `TerraThermalReservoir` tracks a thermal store through an attached temperature sensor.
  * Infinite water/thermal reservoirs represent effectively inexhaustible sources or sinks such as a water main, drain, or external thermal source.
* Uses resident activation handles for actuator requests.
  * `TerraRelayActuator` handles binary loads.
  * `TerraRelayPumpActuator` handles resource-transfer equipment such as pumps, valves, and circulators.
  * `TerraVariableActuator` handles proportional outputs.
* Provides measurement triggers and balancing helpers.
  * Value and range triggers provide hysteresis through de-trigger tolerance and delay.
  * `TerraLinearEdgeBalancer` drives increment/decrement actuator sets around a measured setpoint.
* Includes automatic reservoir scheduling.
  * Reservoir tracking moves through `Assess`, `Fill`, `Condition`, `Distribute`, and `Settle` stages.
  * Transfer decisions are derived from reservoir state and the actuator attachment graph rather than separate route objects.
* Supports event logging and polling-frame data publishing.
  * Sensor data can publish to SD card CSV, WiFiStorage, MQTT, or application code through the publish signal.
  * System logs can go to SD/WiFiStorage and to serial debug output when enabled.
* Supports JSON and binary persistence through the same data/factory model used by the sibling controller libraries.
* Includes six focused examples rather than a large set of overlapping sketches.

Made primarily for Arduino microcontrollers / build environments, but intended to fit PlatformIO, Espressif, Teensy, STM32, Pico/RP2040/RP2350, GIGA, Portenta, and similar MCU platforms. Practical size depends on enabled features and total object count.

*If you value the work that we do, our small team always appreciates a subscription to our [Patreon](www.patreon.com/nachtrave).*

## About

Terraduino is for physical property infrastructure rather than generic smart-home automation. Typical jobs include monitoring a storage tank, moving water between reservoirs, operating a sump pump, circulating stored heat, logging weather measurements, or preventing too many high-current loads from starting at once.

The controller does not require every installation to use the same hardware. A small water-transfer system and a larger property controller can use the same identity, attachment, sensor, actuator, scheduler, logger, and persistence mechanisms.

## Controller Setup

### MCU Requirements

There is no single minimum MCU for every Terraduino build because enabled UI, networking, logging, sensor counts, reservoir counts, and automation complexity can change the program and memory requirements considerably.

As a practical starting point:

Minimum MCU: 256-512kB Flash, 16-24kB SRAM, 16MHz+  
Recommended: 512kB-1MB+ Flash, 24-32kB+ SRAM, 32-48MHz+

Modern 32-bit boards such as Pico RP2040/RP2350, ESP32, Teensy 3.5+, STM32, GIGA, and Portenta-class devices are the natural starting point when tracking, logging, UI, and networking are expected to run together.

Pin-limited systems can use multiplexers, I2C expanders, or remote nodes. Pumps, valves, heaters, and other loads require suitable external power drivers and protection.

### Installation

Install through the Arduino IDE library manager when available, through PlatformIO, or by extracting the library into the Arduino custom libraries folder as `Simple-Homestead-Arduino`.

Start from one of the supplied examples rather than an older wiki or sketch copied from a previous object model.

## Examples

The current example set is intentionally small:

* **BasicHomestead** - Minimal controller lifecycle with water/thermal reservoirs and remote measurements.
* **DataWriter** - JSON stream persistence through the controller's normal data model.
* **RemoteSensor** - Transport-neutral remote sensor reports and stale/offline tracking.
* **ThermalStorage** - Thermal reservoir with an attached temperature sensor.
* **WeatherStation** - Terraduino acting as a WiFi/MQTT sensor station using the existing publisher.
* **FullSystem** - Integrated water transfer, thermal storage, remote data, scheduler operation, and console/debug logging.

`FullSystem` is the broad reference example and the best place to observe normal scheduler/logger output with `TERRA_ENABLE_DEBUG_OUTPUT` enabled.

## Current Object Model

### Sensors

`TerraSensor` is the common measurement object. Current concrete sensor classes include binary, analog, remote, and convenience types for temperature, humidity, level, pressure, flow, rainfall, wind, solar radiation, voltage, current, and leak state.

Remote sensors remain transport-neutral. MQTT, serial radio, RS-485, CAN, ESP-NOW, LoRa, or another application transport can call `receiveReport()` without changing the sensor model.

### Reservoirs

Reservoirs are the resource-storage abstraction:

* `TerraWaterReservoir`
* `TerraThermalReservoir`
* `TerraInfiniteWaterReservoir`
* `TerraInfiniteThermalReservoir`

Finite reservoirs use attached sensors and separate filled/high/low/empty triggers. The separation is intentional: a system can distinguish a hard empty/full condition from normal low/high operating thresholds.

### Actuators

`TerraActuator` resolves one or more resident activation requests according to its enable mode.

`TerraRelayPumpActuator` is the transfer-capable actuator. Source and destination reservoir attachments define what it connects. Pump, valve, and circulator are semantic actuator types using that transfer model; they do not require separate route objects.

### Rails

`TerraSimpleRail` limits the number of simultaneously active linked loads. `TerraRegulatedRail` tracks a maximum power budget using an optional power sensor and limit trigger.

### Triggers and Balancers

`TerraMeasurementValueTrigger` and `TerraMeasurementRangeTrigger` convert sensor measurements into trigger state with configurable de-trigger tolerance/delay.

`TerraLinearEdgeBalancer` drives increment/decrement actuator attachments around a sensor setpoint and target range. Balancers are subobjects used by higher-level control logic; they are not another registered object family.

## Scheduler

`TerraScheduler` is not a generic user task scheduler. It automatically tracks registered reservoirs that have relevant actuator linkages.

The current tracking stages are:

1. `Assess`
2. `Fill`
3. `Condition`
4. `Distribute`
5. `Settle`

A reservoir can be filled by an inbound pump/valve/circulator, conditioned by a heater or local circulator, or distributed to another reservoir that needs resource. Actuator requests are maintained through normal `TerraActuatorAttachment` activation handles.

The scheduler also maintains daily twilight state, date-change notifications, and an environmental report interval used by the controller family.

## Host Tests

The native CMake checkpoint can be run without an Arduino connected:

```sh
./tests/run_tests.sh
```

or:

```sh
cmake -S tests -B build-host
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

The current native target is intentionally limited to the standalone hardening/core-logic layer. Broader controller and hardware-facing tests remain Arduino/development tests instead of pretending the complete Arduino dependency stack is a native host library.

## Setup

#### Header Defines

There are several defines inside of the controller's main `Terraduino.h` header file that allow for more fine-tuned control of the controller. You may edit and uncomment these lines directly, or supply them via custom build flags. While editing the main header file isn't ideal, it is often easiest. Note that editing the controller's main header file directly will affect all projects compiled on your system using those modified controller files.

Alternatively, you may also refer to https://forum.arduino.cc/index.php?topic=602603.0 on how to define custom build flags manually via modifying the `platform[.local].txt` file, or with the Arduino CLI (preferred way going forward).

For the older `platform.local.txt` file override approach, create `platform.local.txt` alongside `platform.txt` located in `%applocaldata%\Arduino15\packages\{platform}\hardware\{arch}\{version}\` (replacing `%applocaldata%\Arduino15` with `~/Library/Arduino15` for macOS, and `~/.arduino15` for Linux), with the contents: `compiler.cpp.extra_flags=-Dname` (replacing `name` with the full name of the define below). Note that it will affect all builds for that platform until again changed or removed. Some build systems may require directly editing `platform.txt` and adding onto the end of its CPP build recipe, e.g. Teensy and `recipe.cpp.o.pattern=<bunch-of-stuff> -Dname`.

From `Terraduino.h`:

```Arduino
// Uncomment or -D this define to completely disable usage of any multitasking commands and libraries. Not recommended.
//#define TERRA_DISABLE_MULTITASKING              // https://github.com/davetcc/TaskManagerIO

// Uncomment or -D this define to disable usage of tcMenu library, which will disable all GUI control. Not recommended.
//#define TERRA_DISABLE_GUI                       // https://github.com/davetcc/tcMenu

// Uncomment or -D this define to enable usage of the platform WiFi library, which enables networking capabilities.
//#define TERRA_ENABLE_WIFI                       // https://reference.arduino.cc/reference/en/libraries/wifi/

// Uncomment or -D this define to enable usage of the external serial AT WiFi library, which enables networking capabilities.
//#define TERRA_ENABLE_AT_WIFI                    // https://github.com/jandrassy/WiFiEspAT

// Uncomment or -D this define to enable usage of the platform Ethernet library, which enables networking capabilities.
//#define TERRA_ENABLE_ETHERNET                   // https://reference.arduino.cc/reference/en/libraries/ethernet/

// Uncomment or -D this define to enable usage of the Arduino MQTT library, which enables IoT data publishing capabilities.
//#define TERRA_ENABLE_MQTT                       // https://github.com/256dpi/arduino-mqtt

// Uncomment or -D this define to enable usage of the Adafruit GPS library, which enables GPS capabilities.
//#define TERRA_ENABLE_GPS                        // https://github.com/adafruit/Adafruit_GPS

// Uncomment or -D this define to enable external data storage (SD card or EEPROM) to save on sketch size. Required for constrained devices.
//#define TERRA_DISABLE_BUILTIN_DATA              // Disables library data existing in Flash, see DataWriter example for exporting details

// Uncomment or -D this define to enable debug output (treats Serial output as attached to serial monitor, waiting on start for connection).
//#define TERRA_ENABLE_DEBUG_OUTPUT

// Uncomment or -D this define to enable verbose debug output (note: adds considerable size to compiled sketch).
//#define TERRA_ENABLE_VERBOSE_DEBUG

// Uncomment or -D this define to enable debug assertions (note: adds significant size to compiled sketch).
//#define TERRA_ENABLE_DEBUG_ASSERTIONS
```

### External Libraries

The active build determines which dependencies are required. Core/common dependencies include ArduinoJson, ArxContainer, ArxSmartPtr, RTClib, Time, OneWire, DHT, I2C EEPROM support, and SolarCalculator. TaskManagerIO/IoAbstraction are used when multitasking is enabled. tcMenu is used when GUI support is enabled. MQTT, WiFi, Ethernet, and GPS libraries are only pulled in by their corresponding feature paths.

## Initialization

The controller is normally created globally and initialized near the top of `setup()`:

```Arduino
#include <Terraduino.h>

Terraduino terraController;

void setup()
{
    Serial.begin(115200);

    terraController.init(Terra_SystemMode_Automatic,
                         Terra_MeasurementMode_Metric);
    terraController.setSystemName("Homestead");

    auto tank = terraController.addWaterReservoir(2000.0f);
    auto level = terraController.addRemoteSensor(Terra_SensorType_Level,
                                                  Terra_UnitsType_LiqVolume_Liters);
    tank->getWaterVolumeSensorAttachment().setObject(level);

    terraController.launch();
}

void loop()
{
    terraController.update();
}
```

Objects created through `TerraFactory` receive the next open position-based identity and are registered with the active controller immediately.

## Persistence

Controller and object data use `TerraData` records and can be saved in JSON or binary form.

Common controller entry points include:

```Arduino
terraController.saveToJSONStream(&Serial, false);
terraController.saveToBinaryStream(&stream);
terraController.initFromJSONStream(&stream);
terraController.initFromBinaryStream(&stream);
```

EEPROM, SD card, and WiFiStorage helpers are also available when the matching storage path is enabled.

Attachments save identity strings rather than raw pointers so relationships can be resolved again after loading.

## Event Logging & Data Publishing

The controller exposes these subsystems directly:

```Arduino
terraController.scheduler;
terraController.logger;
terraController.publisher;
```

Examples:

```Arduino
terraController.enableSysLoggingToSDCard("logs/terra");
terraController.enableDataPublishingToSDCard("data/terra");
```

With MQTT enabled and a connected `MQTTClient`:

```Arduino
terraController.enableDataPublishingToMQTTClient(mqttClient);
```

The publisher tabulates registered sensor measurements by polling frame. The logger records controller/process events and is also the normal source for scheduler stage messages when debug output is enabled.

## Hardware Safety

Terraduino is supervisory control software, not a substitute for equipment safety systems.

* Use correctly rated relays, contactors, motor drivers, fuses, overload protection, and isolation.
* Use independent dry-run/pressure protection where required for pumps.
* Use independent over-temperature, pressure-relief, and combustion safety hardware for thermal equipment.
* Use suitable backflow protection and sanitation practices for potable or reclaimed water systems.
* A software sensor, timer, or controller should not be the only protection against flooding, fire, electrical fault, or equipment damage.

## License

Terraduino is released under the MIT license. See `LICENSE` for the full terms.
