#include "SetVirtualAddressMap.h"
#include "ExitBootServices.h"
#include "Serial.h"

#include <efilib.h>

EFI_SET_VIRTUAL_ADDRESS_MAP OriginalSetVirtualAddressMap = NULL;

UINT64 PhysicalAddress = 0;
UINT64 VirtualAddress = 0;

EFI_STATUS EFIAPI HookedSetVirtualAddressMap(
    UINTN MemoryMapSize,
    UINTN DescriptorSize,
    UINT32 DescriptorVersion,
    EFI_MEMORY_DESCRIPTOR* VirtualMap)
{
    INTN Size;
    EFI_MEMORY_DESCRIPTOR* Map;
    UINTN Length = 0;
    EFI_PHYSICAL_ADDRESS Pointer;

    SerialPrint(L"===== HookedSetVirtualAddressMap =====\r\n");

    do
    {
        // アンフック
        SetServicePointer((VOID**)&gRT->SetVirtualAddressMap, OriginalSetVirtualAddressMap, FALSE);

        Size = MemoryMapSize / DescriptorSize;
        Map = VirtualMap;

        SerialPrint(L"[+] Map size = %d\r\n", Size);
        for (INTN i = 0; i < Size; i++)
        {
            Length = Map->NumberOfPages * 0x1000;
            Pointer = Map->PhysicalStart;

            SerialPrint(L"[+] Number of %d\r\n", i);
            SerialPrint(L"[+]     -> PhysicalStart=0x%llx\r\n", Pointer);
            SerialPrint(L"[+]     -> VirtualStart=0x%llx\r\n", Map->VirtualStart);
            SerialPrint(L"[+]     -> NumberOfPages=0x%llx\r\n", Length);

            if ((Pointer <= PhysicalAddress) && (PhysicalAddress < (Pointer + Length)))
            {
                VirtualAddress = PhysicalAddress - Pointer + Map->VirtualStart;
                SerialPrint(L"[+] Virtual address = 0x%llx\r\n", VirtualAddress);
                break;
            }

            Map = (EFI_MEMORY_DESCRIPTOR*)((INT8*)Map + DescriptorSize);
        }
    } while (FALSE);

    return gRT->SetVirtualAddressMap(MemoryMapSize, DescriptorSize, DescriptorVersion, VirtualMap);
}
