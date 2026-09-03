/*  Terraduino: Simple automation controller for homestead resource and environmental systems.
    Copyright (C) 2026 NachtRaveVL
    Terraduino Data
*/

#include "Terraduino.h"
#include "TerraCoreLogic.h"

static size_t skipBinaryStreamBytes(Stream *streamIn, size_t bytesToSkip)
{
    size_t skippedBytes = 0;
    uint8_t skipBuffer[16];

    while (skippedBytes < bytesToSkip) {
        size_t chunkSize = bytesToSkip - skippedBytes;
        if (chunkSize > sizeof(skipBuffer)) { chunkSize = sizeof(skipBuffer); }

        const size_t skipped = streamIn->readBytes(skipBuffer, chunkSize);
        skippedBytes += skipped;
        if (skipped != chunkSize) { break; }
    }

    return skippedBytes;
}

size_t serializeDataToBinaryStream(const TerraData *data, Stream *streamOut, size_t skipBytes)
{
    return streamOut->write((const uint8_t *)data + skipBytes, data->_size - skipBytes);
}

size_t deserializeDataFromBinaryStream(TerraData *data, Stream *streamIn, size_t skipBytes)
{
    return streamIn->readBytes((uint8_t *)data + skipBytes, data->_size - skipBytes);
}

TerraData *newDataFromBinaryStream(Stream *streamIn)
{
    TerraData baseDecode;
    const size_t baseSize = baseDecode._size;
    const size_t basePayloadSize = baseSize - sizeof(void*);
    size_t readBytes = deserializeDataFromBinaryStream(&baseDecode, streamIn, sizeof(void*));
    const size_t serializedSize = baseDecode._size;
    const bool baseReadValid = readBytes == basePayloadSize && serializedSize >= baseSize;
    TERRA_SOFT_ASSERT(baseReadValid, SFP(TStr_Err_ImportFailure));
    if (!baseReadValid) { return nullptr; }

    TerraData *data = _allocateDataFromBaseDecode(baseDecode);
    TERRA_SOFT_ASSERT(data, SFP(TStr_Err_AllocationFailure));
    if (!data) { return nullptr; }

    const auto readPlan = terraBinaryDataReadPlan(serializedSize, data->_size, baseSize);
    if (readPlan.copyBytes) {
        readBytes += streamIn->readBytes((uint8_t *)data + baseSize, readPlan.copyBytes);
    }

    const size_t skippedBytes = skipBinaryStreamBytes(streamIn, readPlan.skipBytes);
    const bool payloadReadValid = readBytes == basePayloadSize + readPlan.copyBytes &&
                                  skippedBytes == readPlan.skipBytes;
    TERRA_SOFT_ASSERT(payloadReadValid, SFP(TStr_Err_ImportFailure));
    if (!payloadReadValid) {
        delete data;
        return nullptr;
    }

    data->migrateFromBinaryVersion(baseDecode._version);
    return data;
}

TerraData *newDataFromJSONObject(JsonObjectConst &objectIn)
{
    TerraData baseDecode;
    baseDecode.fromJSONObject(objectIn);

    TerraData *data = _allocateDataFromBaseDecode(baseDecode);
    TERRA_SOFT_ASSERT(data, SFP(TStr_Err_AllocationFailure));

    if (data) {
        data->fromJSONObject(objectIn);
        return data;
    }

    return nullptr;
}


TerraData::TerraData()
    : id{.chars={'\000','\000','\000','\000'}}, _version(1), _revision(-1)
{
    _size = sizeof(*this);
}

TerraData::TerraData(char id0, char id1, char id2, char id3, uint8_t version, uint8_t revision)
    : id{.chars={id0,id1,id2,id3}}, _version(version), _revision((int8_t)revision)
{
    _size = sizeof(*this);
    TERRA_HARD_ASSERT(isStandardData(), SFP(TStr_Err_InvalidParameter));
}

TerraData::TerraData(tid_t idType, tid_t objType, tposi_t posIndex, tid_t classType, uint8_t version, uint8_t revision)
    : id{.object={idType,objType,posIndex,classType}}, _version(version), _revision((int8_t)revision)
{
    _size = sizeof(*this);
}

TerraData::TerraData(const TerraIdentity &id)
    : TerraData(id.type, id.objTypeAs.idType, id.posIndex, -1, 1, 0)
{
    _size = sizeof(*this);
}

void TerraData::toJSONObject(JsonObject &objectOut) const
{
    if (isStandardData()) {
        objectOut[SFP(TStr_Key_Type)] = charsToString(id.chars, sizeof(id.chars));
    } else {
        int8_t typeVals[4] = {id.object.idType, id.object.objType, id.object.posIndex, id.object.classType};
        objectOut[SFP(TStr_Key_Type)] = commaStringFromArray(typeVals, 4);
    }
    if (_version > 1) { objectOut[SFP(TStr_Key_Version)] = _version; }
    if (getRevision() > 1) { objectOut[SFP(TStr_Key_Revision)] = getRevision(); }
}

void TerraData::fromJSONObject(JsonObjectConst &objectIn)
{
    JsonVariantConst idVar = objectIn[SFP(TStr_Key_Type)];
    const char *idStr = idVar.as<const char *>();
    if (idStr && idStr[0] == 'T') {
        strncpy(id.chars, idStr, 4);
    } else if (idStr) {
        int8_t typeVals[4];
        commaStringToArray(idStr, typeVals, 4);
        id.object.idType = typeVals[0];
        id.object.objType = typeVals[1];
        id.object.posIndex = typeVals[2];
        id.object.classType = typeVals[3];
    }
    _version = objectIn[SFP(TStr_Key_Version)] | _version;
    _revision = objectIn[SFP(TStr_Key_Revision)] | _revision;
    _revision = abs(_revision);
}


TerraSubData::TerraSubData()
    : type(tid_none)
{ ; }

TerraSubData::TerraSubData(tid_t dataType)
    : type(dataType)
{ ; }

void TerraSubData::toJSONObject(JsonObject &objectOut) const
{
    if (type != tid_none) { objectOut[SFP(TStr_Key_Type)] = type; }
}

void TerraSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    type = objectIn[SFP(TStr_Key_Type)] | type;
}
