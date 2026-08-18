#!/usr/bin/env python3
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
compiler = shutil.which("g++") or shutil.which("c++")
if not compiler:
    print("No host C++ compiler found")
    raise SystemExit(1)

failures = []
examples = sorted((ROOT / "examples").glob("*/*.ino"))
for example in examples:
    name = example.stem
    wrapper = ROOT / "tests" / "host" / "examples" / f"{name}.cpp"
    if not wrapper.exists():
        failures.append((example.relative_to(ROOT), "missing host compile wrapper"))
        continue
    command = [
        compiler, "-std=c++11", "-Wall", "-Wextra", "-Wpedantic", "-Wfloat-equal", "-Werror",
        "-fsyntax-only", "-I", str(ROOT / "src"), "-I", str(ROOT / "src/shared"),
        "-I", str(ROOT / "src/shared/screens"), str(wrapper),
    ]
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode:
        failures.append((example.relative_to(ROOT), result.stderr.strip()))

if failures:
    for example, output in failures:
        print(f"FAIL: {example}")
        print(output)
    raise SystemExit(1)
print(f"PASS example syntax ({len(examples)} sketches)")
