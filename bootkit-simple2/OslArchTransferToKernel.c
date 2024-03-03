#include "OslArchTransferToKernel.h"
#include "Trampoline.h"
#include "Pe.h"
#include "Serial.h"

#include <efilib.h>

typedef struct _PARAMETER_BLOCK
{
    ULONG OsMajorVersion;
    ULONG OsMinorVersion;
    ULONG Length;
    ULONG Reserved;
    LIST_ENTRY LoadOrderListHead;
    LIST_ENTRY MemoryDescriptorListHead;
    LIST_ENTRY BootDriverListHead;
} PARAMETER_BLOCK, * PPARAMETER_BLOCK;

// OslArchTransferToKernelの型
typedef EFI_STATUS(EFIAPI* OSL_ARCH_TRANSFER_TO_KERNEL)(PPARAMETER_BLOCK, VOID*);
// OslArchTransferToKernelの先頭アドレス
OSL_ARCH_TRANSFER_TO_KERNEL OslArchTransferToKernel;

// OslArchTransferToKernelの元命令を保持
CHAR8 OriginalOslArchTransferToKernel[TRAMPOLINE_SIZE] = { 0 };

// OslArchTransferToKernelのシグネチャ
// 33 F6 4C 8B E1 4C 8B EA  0F 09 48 2B C0 66 8E D0
// 48 8B 25 C9 C3 08 00 48  8D 05 F2 C3 08 00 48 8D
CONST CHAR8 SigOslArchTransferToKernel[] = {
    "\x33\xF6\x4C\x8B\xE1\x4C\x8B\xEA\x0F\x09\x48\x2B\xC0\x66\x8E\xD0"
    "\x48\x8B\x25\xC9\xC3\x08\x00\x48\x8D\x05\xF2\xC3\x08\x00\x48\x8D"
};

CONST CHAR8 MaskOslArchTransferToKernel[] = {
    "xxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxx"
};

EFI_STATUS EFIAPI HookedOslArchTransferToKernel(PPARAMETER_BLOCK LoaderBlock, VOID* Entry)
{
    SerialPrint(L"===== HookedOslArchTransferToKernel =====\r\n");

    return OslArchTransferToKernel(LoaderBlock, Entry);
}

EFI_STATUS EFIAPI SetupOslArchTransferToKernel(VOID* WinloadBase)
{
    EFI_STATUS Status = EFI_NOT_FOUND;

    VOID* BaseOslArchTransferToKernel = NULL;

    do
    {
        Print(L"[+] Find OslArchTransferToKernel in winload.efi\r\n");

        // winload.efi!OslArchTransferToKernelの先頭アドレスを探す
        BaseOslArchTransferToKernel = FindPatternFromSections(WinloadBase, SigOslArchTransferToKernel, MaskOslArchTransferToKernel);
        if (BaseOslArchTransferToKernel == NULL)
        {
            Print(L"[-] Failed to find OslArchTransferToKernel\r\n");
            break;
        }
        Print(L"[+] OslArchTransferToKernel is 0x%llx\r\n", BaseOslArchTransferToKernel);

        // winload.efi!OslArchTransferToKernelをフック
        // ntoskrnl.exeのアドレスを取得、ターゲットドライバのエントリーポイントをフックする
        OslArchTransferToKernel = TrampolineHook(HookedOslArchTransferToKernel, BaseOslArchTransferToKernel, OriginalOslArchTransferToKernel, TRUE);
        if (OslArchTransferToKernel == NULL)
        {
            Print(L"[-] Failed to hook OslArchTransferToKernel\r\n");
            break;
        }
        Print(L"[+] Hooked OslArchTransferToKernel\r\n");

        Status = EFI_SUCCESS;
    } while (FALSE);

    return Status;
}