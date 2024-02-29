#include "Bootmgfw.h"

#include <efilib.h>

STATIC CHAR16* WINDOWS_BOOTMGR_PATH = L"\\efi\\microsoft\\boot\\bootmgfw.efi";

EFI_DEVICE_PATH* GetWindowsBootmgrDevicePath(VOID)
{
    EFI_STATUS Status = EFI_SUCCESS;
    UINTN HandleCount = 0;
    EFI_HANDLE* Handles;
    EFI_DEVICE_PATH* DevicePath = NULL;
    EFI_HANDLE ImageHandle = { 0 };

    do
    {
        // Simple File System Protocolのハンドルを全て取得
        Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &HandleCount, &Handles);
        if (EFI_ERROR(Status))
        {
            Print(L"[-] Failed to get filesystem handles(Status: %d)\r\n", Status);
            break;
        }

        for (UINTN i = 0; i < HandleCount && !DevicePath; i++)
        {
            EFI_FILE_IO_INTERFACE* FileSystem;

            // プロトコルインスタンスを開く
            Status = gBS->OpenProtocol(Handles[i], &gEfiSimpleFileSystemProtocolGuid, (VOID**)&FileSystem, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
            if (EFI_ERROR(Status))
            {
                continue;
            }

            EFI_FILE_HANDLE Volume;
            // ボリュームを開き、ルートディレクトリへのファイルハンドルを返す
            Status = FileSystem->OpenVolume(FileSystem, &Volume);
            if (!EFI_ERROR(Status))
            {
                EFI_FILE_HANDLE File;

                // ファイルを開く
                Status = Volume->Open(Volume, &File, WINDOWS_BOOTMGR_PATH, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
                if (!EFI_ERROR(Status))
                {
                    Volume->Close(File);
                    DevicePath = FileDevicePath(Handles[i], WINDOWS_BOOTMGR_PATH);

                }
            }

            gBS->CloseProtocol(Handles[i], &gEfiSimpleFileSystemProtocolGuid, ImageHandle, NULL);
        }
    } while (FALSE);

    if (DevicePath)
    {
        FreePool(DevicePath);
    }

    return DevicePath;
}
