#pragma once
#include <efi.h>

typedef struct _MAP_INFO
{
    VOID* AllocatedBuffer;
    EFI_STATUS AllocatedBufferStatus;
}MAP_INFO, * PMAP_INFO;
