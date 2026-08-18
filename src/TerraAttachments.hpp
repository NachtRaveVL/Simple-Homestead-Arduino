/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Attachments
*/

#ifndef TerraAttachments_HPP
#define TerraAttachments_HPP

#include "TerraAttachments.h"

inline bool terraHasPrimarySensor(const TerraAttachmentSet &attachments) {
    return attachments.find(Terra_AttachmentRole_PrimarySensor) != TERRA_INVALID_KEY;
}

#endif
