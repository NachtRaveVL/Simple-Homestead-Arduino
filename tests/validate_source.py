#!/usr/bin/env python3
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"
errors = []


def require(path):
    p = ROOT / path
    if not p.is_file() or p.stat().st_size == 0:
        errors.append(f"missing or empty required file: {path}")


required = [
    "LICENSE", "README.md", "library.properties", "library.json", "keywords.txt",
    "src/Terraduino.h", "src/Terraduino.cpp", "src/Terraduino.hpp",
    "src/TerraActivation.h", "src/TerraActivation.cpp", "src/TerraActuators.h", "src/TerraActuators.cpp",
    "src/TerraAttachments.h", "src/TerraAttachments.cpp", "src/TerraAttachments.hpp",
    "src/TerraBalancers.h", "src/TerraBalancers.cpp", "src/TerraCallback.hh", "src/TerraCoreLogic.h",
    "src/TerraData.h", "src/TerraData.cpp", "src/TerraDatas.h", "src/TerraDatas.cpp",
    "src/TerraDefines.h", "src/TerraDrivers.h", "src/TerraDrivers.cpp", "src/TerraEnumTrie.h",
    "src/TerraEnvironment.h", "src/TerraEnvironment.cpp", "src/TerraFactory.h", "src/TerraFactory.cpp",
    "src/TerraInlines.hh", "src/TerraInterfaces.h", "src/TerraInterfaces.hpp",
    "src/TerraLogger.h", "src/TerraLogger.cpp", "src/TerraLoggerSubData.h",
    "src/TerraMeasurements.h", "src/TerraMeasurements.cpp", "src/TerraModules.h", "src/TerraModules.cpp",
    "src/TerraObject.h", "src/TerraObject.cpp", "src/TerraPins.h", "src/TerraPins.cpp", "src/TerraPlatform.h",
    "src/TerraPublisher.h", "src/TerraPublisher.cpp", "src/TerraRails.h", "src/TerraRails.cpp",
    "src/TerraResource.h", "src/TerraResource.cpp", "src/TerraScheduler.h", "src/TerraScheduler.cpp",
    "src/TerraSensors.h", "src/TerraSensors.cpp", "src/TerraSetup.h", "src/TerraStreams.h", "src/TerraStreams.cpp",
    "src/TerraStrings.h", "src/TerraStrings.cpp", "src/TerraThermal.h", "src/TerraThermal.cpp",
    "src/TerraTriggers.h", "src/TerraTriggers.cpp", "src/TerraTypes.h", "src/TerraUtils.h", "src/TerraUtils.cpp",
    "src/TerraUtils.hpp", "src/TerraWater.h", "src/TerraWater.cpp",
    "src/shared/TerraUIDefines.h", "src/shared/TerraUIStrings.h", "src/shared/TerraUIStrings.cpp",
    "src/shared/TerraUIData.h", "src/shared/TerraUIData.cpp", "src/shared/TerraDisplayDrivers.h", "src/shared/TerraDisplayDrivers.cpp", "src/shared/TerraDisplayDrivers.hpp",
    "src/shared/TerraInputDrivers.h", "src/shared/TerraInputDrivers.cpp", "src/shared/TerraMenus.h", "src/shared/TerraMenus.cpp",
    "src/shared/TerraOverviews.h", "src/shared/TerraOverviews.cpp", "src/shared/TerraOverviews.hpp",
    "src/shared/TerraRemoteControls.h", "src/shared/TerraRemoteControls.cpp", "src/shared/TerraUIInlines.hh",
    "src/shared/TerraduinoUI.h", "src/shared/TerraduinoUI.cpp", "src/min/TerraduinoUI.h", "src/min/TerraduinoUI.cpp",
    "src/full/TerraduinoUI.h", "src/full/TerraduinoUI.cpp",
    "tests/CMakeLists.txt", "tests/README.md", "tests/run_tests.sh", "tests/generate_enum_trie.py", "tests/validate_examples.py",
    "tests/host/test_terraduino.cpp", "tests/host/test_automation.cpp", "tests/host/test_infrastructure.cpp",
    "tests/EnumConversionTests/EnumConversionTests.ino",
    "tests/EnumTrieExportToCPP/EnumTrieExportToCPP.ino", "tests/JSONExportTests/JSONExportTests.ino",
]
tc_menu_files = [
    "tcMenu_Display_AdaFruitGfx.cpp", "tcMenu_Display_AdaFruitGfx.h", "tcMenu_Display_AdaFruitGfx.hpp",
    "tcMenu_Display_LiquidCrystal.cpp", "tcMenu_Display_LiquidCrystal.h",
    "tcMenu_Display_TfteSpi.cpp", "tcMenu_Display_TfteSpi.h",
    "tcMenu_Display_U8g2.cpp", "tcMenu_Display_U8g2.h",
    "tcMenu_Extra_BspUserSettings.h", "tcMenu_Extra_StChromaArt.cpp", "tcMenu_Extra_StChromaArt.h",
    "tcMenu_Font_AdafruitGFXArial14.h", "tcMenu_Font_tcUnicodeArial14.h",
    "tcMenu_Input_AdaTouchDriver.cpp", "tcMenu_Input_AdaTouchDriver.h",
    "tcMenu_Input_ESP32TouchKeysAbstraction.cpp", "tcMenu_Input_ESP32TouchKeysAbstraction.h",
    "tcMenu_Remote_EthernetTransport.cpp", "tcMenu_Remote_EthernetTransport.h",
    "tcMenu_Remote_SerialTransport.cpp", "tcMenu_Remote_SerialTransport.h",
    "tcMenu_Remote_SimhubConnector.cpp", "tcMenu_Remote_SimhubConnector.h",
    "tcMenu_Remote_WiFiTransport.cpp", "tcMenu_Remote_WiFiTransport.h",
    "tcMenu_Theme_CoolBlueModern.h", "tcMenu_Theme_CoolBlueTraditional.h",
    "tcMenu_Theme_DarkModeModern.h", "tcMenu_Theme_DarkModeTraditional.h",
    "tcMenu_Theme_MonoBordered.h", "tcMenu_Theme_MonoInverse.h",
]
required += [f"src/shared/{name}" for name in tc_menu_files]

for path in required:
    require(path)

examples = ['BasicHomestead', 'CisternManagement', 'DataWriter', 'FullSystem', 'LocalDashboard', 'RainwaterCollection', 'RemoteSensor', 'ThermalStorage', 'UISetup', 'WeatherStation']
for name in examples:
    require(f"examples/{name}/{name}.ino")
    require(f"tests/host/examples/{name}.cpp")
for name in ["EnumConversionTests", "EnumTrieExportToCPP", "JSONExportTests"]:
    require(f"tests/host/examples/{name}.cpp")

# No unfinished implementation markers in code/examples.
marker_re = re.compile(r"\b(TODO|FIXME|TBD|PLACEHOLDER|STUB)\b|not\s+implemented", re.IGNORECASE)
for base in (SRC, ROOT / "examples"):
    for p in base.rglob("*"):
        if not (p.is_file() and p.suffix.lower() in {".h", ".hh", ".hpp", ".cpp", ".ino"}):
            continue
        rel = p.relative_to(ROOT).as_posix()
        if rel.startswith(("src/shared/", "src/min/", "src/full/")) or rel == "examples/UISetup/UISetup.ino":
            continue
        match = marker_re.search(p.read_text(errors="ignore"))
        if match:
            errors.append(f"unfinished-work marker in {p.relative_to(ROOT)}: {match.group(0)}")

# Do not leave copied sibling branding in production source, examples, or tests.
# Repository-local wiki content is intentionally excluded because it may discuss sibling libraries.
foreign_names = ["Hy" + "druino", "He" + "lioduino", "As" + "truino", "As" + "tro"]
foreign_re = re.compile(r"\b(" + "|".join(map(re.escape, foreign_names)) + r")\b", re.IGNORECASE)
for base in (SRC, ROOT / "examples", ROOT / "tests"):
    for p in base.rglob("*"):
        if not p.is_file() or p == Path(__file__):
            continue
        rel = str(p.relative_to(ROOT))
        path_match = foreign_re.search(rel)
        if path_match:
            errors.append(f"stale sibling name in file path: {rel}")
        if p.suffix.lower() in {".h", ".hh", ".hpp", ".cpp", ".ino", ".md", ".txt", ".json", ".properties", ".py"}:
            match = foreign_re.search(p.read_text(errors="ignore"))
            if match:
                errors.append(f"stale sibling name in {rel}: {match.group(0)}")

# Every implementation translation unit must contain behavior, not only an include.
for p in SRC.rglob("*.cpp"):
    rel = p.relative_to(ROOT).as_posix()
    if rel.startswith(("src/shared/", "src/min/", "src/full/")):
        continue
    meaningful = []
    for raw in p.read_text(errors="ignore").splitlines():
        line = raw.strip()
        if not line or line.startswith("//") or line.startswith("/*") or line.startswith("*") or line.startswith("#include"):
            continue
        meaningful.append(line)
    if len(meaningful) < 3:
        errors.append(f"source file appears empty/placeholder-only: {p.relative_to(ROOT)}")

enum_families = [
    ("ObjectType", "terraObjectTypeToString", "terraObjectTypeFromString"),
    ("ResourceType", "terraResourceTypeToString", "terraResourceTypeFromString"),
    ("ResourceState", "terraResourceStateToString", "terraResourceStateFromString"),
    ("WaterSourceType", "terraWaterSourceTypeToString", "terraWaterSourceTypeFromString"),
    ("WaterStorageType", "terraWaterStorageTypeToString", "terraWaterStorageTypeFromString"),
    ("SensorType", "terraSensorTypeToString", "terraSensorTypeFromString"),
    ("ActuatorType", "terraActuatorTypeToString", "terraActuatorTypeFromString"),
    ("MeasurementMode", "terraMeasurementModeToString", "terraMeasurementModeFromString"),
    ("EnableMode", "terraEnableModeToString", "terraEnableModeFromString"),
    ("PinMode", "terraPinModeToString", "terraPinModeFromString"),
    ("Unit", "terraUnitToString", "terraUnitFromString"),
    ("UnitsCategory", "terraUnitsCategoryToString", "terraUnitsCategoryFromString"),
    ("RailType", "terraRailTypeToString", "terraRailTypeFromString"),
    ("Comparison", "terraComparisonToString", "terraComparisonFromString"),
    ("TriggerState", "terraTriggerStateToString", "terraTriggerStateFromString"),
    ("LogLevel", "terraLogLevelToString", "terraLogLevelFromString"),
    ("ModuleType", "terraModuleTypeToString", "terraModuleTypeFromString"),
    ("ControlMode", "terraControlModeToString", "terraControlModeFromString"),
    ("RouteState", "terraRouteStateToString", "terraRouteStateFromString"),
    ("AttachmentRole", "terraAttachmentRoleToString", "terraAttachmentRoleFromString"),
]
strings_h = (SRC / "TerraStrings.h").read_text()
strings_cpp = (SRC / "TerraStrings.cpp").read_text()
trie_h = (SRC / "TerraEnumTrie.h").read_text()
trie_export = (ROOT / "tests/EnumTrieExportToCPP/EnumTrieExportToCPP.ino").read_text()
enum_test = (ROOT / "tests/EnumConversionTests/EnumConversionTests.ino").read_text()
host_test = (ROOT / "tests/host/test_terraduino.cpp").read_text()
for label, enc, dec in enum_families:
    decoder = f"terraDecode{label}"
    checks = [
        ("TerraStrings.h", strings_h, enc), ("TerraStrings.h", strings_h, dec),
        ("TerraStrings.cpp", strings_cpp, enc), ("TerraStrings.cpp", strings_cpp, dec),
        ("TerraEnumTrie.h", trie_h, decoder), ("EnumTrieExportToCPP.ino", trie_export, enc),
        ("EnumConversionTests.ino", enum_test, enc), ("EnumConversionTests.ino", enum_test, dec),
        ("host/test_terraduino.cpp", host_test, enc), ("host/test_terraduino.cpp", host_test, dec),
    ]
    for filename, text, symbol in checks:
        if symbol not in text:
            errors.append(f"{label} string coverage missing {symbol} in {filename}")

if trie_h.count("switch (terraTrieChar") < len(enum_families):
    errors.append("generated minimum enum trie does not contain switch dispatch for every enum family")
if "#include \"TerraEnumTrie.h\"" not in strings_cpp:
    errors.append("TerraStrings.cpp does not use generated TerraEnumTrie.h")

# Generated trie must match its source vocabulary.
result = subprocess.run([sys.executable, str(ROOT / "tests/generate_enum_trie.py"), "--check"], capture_output=True, text=True)
if result.returncode:
    errors.append("TerraEnumTrie.h is stale relative to tests/generate_enum_trie.py")

# Floating-point equality policy is enforced by the compiler, not a regex guess.
cmake = (ROOT / "tests/CMakeLists.txt").read_text()
if "-Wfloat-equal" not in cmake or "-Werror" not in cmake:
    errors.append("strict host build must enforce -Wfloat-equal with -Werror")
for legacy in ["terraNearlyEqual", "nearf(", "nearValue("]:
    for base in [SRC, ROOT / "tests", ROOT / "examples"]:
        for p in base.rglob("*"):
            if p.is_file() and p.suffix.lower() in {".h", ".hh", ".hpp", ".cpp", ".ino"} and legacy in p.read_text(errors="ignore"):
                errors.append(f"legacy floating-point equality helper {legacy} in {p.relative_to(ROOT)}")

# Core persistence/factory paths must cover JSON, binary integrity, and runtime reconstruction.
for symbol in ["toJSON", "fromJSON", "toBinary", "fromBinary", "terraBinaryChecksum"]:
    if symbol not in (SRC / "TerraData.cpp").read_text():
        errors.append(f"system persistence missing {symbol}")
factory_cpp = (SRC / "TerraFactory.cpp").read_text()
for token in ["newObjectFromData", "newDataFromObject", "TerraCisternData", "TerraRemoteSensor", "TerraThermalLoopData",
              "hasPinDriver", "setDriver(driver, true)", "attachmentCount", "isConfiguredAvailable"]:
    if token not in factory_cpp:
        errors.append(f"factory persistence/reconstruction missing {token}")
for token in ["corruptSystem.fromBinary", "test_factory_persistence_reconstruction", "TerraCisternData", "setOutputRequests",
              "getPinSetup", "driverCalibrated", "attachments().find", "publisher.getInterval"]:
    if token not in host_test:
        errors.append(f"host coverage missing {token}")
for token in ["measurementMode", "loggerMinimumLevel", "publisherIntervalMs"]:
    if token not in (SRC / "TerraData.cpp").read_text() or token not in (SRC / "TerraSetup.h").read_text():
        errors.append(f"system persistence missing controller setting {token}")


# Main header keeps the same front-door ordering and active-controller helpers as the sibling libraries.
main_header = (SRC / "Terraduino.h").read_text()
header_sequence = [
    '#include "TerraPlatform.h"', '#include "TerraDefines.h"', '#include "TerraTypes.h"',
    '#include "TerraSetup.h"', '#include "TerraStrings.h"', '#include "TerraInlines.hh"',
    '#include "TerraCallback.hh"', '#include "TerraInterfaces.h"', '#include "TerraActivation.h"',
    '#include "TerraAttachments.h"', '#include "TerraData.h"', '#include "TerraObject.h"',
    '#include "TerraMeasurements.h"', '#include "TerraPins.h"', '#include "TerraUtils.h"',
    '#include "TerraDatas.h"', '#include "TerraStreams.h"', '#include "TerraTriggers.h"',
    '#include "TerraDrivers.h"', '#include "TerraActuators.h"', '#include "TerraSensors.h"',
    '#include "TerraResource.h"', '#include "TerraEnvironment.h"', '#include "TerraWater.h"',
    '#include "TerraThermal.h"', '#include "TerraBalancers.h"', '#include "TerraRails.h"',
    '#include "TerraModules.h"', '#include "TerraScheduler.h"', '#include "TerraLogger.h"',
    '#include "TerraPublisher.h"', '#include "TerraFactory.h"', '#include "TerraCoreLogic.h"',
    '#include "TerraInterfaces.hpp"',
]
last_pos = -1
for include in header_sequence:
    pos = main_header.find(include)
    if pos < 0:
        errors.append(f"main header missing family include: {include}")
    elif pos <= last_pos:
        errors.append(f"main header include order drifted at: {include}")
    last_pos = max(last_pos, pos)
for helper in ["extern Terraduino *getController()", "extern TerraLogger *getLogger()",
               "extern TerraPublisher *getPublisher()", "extern TerraScheduler *getScheduler()",
               '#include "Terraduino.hpp"']:
    if helper not in main_header:
        errors.append(f"main header missing family front-door helper: {helper}")
if '#include "TerraUtils.hpp"' in main_header:
    errors.append("TerraUtils.hpp should be pulled in through TerraUtils.h, matching sibling header layering")


# Stored members and enum values retain the short inline comments used by the sibling libraries.
member_decl = re.compile(r"^(?:const\s+)?[A-Za-z_][A-Za-z0-9_:<>]*(?:\s*[*&])?\s+[A-Za-z_][A-Za-z0-9_]*(?:\[[^]]+\])?(?:\s*=.*)?;")
enum_value = re.compile(r"^Terra_[A-Za-z0-9_]+(?:\s*=\s*[^,]+)?\s*,?$")
for header in SRC.glob("*.h"):
    if header.name in {"TerraEnumTrie.h", "TerraStrings.h"}:
        continue
    lines = header.read_text(errors="ignore").splitlines()
    in_enum = False
    for line_no, line in enumerate(lines, 1):
        stripped = line.strip()
        if "enum " in stripped and "{" in stripped:
            in_enum = True
        if in_enum and enum_value.match(stripped) and "//" not in line:
            errors.append(f"{header.name}:{line_no} enum value is missing a comment")
        if in_enum and "};" in stripped:
            in_enum = False
        if not stripped or stripped.startswith(("#", "//", "/*", "*", "enum", "class ", "struct ", "typedef ", "using ", "return ", "case ", "friend ", "template")):
            continue
        if "(" in stripped or ")" in stripped or stripped.startswith(("virtual ", "inline ", "static ")):
            continue
        if member_decl.match(stripped) and "//" not in line:
            errors.append(f"{header.name}:{line_no} stored member is missing a comment")

for p in ROOT.iterdir():
    if p.is_file() and p.suffix.lower() in {".o", ".obj", ".a", ".exe"}:
        errors.append(f"build artifact at repository root: {p.name}")

if errors:
    print("Terraduino source validation: FAIL")
    for error in errors:
        print(f" - {error}")
    sys.exit(1)

print("Terraduino source validation: PASS")
print(f" - {len(required)} required files present")
print(f" - {len(examples)} examples present with host compile wrappers")
print(f" - {len(enum_families)} enum families covered by encode/decode/generated-trie tests")
print(" - generated trie is current")
print(" - floating-point equality policy is compiler-enforced")
print(" - core/source examples have no unfinished-work markers; UI TODO scaffolding is intentionally retained")
