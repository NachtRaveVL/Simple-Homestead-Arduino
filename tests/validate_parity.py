#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"

# Generic mechanisms already supplied by the Hydro/Helio family may not be
# reimplemented through a second Terraduino-specific framework path.
forbidden = {
    "TerraModuleRegistry": "parallel module registry",
    "Terra_ModuleType": "parallel module type system",
    "Terra_AttachmentRole": "role/key attachment persistence",
    "Terra_Comparison": "parallel comparison trigger system",
    "TerraPinSetup": "parallel pin setup abstraction",
    "TERRA_INVALID_KEY": "arbitrary object-key API",
    "allocateKey(": "arbitrary object-key allocator",
    "terraJsonExtract": "manual JSON parser",
    "TerraSerializable": "parallel serialization interface",
    "setUpdateInterval(": "per-sensor polling clock",
    "_updateIntervalMs": "per-sensor polling clock",
    "_lastReadAt": "per-sensor polling clock",
    "resolveActivations(": "parallel activation resolver",
    "terraResolveActuatorRequests": "second actuator arbitration implementation",
    "TerraSystemSetup": "parallel controller setup bag",
    "addIntervalTask(": "generic callback timer scheduler",
    "addDailyTask(": "generic callback timer scheduler",
    "driverCalibrated": "driver-owned calibration state",
    "setCalibration(": "parallel sensor calibration API",
    "clearCalibration(": "parallel sensor calibration API",
}

errors = []
for path in SRC.rglob("*"):
    if not path.is_file() or path.suffix.lower() not in {".h", ".hh", ".hpp", ".cpp"}:
        continue
    text = path.read_text(errors="ignore")
    for token, reason in forbidden.items():
        if token in text:
            errors.append(f"{path.relative_to(ROOT)}: forbidden {reason}: {token}")

required = {
    "src/Terraduino.h": ["public TerraCalibrations", "public TerraObjectRegistration", "TerraCoreLogic.h", "TerraStreams.h", "TerraDrivers.h", "TerraInterfaces.hpp"],
    "src/TerraObject.h": ["struct TerraIdentity"],
    "src/TerraAttachments.h": ["class TerraDLinkObject", "class TerraAttachment"],
    "src/TerraData.h": ["struct TerraData", "struct TerraSubData", "toJSONObject", "fromJSONObject", "serializeDataToBinaryStream", "newDataFromBinaryStream"],
    "src/TerraMeasurements.h": ["TerraBinaryMeasurement", "TerraSingleMeasurement", "TerraDoubleMeasurement", "TerraTripleMeasurement"],
    "src/TerraTriggers.h": ["class TerraTrigger", "class TerraMeasurementValueTrigger", "class TerraMeasurementRangeTrigger"],
    "src/TerraActuators.h": ["TerraActivationHandle", "TerraDigitalPin", "TerraAnalogPin"],
    "src/TerraSensors.h": ["TerraCalibrationData", "TerraDigitalPin", "TerraAnalogPin"],
    "src/TerraModules.h": ["class TerraCalibrations", "class TerraObjectRegistration"],
    "src/TerraCallback.hh": ["template <class ParameterType> class Slot", "template <class ParameterType, int Slots = 8> class Signal"],
    "src/TerraCoreLogic.h": ["terraElapsedTime", "terraUpdateStableBinaryState", "TerraBinaryDataReadPlan"],
    "src/TerraDrivers.h": ["class TerraInputDriver", "class TerraOutputDriver", "TerraSingleMeasurement", "TerraPinData"],
    "src/TerraStreams.h": ["class TerraBufferSink", "class TerraPrintSink"],
    "src/TerraLoggerSubData.h": ["struct TerraLoggerSubData", "public TerraSubData"],
    "src/TerraInterfaces.hpp": ["class TerraNullSink"],
    "src/TerraSmartPtr.h": ["template<typename T> using SharedPtr", "terraStaticPointerCast"],
    "src/TerraLogger.h": ["TerraLogEvent", "Signal<const TerraLogEvent", "TerraLoggerSubData"],
    "src/TerraPublisher.h": ["TerraDataColumn", "advancePollingFrame", "getPublishSignal", "TerraPublisherSubData"],
    "src/TerraScheduler.h": ["class TerraScheduler", "setNeedsScheduling", "performScheduling"],
}

for relative, tokens in required.items():
    path = ROOT / relative
    if not path.is_file():
        errors.append(f"missing required framework/domain file: {relative}")
        continue
    text = path.read_text(errors="ignore")
    for token in tokens:
        if token not in text:
            errors.append(f"{relative}: missing expected construct: {token}")

# Drivers are an I/O bridge only. They must not own calibration or activation arbitration.
drivers_text = ""
for relative in ("src/TerraDrivers.h", "src/TerraDrivers.cpp"):
    path = ROOT / relative
    if path.is_file():
        drivers_text += path.read_text(errors="ignore")
for token in ("CalibrationData", "resolveActivations", "TerraActivationHandle"):
    if token in drivers_text:
        errors.append(f"TerraDrivers: hardware bridge contains forbidden framework responsibility: {token}")

# Calibration data follows Hydro/Helio identity-string ownership, not an alternate raw-key record.
for relative, owner_token in (("src/AstroDatas.h", "ownerName"),) if "Astro" in str(ROOT) else (("src/TerraDatas.h", "ownerName"),):
    path = ROOT / relative
    if path.is_file():
        text = path.read_text(errors="ignore")
        if owner_token not in text:
            errors.append(f"{relative}: calibration data missing canonical ownerName")
        if "ownerKey" in text:
            errors.append(f"{relative}: calibration data retains parallel ownerKey")

if errors:
    for error in errors:
        print(f"FAIL: {error}")
    sys.exit(1)

print("PASS Terraduino Hydro/Helio parity guard")
