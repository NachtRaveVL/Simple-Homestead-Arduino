# Terraduino
Terraduino: Simple Homestead Automation Controller.

**Simple-Homestead-Arduino v0.7.2.0**

Simple automation controller for homestead resource and environmental systems.
Licensed under the non-restrictive MIT license.

Created by NachtRaveVL, 2026.

This project is part of a four-library controller family: **Simple-Hydroponics-Arduino (Hydruino)**, **Simple-SolarTracker-Arduino (Helioduino)**, **Simple-Homestead-Arduino (Terraduino)**, and **Simple-AstroTracker-Arduino (Astruino)**.

This controller manages water storage and transfer, thermal storage, environmental sensing, pumps, valves, heaters, shared power rails, scheduling, logging, publishing, and persistent configuration for physical property infrastructure.

The Keep-It-Simple controller system:

* Can be used entirely offline with an RTC module for timekeeping and either a known static location or optional GPS module for location, or used online through enabled on-board WiFi/Ethernet or an external ESP-AT WiFi module.
  * Uses [SolarCalculator](https://github.com/jpb10/SolarCalculator), inspired by the NOAA Solar Calculator, for fine offline calculations of the sun's solar position (including sunrise, sunset, & transit times), accurate until 2100.
* Exportable system configuration to EEPROM, SD card, or WiFiStorage external storage device.
  * Saved in pretty-print JSON for human-readability & easy text editing, or in raw binary for compactness & speed.
  * Auto-save, backup auto-save (for auto-recovery), and low external storage space cleanup functionality.
  * Import string decode functions are pre-optimized with minimum spanning trie for ultra-fast text parsing & reduced loading times.
* Supports interval-based sensor data publishing and system event logging to MQTT IoT broker (for further IoT-integrated processing) or to external storage in .csv/.txt format (/w date in filename, segmented daily).
  * Can be extended to work with other JSON-based Web APIs or Client-like derivatives (for DB storage or server-endpoint support).
* Enabled GUI works with a large variety of common Arduino-compatible LCD/OLED/TFT displays, touchscreens, matrix keypads, analog joysticks, rotary encoders, and momentary buttons (support by [tcMenu](https://github.com/davetcc/tcMenuLib)).
  * Remote tcMenu connections can use enabled Serial, WiFi, Ethernet, or Simhub transports with compatible tcMenu clients such as [embedCONTROL](https://github.com/davetcc/tcMenu/releases).
  * GUI input handling supports interrupt-assisted or polling operation depending on the selected control type and the capabilities of the pins used.
  * System examples can be compiled in:
    * Disabled UI mode, which removes all GUI code entirely, freeing a large amount of Flash size for constrained (<=256kB Flash) devices.
    * Minimal UI mode, which saves on compiled sketch size through optimized code stripping at the cost of having to modify/re-upload a new sketch to change most system settings (or to change system object structure).
    * Full UI mode, which uses large amounts of Flash space available on modern MCUs to provide everything all at once, with only major system (or static linked component) changes requiring a sketch modify/re-upload.
* Supports finite and effectively infinite water and thermal reservoirs for modeling property resources.
* Transfer-capable pumps, valves, and circulators use source and destination reservoir attachments to define resource flow.
* Includes automatic reservoir scheduling through Assess, Fill, Condition, Distribute, and Settle stages.
* Supports local and remote sensor measurements for environmental and infrastructure monitoring.
* Actuator & Sensor pins can be multiplexed or expanded along with any control input pins through 8/16-bit i2c expanders for pin-limited controllers.

Designed primarily for Arduino and Arduino-compatible build environments. PlatformIO can also be used with supported Arduino cores for Espressif, Teensy, STM32, RP2040/RP2350, and other compatible targets.

*If this work is useful, project support is always appreciated through [Patreon](https://www.patreon.com/nachtrave).*

## About

The goal is to make homestead resource automation more accessible to DIY builders by using widely available, low-cost microcontrollers (MCUs).

Modern low-cost MCUs provide enough processing power, memory, and I/O to monitor reservoirs, move water, manage stored heat, record weather measurements, and coordinate property equipment. Homestead resource automation is a strong fit for these devices as a local controller, data logger, and process monitor. Commercial controller systems can cost hundreds or thousands of dollars, while DIY systems can be built for substantially less.

Terraduino is written primarily for Arduino and Arduino-compatible MCUs. It combines pumps, valves, relays, sensors, heaters, storage tanks, thermal stores, and other widely available low-cost hardware into a functional DIY property automation system. The physical implementation remains open to the builder.

## Controller Setup

### MCU Requirements

There is no single minimum MCU for every Terraduino build because enabled UI, networking, logging, sensor counts, reservoir counts, and automation complexity can change the program and memory requirements considerably.

As a practical starting point:

Minimum planning target: 256–512kB Flash, 16–24kB SRAM, 16MHz+

Recommended: 512kB–1MB+ Flash, 24–32kB+ SRAM, 32–48MHz+

Modern 32-bit boards such as Pico RP2040/RP2350, ESP32, Teensy 3.5+, STM32, GIGA, and Portenta-class devices are the natural starting point when automation, logging, UI, and networking are expected to run together.

Terraduino systems may need to service numerous sensors, pumps, valves, heaters, reservoirs, rails, and timed processes concurrently. Sensor polling, actuator response, control-loop activity, display load, logging, and communication traffic can therefore matter more than Flash size alone when selecting the MCU.

### Installation

Installation through the Arduino IDE Library Manager or a package manager such as PlatformIO is the simplest option. Manual installation consists of extracting the library into a `Simple-Homestead-Arduino` directory under the Arduino custom libraries directory, typically `[My ]Documents\Arduino\libraries` on Windows or `~/Documents/Arduino/libraries/` on Linux/macOS.

The Basic Homestead Example is the recommended starting point because it is the smallest complete system. The Full System Example is the larger integrated reference.

Storage-constrained MCUs (< 512kB Flash, particularly <= 256kB) may require smaller feature sets, adjusted max-size defines, or external EEPROM/SD storage; see the Data Writer Example. Modern MCUs with larger Flash and SRAM can enable more of the controller at once.

### Reservoirs and Scheduling

Terraduino models water and thermal resources through finite and effectively infinite reservoirs. Finite reservoirs use attached sensors and filled/high/low/empty triggers so hard end conditions can remain separate from normal operating thresholds.

Transfer-capable actuators connect resources through source and destination reservoir attachments. Pumps, valves, and circulators can therefore describe resource movement directly through their attached reservoirs.

The scheduler automatically coordinates reservoir processes through Assess, Fill, Condition, Distribute, and Settle stages.

### Host Tests

Host-side tests can be run with CMake:

```sh
cmake -S tests -B build-host
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

### Setup

#### Header Defines

Several defines inside the controller's main `Terraduino[UI].h` header file provide fine-grained control over optional features and build behavior. These may be edited directly or supplied through custom build flags. Editing the main header is often the simplest approach, but affects every project compiled against that modified library.

Custom build flags can also be supplied through the Arduino CLI or the older `platform.local.txt` override approach. See <https://forum.arduino.cc/index.php?topic=602603.0> for additional details.

For the older `platform.local.txt` override, create `platform.local.txt` alongside `platform.txt` in `%applocaldata%\Arduino15\packages\{platform}\hardware\{arch}\{version}\` (replace `%applocaldata%\Arduino15` with `~/Library/Arduino15` on macOS or `~/.arduino15` on Linux) and add `compiler.cpp.extra_flags=-Dname`, replacing `name` with the required define. This affects all builds for that platform until changed or removed. Some build systems, including Teensy, may instead require editing `platform.txt` and appending the define to the C++ build recipe.

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

From shared/TerraduinoUI.h:
```Arduino
// Uncomment or -D this define to enable usage of the XPT2046_Touchscreen library, in place of the Adafruit FT6206 library.
//#define TERRA_UI_ENABLE_XPT2046TS               // https://github.com/PaulStoffregen/XPT2046_Touchscreen

// Uncomment or -D this define to enable usage of the StChromaArt LDTC framebuffer capable canvas in place of default U8g2Drawable canvas (STM32/mbed only, note: requires advanced setup)
//#define TERRA_UI_ENABLE_STCHROMA_LDTC

// Uncomment or -D this define to enable usage of the StChromaArt BSP touch screen interrogator in place of the default AdaLibTouchInterrogator (STM32/mbed only, note: requires advanced setup, see tcMenu_Extra_BspUserSettings.h)
//#define TERRA_UI_ENABLE_BSP_TOUCH

// Uncomment or -D this define to enable usage of the debug menu
//#define TERRA_UI_ENABLE_DEBUG_MENU
```

#### External Libraries

Terraduino uses the following controller-side libraries depending on the enabled hardware and features:

* **ArduinoJson** for JSON configuration data.
* **ArxContainer** and **ArxSmartPtr** for container and shared-pointer support on Arduino targets.
* **DHT sensor library** and **Adafruit Unified Sensor** are current build dependencies.
* **I2C_EEPROM** for external I2C EEPROM storage.
* **RTClib** and **Time** for RTC and system time handling.
* **SolarCalculator** for offline solar position, sunrise, sunset, and transit calculations.
* **TaskManagerIO**, **IoAbstraction**, and **SimpleCollections** for multitasking and I/O support when multitasking is enabled.
* **Adafruit GPS** when GPS support is enabled.
* **MQTT** when MQTT publishing is enabled.
* **SD** plus the platform SPI/Wire support for local storage and buses.
* **WiFi101**, **WiFiNINA_Generic**, **WiFiEspAT**, or **Ethernet** when the matching optional network path is enabled.

Networking is optional. An offline Terraduino system does not need a WiFi, Ethernet, or MQTT library.

#### External UI Libraries

The optional tcMenu UI layer can use the following display and input libraries as required by the selected hardware:

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

### Initialization

There are several initialization mode settings exposed through this controller that are used for more fine-tuned control.

#### Class Instantiation

Instantiate the controller object before `setup()`, typically near the sketch's pin and device configuration. The constructor accepts optional hardware-device setup values; defaults select no external devices.

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

Call the controller object's `init[From…](…)` method from `setup()` to initialize a new system or load a saved configuration. For a new system, `init()` selects the system mode, measurement mode, control-input mode, and display-output mode. Defaults select an Automatic system using the default measurement units with control input and display output disabled.

From Terraduino.h, in class Terraduino:
```Arduino
    // Initializes default empty system. Typically called near top of setup().
    // See individual enums for more info.
    void init(Terra_SystemMode systemMode = Terra_SystemMode_Automatic,                 // What controller operating mode should be used
              Terra_MeasurementMode measureMode = Terra_MeasurementMode_Default,        // What units of measurement should be used
              Terra_DisplayOutputMode dispOutMode = Terra_DisplayOutputMode_Disabled,   // What display output mode should be used
              Terra_ControlInputMode ctrlInMode = Terra_ControlInputMode_Disabled);     // What control input mode should be used

    // Initializes system from EEPROM save, returning success flag
    // Set system data address with setSystemEEPROMAddress
    bool initFromEEPROM(bool jsonFormat = false);
    // Initializes system from SD card file save, returning success flag
    // Set config file name with setSystemConfigFilename
    bool initFromSDCard(bool jsonFormat = true);
#ifdef TERRA_USE_WIFI_STORAGE
    // Initializes system from a WiFiStorage file save, returning success flag
    // Set config file name with setSystemConfigFilename
    bool initFromWiFiStorage(bool jsonFormat = true);
#endif
    // Initializes system from custom JSON-based stream, returning success flag
    bool initFromJSONStream(Stream *streamIn);
    // Initializes system from custom binary stream, returning success flag
    bool initFromBinaryStream(Stream *streamIn);
```

The controller can also be initialized from a saved configuration, such as from an EEPROM or SD card, or other JSON or Binary stream. A saved configuration of the system can be made via the controller class object's `saveTo…(…)` methods, or called automatically on timer by setting an Autosave mode/interval.

From Terraduino.h, in class Terraduino:
```Arduino
    // Saves current system setup to EEPROM save, returning success flag
    // Set system data address with setSystemEEPROMAddress
    bool saveToEEPROM(bool jsonFormat = false);
    // Saves current system setup to SD card file save, returning success flag
    // Set config file name with setSystemConfigFilename
    bool saveToSDCard(bool jsonFormat = true);
#ifdef TERRA_USE_WIFI_STORAGE
    // Saves current system setup to WiFiStorage file save, returning success flag
    // Set config file name with setSystemConfigFilename
    bool saveToWiFiStorage(bool jsonFormat = true);
#endif
    // Saves current system setup to custom JSON-based stream, returning success flag
    bool saveToJSONStream(Stream *streamOut, bool compact = true);
    // Saves current system setup to custom binary stream, returning success flag
    bool saveToBinaryStream(Stream *streamOut);
```

### Event Logging & Data Publishing

After initialization, the controller can write timestamped system logs and sensor data for external analysis. Log entries record controller events, while data files can be imported into plotting tools or databases. File prefixes are typically used as subfolders and are appended with the date in `YYMMDD` format.

Serial logging output can also be enabled with `TERRA_ENABLE_DEBUG_OUTPUT`, described above under Header Defines.

Some embedded SD/FAT library configurations use 8.3 filenames, limiting file/folder names to eight characters plus a three-character extension. Long-filename support depends on the storage library and build configuration.

From Terraduino.h, in class Terraduino:
```Arduino
    // Enables system logging to the SD card. Log file names will append YYMMDD.txt to the specified prefix. Returns success flag.
    inline bool enableSysLoggingToSDCard(String logFilePrefix);

    // Enables data publishing to the SD card. Data file names will append YYMMDD.csv to the specified prefix. Returns success flag.
    inline bool enableDataPublishingToSDCard(String dataFilePrefix);
```

## Hookup Callouts

Many of the various electronic components and systems this controller is designed to work with may have specific setup procedures and/or wiring requirements. While advanced users may find this section a refresher at best, the below callouts are highlighted in order to help prevent device damage and ensure proper controller operation.

### General

* Use the supply and logic voltages required by the selected MCU and peripherals. Arduino-compatible hardware commonly uses either 5V or 3.3V logic, and many 3.3V devices are not 5V tolerant. Check the MCU, module, and IC datasheets before connecting signals.
* When two devices use incompatible logic levels, use a level-shifting method appropriate to the signal. A unidirectional resistor divider can be suitable for some slow 5V-to-3.3V signals, while bidirectional or high-speed buses generally need a proper level shifter. A single series resistor is not a general-purpose voltage converter.
* Never apply a signal outside the receiving pin's absolute-maximum and input-threshold specifications. Do not assume that a 3.3V HIGH will always satisfy a 5V input, or that a breakout board provides level shifting unless its documentation says so.

### Serial UART

Serial UART uses individual communication lines for each device, with the receive `RX` pin of one being the transmit `TX` pin of the other - thus having to "flip wires" when connecting. However, devices can always be active and never have to share their access. UART commonly operates from low kilobit/s rates into the hundreds of kilobits/s and is useful for simple point-to-point device control.

* When wiring up modules that use Serial UART, make sure to flip `RX`/`TX` pins.
* Always ensure that any data output pins and data input pins have compatible voltages.

Serial UART Devices Supported: Bluetooth-AT modules, ESP-AT WiFi modules, NMEA-AT GPS modules

### SPI Bus

SPI devices can be chained together on the same shared data lines, which are typically labeled `COPI` (or `MOSI`), `CIPO` (or `MISO`), and `SCK`, often with an additional `CS` (or `SS`). Each SPI device requires its own individual chip-select `CS` wire as only one SPI device may be active at any given time - accomplished by pulling its `CS` line of that device low (aka active-low). SPI runs at MHz speeds and is useful for large data block transfers.

* The `CS` pin may be connected to any digital output pin, but it's common to use the `CS` (or `SS`) pin for the first device. Additional devices are not restricted to what pin they can or should use, but given it's not a data pin not using a choice interrupt-capable pin allows those to be used for interrupt driven mechanisms.
* SD-card compatibility depends on the card, breakout hardware, voltage translation, and the storage library used by the target. FAT32 is widely supported on embedded Arduino setups, while maximum card size and long-filename support vary by library and configuration.
  * Validate the exact card and module combination on the target hardware before relying on it for configuration, logging, or autosave.
* Many various graphical displays may have an additional `DC` (or `RS`) pin, which is required to be connected to any open digital pin in addition to its `CS` pin.
  * There is often an additional `Reset` (or `RST`) pin that needs either wired to an open digital pin for MCU control, otherwise typically will need hard-tied to a HIGH signal (such as that from `Vcc`) in order for the display to function/turn-on.
  * There is also often an additional `LED` (or `BL`) pin that controls the backlight that can be either optionally wired to an open digital or analog pin for MCU control, otherwise can be hard-tied typically to a HIGH signal (such as that from `Vcc`) in order to stay always-on, or simply left disconnected for device default.
* Always ensure that any data output pins and data input pins have compatible voltages.

SPI Devices Supported: SD card modules, NMEA GPS modules, 128x128+ LCD/OLED/TFT graphical displays, XPT2046 touchscreens

### I2C Bus

I2C (aka I²C, IIC, TwoWire, TWI) devices share the same `SCL` and `SDA` lines. Every active device on a bus must have a non-conflicting address; whether two modules can coexist depends on their configurable address ranges, not simply on whether they are the same device type. I2C commonly runs at 100kHz or 400kHz, with other rates supported by some hardware.

* When addresses conflict, use the device's address-select pins/pads when available, place the devices on separate buses, or use an appropriate I2C multiplexer. Check the specific breakout and library documentation for supported addressing.
* Note that not all the I2C libraries used support multi-addressable I2C devices at this time (read as: may only use one). Currently, this restriction applies to: RTC devices.
* Always ensure that any data output pins and data input pins have compatible voltages.

I2C Devices Supported: DS*/PCF* RTC modules, AT24C* EEPROM modules, NMEA GPS modules, 16x2/20x4 LCD displays, 128x32/128x64 OLED displays, FT6206 touchscreens, 8/16-bit pin expanders

### OneWire Bus

Dallas/Maxim OneWire devices can share a single data line and are identified by a 64-bit ROM address. Some devices support parasite power, but externally powered operation is often more robust for larger or electrically noisy installations.

* Practical bus length and device count depend on cable capacitance, topology, pull-up strength, power mode, timing, and the devices in use. Validate the real wiring rather than relying on a fixed universal distance or device-count limit.
* When more than one OneWire device is present, the controller can identify devices by their 64-bit ROM address and may also use an enumeration position where supported. The ROM address is the stable identity; enumeration order can change if the bus population or topology changes.
* Always ensure that any data output pins and data input pins have compatible voltages.

DHT sensors use their own single-data-wire protocol and are not Dallas/Maxim OneWire devices.

### Analog IO

* Analog sensors must stay within the electrical input range of the MCU pin. A sensor that can output more voltage than the ADC input allows needs attenuation, level conversion, or a different interface before it is connected.
  * Altering a sensor's factory calibration or output range may require calibration references, fluids, or procedures specific to that device. Refer to its datasheet.
* ADC reference behavior is board-specific. `AREF` can select or accept an analog reference on boards that support it when configured appropriately; `IOREF` normally indicates the board's logic voltage and is not interchangeable with `AREF`. Check the board documentation before using either pin.
  * Applying voltage beyond a pin's rated range can permanently damage the MCU. Verify both the normal operating range and absolute-maximum rating.
* ADC-capable pins, PWM-capable pins, and supported ADC/DAC resolutions vary substantially by MCU and board. The `ADC_RESOLUTION` and `DAC_RESOLUTION` defines describe the configured conversion resolution; they do not imply that every GPIO supports analog input or output. Refer to the target board's pinout and datasheet.

### Sensors

* Many different kinds of hobbyist sensors label their analog output `AO` (or `Ao`) - however, always check your specific sensor's datasheet, as some may have non-standard pin designations.
  * Ensure the sensor output remains within the configured ADC input range and reference used by the target board.
* Many sensor modules expose a digital threshold output labeled `DO` (or `Do`). It is optional when software measurement triggers are sufficient, but it can be useful when the hardware threshold itself should be monitored.
  * Connect that output through a `TerraBinarySensor` when it should participate in the controller, optionally using an ISR-capable pin where appropriate.

### Pumps, Valves, and Thermal Equipment

* Use correctly rated relays, contactors, motor drivers, fuses, overload protection, and isolation.
* Use independent dry-run/pressure protection where required for pumps.
* Use independent over-temperature, pressure-relief, and combustion safety hardware for thermal equipment.
* Use suitable backflow protection and sanitation practices for potable or reclaimed water systems.
* A software sensor, timer, or controller should not be the only protection against flooding, fire, electrical fault, or equipment damage.

### Networking & Wireless

* Networking of any kind is 100% optional. Base controller operation works offline using an RTC for timekeeping and either an optional GPS receiver or known static location for location data.
  * WiFi or Ethernet can be enabled when remote control, MQTT, or network storage is wanted.
* Devices with built-in WiFi or Ethernet can enable such through header/build defines while other devices can utilize an external [serial ESP WiFi module](http://www.instructables.com/id/Cheap-Arduino-WiFi-Shield-With-ESP8266/) on any open Serial line.
  * Warning: While WiFi password is encrypted into system settings data, it should not be considered secure.
* Serial Bluetooth-AT modules can be used on any open Serial port to provide remote device control (only).
* MQTT requires remotely accessible broker daemon in order to publish sensor data (setup separately).
* Note: Geo-location APIs require external 3rd party monthly subscription fees, thus isn't included as a feature.

## Memory Callouts

* The total number of objects and different kinds of objects (reservoirs, pumps, valves, sensors, rails, etc.) that the controller can support at once depends on how much free Flash storage and SRAM your MCU has available.
  * For our supported microcontroller range, on the low end we have devices with 256kB of Flash and at least 16kB of SRAM, while on the upper end we have more modern devices with 1MB+ of Flash and 32kB+ of SRAM. Devices with < 24kB of SRAM may struggle with system builds and may be limited to minimal system setups (such as no WiFi, no data publishing, no built-in library data, only minimal-to-no GUI, etc.), while other newer devices with more capacity build with everything enabled.
* For AVR, SAM, and other build architectures that do not have C++11 STL (standard container library) support, there are a series of *`_MAXSIZE` defines nearer to the top of `Terra[UI]Defines.h` that can be modified to adjust how much memory space is allocated for the various static array structures the controller instead uses.
* To save on the cost of code size for constrained devices, focus on not enabling that which you won't need, which has the benefit of being able to utilize code stripping to remove sections of code that don't get used.
  * There are also header defines that can strip out certain libraries and functionality, such as ones that disable the GUI, multi-tasking subsystems, etc.
* To further save on code size cost, built-in data can be externalized onto an SD Card or EEPROM where supported.
  * Note: Upgrading between versions or changing custom/program data may require you to re-build and re-deploy to such external device.

## Object Model

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

`TerraRelayPumpActuator` is the transfer-capable binary actuator class. Its source and destination reservoir attachments describe resource flow. Pump, valve, and circulator actuator types use this transfer model.

`TerraVariableActuator` represents proportional outputs.

### Rails

`TerraSimpleRail` limits the number of simultaneously active linked loads. `TerraRegulatedRail` tracks a maximum power budget using an optional power sensor and limit trigger.

Software rails coordinate load capacity; they are not substitutes for fuses, breakers, overload protection, or correctly sized conductors.

### Triggers and Balancers

`TerraMeasurementValueTrigger` and `TerraMeasurementRangeTrigger` convert sensor measurements into trigger state with configurable de-trigger tolerance/delay.

`TerraLinearEdgeBalancer` drives increment/decrement actuator attachments around a sensor setpoint and target range. Balancers are subobjects used by higher-level control logic.

## Scheduler

`TerraScheduler` is not a generic user task scheduler. It automatically tracks registered reservoirs that have relevant actuator linkages.

The tracking stages are:

1. `Assess`
2. `Fill`
3. `Condition`
4. `Distribute`
5. `Settle`

A reservoir can be filled by an inbound transfer actuator, conditioned by a heater or local circulator, or distributed to another reservoir that needs resource. Actuator requests are maintained through the normal `TerraActuatorAttachment` activation-handle path.

The scheduler also maintains daily twilight state, date-change notifications, and environmental report timing.

## Example Usage

Below are several examples of controller usage.

### Basic Homestead System Example

The Basic Homestead Example shows how a small Terraduino system can be set up with a water reservoir and remote level sensor. Only the objects used by the sketch are built into the final binary, making it a lean starting point.

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

### Main System Examples

The supplied examples cover the main Terraduino system roles. The Basic Homestead Example above is the recommended starting point; the remaining examples provide focused references for additional controller features.

* **BasicHomestead** - Minimal controller lifecycle with water/thermal reservoirs and measurements.
* **FullSystem** - Integrated water transfer, thermal storage, remote data, scheduler operation, and console/debug logging.
* **ThermalStorage** - Thermal reservoir with attached temperature sensing.
* **RemoteSensor** - Transport-neutral remote reports and stale/offline tracking.
* **WeatherStation** - WiFi/MQTT sensor-station usage through the normal publisher.
* **DataWriter** - JSON persistence through the controller data model.

### Data Writer Example

The Data Writer Example demonstrates JSON persistence through the normal controller data model.

It initializes a small system, creates water and thermal reservoirs with attached remote sensors, and writes the resulting configuration as pretty-print JSON.

The same persistence path can be used with EEPROM, SD card, WiFiStorage, JSON streams, or binary streams when the matching storage path is enabled.

Serial logging output can also be enabled with `TERRA_ENABLE_DEBUG_OUTPUT`, described above under Header Defines.
