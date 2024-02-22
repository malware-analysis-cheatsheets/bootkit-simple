#include "Trampoline.h"
#include "Pe.h"

#include <efilib.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))

#define HookTemplateOffset 6

extern BOOLEAN ExecutedExitBootServices;

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

VOID* FindPatternFromSections(CHAR8* base, CONST CHAR8* pattern, CONST CHAR8* mask)
{
    VOID* addr = NULL;
    CHAR8 SectionName[SECTION_NAME_SIZE + 1] = { 0 };

    INTN NumberOfSections = 0;
    PIMAGE_SECTION_HEADER Section = NULL;

    if (base == NULL || pattern == NULL || mask == NULL)
    {
        return NULL;
    }

    NumberOfSections = GetSectionHeader(base, &Section);
    for (INTN i = 0; i < NumberOfSections; i++)
    {
        ZeroMem(SectionName, SECTION_NAME_SIZE);
        CopyMem(SectionName, Section[i].Name, MIN(strlena(Section[i].Name), SECTION_NAME_SIZE));
        Print(L"[+]      -> %c%c%c%c%c%c%c%c\r\n",
            Section[i].Name[0],
            Section[i].Name[1],
            Section[i].Name[2],
            Section[i].Name[3],
            Section[i].Name[4],
            Section[i].Name[5],
            Section[i].Name[6],
            Section[i].Name[7]);

        addr = FindPattern((CHAR8*)RVA_TO_VA(base, Section[i].VirtualAddress), MIN(Section[i].SizeOfRawData, Section[i].Misc.VirtualSize), pattern, mask);
        if (addr)
        {
            Print(L"[+]           -> Found\r\n");
            break;
        }
        else
        {
            Print(L"[+]           -> Not found\r\n");
        }
    }

    return addr;
}

VOID* TrampolineHook(VOID* dst, VOID* src, UINT8* orig)
{
    EFI_TPL Tpl = TPL_HIGH_LEVEL;
    if (dst == NULL || src == NULL)
    {
        return NULL;
    }

    if (!ExecutedExitBootServices)
    {
        Tpl = gBS->RaiseTPL(Tpl);
    }

    if (orig)
    {
        CopyMem(orig, src, TRAMPOLINE_SIZE);
    }

    CopyMem(src, HookTemplate, TRAMPOLINE_SIZE);
    *(UINT64*)((UINT8*)src + HookTemplateOffset) = (UINT64)dst;

    if (!ExecutedExitBootServices)
    {
        gBS->RestoreTPL(Tpl);
    }

    return src;
}

VOID TrampolineUnhook(VOID* dst, VOID* orig)
{
    EFI_TPL Tpl = TPL_HIGH_LEVEL;
    if (dst == NULL || orig == NULL)
    {
        return;
    }

    if (!ExecutedExitBootServices)
    {
        Tpl = gBS->RaiseTPL(Tpl);
    }

    CopyMem(dst, orig, TRAMPOLINE_SIZE);

    if (!ExecutedExitBootServices)
    {
        gBS->RestoreTPL(Tpl);
    }
}
