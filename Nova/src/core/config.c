#include <Nova/core/config.h>
#include <Nova/core/file.h>
#include <Nova/core/string.h>
#include <Nova/core/arena.h>
#include <stb/stb_ds.h>
#include <stdlib.h>
#include <stdio.h>

#define ASSERT_OUT_NOT_NULL NV_ASSERT(out != NULL, "output config pointer cannot be NULL")
#define ASSERT_CONFIG_NOT_NULL NV_ASSERT(config != NULL, "config cannot be NULL")
#define MAX_LINE_LENGTH 512

void NvConfigCreate(NvConfig *out)
{
    ASSERT_OUT_NOT_NULL;

    *out = (NvConfig){
        .variables = NULL,
    };

    sh_new_arena(out->variables);
}

bool NvConfigCreateFromIniFile(NvConfig *out, const wchar_t *filepath)
{
    NvArena arena = {0};
    NvAllocator allocator = NvArenaGetAllocator(&arena);

    NvFile file;
    if (!NvFileOpenUnicode(&file, filepath, L"r"))
    {
        fprintf(stderr, "Failed to create INI config, couldn't load config file.\n");
        return false;
    }

    NvConfigVar *variables = NULL;
    sh_new_arena(variables);

    char *lineBuffer = NvArenaMalloc(&arena, sizeof(char) * MAX_LINE_LENGTH);

    char *currentSectionName = {0};

    while (true)
    {
        if (NvFileReadLine(&file, MAX_LINE_LENGTH, lineBuffer) == NULL)
            break;

        NvStringView line = NV_SV(lineBuffer);

        // ignore comments or empty lines
        if (NvStringStartsWithChar(line, ';') || NvStringIsEmpty(line))
            continue;

        if (NvStringStartsWithChar(line, '['))
        {
            currentSectionName = NvStringDuplicate(NvStringSubstring(NvStringRStrip(line), 1, -1), &allocator);
            continue;
        }

        // remove trailing white characters
        NvStringView valueStr;
        NvStringView name = NvStringSplit(line, '=', &valueStr);

        valueStr = NvStringStrip(valueStr);
        name = NvStringStrip(name);

        NvConfigValue value;
        bool parseSuccess = false;

        if (NvStringContainsAny(valueStr, NV_SV(",.")))
        {
            valueStr = NvStringReplace(valueStr, ',', '.');
            parseSuccess = NvStringTryParseDouble(valueStr, &value.asDouble);
        }
        else
            parseSuccess = NvStringTryParseInt64(valueStr, &value.asInt64);

        if (!parseSuccess)
        {
            if (NvStringStartsWithChar(valueStr, '"'))
            {
                if (NvStringEndsWithChar(valueStr, '"'))
                {
                    // not using arena here cause resulting string will be used as config value
                    value.asString = NvStringDuplicate(NvStringSubstring(valueStr, 1, -1), NULL);
                    parseSuccess = true;
                }
                else
                {
                    fprintf(stderr, "String config value not terminated properly (missing closing \").\n");
                    parseSuccess = false;
                }
            }
            else
            {
                if (!NvStringEndsWithChar(valueStr, '"'))
                {
                    // not using arena here cause resulting string will be used as config value
                    value.asString = NvStringDuplicate(valueStr, NULL);
                    parseSuccess = true;
                }
                else
                {
                    fprintf(stderr, "String config value doesn't begin with \" character but has a closing one.\n");
                    parseSuccess = false;
                }
            }
        }

        if (parseSuccess)
        {
            char *nameResult;

            if (currentSectionName)
            {
                NvStringView nameParts[] = {
                    NV_SV(currentSectionName),
                    NV_SV("."),
                    name,
                };

                // shput automatically duplicates string key so we can allocate using temporary arena
                nameResult = NvStringConcat(NV_ARRLEN(nameParts), nameParts, &allocator);
            }
            else
                nameResult = name.data;

            shput(variables, nameResult, value);
        }
        else
        {
            shfree(variables);
            NvArenaDestroy(&arena);
            return false;
        }
    }

    *out = (NvConfig){
        .variables = variables,
    };

    NvArenaDestroy(&arena);
    return true;
}

bool NvConfigCreateFromJsonFile(NvConfig *out, const wchar_t *filepath)
{
    return true;
}

bool NvConfigCreateFromYamlFile(NvConfig *out, const wchar_t *filepath)
{
    return true;
}

void NvConfigDestroy(NvConfig *config)
{
    if (!config)
        return;

    shfree(config->variables);
    memset(config, 0, sizeof(NvConfig));
}

void NvConfigSetVariable(NvConfig *config, const char *name, NvConfigValue value)
{
    ASSERT_CONFIG_NOT_NULL;

    NvConfigVar *var = shgetp_null(config->variables, name);
    if (!var)
        shput(config->variables, name, value);
    else
        var->value = value;
}

bool NvConfigHasVariable(NvConfig *config, const char *name)
{
    ASSERT_CONFIG_NOT_NULL;
    return shgeti(config->variables, name) >= 0;
}

NvConfigValue NvConfigGetVariable(NvConfig *config, const char *name)
{
    ASSERT_CONFIG_NOT_NULL;
    return shget(config->variables, name);
}

bool NvConfigTryGetVariable(NvConfig *config, const char *name, NvConfigValue *outValue)
{
    ASSERT_CONFIG_NOT_NULL;

    NvConfigVar *var = shgetp_null(config->variables, name);
    if (!var)
        return false;

    *outValue = var->value;
    return true;
}

void NvConfigRemoveVariable(NvConfig *config, const char *name)
{
    ASSERT_CONFIG_NOT_NULL;
    shdel(config->variables, name);
}
