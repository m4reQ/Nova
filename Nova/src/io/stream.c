#include <Nova/io/stream.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ASSERT_STREAM_VALID(stream) NV_ASSERT(stream && stream->_setPosFn && stream->_getPosFn && stream->_tryReadFn, "Stream is not valid (NULL or missing members)")
#define GET_STREAM_BUFFER(stream) (BufferStreamData *)stream->data

typedef struct
{
    void *buffer;
    size_t bufferSize;
    size_t currentPos;
} BufferStreamData;

static size_t BufferGetPos(NvStream *stream)
{
    BufferStreamData *data = GET_STREAM_BUFFER(stream);
    return data->currentPos;
}

static void BufferSetPos(NvStream *stream, int64_t pos, bool relative)
{
    BufferStreamData *data = GET_STREAM_BUFFER(stream);
    data->currentPos = relative ? data->currentPos + pos : pos;
}

static bool BufferHasEnded(NvStream *stream)
{
    BufferStreamData *data = GET_STREAM_BUFFER(stream);
    return data->currentPos >= data->bufferSize;
}

static bool BufferTryRead(NvStream *stream, void *buffer, size_t nBytes, size_t *nRead)
{
    BufferStreamData *data = GET_STREAM_BUFFER(stream);

    size_t maxRead = min(data->bufferSize - data->currentPos, nBytes);

    // TODO If using buffer with readable and writable flags, instead of copying data just reassign pointer
    memcpy(buffer, (char *)data->buffer + data->currentPos, maxRead);

    if (nRead)
        *nRead = maxRead;

    data->currentPos += maxRead;

    return maxRead == nBytes;
}

size_t NvStreamGetPos(NvStream *stream)
{
    ASSERT_STREAM_VALID(stream);
    return stream->_getPosFn(stream);
}

void NvStreamSetPos(NvStream *stream, int64_t pos, bool relative)
{
    ASSERT_STREAM_VALID(stream);
    NV_ASSERT(!(!relative && pos < 0), "Cannot set absolute stream position to negative value");
    stream->_setPosFn(stream, pos, relative);
}

void NvStreamReset(NvStream *stream)
{
    ASSERT_STREAM_VALID(stream);
    stream->_setPosFn(stream, 0, false);
}

bool NvStreamIsReadable(const NvStream *stream)
{
    ASSERT_STREAM_VALID(stream);
    return (stream->flags & NV_STREAM_FLAG_READABLE) == NV_STREAM_FLAG_READABLE;
}

bool NvStreamIsWritable(const NvStream *stream)
{
    ASSERT_STREAM_VALID(stream);
    return (stream->flags & NV_STREAM_FLAG_WRITABLE) == NV_STREAM_FLAG_WRITABLE;
}

bool NvStreamTryRead(NvStream *stream, void *buffer, size_t nBytes, size_t *nRead)
{
    ASSERT_STREAM_VALID(stream);

    if (!NvStreamIsReadable(stream))
    {
        fprintf(stderr, "Stream is not readable.\n");
        return false;
    }

    return stream->_tryReadFn(stream, buffer, nBytes, nRead);
}

bool NvStreamReadByte(NvStream *stream, int8_t *outByte)
{
    ASSERT_STREAM_VALID(stream);

    if (!NvStreamIsReadable(stream))
    {
        fprintf(stderr, "Stream is not readable.\n");
        return false;
    }

    return NvStreamTryRead(stream, outByte, sizeof(char), NULL);
}

bool NvStreamHasEnded(NvStream *stream)
{
    ASSERT_STREAM_VALID(stream);
    return stream->_hasEnded(stream);
}

NvStream NvStreamCreateFromString(const char *string)
{
    return NvStreamCreateFromBuffer(
        (void *)string,
        sizeof(char) * (strlen(string) + 1),
        NV_STREAM_FLAG_READABLE);
}

NvStream NvStreamCreateFromBuffer(void *buffer, size_t bufferSize, NvStreamFlags flags)
{
    // TODO Dont allocate memory for string stream data
    BufferStreamData *data = malloc(sizeof(BufferStreamData));
    *data = (BufferStreamData){
        .buffer = buffer,
        .bufferSize = bufferSize,
        .currentPos = 0,
    };

    return (NvStream){
        .data = data,
        .flags = flags,
        ._getPosFn = BufferGetPos,
        ._setPosFn = BufferSetPos,
        ._hasEnded = BufferHasEnded,
        ._tryReadFn = BufferTryRead,
    };
}
