/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Streams
*/

#include "TerraStreams.h"

TerraBufferSink::TerraBufferSink() : _buffer() { }

void TerraBufferSink::write(const TerraString &text) {
    _buffer += text;
}

void TerraBufferSink::clear() {
    _buffer = TerraString();
}

#if defined(ARDUINO)
TerraPrintSink::TerraPrintSink(Print &print) : _print(print) { }

void TerraPrintSink::write(const TerraString &text) {
    _print.print(text);
}
#endif
