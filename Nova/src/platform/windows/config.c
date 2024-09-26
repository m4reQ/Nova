#include <Nova/core/config.h>
#include <stb/stb_ds.h>
#include <stdio.h>

#define MAX_VALUE_NAME 16383
#define MAX_VALUE_SIZE 256
#define MAX_ERROR_MSG_BUF_SIZE 1024 * 64

static int32_t Int32FromBigEndian(int32_t value)
{
    value = ((value << 8) & 0xFF00FF00) | ((value >> 8) & 0xFF00FF);
    return (value << 16) | ((value >> 16) & 0xFFFF);
}

static int64_t Int64FromBigEndian(int64_t value)
{
    value = ((value << 8) & 0xFF00FF00FF00FF00ULL) | ((value >> 8) & 0x00FF00FF00FF00FFULL);
    value = ((value << 16) & 0xFFFF0000FFFF0000ULL) | ((value >> 16) & 0x0000FFFF0000FFFFULL);
    return (value << 32) | ((value >> 32) & 0xFFFFFFFFULL);
}

static const char *GetWin32StatusString(LSTATUS status)
{
    static char buffer[MAX_ERROR_MSG_BUF_SIZE];
    DWORD bytesWritten = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        (DWORD)status,
        0,
        buffer,
        MAX_ERROR_MSG_BUF_SIZE,
        NULL);
    if (bytesWritten == 0)
        return "";

    return buffer;
}

// TODO Handle subkeys
bool NvConfigCreateFromRegistry(NvConfig *out, HKEY rootKey, const char *directoryName)
{
    HKEY key;
    LSTATUS status = RegOpenKeyExA(
        rootKey,
        directoryName,
        0,
        KEY_READ,
        &key);
    if (status != ERROR_SUCCESS) // lol error_success
    {
        fprintf(stderr, "Config failed to open registry key: %s.\n", GetWin32StatusString(status));
        return false;
    }

    DWORD valuesCount;
    DWORD maxValueNameLen;
    DWORD maxValueLen;

    status = RegQueryInfoKeyA(
        key,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &valuesCount,
        &maxValueNameLen,
        &maxValueLen,
        NULL,
        NULL);
    if (status != ERROR_SUCCESS)
    {
        fprintf(stderr, "Config failed to query registry key: %s.\n", GetWin32StatusString(status));
        return false;
    }

    char valueName[MAX_VALUE_NAME];
    char valueData[MAX_VALUE_SIZE];

    NvConfigVar *variables;
    sh_new_arena(variables);

    for (DWORD i = 0; i < valuesCount; i++)
    {
        DWORD type;
        DWORD nameLength = MAX_VALUE_NAME;
        DWORD valueLength = MAX_VALUE_SIZE;
        status = RegEnumValueA(
            key,
            i,
            valueName,
            &nameLength,
            NULL,
            &type,
            valueData,
            &valueLength);
        if (status != ERROR_SUCCESS)
        {
            fprintf(stderr, "Config failed to read registry value: %s.\n", GetWin32StatusString(status));
            shfree(variables);

            return false;
        }

        valueName[nameLength] = '\0';

        NvConfigValue value;
        switch (type)
        {
        case REG_DWORD:
            value.asInt32 = *(int32_t *)valueData;
            break;
        case REG_DWORD_BIG_ENDIAN:
            value.asInt32 = Int32FromBigEndian(*(int32_t *)valueData);
            break;
        case REG_QWORD:
            value.asInt64 = *(int64_t *)valueData;
            break;
        case REG_SZ:
        case REG_EXPAND_SZ:
        {
            // TODO This will leak memory as we cannot free allocated config string values
            char *string = malloc(valueLength);
            value.asString = strcpy(string, valueData);
            break;
        }
        case REG_LINK:
        {
            wchar_t *string = malloc(valueLength);
            value.asWideString = wcscpy(string, (wchar_t *)valueData);
            break;
        }
        default:
            printf("Loading config value from Win32 reigstry value of type %x is not supported. Skipping value \"%s\"...\n", type, valueName);
            continue;
        }

        shput(variables, valueName, value);
    }

    RegCloseKey(key);

    *out = (NvConfig){
        .variables = variables,
    };

    return true;
}
