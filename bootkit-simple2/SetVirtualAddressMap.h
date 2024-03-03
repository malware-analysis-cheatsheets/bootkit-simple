#pragma once
#include <efi.h>

EFI_STATUS EFIAPI HookedSetVirtualAddressMap(
    UINTN MemoryMapSize, 
    UINTN DescriptorSize, 
    UINT32 DescriptorVersion, 
    EFI_MEMORY_DESCRIPTOR* VirtualMap);
