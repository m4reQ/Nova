#include <Nova/core/file.h>
#include <Nova/core/string.h>
#include <stdlib.h>

#define ASSERT_FILE_NOT_NULL NV_ASSERT(file != NULL, "File cannot be NULL")

bool NvFileOpen(NvFile *out, const char *filepath, const char *mode)
{
    FILE *file = NULL;
    errno_t err = fopen_s(&file, filepath, mode);
    if (err != 0)
    {
        fprintf(stderr, "Failed to open file \"%s\": %s.\n", filepath, strerror(err));
        return false;
    }

    *out = (NvFile){
        .file = file,
        .fileSize = _NvFileGetSize(file),
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

size_t NvFileReadToEnd(NvFile *file, size_t bufferSize, void *buffer)
{
    ASSERT_FILE_NOT_NULL;

    if (bufferSize < file->fileSize)
    {
        fprintf(stderr, "Failed to read all file contents, provided buffer is too small.\n");
        return -1;
    }

    return fread(buffer, sizeof(char), file->fileSize, file->file);
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
