#include "Print.h"

#include <efilib.h>

EFI_STATUS PrintLoadedImageInfo(EFI_HANDLE* ImageHandle)
{
    EFI_STATUS Status;
    EFI_LOADED_IMAGE_PROTOCOL* LoadedImageInfo = NULL;
    CHAR16* Path = NULL;

    do
    {
        Status = gBS->OpenProtocol(*ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID**)&LoadedImageInfo, *ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
        if (EFI_ERROR(Status))
        {
            Print(L"[-] Failed to OpenProtocol(Status: %d)\r\n", Status);
            break;
        }

        Path = DevicePathToStr(LoadedImageInfo->FilePath);
        if (Path == NULL)
        {
            Print(L"[-] Failed to DevicePathToStr\r\n");
            break;
        }

        Print(L"[+] Loaded image path is %s\r\n", Path);
        Print(L"[+]     -> ImageBase = 0x%llx\r\n", LoadedImageInfo->ImageBase);
        Print(L"[+]     -> ImageSize = 0x%llx\r\n", LoadedImageInfo->ImageSize);
    } while (FALSE);

    return Status;
}
