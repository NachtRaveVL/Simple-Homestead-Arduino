# Terraduino Tests

Run the host suite from the repository root:

```sh
./tests/run_tests.sh
```

The script configures the host CMake build, builds the Terraduino source and all host test targets, and runs CTest. An alternate build directory can be supplied as the first argument:

```sh
./tests/run_tests.sh /tmp/terraduino-build
```

The host suite uses strict warnings with `-Wall -Wextra -Wpedantic -Wfloat-equal -Werror`. It covers standalone hardening helpers, controller automation and infrastructure, factory-created hardware objects, registration, calibration lifecycle, and object reconstruction. Serialization coverage round-trips system, calibration, actuator, binary sensor, and trigger data through ArduinoJson and exercises JSON-based data allocation.

## Developer Arduino sketches

`EnumConversionTests` checks forward and reverse conversion for enum and unit families.

`EnumTrieExportToCPP` emits the minimum decision-tree form used for enum decoding.

`JSONExportTests` exercises ArduinoJson serialization for current system, reservoir, sensor, actuator, and allocation data.

These sketches require an Arduino-compatible board/toolchain.
