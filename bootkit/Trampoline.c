#include "Trampoline.h"

#include <efilib.h>

#define HookTemplateOffset 6

BOOLEAN CheckMask(CHAR8* base, CONST CHAR8* pattern, CONST CHAR8* mask)
{
    for (; *mask; ++base, ++pattern, ++mask)
    {
        if (*mask == 'x' && *base != *pattern)
        {
            return FALSE;
        }
    }

    return TRUE;
}

VOID* FindPattern(CHAR8* base, UINTN size, CONST CHAR8* pattern, CONST CHAR8* mask)
{
    size -= strlena(mask);
    for (UINTN i = 0; i <= size; ++i)
    {
        VOID* addr = &base[i];
        if (CheckMask(addr, pattern, mask))
        {
            return addr;
        }
    }

    return NULL;
}

VOID* TrampolineHook(VOID* dst, VOID* src, UINT8* orig)
{
    EFI_TPL Tpl = TPL_HIGH_LEVEL;
    if (dst == NULL || src == NULL)
    {
        return NULL;
    }

    Tpl = gBS->RaiseTPL(Tpl);

    if (orig)
    {
        CopyMem(orig, src, TRAMPOLINE_SIZE);
    }

    CopyMem(src, HookTemplate, TRAMPOLINE_SIZE);
    *(UINT64*)((UINT8*)src + HookTemplateOffset) = (UINT64)dst;

    gBS->RestoreTPL(Tpl);

    return src;
}

VOID TrampolineUnhook(VOID* dst, VOID* orig)
{
    EFI_TPL Tpl = TPL_HIGH_LEVEL;
    if (dst == NULL || orig == NULL)
    {
        return;
    }

    Tpl = gBS->RaiseTPL(Tpl);

    CopyMem(dst, orig, TRAMPOLINE_SIZE);

    gBS->RestoreTPL(Tpl);
}
