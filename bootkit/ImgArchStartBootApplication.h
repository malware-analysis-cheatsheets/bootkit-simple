#pragma once
#include <efi.h>

/**
 * @brief ImgArchStartBootApplicationをフックする
 * @param[in] BootmgrHandle
 */
EFI_STATUS EFIAPI SetupImgArchStartBootApplication(EFI_HANDLE BootmgrHandle);
