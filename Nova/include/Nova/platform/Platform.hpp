#pragma once

#ifdef NV_WINDOWS
#include <Nova/platform/windows/startupData.h>
#include <Nova/platform/windows/WindowData.hpp>
#else
#error "Only windows is supported"
#endif
