#pragma once
#include <Nova/core/build.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    NV_STREAM_FLAG_READABLE,
    NV_STREAM_FLAG_WRITABLE,
} NvStreamFlags;

typedef struct NvStream
{
    void (*_setPosFn)(struct NvStream *stream, int64_t pos, bool relative);
    size_t (*_getPosFn)(struct NvStream *stream);
    bool (*_tryReadFn)(struct NvStream *stream, void *buffer, size_t nBytes, size_t *nRead);
    bool (*_hasEnded)(struct NvStream *stream);
    NvStreamFlags flags;
    void *data;
} NvStream;

NV_API size_t NvStreamGetPos(NvStream *stream);
NV_API void NvStreamSetPos(NvStream *stream, int64_t pos, bool relative);
NV_API void NvStreamReset(NvStream *stream);
NV_API bool NvStreamIsReadable(const NvStream *stream);
NV_API bool NvStreamIsWritable(const NvStream *stream);
NV_API bool NvStreamTryRead(NvStream *stream, void *buffer, size_t nBytes, size_t *nRead);
NV_API bool NvStreamReadByte(NvStream *stream, int8_t *outByte);
NV_API bool NvStreamHasEnded(NvStream *stream);

NV_API NvStream NvStreamCreateFromString(const char *string);
NV_API NvStream NvStreamCreateFromBuffer(void *buffer, size_t bufferSize, NvStreamFlags flags);
