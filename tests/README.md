# Terraduino Tests

Terraduino has host-side tests for controller logic and Arduino sketches for developer-side export and conversion checks.

## Run the host suite

From the repository root:

```sh
./tests/run_tests.sh
```

The script performs these checks:

1. Verifies that `src/TerraEnumTrie.h` matches the generated minimum enum decoder.
2. Runs the source completeness validator.
3. Syntax-checks every shipped example against the public API.
4. Builds all production C++ sources with strict warnings, including `-Wfloat-equal -Werror`.
5. Builds all host example wrappers and developer test wrappers.
6. Runs the behavioral CTest suite.

An alternate build directory can be supplied as the first argument:

```sh
./tests/run_tests.sh /tmp/terraduino-build
```

## Behavioral coverage

The host suite covers water resources, cisterns, rain catchment, first flush, water routing, balancing, thermal storage, differential circulation, environmental observations, sensors, actuators, triggers, scheduler behavior, logging, publishing, local UI, remote control commands, persistence, factory reconstruction, pin-backed drivers, analog calibration, attachments, and enum/string conversion.

Floating-point equality semantics use `isFPEqual`. Direct floating-point `==` and `!=` are rejected by the strict compiler flags.

## Developer Arduino sketches

`EnumConversionTests` checks forward and reverse conversion for all enum families.

`EnumTrieExportToCPP` emits the minimum decision-tree form used for enum decoding.

`JSONExportTests` exercises the Arduino-facing JSON and binary persistence APIs, including cistern, attachment, pin-driver, thermal, environment, and controller settings.

These sketches still need an Arduino-compatible board/toolchain to execute as Arduino programs. The host build compile-checks their public API surface but does not pretend to execute the Arduino runtime.

## Hardware validation

Host tests cannot validate electrical behavior. Real hardware testing is still required for pin voltage levels, ADC calibration, sensors, relays, pumps, valves, flow meters, pressure systems, thermal equipment, and any network/radio transport used by an installation.
