#include <Nova/core/exception.h>
#include <Nova/core/build.h>
#include <stdio.h>
#include <stdlib.h>

static NvExceptionHandler s_Handler;
static bool s_Thrown;

void NvExceptionSetHandler(NvExceptionHandler handler)
{
    s_Handler = handler;
}

void NvExceptionThrow(const char *message)
{
    s_Thrown = true;

#if NV_DEBUG
    fprintf(stderr, message);
    fflush(stderr);

    NV_ASSERT(false, "Exception occurred.");
#endif

    if (s_Handler)
        s_Handler(message);

    exit(1);
}

bool NvExceptionIsSet(void)
{
    return s_Thrown;
}
