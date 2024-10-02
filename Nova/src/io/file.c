#include <Nova/io/file.h>
#include <Nova/core/string.h>
#include <stdlib.h>

#define ASSERT_FILE_NOT_NULL NV_ASSERT(file != NULL, "File cannot be NULL")
#define DEFAULT_CHUNK_SIZE 1024
#define GET_STREAM_FILE(stream) (NvFile *)stream->data

static size_t FileStreamGetPos(NvStream *stream)
{
    NvFile *file = GET_STREAM_FILE(stream);
    // TODO Check if ftell produce valid results for text files
    return ftell(file->file);
}

static void FileStreamSetPos(NvStream *stream, int64_t pos, bool relative)
{
    NvFile *file = GET_STREAM_FILE(stream);

    if (pos == 0)
        fseek(file->file, 0, SEEK_SET);
    else
        fseek(file->file, pos, relative ? SEEK_CUR : SEEK_SET);
}

static bool FileStreamHasEnded(NvStream *stream)
{
    NvFile *file = GET_STREAM_FILE(stream);
    return feof(file->file);
}

static bool FileStreamTryRead(NvStream *stream, void *buffer, size_t nBytes, size_t *nRead)
{
    NvFile *file = GET_STREAM_FILE(stream);
    size_t _nRead = fread(buffer, 1, nBytes, file->file);

    if (nRead)
        *nRead = _nRead;

    return _nRead == nBytes;
}

bool NvFileOpen(NvFile *out, const char *filepath, const char *mode)
{
    FILE *file = NULL;
    errno_t err = fopen_s(&file, filepath, mode);
    if (err != 0)
    {
        fprintf(stderr, "Failed to open file \"%s\": %s.\n", filepath, strerror(err));
        return false;
    }

    size_t fileSize = _NvFileGetSize(file);
    if (fileSize == -1)
    {
        fprintf(stderr, "Failed to acquire file size for the opened file.\n");
        return false;
    }

    *out = (NvFile){
        .file = file,
        .fileSize = fileSize,
    };

    return true;
}

bool NvFileOpenUnicode(NvFile *out, const wchar_t *filepath, const wchar_t *mode)
{
    FILE *file = NULL;
    errno_t err = _wfopen_s(&file, filepath, mode);
    if (err != 0)
    {
        fprintf(stderr, "Failed to open file \"%ls\": %s.\n", filepath, strerror(err));
        return false;
    }

    *out = (NvFile){
        .file = file,
        .fileSize = _NvFileGetSize(file),
    };

    return true;
}

void NvFileClose(NvFile *file)
{
    if (!file || !file->file)
        return;

    fclose(file->file);
    memset(file, 0, sizeof(NvFile));
}

size_t NvFileRead(NvFile *file, size_t bytesCount, void *buffer)
{
    ASSERT_FILE_NOT_NULL;
    return fread(buffer, sizeof(char), bytesCount, file->file);
}

void *NvFileReadToEnd(NvFile *file, size_t *size)
{
    ASSERT_FILE_NOT_NULL;

    char *buffer = NULL;
    size_t bufferUsed = 0;
    size_t bufferSize = 0;

    while (true)
    {
        if (bufferUsed + DEFAULT_CHUNK_SIZE + 1 > bufferSize)
        {
            bufferSize = bufferUsed + DEFAULT_CHUNK_SIZE + 1;
            buffer = realloc(buffer, bufferSize);
        }

        size_t currentRead = fread(buffer + bufferUsed, sizeof(char), DEFAULT_CHUNK_SIZE, file->file);
        if (currentRead == 0)
            break;

        bufferUsed += currentRead;
    }

    if (ferror(file->file))
    {
        free(buffer);
        return NULL;
    }

    buffer = realloc(buffer, bufferUsed + 1);
    buffer[bufferUsed] = '\0';

    if (size)
        *size = bufferUsed + 1;

    return buffer;
}

char *NvFileReadLine(NvFile *file, size_t bufferSize, void *buffer)
{
    ASSERT_FILE_NOT_NULL;
    return fgets(buffer, bufferSize, file->file);
}

size_t NvFileWrite(NvFile *file, size_t bytesCount, const void *data)
{
    ASSERT_FILE_NOT_NULL;
    return fwrite(data, sizeof(char), bytesCount, file->file);
}

NvStream NvFileGetStream(NvFile *file)
{
    return (NvStream){
        .data = file,
        // TODO Upon file opening store its access flags
        .flags = NV_STREAM_FLAG_READABLE | NV_STREAM_FLAG_WRITABLE,
        ._getPosFn = FileStreamGetPos,
        ._setPosFn = FileStreamSetPos,
        ._hasEnded = FileStreamHasEnded,
        ._tryReadFn = FileStreamTryRead,
    };
}
