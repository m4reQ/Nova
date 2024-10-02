#pragma once
#include <Nova/core/build.h>
#include <Nova/core/arena.h>
#include <Nova/io/stream.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    NV_JSON_TYPE_NULL,
    NV_JSON_TYPE_BOOL,
    NV_JSON_TYPE_NUMBER,
    NV_JSON_TYPE_STRING,
    NV_JSON_TYPE_ARRAY,
    NV_JSON_TYPE_OBJECT,
} NvJsonType;

typedef union
{
    int64_t asInt;
    double asDouble;
    char *asString;
    void *asObjPtr; // NvJsonObject*
} NvJsonValue;

typedef struct
{
    const char *key; // DON'T RENAME OR MOVE, used by std hash map
    size_t length;
    bool isDouble;
    NvJsonValue value;
    NvJsonType type;
} NvJsonObject;

typedef struct
{
    NvJsonObject object;
    NvArena memory;
} NvJson;

NV_API bool NvJsonDeserializeString(NvJson *out, const char *string);
NV_API bool NvJsonDeserializeFile(NvJson *out, const wchar_t *filepath);
NV_API bool NvJsonDeserializeStream(NvJson *out, NvStream *stream);
NV_API void NvJsonDestroy(NvJson *json);

NV_API bool NvJsonObjectIsType(const NvJsonObject *obj, NvJsonType type);
NV_API bool NvJsonObjectIsNull(const NvJsonObject *obj);

NV_API NvJsonObject *NvJsonObjectGet(NvJsonObject *obj, const char *childName);
NV_API NvJsonObject *NvJsonObjectGetAt(NvJsonObject *obj, size_t idx);
NV_API bool NvJsonObjectHasChildren(const NvJsonObject *obj);
NV_API bool NvJsonObjectHasChild(NvJsonObject *obj, const char *childName);
NV_API size_t NvJsonObjectGetChildrenCount(NvJsonObject *obj);

NV_API NvJsonObject *NvJsonArrayGet(const NvJsonObject *obj, size_t index);
NV_API size_t NvJsonArrayGetSize(const NvJsonObject *obj);

NV_API const char *NvJsonStringGet(const NvJsonObject *obj);

NV_API int64_t NvJsonNumberGetInt(const NvJsonObject *obj);
NV_API double NvJsonNumberGetDouble(const NvJsonObject *obj);
NV_API bool NvJsonNumberIsDouble(const NvJsonObject *obj);

NV_API bool NvJsonBoolGet(const NvJsonObject *obj);
