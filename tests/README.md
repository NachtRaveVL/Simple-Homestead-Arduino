# Terraduino Tests

Terraduino currently has a small host-side hardening target plus Arduino sketches and broader test sources that are being brought forward with the current controller API.

## Run the host suite

From the repository root:

```sh
./tests/run_tests.sh
```

The script configures the host CMake build, builds the standalone hardening target, and runs CTest.

An alternate build directory can be supplied as the first argument:

```sh
./tests/run_tests.sh /tmp/terraduino-build
```

The host target uses strict warnings with `-Wall -Wextra -Wpedantic -Wfloat-equal -Werror` and does not require the Arduino dependency stack.

## Host coverage

`host/test_hardening.cpp` exercises the standalone helpers in `TerraCoreLogic.h`, including timer rollover, binary debounce, balancer range handling, and binary record migration planning.

The other host test sources remain in `tests/host` while their controller-facing APIs are brought into alignment. They are not currently part of the native CMake target because the production controller depends on the Arduino runtime and this branch does not contain a parallel host Arduino shim.

## Developer Arduino sketches

`EnumConversionTests` checks forward and reverse conversion for enum and unit families.

`EnumTrieExportToCPP` emits the minimum decision-tree form used for enum decoding.

`JSONExportTests` exercises ArduinoJson serialization for current system, reservoir, sensor, actuator, and allocation data.

These sketches require an Arduino-compatible board/toolchain.
