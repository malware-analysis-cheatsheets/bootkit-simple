#include "ExitBootServices.h"

#include <efilib.h>

EFI_EXIT_BOOT_SERVICES OriginalExitBootServices = NULL;

VOID* SetServicePointer(IN OUT VOID** ServiceTableFunction, IN VOID* NewFunction, BOOLEAN IsTpl)
{
    VOID* OriginalFunction = NULL;
    EFI_TPL Tpl = TPL_HIGH_LEVEL;

    if (ServiceTableFunction == NULL || NewFunction == NULL)
    {
        return OriginalFunction;
    }

    if (gBS == NULL)
    {
        return OriginalFunction;
    }

    if (IsTpl)
    {
        Tpl = gBS->RaiseTPL(Tpl);
    }

    OriginalFunction = *ServiceTableFunction;
    *ServiceTableFunction = NewFunction;

    if (IsTpl)
    {
        gBS->RestoreTPL(Tpl);
    }

    return OriginalFunction;
}

// winload.efiから呼び出され、ブートサービスを終了させる
EFI_STATUS HookedExitBootServices(EFI_HANDLE ImageHandle, UINTN MapKey)
{
    Print(L"===== HookedExitBootServices =====\r\n");

    SetServicePointer((VOID**)&gBS->ExitBootServices, OriginalExitBootServices, FALSE);

    gBS->Stall(2 * 1000000);

    return gBS->ExitBootServices(ImageHandle, MapKey);
}

