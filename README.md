# Terraduino
Terraduino: Simple Homestead Automation Controller.

**Simple-Homestead-Arduino v0.7.2.0**

Simple automation controller for homestead resource and environmental systems.
Licensed under the non-restrictive MIT license.

Created by NachtRaveVL, 2026.

This project is part of a four-library controller family: **Simple-Hydroponics-Arduino (Hydruino)**, **Simple-SolarTracker-Arduino (Helioduino)**, **Simple-Homestead-Arduino (Terraduino)**, and **Simple-AstroTracker-Arduino (Astruino)**.

This controller manages water storage and transfer, thermal storage, environmental sensing, pumps, valves, heaters, shared power rails, scheduling, logging, publishing, and persistent configuration for physical property infrastructure.

The Keep-It-Simple controller system:

* Can be used entirely offline with RTC module and optional GPS module (or known static location) for accurate time keeping, or used online through enabled on-board WiFi/Ethernet or external ESP-AT WiFi module.
  * Uses [SolarCalculator](https://github.com/jpb10/SolarCalculator), inspired by the NOAA Solar Calculator, for fine offline calculations of the sun's solar position (including sunrise, sunset, & transit times), accurate until 2100.
* Exportable system configuration to EEPROM, SD card, or WiFiStorage external storage device.
  * Saved in pretty-print JSON for human-readability & easy text editing, or in raw binary for compactness & speed.
  * Auto-save, backup auto-save (for auto-recovery), and low external storage space cleanup (TODO) functionality.
  * Import string decode functions are pre-optimized with minimum spanning trie for ultra-fast text parsing & reduced loading times.
* Supports interval-based sensor data publishing and system event logging to MQTT IoT broker (for further IoT-integrated processing) or to external storage in .csv/.txt format (/w date in filename, segmented daily).
  * Can be extended to work with other JSON-based Web APIs or Client-like derivatives (for DB storage or server-endpoint support).
  * Can add a piezo buzzer for audible system warning/failure alerting (TODO), or a LCD/OLED/TFT display for current readings & recent logging messages (TODO).
* Enabled GUI works with a large variety of common Arduino-compatible LCD/OLED/TFT displays, touchscreens, matrix keypads, analog joysticks, rotary encoders, and momentary buttons (support by [tcMenu](https://github.com/davetcc/tcMenuLib)).
  * Contains at-a-glance system overview screen and interactive menu system for system configuration, sensor calibration, and more (TODO).
  * Critical system config menus can be pin-coded to prevent setup tampering.
  * Includes remote GUI menu access through enabled WiFi, Ethernet, Bluetooth, Serial, and/or Simhub connection via tcMenu's excellent [embedCONTROL](https://github.com/davetcc/tcMenu/releases) desktop application, available for Linux/OSX/Windows.
  * GUI I/O pins can be setup as fully interrupt driven (5-25ms latency), partially interrupt driven (only keys & buttons polled), or polling based (75-100ms+ latency), and can be automatically selected depending on pins used.
  * System examples can be compiled in:
    * Disabled UI mode, which removes all GUI code entirely, freeing a large amount of Flash size for constrained (<=256kB Flash) devices.
    * Minimal UI mode, which saves on compiled sketch size through optimized code stripping at the cost of having to modify/re-upload a new sketch to change most system settings (or to change system object structure).
    * Full UI mode, which uses large amounts of Flash space available on modern MCUs to provide everything all at once, with only major system (or static linked component) changes requiring a sketch modify/re-upload.
* Supports finite and effectively infinite water and thermal reservoirs for modeling property resources.
* Transfer-capable pumps, valves, and circulators use source and destination reservoir attachments to define resource flow.
* Includes automatic reservoir scheduling through Assess, Fill, Condition, Distribute, and Settle stages.
* Supports local and remote sensor measurements for environmental and infrastructure monitoring.
* Actuator & Sensor pins can be multiplexed or expanded along with any control input pins through 8/16-bit i2c expanders for pin-limited controllers.

Designed primarily for Arduino and Arduino-compatible build environments. PlatformIO, Espressif, Teensy, STM32, Pico, and other supported toolchains may also be used.

*If this work is useful, project support is always appreciated through [Patreon](www.patreon.com/nachtrave).*

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
* **DHT sensor library** and **Adafruit Unified Sensor** for DHT environmental sensors.
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

FAT32-based SD cards use 8.3 filenames, limiting file/folder names to eight characters plus a three-character extension.

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

* The recommended Vcc power supply and logic level is 5v, with most newer MCUs restricted to 3.3v.
  * There are many devices that are 3.3v only and not 5v tolerant. Check your IC's datasheet for details.
* 5v device output pins that interface with any 3.3v device input pins that are not 5v tolerant (such as a 5v AVR interfacing with a 3.3v-only [serial ESP-AT WiFi module](http://www.instructables.com/id/Cheap-Arduino-WiFi-Shield-With-ESP8266/), or a 3.3v MCU interfacing with a 5v analog sensor), will require a bi-directional logic level converter/shifter to use, especially for any high-speed digital data transfer lines.
  * Alternatively, using a 10kΩ resistor can often times be enough to 'convert' 5v to 3.3v, but the correct way is to utilize a 1kΩ resistor and a 2kΩ resistor (or any size with a 1:2 ratio) in a [simple voltage divider circuit](https://randomnerdtutorials.com/how-to-level-shift-5v-to-3-3v/).

### Serial UART

Serial UART uses individual communication lines for each device, with the receive `RX` pin of one being the transmit `TX` pin of the other - thus having to "flip wires" when connecting. However, devices can always be active and never have to share their access. UART runs at low to mid kHz speeds and is useful for simple device control, albeit somewhat clumsy at times.

* When wiring up modules that use Serial UART, make sure to flip `RX`/`TX` pins.
* Always ensure that any data output pins and data input pins have compatible voltages.

Serial UART Devices Supported: Bluetooth-AT modules, ESP-AT WiFi modules, NMEA-AT GPS modules

### SPI Bus

SPI devices can be chained together on the same shared data lines, which are typically labeled `COPI` (or `MOSI`), `CIPO` (or `MISO`), and `SCK`, often with an additional `CS` (or `SS`). Each SPI device requires its own individual cable-select `CS` wire as only one SPI device may be active at any given time - accomplished by pulling its `CS` line of that device low (aka active-low). SPI runs at MHz speeds and is useful for large data block transfers.

* The `CS` pin may be connected to any digital output pin, but it's common to use the `CS` (or `SS`) pin for the first device. Additional devices are not restricted to what pin they can or should use, but given it's not a data pin not using a choice interrupt-capable pin allows those to be used for interrupt driven mechanisms.
* Many low-cost SPI-based SD card modules on market only read SDHC sized SD cards (2GB to 32GB) formatted in FAT32 (filenames limited to 8 characters plus 3 character file extension).
  * Some SD cards simply will not play nicely with these modules and you may have to try another SD card manufacturer. We recommend 32GB SD cards due to overall lowest cost (smaller SD cards actually becoming _more_ expensive).
* Many various graphical displays may have an additional `DC` (or `RS`) pin, which is required to be connected to any open digital pin in addition to its `CS` pin.
  * There is often an additional `Reset` (or `RST`) pin that needs either wired to an open digital pin for MCU control, otherwise typically will need hard-tied to a HIGH signal (such as that from `Vcc`) in order for the display to function/turn-on.
  * There is also often an additional `LED` (or `BL`) pin that controls the backlight that can be either optionally wired to an open digital or analog pin for MCU control, otherwise can be hard-tied typically to a HIGH signal (such as that from `Vcc`) in order to stay always-on, or simply left disconnected for device default.
* Always ensure that any data output pins and data input pins have compatible voltages.

SPI Devices Supported: SD card modules, NMEA GPS modules, 128x128+ LCD/OLED/TFT graphical displays, XPT2046 touchscreens

### I2C Bus

I2C (aka I²C, IIC, TwoWire, TWI) devices can be chained together on the same shared data lines (no flipping of wires), which are typically labeled `SCL` and `SDA`. Only different kinds of I2C devices can be used on the same data line together using factory default settings, otherwise manual addressing must be performed. I2C runs at mid to high kHz speeds and is useful for advanced device control.

* When more than one I2C device of the same kind is to be used on the same data line, each device must be set to use a different address. This is accomplished via the A0-A2 (sometimes A0-A5) pins/pads on the physical device that must be set either open or closed (typically via a de-solderable resistor, or by shorting a pin/pad). Check your specific breakout's datasheet for details.
* Note that not all the I2C libraries used support multi-addressable I2C devices at this time (read as: may only use one). Currently, this restriction applies to: RTC devices.
* Always ensure that any data output pins and data input pins have compatible voltages.

I2C Devices Supported: DS*/PCF* RTC modules, AT24C* EEPROM modules, NMEA GPS modules, 16x2/20x4 LCD displays, 128x32/128x64 OLED displays, FT6206 touchscreens, 8/16-bit pin expanders

### OneWire Bus

OneWire devices can be chained together on the same shared data lines (no flipping of wires). Devices can be of the same or different types, require minimal setup (and often no soldering), and most can even operate in "parasite" power mode where they use the power from the data line (and an internal capacitor) to function (thus saving a `Vcc` line, only requiring `Data` and return `GND`). OneWire runs only in the low kb/s speeds and is useful for light-weight digital sensors.

* Typically, sensors are limited to 20 devices along a maximum 100m of wire.
* When more than one OneWire device is on the same device line, each device registers itself an enumeration index (0 - N) along with its own 64-bit unique identifier (UUID, with last byte being CRC). The device can then be referenced via this UUID by the system in the future indefinitely, or enumeration index so long as the device doesn't change its line position.
* Always ensure that any data output pins and data input pins have compatible voltages.

OneWire Devices Supported: DHT* 1W air temp/humidity sensors

### Analog IO

* All analog sensors will need to have the same operational voltage range as the controller supports. Many analog sensors are set to use 0v to 5v by default, but some can go -5v to +5v, some even up to 5.5v.
  * Note: Altering default factory calibration settings may require addition tools for setting up a new calibration, such as special calibration fluids/procedures/etc. Refer to the datasheet of your device for details.
* The `AREF` (or `IOREF`) pin, which controls the upper-bound of this range, by default if left not-connected (NC) is the same voltage as the MCU. Analog sensors must not exceed this voltage limit.
  * 5v analog sensor output signals connecting to 3.3v MCUs that are not 5v tolerant **must** either be: [level converted](https://randomnerdtutorials.com/how-to-level-shift-5v-to-3-3v/) in order to connect, or configured to output 0v to `AREF` (or `IOREF`) in voltage calibration output range (if able to calibrate - see note above).
  * Warning: Too high of applied voltage to any pin incapable of receiving such high a voltage risks permanent damage to that device. _Always_ ensure that the applied voltage level coming out a device is supported when going back into another. Some breakouts/IC's have 5v tolerance built-in, some do not. Refer to the datasheet of your MCU/device for details.
  * Note: Typically a 3.3v output signal will _not_ need level converted up to 5v for a 5v digital input to operate (read as: 3.3v is plenty enough to trigger HIGH on 5v device inputs).
* The SAM/SAMD family of MCUs (e.g. Due, Zero, MKR, Nano 33, etc.) as well as many more modern MCUs support different bit resolutions for analog/PWM pins (tied to overridable `DAC_RESOLUTION` & `ADC_RESOLUTION` defines), with some (e.g. Pico, ESP32, etc.) supporting any pin being digital or analog w/o restriction. Refer to the datasheet of your MCU for details.

### Sensors

* Many different kinds of hobbyist sensors label their analog output `AO` (or `Ao`) - however, always check your specific sensor's datasheet, as some may have non-standard pin designations.
  * Again, make sure all analog sensors are calibrated to output the same 0v - `AREF` (or `IOREF`) volts in range.
* Sensor pins used for event triggering when measurements go above/below a pre-set tolerance - many of which are deceptively labeled `DO` (or `Do`), despite having nothing to do with being `D`ata lines of any kind - can be safely ignored, as software measurement triggers can provide the same decision point when needed.
  * Such connections can still be used through a `TerraBinarySensor` when a hardware threshold output is useful, possibly using an ISR-capable pin if desired.

### Pumps, Valves, and Thermal Equipment

* Use correctly rated relays, contactors, motor drivers, fuses, overload protection, and isolation.
* Use independent dry-run/pressure protection where required for pumps.
* Use independent over-temperature, pressure-relief, and combustion safety hardware for thermal equipment.
* Use suitable backflow protection and sanitation practices for potable or reclaimed water systems.
* A software sensor, timer, or controller should not be the only protection against flooding, fire, electrical fault, or equipment damage.

### Networking & Wireless

* Networking of any kind is 100% optional. Base controller operation works offline using an RTC and optional GPS or known static location.
  * WiFi or Ethernet can be enabled when remote control, MQTT, network time, or network storage is wanted.
* Devices with built-in WiFi or Ethernet can enable such through header/build defines while other devices can utilize an external [serial ESP WiFi module](http://www.instructables.com/id/Cheap-Arduino-WiFi-Shield-With-ESP8266/) on any open Serial line.
  * Warning: While WiFi password is encrypted into system settings data, it should not be considered secure.
* Serial Bluetooth-AT modules can be used on any open Serial port to provide remote device control (only).
* MQTT requires remotely accessible broker daemon in order to publish sensor data (setup separately).
* UDP time server requires remotely accessible time & date API service in order to sync time (TODO).
  * RTC not required / used in reserve when UDP service enabled.
* Note: Geo-location APIs require external 3rd party monthly subscription fees, thus isn't included as a feature.

## Memory Callouts

* The total number of objects and different kinds of objects (reservoirs, pumps, valves, sensors, rails, etc.) that the controller can support at once depends on how much free Flash storage and SRAM your MCU has available.
  * For our supported microcontroller range, on the low end we have devices with 256kB of Flash and at least 16kB of SRAM, while on the upper end we have more modern devices with 1MB+ of Flash and 32kB+ of SRAM. Devices with < 24kB of SRAM may struggle with system builds and may be limited to minimal system setups (such as no WiFi, no data publishing, no built-in library data, only minimal-to-no GUI, etc.), while other newer devices with more capacity build with everything enabled.
* For AVR, SAM, and other build architectures that do not have C++0x11 STL (standard container library) support, there are a series of *`_MAXSIZE` defines nearer to the top of `Terra[UI]Defines.h` that can be modified to adjust how much memory space is allocated for the various static array structures the controller instead uses.
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
