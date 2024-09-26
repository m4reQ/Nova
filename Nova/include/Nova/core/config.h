#pragma once
#include <Nova/core/build.h>
#include <stdint.h>
#include <stdbool.h>

typedef union
{
    int8_t asInt8;
    int16_t asInt16;
    int32_t asInt32;
    int64_t asInt64;
    float asFloat;
    double asDouble;
    bool asBool;
    const char *asString;
    const wchar_t *asWideString;
} NvConfigValue;

typedef struct
{
    const char *key;
    NvConfigValue value;
} NvConfigVar;

typedef struct
{
    NvConfigVar *variables;
} NvConfig;

NV_API void NvConfigCreate(NvConfig *out);
NV_API bool NvConfigCreateFromIniFile(NvConfig *out, const wchar_t *filepath);
NV_API bool NvConfigCreateFromJsonFile(NvConfig *out, const wchar_t *filepath);
NV_API bool NvConfigCreateFromYamlFile(NvConfig *out, const wchar_t *filepath);
NV_API void NvConfigDestroy(NvConfig *config);
NV_API void NvConfigSetVariable(NvConfig *config, const char *name, NvConfigValue value);
NV_API bool NvConfigHasVariable(NvConfig *config, const char *name);
NV_API NvConfigValue NvConfigGetVariable(NvConfig *config, const char *name);
NV_API bool NvConfigTryGetVariable(NvConfig *config, const char *name, NvConfigValue *outValue);
NV_API void NvConfigRemoveVariable(NvConfig *config, const char *name);

#if NV_WINDOWS
#include <Windows.h>
NV_API bool NvConfigCreateFromRegistry(NvConfig *out, HKEY rootKey, const char *directoryName);
#endif
