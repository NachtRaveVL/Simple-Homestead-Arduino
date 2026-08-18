/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Defines
*/

#ifndef TerraDefines_H
#define TerraDefines_H

#include "TerraPlatform.h"

#define TERRA_NAME_MAXSIZE                 32
#define TERRA_MAX_OBJECTS                  48
#define TERRA_MAX_ATTACHMENTS               8
#define TERRA_MAX_SCHEDULE_TASKS           16
#define TERRA_MAX_LOG_RECORDS              24
#define TERRA_MAX_PUBLISH_CHANNELS         24
#define TERRA_MAX_MODULES                  12
#define TERRA_INVALID_PIN                 255
#define TERRA_INVALID_KEY                   0UL
#define TERRA_DEFAULT_REMOTE_STALE_MS   300000UL
#define TERRA_DEFAULT_PUBLISH_INTERVAL   60000UL
#define TERRA_DEFAULT_LOG_INTERVAL       60000UL
#ifndef TERRA_FLT_EPSILON
#define TERRA_FLT_EPSILON                  0.00001f      // Single-precision floating point error tolerance
#endif
#ifndef TERRA_DBL_EPSILON
#define TERRA_DBL_EPSILON                  0.0000000000001 // Double-precision floating point error tolerance
#endif
#ifndef TERRA_EPSILON
#define TERRA_EPSILON                      TERRA_FLT_EPSILON
#endif

// Networking is optional. Defining these enables transport-specific integration
// in user code without making networking a requirement for Terraduino itself.
//#define TERRA_ENABLE_WIFI
//#define TERRA_ENABLE_ETHERNET
//#define TERRA_ENABLE_MQTT

#endif
