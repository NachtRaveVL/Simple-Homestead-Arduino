/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Streams
*/

#ifndef TerraStreams_H
#define TerraStreams_H

#include "TerraInterfaces.h"

class TerraBufferSink : public TerraTextSink {
public:
    TerraBufferSink();
    void write(const TerraString &text) override;
    const TerraString &buffer() const { return _buffer; }
    void clear();

protected:
    TerraString _buffer;                                    // Buffered stream text
};

#if defined(ARDUINO)
class TerraPrintSink : public TerraTextSink {
public:
    explicit TerraPrintSink(Print &print);
    void write(const TerraString &text) override;

protected:
    Print &_print;
};
#endif

#endif
