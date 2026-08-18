/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Callbacks
*/

#ifndef TerraCallback_HH
#define TerraCallback_HH

#include "TerraPlatform.h"

struct TerraMeasurement;

typedef float (*TerraReadCallback)(void *context);
typedef bool (*TerraBinaryReadCallback)(void *context);
typedef void (*TerraWriteCallback)(void *context, float value);
typedef void (*TerraTaskCallback)(void *context);
typedef void (*TerraPublishCallback)(void *context, const char *channel, const TerraMeasurement &measurement);

#endif
