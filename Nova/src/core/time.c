#include <Nova/core/time.h>
#include <time.h>

uint64_t NvTimeUtcGetNow(void)
{
    return (uint64_t)time(NULL);
}

uint64_t NvTimeLocalGetNow(void)
{
    const time_t nowUtc = time(NULL);
    const struct tm *nowLocal = localtime(&nowUtc);
    return (uint64_t)mktime(nowLocal);
}
