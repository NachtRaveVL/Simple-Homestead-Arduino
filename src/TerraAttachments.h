/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Attachments
*/

#ifndef TerraAttachments_H
#define TerraAttachments_H

#include "TerraDefines.h"
#include "TerraTypes.h"

struct TerraAttachment {
    uint32_t objectKey;                                     // Attached object key
    Terra_AttachmentRole role;                              // Attachment role

    TerraAttachment(uint32_t key = TERRA_INVALID_KEY, Terra_AttachmentRole roleIn = Terra_AttachmentRole_Undefined)
        : objectKey(key), role(roleIn) { }
};

class TerraAttachmentSet {
public:
    TerraAttachmentSet();

    bool attach(uint32_t objectKey, Terra_AttachmentRole role);
    bool detach(uint32_t objectKey, Terra_AttachmentRole role = Terra_AttachmentRole_Undefined);
    uint32_t find(Terra_AttachmentRole role) const;
    bool contains(uint32_t objectKey) const;
    uint8_t size() const { return _count; }
    const TerraAttachment *at(uint8_t index) const;

protected:
    TerraAttachment _attachments[TERRA_MAX_ATTACHMENTS];    // Object attachment relationships
    uint8_t _count;                                         // Active entry count
};

#endif
