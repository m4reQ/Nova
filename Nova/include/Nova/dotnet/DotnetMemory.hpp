#pragma once

namespace Nova
{
    namespace DotnetMemory
    {
        void *AllocHGlobal(size_t size);
        void FreeHGlobal(void *memory);
    };
}
