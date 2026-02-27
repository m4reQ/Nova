#pragma once

#define NV_ALIGN_STD140 alignas(16)
#define NV_ALIGN_STD430 alignas(16)
#define NV_STD140_VEC3(type, name) NV_ALIGN_STD140 type name; float _pad##name