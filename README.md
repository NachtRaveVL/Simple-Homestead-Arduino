# Terraduino

**Simple-Homestead-Arduino v0.7.0.0**

Terraduino is a simple automation controller for homestead resource and environmental systems using Arduino-compatible microcontrollers.

The library is intended for practical DIY property infrastructure rather than generic smart-home automation. It focuses on the systems that keep a homestead supplied, monitored, and operating locally even when no network connection is available.

## What Terraduino manages

Terraduino currently provides reusable controller objects and logic for:

* Rainwater collection, cisterns, wells, surface water, reclaimed water, and stored water sources
* Water source priority, reserve levels, storage capacity, transfer hysteresis, and flow-fault detection
* Pumps, valves, diverters, heaters, circulators, digital outputs, and variable outputs
* Water routing between sources and storage with start and stop thresholds that avoid rapid cycling
* First-flush rainwater diversion tracking
* Thermal storage, target temperature ranges, differential circulation, and upper temperature limits
* Weather and environmental observations including temperature, humidity, pressure, rainfall, rainfall rate, wind, and solar irradiance
* Dew point and freezing-condition evaluation when the required observations are valid
* Local, analog, digital, callback-driven, and remote sensor inputs
* Remote sensor freshness and offline detection without requiring a specific network transport
* Threshold and range triggers with hysteresis
* Interval and daily scheduling
* Logging and measurement publishing
* JSON configuration and a versioned checksummed binary system record
* Persisted controller preferences for measurement units, logging threshold, and publishing cadence
* Object registration, typed factory reconstruction, persisted attachments, modules, power rails, and driver abstractions
* Persisted pin-backed sensor/actuator setup including active-low outputs, PWM scale, and analog calibration
* tcMenu display, input, remote transport, font, and theme adapter files carried over from the established controller libraries
* Terraduino-specific menu, overview, UI-data, and min/full UI setup files retained as explicit TODO scaffolding while the homestead menus are designed
* Unit conversion for the measurements used by the homestead systems

## Networking is optional

Terraduino does not require WiFi, Ethernet, MQTT, a cloud account, or any other network service for normal local operation.

A remote sensor can feed a `TerraRemoteSensor` through MQTT, ESP-NOW, LoRa, RS-485, CAN, serial, or another transport chosen by the project. The transport is intentionally separate from the sensor object. Loss of that link can mark the sensor stale without preventing unrelated local control from continuing. Transport callbacks and other function pointers are runtime application wiring rather than serialized configuration.

## Safety boundary

Terraduino is supervisory software. It is not a substitute for safety-rated hardware.

Installations must still use the protection appropriate to the equipment involved. Examples include pressure relief, dry-run and overload protection, thermal cutoffs, combustion controls, backflow prevention, mains isolation, overcurrent protection, and other code-required or manufacturer-required safeguards.

The library is designed to command suitable external interfaces. It does not make high voltage, high current, pressure, combustion, or stored-energy systems safe by itself.

## Project layout

The core follows the same general controller architecture as the related automation libraries while keeping the homestead-specific domain separate.

Useful starting points are:

* `src/Terraduino.*` - main controller lifecycle
* `src/TerraObject.*` and `src/TerraFactory.*` - object registration and lookup
* `src/TerraSensors.*` and `src/TerraDrivers.*` - measurements and hardware/transport input abstraction
* `src/TerraActuators.*` - output objects
* `src/TerraWater.*` - water sources, storage, routes, and first-flush handling
* `src/TerraThermal.*` - thermal stores and differential circulation loops
* `src/TerraEnvironment.*` - weather and environmental observations
* `src/TerraBalancers.*` - water and thermal balancing decisions
* `src/TerraScheduler.*` - interval and daily tasks
* `src/TerraLogger.*` and `src/TerraPublisher.*` - logs and measurement output
* `src/TerraData.*` and `src/TerraDatas.*` - system and object persistence records
* `src/TerraStrings.*` and `src/TerraEnumTrie.h` - enum encoding plus generated minimum-trie decoding
* `src/shared/`, `src/min/`, and `src/full/` - tcMenu adapter layer plus Terraduino-specific UI setup scaffolding
* `tests/EnumTrieExportToCPP/` and `tests/generate_enum_trie.py` - enum decoder generation tools


## User interface status

The shared tcMenu adapter files are present so Terraduino keeps the same UI integration layout as the related controller libraries. The Terraduino-specific menu definitions, overview screens, UI data, and min/full UI classes are intentionally left with TODO markers until the homestead interface is designed. Core water, thermal, environmental, scheduling, logging, persistence, sensor, and actuator behavior does not depend on the unfinished GUI.

## Basic use

```cpp
#include <Terraduino.h>

Terraduino terra;
TerraCistern cistern(5000.0f, 0, "Main Cistern");
TerraEnvironment weather(0, "Outside");

void setup() {
    TerraSystemSetup setup;
    setup.systemName = "Homestead";
    setup.controlMode = Terra_ControlMode_Automatic;

    terra.init(setup);
    cistern.setThresholds(15.0f, 30.0f, 95.0f);
    cistern.configureFillBand(35.0f, 90.0f, 99.0f);

    terra.registerObject(&cistern);
    terra.registerObject(&weather);
    terra.launch();
}

void loop() {
    terra.update();
}
```

## Examples

* `BasicHomestead` shows the controller lifecycle with weather, cistern, and thermal-store objects.
* `CisternManagement` shows fill hysteresis, reserve protection, source limits, and pump control.
* `RainwaterCollection` shows roof catchment, first flush, and overflow-aware cistern storage.
* `ThermalStorage` shows differential circulation into a stored-heat system.
* `WeatherStation` shows the local environmental snapshot used for monitoring and control.
* `RemoteSensor` shows the transport-neutral remote measurement path.
* `FullSystem` combines the main water, thermal, environmental, remote-sensor, and publishing pieces.
* `UISetup` keeps the tcMenu setup pattern visible while the Terraduino-specific menus remain TODO work.

## Tests

The core logic can be built and tested on a normal desktop compiler without Arduino hardware:

```sh
./tests/run_tests.sh
```

The CMake build also compiles and links all shipped examples against the same public API. This catches API drift in examples before hardware testing.

Host tests cover resource thresholds, cistern fill/overflow behavior, rain capture and first flush, transfer hysteresis, source priority, flow faults, thermal differential control, weather validity, sensors, remote-sensor staleness, actuator request aggregation and runtime faults, scheduling, logging, publishing, JSON and binary persistence, pin/calibration persistence, attachment reconstruction, runtime factory reconstruction, units, and all 20 enum/string conversion families. The build also checks that the generated minimum trie is current and compiles with `-Wfloat-equal -Werror`.

Real hardware validation is still required for electrical interfaces, sensor accuracy, pump and valve behavior, flow-meter calibration, pressure systems, thermal equipment, radio/network reliability, and installation-specific safety behavior.

## License

Terraduino is released under the MIT License. See `LICENSE` for details.
