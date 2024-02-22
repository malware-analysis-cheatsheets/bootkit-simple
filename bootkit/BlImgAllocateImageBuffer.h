#pragma once
#include <efi.h>

/**
* @brief BlImgAllocateImageBufferをフックする
* @param[in] WinloadBase winload.efiのベースアドレス
*/
EFI_STATUS EFIAPI SetupBlImgAllocateImageBuffer(VOID* WinloadBase);
