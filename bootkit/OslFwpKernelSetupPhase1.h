#pragma once
#include <efi.h>

/**
* @brief OslFwpKernelSetupPhase1をフックする
* @param[in] WinloadBase winload.efiのベースアドレス
*/
EFI_STATUS EFIAPI SetupOslFwpKernelSetupPhase1(VOID* WinloadBase);
