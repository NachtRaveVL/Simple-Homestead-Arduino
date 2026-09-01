# Terraduino
Terraduino: Simple Homestead Automation Controller.

**Simple-Homestead-Arduino v0.7.2.0**

Simple automation controller for homestead resource and environmental systems.  
Licensed under the non-restrictive MIT license.

Created by NachtRaveVL, 2026.

This controller manages water storage and transfer, thermal storage, environmental sensing, pumps, valves, heaters, shared power rails, scheduling, logging, publishing, and persistent configuration for physical property infrastructure. Terraduino uses the same controller, object registration, attachments, measurement, scheduler, logger, publisher, and persistence patterns established by Hydruino and Helioduino while applying them to homestead resource systems.

Our Keep-It-Simple controller system:

* Can be used entirely offline with an RTC module and optional GPS module (or known static location) for accurate time keeping, or used online through enabled on-board WiFi/Ethernet or an external ESP-AT WiFi module.
  * Network loss does not need to interrupt unrelated reservoir, actuator, sensor, or scheduler behavior.
* Exportable system configuration to EEPROM, SD card, or WiFiStorage external storage device.
  * Saved in pretty-print JSON for human-readability & easy text editing, or in raw binary for compactness & speed.
  * Attachments save object identity so relationships can be restored after loading.
* Supports interval-based sensor data publishing and system event logging to an MQTT IoT broker or to external storage in .csv/.txt format (/w date in filename, segmented daily).
  * Remote sensors remain transport-neutral; MQTT, serial radio, RS-485, CAN, ESP-NOW, LoRa, or application code can feed reports without changing the sensor model.
* Models the system with the same small set of registered object families used throughout the controller architecture.
  * Sensors provide measurements.
  * Actuators perform work.
  * Reservoirs represent stored or effectively infinite resources.
  * Rails limit shared electrical capacity.
* Uses attachments instead of parallel route/source object systems to describe relationships.
  * Transfer-capable actuators attach to source and destination reservoirs.
  * Reservoirs attach to sensors and filled/high/low/empty triggers.
  * Actuators can attach to parent reservoirs and power rails.
* Supports water and thermal reservoirs.
  * Finite reservoirs use attached sensors to track state.
  * Infinite reservoirs represent effectively inexhaustible sources or sinks such as a water main, drain, ambient environment, or external thermal source.
* Includes automatic reservoir scheduling.
  * Reservoir processes move through `Assess`, `Fill`, `Condition`, `Distribute`, and `Settle` stages.
  * Transfer decisions are derived from reservoir state and actuator attachments rather than a second route graph.
* Actuator & Sensor pins can be multiplexed or expanded along with control input pins through supported I/O abstraction hardware where appropriate.
* Networking, GUI support, remote sensors, and external services remain optional to normal local automation.

Made primarily for Arduino microcontrollers / build environments, but intended to fit PlatformIO, Espressif, Teensy, STM32, Pico/RP2040/RP2350, GIGA, Portenta, and similar MCU platforms. Practical size depends on enabled features and total object count.

*If you value the work that we do, our small team always appreciates a subscription to our [Patreon](www.patreon.com/nachtrave).*

## About

We want to make practical property automation more accessible to DIY'ers by utilizing the widely-available low-cost IoT and IoT-like microcontrollers (MCUs) of today.

Terraduino is aimed at physical homestead infrastructure rather than generic smart-home automation. Typical jobs include monitoring a storage tank, moving water between reservoirs, operating a sump pump, circulating stored heat, logging weather measurements, controlling heaters or ventilation, or preventing too many high-current loads from starting at once.

Terraduino is a MCU-based solution primarily written for Arduino and Arduino-like MCU devices. A small water-transfer system and a larger property controller can use the same identity, attachment, sensor, actuator, scheduler, logger, and persistence mechanisms. The controller does not require every installation to use the same hardware or communications path.

## Controller Setup

### MCU Requirements

There is no single minimum MCU for every Terraduino build because enabled UI, networking, logging, sensor counts, reservoir counts, and automation complexity can change the program and memory requirements considerably.

As a practical starting point:

Minimum planning target: 256–512kB Flash, 16–24kB SRAM, 16MHz+

Recommended: 512kB–1MB+ Flash, 24–32kB+ SRAM, 32–48MHz+

Modern 32-bit boards such as Pico RP2040/RP2350, ESP32, Teensy 3.5+, STM32, GIGA, and Portenta-class devices are the natural starting point when monitoring, logging, UI, networking, and several automated resource processes are expected to run together.

Terraduino systems may need to service numerous sensors, pumps, valves, heaters, reservoirs, power rails, and scheduled processes concurrently. Sensor polling, actuator response, balancing activity, display load, logging, and communication traffic can therefore matter more than Flash size alone when selecting the MCU.

### Installation

The easiest way to install this controller is to utilize the Arduino IDE library manager when available, or through a package manager such as PlatformIO. Otherwise, simply download this controller and extract its files into a `Simple-Homestead-Arduino` folder in your Arduino custom libraries folder, typically found in your `[My ]Documents\Arduino\libraries` folder (Windows), or `~/Documents/Arduino/libraries/` folder (Linux/OSX).

From there, make a local copy of one of the supplied example sketches based on the kind of system setup you want to use. If you are unsure of which, start with the Basic Homestead Example because it demonstrates the normal controller lifecycle, sensors, reservoirs, and attachments with the least surrounding complexity. The Full System Example is the larger reference implementation.

The current example set includes:

* **BasicHomestead** - Minimal controller lifecycle with water/thermal reservoirs and measurements.
* **DataWriter** - JSON/external-data persistence workflow.
* **RemoteSensor** - Transport-neutral remote reports and stale/offline tracking.
* **ThermalStorage** - Thermal reservoir with attached temperature sensing.
* **WeatherStation** - WiFi/MQTT sensor-station usage through the normal publisher.
* **FullSystem** - Integrated water transfer, thermal storage, remote data, scheduler operation, and console/debug logging.

Storage constrained MCUs (< 512kB Flash, particularly <= 256kB) may need further setup file/max-size tweaking and possibly external storage hardware such as EEPROM or SD card. Modern MCUs with more Flash and SRAM are strongly preferred when logging, networking, GUI, and many reservoir/sensor objects are expected to operate together.

### Reservoirs and Scheduling

Reservoirs are Terraduino's resource-storage abstraction.

Finite water and thermal reservoirs use attached sensors to track their current state. Filled/high/low/empty triggers remain separate so a system can distinguish hard end conditions from normal operating thresholds.

Transfer-capable actuators connect resources by attachment: the actuator's source reservoir and destination reservoir define the transfer path. This follows the same attachment-first design used throughout the controller family instead of inventing a separate route object model.

The scheduler automatically tracks registered reservoirs with relevant actuator linkages through:

1. `Assess`
2. `Fill`
3. `Condition`
4. `Distribute`
5. `Settle`

A reservoir can be filled from an inbound source, conditioned by local equipment, or distributed to another reservoir that needs resource. Activation is maintained through the normal actuator attachment/activation-handle mechanism.

### Host Tests

Core logic can be run without an Arduino connected:

```sh
./tests/run_tests.sh
```

or:

```sh
cmake -S tests -B build-host
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

The native target covers the standalone/core-logic layer. Hardware-facing controller behavior still requires the Arduino/platform environment and real-device testing.

### Setup

#### Header Defines

There are several defines inside of the controller's main `Terraduino.h` header file that allow for more fine-tuned control of the controller. You may edit and uncomment these lines directly, or supply them via custom build flags. While editing the main header file isn't ideal, it is often easiest. Note that editing the controller's main header file directly will affect all projects compiled on your system using those modified controller files.

Alternatively, you may also refer to <https://forum.arduino.cc/index.php?topic=602603.0> on how to define custom build flags manually via modifying the `platform[.local].txt` file, or with the Arduino CLI (preferred way going forward).

For the older `platform.local.txt` file override approach, create `platform.local.txt` alongside `platform.txt` located in `%applocaldata%\Arduino15\packages\{platform}\hardware\{arch}\{version}\` (replacing `%applocaldata%\Arduino15` with `~/Library/Arduino15` for OSX, and `~/.arduino15` for Linux), with the contents: `compiler.cpp.extra_flags=-Dname` (replacing `name` with full name of below define). Note that it will affect all builds for that platform until again changed/removed. Some build systems may require directly editing `platform.txt` and adding onto the end of its CPP build recipe, e.g. Teensy & `recipe.cpp.o.pattern=<bunch-of-stuff> -Dname`.

From Terraduino.h:
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

Unlike Hydruino and Helioduino, the current `develop` version of `shared/TerraduinoUI.h` does not yet expose the sibling libraries' additional UI feature-switch defines. UI-specific build flags should therefore only be documented here once they actually exist in the Terraduino UI implementation.

#### External Libraries

Terraduino uses the following controller-side libraries depending on the enabled hardware and features:

* **ArduinoJson** for JSON configuration data.
* **ArxContainer** and **ArxSmartPtr** for container and shared-pointer support on Arduino targets.
* **DHT sensor library** and **OneWire** for supported environmental/sensor paths.
* **I2C_EEPROM** for external I2C EEPROM storage.
* **RTClib** and **Time** for RTC and system time handling.
* **SolarCalculator** for sunrise/sunset and daily environmental timing where used.
* **TaskManagerIO** and **IoAbstraction** for multitasking and I/O support when multitasking is enabled.
* **Adafruit GPS** when GPS support is enabled.
* **MQTT** when MQTT publishing is enabled.
* **SD** plus the platform SPI/Wire support for local storage and buses.
* **WiFi101**, **WiFiNINA_Generic**, **WiFiEspAT**, or **Ethernet** when the matching optional network path is enabled.

Networking is optional. An offline Terraduino system does not need WiFi, Ethernet, or MQTT for normal reservoir, sensor, actuator, scheduling, or local logging behavior.

#### External UI Libraries

Terraduino follows the same tcMenu-based UI architecture as the controller family, but the Terraduino-specific UI layer on `develop` is still being ported toward Hydruino/Helioduino parity.

The controller already carries the family display/control-input modes, but the current shared Terraduino UI wrapper remains a placeholder. Do not document Hydruino/Helioduino-specific UI feature defines or imply complete UI feature parity until the corresponding Terraduino implementation exists.

### Initialization

There are several initialization mode settings exposed through this controller that are used for more fine-tuned control.

#### Class Instantiation

The controller's class object must first be instantiated, commonly at the top of the sketch where pin setups are defined. The constructor configures controller-level devices and interfaces, with defaults providing no optional device specified.

From Terraduino.h, in class Terraduino:
```Arduino
    // Controller constructor. Typically called during class instantiation, before setup().
    Terraduino(pintype_t piezoBuzzerPin = -1,                       // Piezo buzzer pin, else -1
               Terra_EEPROMType eepromType = Terra_EEPROMType_None, // EEPROM device type/size, else None
               DeviceSetup eepromSetup = DeviceSetup(),             // EEPROM device setup (i2c only)
               Terra_RTCType rtcType = Terra_RTCType_None,          // RTC device type, else None
               DeviceSetup rtcSetup = DeviceSetup(),                // RTC device setup (i2c only)
               DeviceSetup sdSetup = DeviceSetup(),                 // SD card device setup (spi only)
               DeviceSetup netSetup = DeviceSetup(),                // Network device setup (spi/uart)
               DeviceSetup gpsSetup = DeviceSetup(),                // GPS device setup (uart/i2c/spi)
               pintype_t *ctrlInputPins = nullptr,                  // Control input pins, else nullptr
               DeviceSetup displaySetup = DeviceSetup());           // Display device setup (i2c/spi)
```

#### Controller Initialization

Additionally, a call is expected to be provided to the controller class object's `init[From…](…)` method, commonly called inside of the sketch's `setup()` function. This allows one to set the controller's system mode, units of measurement, control input mode, and display output mode.

From Terraduino.h, in class Terraduino:
```Arduino
    // Initializes default empty system. Typically called near top of setup().
    // See individual enums for more info.
    void init(Terra_SystemMode systemMode = Terra_SystemMode_Automatic,
              Terra_MeasurementMode measureMode = Terra_MeasurementMode_Default,
              Terra_DisplayOutputMode dispOutMode = Terra_DisplayOutputMode_Disabled,
              Terra_ControlInputMode ctrlInMode = Terra_ControlInputMode_Disabled);

    bool initFromEEPROM(bool jsonFormat = false);
    bool initFromSDCard(bool jsonFormat = true);
#ifdef TERRA_USE_WIFI_STORAGE
    bool initFromWiFiStorage(bool jsonFormat = true);
#endif
    bool initFromJSONStream(Stream *streamIn);
    bool initFromBinaryStream(Stream *streamIn);
```

The controller can also be initialized from a saved configuration, such as from EEPROM or SD card, or another JSON/Binary stream. A saved configuration can be made through the matching `saveTo…(…)` methods or through configured autosave behavior.

A normal lifecycle is:

```Arduino
#include <Terraduino.h>

Terraduino terraController;

void setup()
{
    terraController.init(Terra_SystemMode_Automatic,
                         Terra_MeasurementMode_Metric);

    terraController.setSystemName("Homestead");

    // Add reservoirs, sensors, actuators, and rails here.

    terraController.launch();
}

void loop()
{
    terraController.update();
}
```

### Event Logging & Data Publishing

The controller can, after initialization, produce logs and sensor data that can be used by other applications. Log entries are timestamped and can track scheduler stages, actuator state changes, warnings, and other controller events, while published data can be read into plotting applications or exported to a database for further processing.

Note: The same logging output can also be sent to the Serial device by defining `TERRA_ENABLE_DEBUG_OUTPUT`, described above in Header Defines.

The controller exposes:

```Arduino
terraController.scheduler;
terraController.logger;
terraController.publisher;
```

Local SD-card logging and publishing are available through:

```Arduino
terraController.enableSysLoggingToSDCard("logs/te");
terraController.enableDataPublishingToSDCard("data/te");
```

WiFiStorage and MQTT publishing are available when the matching feature paths are enabled.

The publisher tabulates registered sensor measurements by polling frame. Scheduler stage messages use the normal logger/debug-output path rather than a second console-reporting mechanism.

## Current Object Model

### Sensors

`TerraSensor` is the common measurement object. Concrete sensor paths include binary, analog, remote, and convenience sensor types for environmental/resource measurements.

Remote sensors remain transport-neutral. MQTT, serial radio, RS-485, CAN, ESP-NOW, LoRa, or another application transport can call `receiveReport()` without changing the sensor model.

### Reservoirs

Reservoirs are the resource-storage abstraction:

* `TerraWaterReservoir`
* `TerraThermalReservoir`
* `TerraInfiniteWaterReservoir`
* `TerraInfiniteThermalReservoir`

Finite reservoirs use attached sensors and separate filled/high/low/empty triggers. The separation is intentional: a system can distinguish a hard empty/full condition from normal low/high operating thresholds.

### Actuators

`TerraActuator` resolves resident activation requests according to its enable mode.

`TerraRelayActuator` represents ordinary on/off equipment.

`TerraRelayPumpActuator` is the current transfer-capable binary actuator class. Its source and destination reservoir attachments describe resource flow. Pump/valve/circulator semantics should be represented through the established actuator type and attachment model rather than by creating a parallel route graph.

`TerraVariableActuator` represents proportional outputs. Variable/throttled pump support remains a planned port alongside the corresponding Hydruino implementation.

### Rails

`TerraSimpleRail` limits the number of simultaneously active linked loads. `TerraRegulatedRail` tracks a maximum power budget using an optional power sensor and limit trigger.

Software rails coordinate load capacity; they are not substitutes for fuses, breakers, overload protection, or correctly sized conductors.

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

A reservoir can be filled by an inbound transfer actuator, conditioned by a heater or local circulator, or distributed to another reservoir that needs resource. Actuator requests are maintained through the normal `TerraActuatorAttachment` activation-handle path.

The scheduler also maintains daily twilight state, date-change notifications, and the controller-family environmental report interval.

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

## Hookup Callouts

Many of the electronic components and systems this controller is designed to work with have specific setup procedures and wiring requirements. The below callouts are intended to help prevent device damage and ensure reliable controller operation.

### General

* Verify MCU and peripheral logic voltage before connection.
* Do not power pumps, valves, heaters, relays, contactors, motors, or similar loads directly from MCU pins.
* Use suitable drivers, relay/MOSFET interfaces, fusing, grounding, isolation, and properly rated power supplies.
* Software state, timing, and sensor checks are secondary protections and should not be the only protection against flooding, overheating, fire, electrical fault, or equipment damage.

### Serial UART

* When wiring modules that use Serial UART, connect device TX to controller RX and device RX to controller TX.
* Always ensure that data output/input logic voltages are compatible.

Serial UART devices can include Bluetooth-AT modules, ESP-AT WiFi modules, GPS modules, and transport devices used by application-level remote sensors.

### SPI Bus

* Each SPI device normally requires its own `CS`/`SS` line while sharing the main data/clock lines.
* SD-card, display, Ethernet, and other device requirements vary by module and MCU.
* Always ensure that data output/input logic voltages are compatible.

### I2C Bus

* Devices sharing an I2C bus must use non-conflicting addresses.
* Check pull-up voltage and total bus length/capacitance.
* Long property runs may require slower buses, buffering, differential interfaces, or a different transport rather than stretching local I2C beyond its practical use.

### OneWire Bus

* OneWire sensors can share a data line when the selected device/library supports the required topology.
* Verify pull-up voltage and cable length for the installation.
* Keep pump/motor/heater wiring away from sensitive sensor lines where practical.

### Analog IO

* Analog input voltage must remain within the MCU's supported ADC/input range.
* Use suitable scaling, conditioning, and protection for higher-voltage pressure, level, current, or other transducers.
* Calibrate sensors in the units actually used by the controller.

### Pumps, Valves, and Thermal Equipment

* Use correctly rated relays, contactors, motor drivers, fuses, overload protection, and isolation.
* Use independent dry-run/pressure protection where required for pumps.
* Use independent over-temperature, pressure-relief, and combustion safety hardware for thermal equipment.
* Use suitable backflow protection and sanitation practices for potable or reclaimed water systems.

### Networking & Wireless

* Networking is optional. Base controller operation works locally.
* WiFi or Ethernet can be enabled when remote control, MQTT, network storage, or other remote functionality is wanted.
* Remote sensor transport should be chosen for the distance/noise/environment rather than forcing a local bus protocol into a long-run application.

## Memory Callouts

* The number of reservoirs, sensors, actuators, rails, attachments, GUI objects, networking features, logging buffers, and debug options all affect Flash/SRAM usage.
* Disabling unused GUI/network/debug functionality allows the compiler to remove code that is not needed.
* On architectures without normal STL support, the `TERRA_*_MAXSIZE` values in the defines headers control several fixed-capacity containers and may need tuning for the intended build.
* Larger 32-bit MCUs are recommended when many resource, sensor, and communication subsystems are active together.

## Example Usage

A small system follows the same lifecycle used by the other controller libraries:

```Arduino
#include <Terraduino.h>

Terraduino terraController;

void setup()
{
    terraController.init(Terra_SystemMode_Automatic,
                         Terra_MeasurementMode_Metric);

    auto tank = terraController.addWaterReservoir(2000.0f);
    auto level = terraController.addRemoteSensor(
        Terra_SensorType_Level,
        Terra_UnitsType_LiqVolume_Liters);

    tank->getWaterVolumeSensorAttachment().setObject(level);

    terraController.launch();
}

void loop()
{
    terraController.update();
}
```

The supplied examples provide focused demonstrations of thermal storage, remote sensors, weather publishing, persistence, and a larger integrated system.

### Data Writer Example

The Data Writer Example demonstrates the controller family's external-data/persistence workflow without requiring a complete homestead system to be operating. Use it when preparing data for constrained builds or when validating storage output independently from the main automation sketch.

## License

Terraduino is released under the MIT License. See `LICENSE` for details.
