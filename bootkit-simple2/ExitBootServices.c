#include "ExitBootServices.h"
#include "Pe.h"

#include <efilib.h>

EFI_EXIT_BOOT_SERVICES OriginalExitBootServices = NULL;

UINT64 RetExitBootServices = 0;

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

UINT64 FindBaseWinload(UINT64 Addr)
{
    UINT64 Status = 0;

    UINT64 BasePe = 0;

    do
    {
        if (Addr == 0)
        {
            break;
        }

        BasePe = (Addr & (~0xFFF));
        for (UINT32 i = 0; i < 0x10; i++)
        {
            BasePe -= 0x1000;
            if (*(UINT16*)BasePe == IMAGE_EFI_SIGNATURE)
            {
                Status = BasePe;
                break;
            }
        }
    } while (FALSE);

    return Status;
}

// winload.efiから呼び出され、ブートサービスを終了させる
EFI_STATUS HookedExitBootServices(EFI_HANDLE ImageHandle, UINTN MapKey)
{
    UINT64 BaseWinload = 0;

    Print(L"===== HookedExitBootServices =====\r\n");

    do
    {
        if (RetExitBootServices == 0)
        {
            Print(L"[-] Failed to find return address of ExitBootServices\r\n");
            break;
        }
        Print(L"[+] RetExitBootServices = 0x%llx\r\n", RetExitBootServices);

        BaseWinload = FindBaseWinload(RetExitBootServices);
        if (BaseWinload == 0)
        {
            Print(L"[-] Failed to find base address of ExitBootServices\r\n");
            break;
        }
        Print(L"[+] BaseWinload = 0x%llx\r\n", BaseWinload);

        SetServicePointer((VOID**)&gBS->ExitBootServices, OriginalExitBootServices, FALSE);

        gBS->Stall(2 * 1000000);
    } while (FALSE);

    return gBS->ExitBootServices(ImageHandle, MapKey);
}

