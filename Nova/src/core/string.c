#include <Nova/core/string.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define SV_EMPTY(x) (x.data == NULL || x.length == 0)
#define SV_MAKE_EMPTY \
    (NvStringView) { 0 }

NvStringView NvStringLStrip(const NvStringView sv)
{
    if (SV_EMPTY(sv))
        return sv;

    char *ptr = sv.data;
    size_t newLength = sv.length;
    while (isspace(*ptr))
    {
        ptr++;
        newLength--;
    }

    return (NvStringView){
        .data = ptr,
        .length = newLength,
    };
}

NvStringView NvStringRStrip(const NvStringView sv)
{
    if (SV_EMPTY(sv))
        return SV_MAKE_EMPTY;

    char *back = sv.data + sv.length;
    size_t newLength = sv.length;
    while (isspace(*(--back)))
    {
        *back = '\0';
        newLength--;
    }

    return (NvStringView){
        .data = sv.data,
        .length = newLength,
    };
}

NvStringView NvStringStrip(const NvStringView sv)
{
    return NvStringLStrip(NvStringRStrip(sv));
}

NvStringView NvStringSplit(const NvStringView sv, char delim, NvStringView *rest)
{
    if (SV_EMPTY(sv))
        return SV_MAKE_EMPTY;

    char *delimPtr = strchr(sv.data, (int)delim);
    if (delimPtr == NULL)
    {
        if (rest != NULL)
            *rest = SV_MAKE_EMPTY;

        return SV_MAKE_EMPTY;
    }

    *delimPtr = '\0';

    NvStringView result = {
        .data = sv.data,
        .length = delimPtr - sv.data,
    };

    if (rest != NULL)
        *rest = (NvStringView){
            .data = delimPtr + 1,
            .length = sv.length - result.length - 1,
        };

    return result;
}

char *NvStringDuplicate(const NvStringView sv, NvAllocator *allocator)
{
    if (SV_EMPTY(sv))
        return NULL;

    size_t size = sizeof(char) * (sv.length + 1);
    char *buffer = NV_ALLOCATOR_MALLOC(allocator, size);
    memcpy(buffer, sv.data, size);

    return buffer;
}

NvStringView NvStringToLower(const NvStringView sv)
{
    if (SV_EMPTY(sv))
        return SV_MAKE_EMPTY;

    for (char *p = sv.data; *p; ++p)
        *p = tolower(*p);

    return (NvStringView){
        .data = sv.data,
        .length = sv.length,
    };
}

NvStringView NvStringToUpper(const NvStringView sv)
{
    if (SV_EMPTY(sv))
        return SV_MAKE_EMPTY;

    for (char *p = sv.data; *p; ++p)
        *p = toupper(*p);

    return (NvStringView){
        .data = sv.data,
        .length = sv.length,
    };
}

bool NvStringTryParseInt32(const NvStringView sv, int32_t *out)
{
    if (SV_EMPTY(sv))
        return false;

    char *end = NULL;
    int32_t value = strtol(sv.data, &end, 10);

    if (sv.data == end)
        return false;

    *out = value;
    return true;
}

bool NvStringTryParseInt64(const NvStringView sv, int64_t *out)
{
    if (SV_EMPTY(sv))
        return false;

    char *end = NULL;
    int64_t value = strtoll(sv.data, &end, 10);

    if (sv.data == end)
        return false;

    *out = value;
    return true;
}

bool NvStringTryParseFloat(const NvStringView sv, float *out)
{
    if (SV_EMPTY(sv))
        return false;

    char *end = NULL;
    float value = strtof(sv.data, &end);

    if (sv.data == end)
        return false;

    *out = value;
    return true;
}

bool NvStringTryParseDouble(const NvStringView sv, double *out)
{
    if (SV_EMPTY(sv))
        return false;

    char *end = NULL;
    double value = strtod(sv.data, &end);

    if (sv.data == end)
        return false;

    *out = value;
    return true;
}

NvStringView NvStringChopLeft(const NvStringView sv, size_t count)
{
    if (SV_EMPTY(sv))
        return SV_MAKE_EMPTY;

    if (count < 0)
        return sv;

    char *new = (sv.data + count);

    return (NvStringView){
        .data = new,
        .length = sv.length - count,
    };
}

NvStringView NvStringChopRight(const NvStringView sv, size_t count)
{
    if (SV_EMPTY(sv))
        return SV_MAKE_EMPTY;

    if (count < 0)
        return sv;

    size_t newLength = sv.length - count;
    char *end = (sv.data + newLength);
    *end = '\0';

    return (NvStringView){
        .data = sv.data,
        .length = newLength,
    };
}

bool NvStringContains(const NvStringView sv, char character)
{
    if (SV_EMPTY(sv))
        return false;

    return strchr(sv.data, (int)character) != NULL;
}

bool NvStringContainsAny(const NvStringView sv, const NvStringView characters)
{
    if (SV_EMPTY(sv))
        return false;

    for (size_t i = 0; i < sv.length; i++)
        for (size_t checkedI = 0; checkedI < characters.length; checkedI++)
            if (sv.data[i] == characters.data[checkedI])
                return true;

    return false;
}

NvStringView NvStringReplace(const NvStringView sv, char from, char to)
{
    if (SV_EMPTY(sv))
        return SV_MAKE_EMPTY;

    if (from == to)
        return sv;

    char *data = sv.data;
    for (size_t i = 0; i < sv.length; i++)
        if (data[i] == from)
            data[i] = to;

    return (NvStringView){
        .data = data,
        .length = sv.length,
    };
}

NvStringView NvStringSubstring(const NvStringView sv, size_t begin, int64_t end)
{
    NV_ASSERT(begin > 0, "Substring begin index cannot be negative.");

    if (SV_EMPTY(sv))
        return SV_MAKE_EMPTY;

    if (end < 0)
        end = sv.length - abs(end);

    char *data = sv.data;
    data[end] = '\0';

    return (NvStringView){
        .data = data + begin,
        .length = end - begin,
    };
}

char *NvStringConcat(size_t partsCount, NvStringView *parts, NvAllocator *allocator)
{
    size_t requiredBufferLength = 1;
    for (size_t i = 0; i < partsCount; i++)
        requiredBufferLength += parts[i].length;

    size_t bufferSize = requiredBufferLength * sizeof(char);
    char *buffer = NV_ALLOCATOR_MALLOC(allocator, bufferSize);

    size_t currentOffset = 0;
    for (size_t i = 0; i < partsCount; i++)
    {
        strcpy(buffer + currentOffset, parts[i].data);
        currentOffset += parts[i].length;
    }

    return buffer;
}

bool NvStringStartsWithChar(const NvStringView sv, char value)
{
    if (SV_EMPTY(sv))
        return false;

    return sv.data[0] == value;
}

bool NvStringEndsWithChar(const NvStringView sv, char value)
{
    if (SV_EMPTY(sv))
        return false;

    return sv.data[sv.length - 1] == value;
}

bool NvStringIsEmpty(const NvStringView sv)
{
    if (SV_EMPTY(sv))
        return true;

    for (size_t i = 0; i < sv.length; i++)
        if (!isspace(sv.data[i]))
            return false;

    return true;
}

bool NvStringStartsWith(const NvStringView sv, const NvStringView other)
{
    if (SV_EMPTY(other))
        return true;

    if (SV_EMPTY(sv) && !SV_EMPTY(other))
        return false;

    if (sv.length < other.length)
        return false;

    return !strncmp(sv.data, other.data, other.length);
}
