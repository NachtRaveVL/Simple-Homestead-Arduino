/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Attachments
*/

#include "TerraAttachments.h"

TerraAttachmentSet::TerraAttachmentSet() : _count(0) { }

bool TerraAttachmentSet::attach(uint32_t objectKey, Terra_AttachmentRole role) {
    if (!objectKey || role == Terra_AttachmentRole_Undefined || _count >= TERRA_MAX_ATTACHMENTS) return false;
    for (uint8_t i = 0; i < _count; ++i) {
        if (_attachments[i].objectKey == objectKey && _attachments[i].role == role) return true;
    }
    _attachments[_count++] = TerraAttachment(objectKey, role);
    return true;
}

bool TerraAttachmentSet::detach(uint32_t objectKey, Terra_AttachmentRole role) {
    for (uint8_t i = 0; i < _count; ++i) {
        bool roleMatches = role == Terra_AttachmentRole_Undefined || _attachments[i].role == role;
        if (_attachments[i].objectKey == objectKey && roleMatches) {
            for (uint8_t j = i + 1; j < _count; ++j) _attachments[j - 1] = _attachments[j];
            --_count;
            return true;
        }
    }
    return false;
}

uint32_t TerraAttachmentSet::find(Terra_AttachmentRole role) const {
    for (uint8_t i = 0; i < _count; ++i) if (_attachments[i].role == role) return _attachments[i].objectKey;
    return TERRA_INVALID_KEY;
}

bool TerraAttachmentSet::contains(uint32_t objectKey) const {
    for (uint8_t i = 0; i < _count; ++i) if (_attachments[i].objectKey == objectKey) return true;
    return false;
}

const TerraAttachment *TerraAttachmentSet::at(uint8_t index) const {
    return index < _count ? &_attachments[index] : nullptr;
}
