#pragma once
#include <efi.h>

/**
* @brief OslArchTransferToKernelをフックする
*/
EFI_STATUS EFIAPI SetupOslArchTransferToKernel(VOID* WinloadBase);
