#pragma once
#include <efi.h>

VOID* SetServicePointer(IN OUT VOID** ServiceTableFunction, IN VOID* NewFunction, BOOLEAN IsTpl);

EFI_STATUS EFIAPI _ExitBootServices(EFI_HANDLE ImageHandle, UINTN MapKey);

EFI_STATUS EFIAPI HookedExitBootServices(EFI_HANDLE ImageHandle, UINTN MapKey);
