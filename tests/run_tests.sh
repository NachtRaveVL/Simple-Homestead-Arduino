#!/usr/bin/env sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR="${1:-$ROOT/build-host}"

python3 "$ROOT/tests/generate_enum_trie.py" --check
python3 "$ROOT/tests/validate_source.py"
python3 "$ROOT/tests/validate_examples.py"
cmake -S "$ROOT/tests" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR"
ctest --test-dir "$BUILD_DIR" --output-on-failure
