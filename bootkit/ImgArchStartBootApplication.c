#include "ImgArchStartBootApplication.h"
#include "Trampoline.h"
#include "Pe.h"

#include <efilib.h>

// ImgArchStartBootApplicationの型
typedef EFI_STATUS(EFIAPI* IMG_ARCH_START_BOOT_APPLICATION)(VOID*, VOID*, UINT32, UINT8, VOID*);
// ImgArchStartBootApplicationの先頭アドレス
IMG_ARCH_START_BOOT_APPLICATION ImgArchStartBootApplication;

// ImgArchStartBootApplicationの元命令を保持
CHAR8 OriginalImgArchStartBootApplication[TRAMPOLINE_SIZE] = { 0 };

// ImgArchStartBootApplicationのシグネチャ
// 48 8B C4 48 89 58 20 44  89 40 18 48 89 50 10 48
// 89 48 08 55 56 57 41 54  41 55 41 56 41 57 48 8D
// 68 A9 48 81 EC C0 00 00  00 48 8B F1 45 8B C0 41
CONST CHAR8 SigImgArchStartBootApplication[] = {
    "\x48\x8B\xC4\x48\x89\x58\x20\x44\x89\x40\x18\x48\x89\x50\x10\x48"
    "\x89\x48\x08\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D"
    "\x68\xA9\x48\x81\xEC\xC0\x00\x00\x00\x48\x8B\xF1\x45\x8B\xC0\x41"
};

CONST CHAR8 MaskImgArchStartBootApplication[] = {
    "xxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxx"
};

EFI_STATUS EFIAPI HookedImgArchStartBootApplication(VOID* AppEntry, VOID* ImageBase, UINT32 ImageSize, UINT8 BootOption, VOID* ReturnArguments)
{
    UINTN Event;

    Print(L"===== HookedImgArchStartBootApplication =====\r\n");

    do
    {
        // bootmgfw.efi!ImgArchStartBootApplicationをアンフックする
        TrampolineUnhook(ImgArchStartBootApplication, OriginalImgArchStartBootApplication);
        if (*(UINT16*)ImageBase != IMAGE_EFI_SIGNATURE)
        {
            Print(L"[-] Failed to load image magic(0x%x)\r\n", *(UINT16*)ImageBase);
            break;
        }
        Print(L"[+] Load image path winload.efi\r\n");
        Print(L"[+]     -> ImageBase = 0x%llx\r\n", ImageBase);
        Print(L"[+]     -> ImageSize = 0x%llx\r\n", ImageSize);
    } while (FALSE);

    Print(L"\n%EPress any key to start.%N\n");
    gST->ConIn->Reset(gST->ConIn, FALSE);
    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Event);

    return ImgArchStartBootApplication(AppEntry, ImageBase, ImageSize, BootOption, ReturnArguments);
}

EFI_STATUS EFIAPI SetupImgArchStartBootApplication(EFI_HANDLE BootmgrHandle)
{
    EFI_STATUS Status;

    EFI_LOADED_IMAGE* Bootmgr;
    VOID* BaseImgArchStartBootApplication = NULL;

    do
    {
        Status = gBS->HandleProtocol(BootmgrHandle, &gEfiLoadedImageProtocolGuid, (VOID**)&Bootmgr);
        if (EFI_ERROR(Status))
        {
            Print(L"[-] Failed to get the boot manager image(Status=%d)\r\n", Status);
            break;
        }

        // bootmgfw.efi!ImgArchStartBootApplicationの先頭アドレスを探す
        BaseImgArchStartBootApplication = FindPattern(Bootmgr->ImageBase, Bootmgr->ImageSize, SigImgArchStartBootApplication, MaskImgArchStartBootApplication);
        if (BaseImgArchStartBootApplication == NULL)
        {
            Print(L"[-] Failed to find ImgArchStartBootApplication\r\n");
            Status = EFI_NOT_FOUND;
            break;
        }
        Print(L"[+] ImgArchStartBootApplication is 0x%llx\r\n", BaseImgArchStartBootApplication);

        // bootmgfw.efi!ImgArchStartBootApplicationをフックする
        ImgArchStartBootApplication = TrampolineHook(HookedImgArchStartBootApplication, BaseImgArchStartBootApplication, OriginalImgArchStartBootApplication);
        if (ImgArchStartBootApplication == NULL)
        {
            Print(L"[-] Failed to hook ImgArchStartBootApplication\r\n");
            Status = EFI_NOT_FOUND;
            break;
        }
        Print(L"[+] Hooked ImgArchStartBootApplication\r\n");
    } while (FALSE);

    return Status;
}
