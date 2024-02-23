#include "OslFwpKernelSetupPhase1.h"
#include "Trampoline.h"
#include "Serial.h"
#include "Mapper.h"

#include <efilib.h>

typedef struct _LOADER_PARAMETER_BLOCK
{
    UINT32 OsMajorVersion;
    UINT32 OsMinorVersion;
    UINT32 Size;
    UINT32 OsLoaderSecurityVersion;
    LIST_ENTRY LoadOrderListHead;
} LOADER_PARAMETER_BLOCK;

// OslFwpKernelSetupPhase1の型
typedef EFI_STATUS(EFIAPI* OSL_FWP_KERNEL_SETUP_PHASE_1)(LOADER_PARAMETER_BLOCK*);
// OslFwpKernelSetupPhase1の先頭アドレス
OSL_FWP_KERNEL_SETUP_PHASE_1 OslFwpKernelSetupPhase1;

// OslFwpKernelSetupPhase1の元命令を保持
CHAR8 OriginalOslFwpKernelSetupPhase1[TRAMPOLINE_SIZE] = { 0 };

// OslFwpKernelSetupPhase1のシグネチャ
// 48 89 4C 24 08 55 53 56  57 41 54 41 55 41 56 41
// 57 48 8D 6C 24 E1 48 81  EC A8 00 00 00 45 33 E4
CONST CHAR8 SigOslFwpKernelSetupPhase1[] = {
    "\x48\x89\x4C\x24\x08\x55\x53\x56\x57\x41\x54\x41\x55\x41\x56\x41"
    "\x57\x48\x8D\x6C\x24\xE1\x48\x81\xEC\xA8\x00\x00\x00\x45\x33\xE4"
};

CONST CHAR8 MaskOslFwpKernelSetupPhase1[] = {
    "xxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxx"
};

extern MAP_INFO Mapper;

EFI_STATUS EFIAPI HookedOslFwpKernelSetupPhase1(LOADER_PARAMETER_BLOCK* LoaderParameterBlock)
{
    SerialPrint(L"===== HookedOslFwpKernelSetupPhase1 =====\r\n");

    do
    {
        // winload.efi!OslFwpKernelSetupPhase1をアンフックする
        TrampolineUnhook(OslFwpKernelSetupPhase1, OriginalOslFwpKernelSetupPhase1, FALSE);

        if (!Mapper.AllocatedBuffer)
        {
            SerialPrint(L"[-] Failed to allocate mapper\r\n");
            break;
        }
    } while (FALSE);

    return OslFwpKernelSetupPhase1(LoaderParameterBlock);
}

EFI_STATUS EFIAPI SetupOslFwpKernelSetupPhase1(VOID* WinloadBase)
{
    EFI_STATUS Status = EFI_NOT_FOUND;

    VOID* BaseOslFwpKernelSetupPhase1 = NULL;

    do
    {
        Print(L"[+] Find OslFwpKernelSetupPhase1 in winload.efi\r\n");

        // winload.efi!OslFwpKernelSetupPhase1の先頭アドレスを探す
        BaseOslFwpKernelSetupPhase1 = FindPatternFromSections(WinloadBase, SigOslFwpKernelSetupPhase1, MaskOslFwpKernelSetupPhase1);
        if (BaseOslFwpKernelSetupPhase1 == NULL)
        {
            Print(L"[-] Failed to find OslFwpKernelSetupPhase1\r\n");
            break;
        }
        Print(L"[+] OslFwpKernelSetupPhase1 is 0x%llx\r\n", BaseOslFwpKernelSetupPhase1);

        // winload.efi!OslFwpKernelSetupPhase1をフック
        // ntoskrnl.exeのアドレスを取得、ターゲットドライバのエントリーポイントをフックする
        OslFwpKernelSetupPhase1 = TrampolineHook(HookedOslFwpKernelSetupPhase1, BaseOslFwpKernelSetupPhase1, OriginalOslFwpKernelSetupPhase1, TRUE);
        if (OslFwpKernelSetupPhase1 == NULL)
        {
            Print(L"[-] Failed to hook OslFwpKernelSetupPhase1\r\n");
            break;
        }
        Print(L"[+] Hooked OslFwpKernelSetupPhase1\r\n");

        Status = EFI_SUCCESS;
    } while (FALSE);

    return Status;
}
