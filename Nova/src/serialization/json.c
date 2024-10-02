#include <Nova/serialization/json.h>
#include <Nova/io/file.h>
#include <Nova/io/stream.h>
#include <Nova/core/string.h>
#include <stb/stb_ds.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUMBER_LENGTH 64
#define INITIAL_ARRAY_LENGTH 2
#define ARRAY_GROWTH_FACTOR 2
#define ASSERT_OBJ_NOT_NULL(obj) NV_ASSERT(obj != NULL, "Json object cannot be NULL")
#define ASSERT_OBJ_TYPE(obj, _type) NV_ASSERT(obj->type == _type, "Json object has to be of type " #_type)

typedef enum
{
    PARSE_RES_SUCCESS,
    PARSE_RES_CANT_PARSE,
    PARSE_RES_NUMBER_TOO_LONG,
    PARSE_RES_EOS,
} ParseResult;

static ParseResult TryParseJsonObject(NvStream *stream, NvJsonObject *out, NvAllocator *allocator);
static ParseResult TryParseJsonArray(NvStream *stream, NvJsonObject *out, NvAllocator *allocator);

static bool ConsumeWhitespace(NvStream *stream, char *current)
{
    while (true)
    {
        if (!NvStreamReadByte(stream, current))
            return false;

        if (!isspace(*current))
            break;
    }

    return true;
}

static bool IsDecimalCharacter(char value)
{
    return value == '.' ||
           value == '+' ||
           value == '-' ||
           value == 'e' ||
           isdigit(value);
}

static char *TryParseStringLiteral(NvStream *stream, NvAllocator *allocator, size_t *outLength)
{
    size_t strLength = 0;
    while (true)
    {
        char currentChar;
        if (!NvStreamReadByte(stream, &currentChar))
            return NULL;

        if (currentChar == '"')
            break;

        strLength++;
    }

    NvStreamSetPos(stream, -(strLength + 1), true);

    char *str = NV_ALLOCATOR_MALLOC(allocator, sizeof(char) * (strLength + 1));
    str[strLength] = '\0';

    size_t bytesRead;
    bool readResult = NvStreamTryRead(stream, str, strLength, &bytesRead);
    NV_ASSERT(readResult && (bytesRead == strLength), "Failed to read string data.");

    NvStreamSetPos(stream, 1, true);

    *outLength = strLength;
    return str;
}

static bool TryParseJsonString(NvStream *stream, NvJsonObject *out, NvAllocator *allocator)
{
    size_t strLength = 0;
    char *string = TryParseStringLiteral(stream, allocator, &strLength);
    if (!string)
        return PARSE_RES_EOS;

    out->type = NV_JSON_TYPE_STRING;
    out->length = strLength;
    out->value.asString = string;

    return PARSE_RES_SUCCESS;
}

static bool TryParseJsonNumber(NvStream *stream, NvJsonObject *out)
{
    NvStreamSetPos(stream, -1, true);

    char numberString[MAX_NUMBER_LENGTH];
    size_t currentIdx = 0;
    while (true)
    {
        if (currentIdx > MAX_NUMBER_LENGTH)
            return PARSE_RES_NUMBER_TOO_LONG;

        if (!NvStreamReadByte(stream, &numberString[currentIdx]))
            break;

        if (!IsDecimalCharacter(numberString[currentIdx]))
            break;

        currentIdx++;
    }

    char *end;

    out->value.asInt = strtoll(numberString, &end, 10);
    if (end != numberString && end[0] != '.')
    {
        out->type = NV_JSON_TYPE_NUMBER;

        NvStreamSetPos(stream, -1, true);
        return PARSE_RES_SUCCESS;
    }

    out->value.asDouble = strtod(numberString, &end);
    if (end != numberString)
    {
        out->type = NV_JSON_TYPE_NUMBER;
        out->isDouble = true;

        NvStreamSetPos(stream, -1, true);
        return PARSE_RES_SUCCESS;
    }

    NvStreamSetPos(stream, -currentIdx, true);
    return PARSE_RES_CANT_PARSE;
}

static ParseResult TryParseJsonObject(NvStream *stream, NvJsonObject *out, NvAllocator *allocator)
{
    NvJsonObject *items = NULL;
    sh_new_arena(items);

    size_t itemsCount = 0;
    while (true)
    {
        char current;
        if (!ConsumeWhitespace(stream, &current))
            return PARSE_RES_EOS;

        if (current != '"')
            return PARSE_RES_CANT_PARSE;

        size_t itemNameLength = 0;
        const char *itemName = TryParseStringLiteral(stream, allocator, &itemNameLength);
        if (!itemName)
            return PARSE_RES_EOS;

        if (!NvStreamReadByte(stream, &current))
            return PARSE_RES_EOS;

        if (current != ':')
            return PARSE_RES_CANT_PARSE;

        if (NvStreamHasEnded(stream))
            return PARSE_RES_EOS;

        NvJsonObject item;
        ParseResult itemParseResult = TryParseJsonValue(stream, &item, allocator);
        if (itemParseResult != PARSE_RES_SUCCESS)
            return itemParseResult;

        item.key = itemName;

        shputs(items, item);
        itemsCount++;

        if (!NvStreamReadByte(stream, &current))
            return PARSE_RES_EOS;

        if (current == '}')
            break;
        else if (current != ',')
            return PARSE_RES_CANT_PARSE;
    }

    out->type = NV_JSON_TYPE_OBJECT;
    out->value.asObjPtr = items;
    out->length = itemsCount;

    return PARSE_RES_SUCCESS;
}

static ParseResult TryParseJsonNull(NvStream *stream, NvJsonObject *out)
{
    // first char already consumed by TryParseJsonValue
    char data[3];

    if (NvStreamTryRead(stream, data, sizeof(char) * 3, NULL))
    {
        if (!strncmp(data, "ull", 3))
        {
            out->type = NV_JSON_TYPE_NULL;
            out->value.asInt = 0;
            return PARSE_RES_SUCCESS;
        }

        return PARSE_RES_CANT_PARSE;
    }

    return PARSE_RES_EOS;
}

static ParseResult TryParseJsonFalse(NvStream *stream, NvJsonObject *out)
{
    char buffer[4];
    if (!NvStreamTryRead(stream, buffer, sizeof(char) * 4, NULL))
        return PARSE_RES_EOS;

    if (strncmp(buffer, "alse", 4))
        return PARSE_RES_CANT_PARSE;

    out->type = NV_JSON_TYPE_BOOL;
    out->value.asInt = 0;

    return PARSE_RES_SUCCESS;
}

static ParseResult TryParseJsonTrue(NvStream *stream, NvJsonObject *out)
{
    char buffer[3];
    if (!NvStreamTryRead(stream, buffer, sizeof(char) * 3, NULL))
        return PARSE_RES_EOS;

    if (strncmp(buffer, "rue", 3))
        return PARSE_RES_CANT_PARSE;

    out->type = NV_JSON_TYPE_BOOL;
    out->value.asInt = INT64_MAX;

    return PARSE_RES_SUCCESS;
}

static ParseResult TryParseJsonArray(NvStream *stream, NvJsonObject *out, NvAllocator *allocator)
{
    if (NvStreamHasEnded(stream))
        return PARSE_RES_EOS;

    size_t arrayCapacity = INITIAL_ARRAY_LENGTH;
    size_t arraySize = 0;
    NvJsonObject *arrayData = NV_ALLOCATOR_MALLOC(allocator, sizeof(NvJsonObject) * arrayCapacity);

    while (true)
    {
        if (NvStreamHasEnded(stream))
            return PARSE_RES_EOS;

        ParseResult childParseResult = TryParseJsonValue(stream, &arrayData[arraySize], allocator);
        if (childParseResult != PARSE_RES_SUCCESS)
            return childParseResult;

        char end;
        if (!NvStreamReadByte(stream, &end))
            return PARSE_RES_EOS;

        arraySize++;

        if (end == ']')
            break;
        else if (end != ',')
            return PARSE_RES_CANT_PARSE;

        if (arraySize >= arrayCapacity)
        {
            arrayCapacity *= ARRAY_GROWTH_FACTOR;
            arrayData = NV_ALLOCATOR_REALLOC(allocator, arrayData, sizeof(NvJsonObject) * arrayCapacity);
        }
    }

    out->type = NV_JSON_TYPE_ARRAY;
    out->value.asObjPtr = arrayData;
    out->length = arraySize;

    return PARSE_RES_SUCCESS;
}

static ParseResult TryParseJsonValue(NvStream *stream, NvJsonObject *out, NvAllocator *allocator)
{
    NV_ASSERT(out != NULL, "Out json object cannot be NULL");

    char current;
    if (!ConsumeWhitespace(stream, &current))
        return PARSE_RES_EOS;

    ParseResult parseResult;
    switch (current)
    {
    case 'n':
        parseResult = TryParseJsonNull(stream, out);
        break;
    case 't':
        parseResult = TryParseJsonTrue(stream, out);
        break;
    case 'f':
        parseResult = TryParseJsonFalse(stream, out);
        break;
    case '"':
        parseResult = TryParseJsonString(stream, out, allocator);
        break;
    case '[':
        parseResult = TryParseJsonArray(stream, out, allocator);
        break;
    case '{':
        parseResult = TryParseJsonObject(stream, out, allocator);
        break;
    default:
        parseResult = TryParseJsonNumber(stream, out);
        break;
    }

    if (parseResult != PARSE_RES_SUCCESS)
        return parseResult;
    {
        // NV_ASSERT(false, "debug");
    }

    ConsumeWhitespace(stream, &current);
    NvStreamSetPos(stream, -1, true);

    return parseResult;
}

bool NvJsonDeserializeStream(NvJson *out, NvStream *stream)
{
    NV_ASSERT(out != NULL, "Output JSON object cannot be null");

    if (!NvStreamIsReadable(stream))
    {
        fprintf(stderr, "Failed to deserialize JSON stream: stream is not readable.\n");
        return false;
    }

    NvAllocator allocator = NvArenaGetAllocator(&out->memory);
    ParseResult result = TryParseJsonValue(stream, &out->object, &allocator);

    if (result == PARSE_RES_SUCCESS)
        return true;

    char nextChar;
    size_t failPos = NvStreamGetPos(stream);
    if (!NvStreamReadByte(stream, &nextChar))
        nextChar = '?';

    switch (result)
    {
    case PARSE_RES_EOS:
        fprintf(stderr, "JSON deserialize failure: stream ended while parsing value. Stream position: %zu (next character: %c)\n", failPos, nextChar);
        break;
    case PARSE_RES_CANT_PARSE:
        fprintf(stderr, "JSON deserialize failure: couldn't parse JSON value. Stream position: %zu (next character: %c)\n", failPos, nextChar);
        break;
    case PARSE_RES_NUMBER_TOO_LONG:
        fprintf(stderr, "JSON deserialize failure: tried to parse number that is too long. Stream position: %zu (next character: %c)\n", failPos, nextChar);
        break;
    }

    return false;
}

bool NvJsonDeserializeString(NvJson *out, const char *string)
{
    NV_ASSERT(out != NULL, "Output JSON object cannot be null");

    NvStream stream = NvStreamCreateFromString(string);
    return NvJsonDeserializeStream(out, &stream);
}

bool NvJsonDeserializeFile(NvJson *out, const wchar_t *filepath)
{
    // TODO Use stream API to save memory while parsing JSON file
    NV_ASSERT(out != NULL, "Output JSON object cannot be null");

    NvFile file;
    if (!NvFileOpenUnicode(&file, filepath, L"rbR"))
    {
        fprintf(stderr, "Failed to deserialize JSON file: failed to open file.\n");
        return false;
    }

    NvStream stream = NvFileGetStream(&file);
    bool result = NvJsonDeserializeStream(out, &stream);

    NvFileClose(&file);
    return result;
}

void NvJsonDestroy(NvJson *json)
{
    if (!json)
        return;

    NvArenaDestroy(&json->memory);
    memset(json, 0, sizeof(NvJson));
}

bool NvJsonObjectIsType(const NvJsonObject *obj, NvJsonType type)
{
    NV_ASSERT(obj != NULL, "Json object cannot be NULL");
    return obj->type == type;
}

bool NvJsonObjectIsNull(const NvJsonObject *obj)
{
    NV_ASSERT(obj != NULL, "Json object cannot be NULL");
    return NvJsonObjectIsType(obj, NV_JSON_TYPE_NULL);
}

// json array
NvJsonObject *NvJsonArrayGet(const NvJsonObject *obj, size_t index)
{
    ASSERT_OBJ_NOT_NULL(obj);
    ASSERT_OBJ_TYPE(obj, NV_JSON_TYPE_ARRAY);
    NV_ASSERT(obj->length > index, "Json array doesn't contain enough items");

    return &((NvJsonObject *)obj->value.asObjPtr)[index];
}

size_t NvJsonArrayGetSize(const NvJsonObject *obj)
{
    ASSERT_OBJ_NOT_NULL(obj);
    ASSERT_OBJ_TYPE(obj, NV_JSON_TYPE_ARRAY);

    return obj->length;
}

// json object
NvJsonObject *NvJsonObjectGet(NvJsonObject *obj, const char *childName)
{
    ASSERT_OBJ_NOT_NULL(obj);
    ASSERT_OBJ_TYPE(obj, NV_JSON_TYPE_OBJECT);

    return shgetp_null((NvJsonObject *)obj->value.asObjPtr, childName);
}

NvJsonObject *NvJsonObjectGetAt(NvJsonObject *obj, size_t idx)
{
    ASSERT_OBJ_NOT_NULL(obj);
    ASSERT_OBJ_TYPE(obj, NV_JSON_TYPE_OBJECT);

    return &((NvJsonObject *)obj->value.asObjPtr)[idx];
}

size_t NvJsonObjectGetChildrenCount(NvJsonObject *obj)
{
    ASSERT_OBJ_NOT_NULL(obj);
    ASSERT_OBJ_TYPE(obj, NV_JSON_TYPE_OBJECT);

    return shlenu((NvJsonObject *)obj->value.asObjPtr);
}

bool NvJsonObjectHasChildren(const NvJsonObject *obj)
{
    ASSERT_OBJ_NOT_NULL(obj);
    return shlen((NvJsonObject *)obj->value.asObjPtr) > 0;
}

bool NvJsonObjectHasChild(NvJsonObject *obj, const char *childName)
{
    ASSERT_OBJ_NOT_NULL(obj);
    return shgeti((NvJsonObject *)obj->value.asObjPtr, childName) != -1;
}

// json string
const char *NvJsonStringGet(const NvJsonObject *obj)
{
    ASSERT_OBJ_NOT_NULL(obj);
    ASSERT_OBJ_TYPE(obj, NV_JSON_TYPE_STRING);

    return obj->value.asString;
}

// json number
int64_t NvJsonNumberGetInt(const NvJsonObject *obj)
{
    ASSERT_OBJ_NOT_NULL(obj);
    ASSERT_OBJ_TYPE(obj, NV_JSON_TYPE_NUMBER);

    return obj->value.asInt;
}

double NvJsonNumberGetDouble(const NvJsonObject *obj)
{
    ASSERT_OBJ_NOT_NULL(obj);
    ASSERT_OBJ_TYPE(obj, NV_JSON_TYPE_NUMBER);

    return obj->value.asDouble;
}

bool NvJsonNumberIsDouble(const NvJsonObject *obj)
{
    ASSERT_OBJ_NOT_NULL(obj);
    ASSERT_OBJ_TYPE(obj, NV_JSON_TYPE_NUMBER);

    return obj->isDouble;
}

// json bool
bool NvJsonBoolGet(const NvJsonObject *obj)
{
    ASSERT_OBJ_NOT_NULL(obj);
    ASSERT_OBJ_TYPE(obj, NV_JSON_TYPE_BOOL);

    return (bool)obj->value.asInt;
}
