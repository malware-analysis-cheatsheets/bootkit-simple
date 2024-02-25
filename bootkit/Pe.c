#include "Pe.h"


#define DosHeader(p) ((PIMAGE_DOS_HEADER)p)
#define NtHeaders(p) ((PIMAGE_NT_HEADERS64)((ULONGLONG)p + DosHeader(p)->e_lfanew))
#define SectionHeader(p) ((PIMAGE_SECTION_HEADER)IMAGE_FIRST_SECTION(NtHeaders(p)))

#define FileHeader(p) NtHeaders(p)->FileHeader
#define OptionalHeader(p) NtHeaders(p)->OptionalHeader
#define DataDirectory(p) (OptionalHeader(p)).DataDirectory
#define Signature(p) NtHeaders(p)->Signature

INT32 GetImageSize(VOID* Base)
{
    if (Base == NULL)
    {
        return NULL;
    }

    return OptionalHeader(Base).SizeOfImage;
}

INT32 GetSectionHeader(VOID* Base, PIMAGE_SECTION_HEADER* SectionHeader)
{
    PIMAGE_DOS_HEADER DosHeader = NULL;
    PIMAGE_NT_HEADERS64 NtHeaders = NULL;
    INT32 NumberOfSection = 0;

    do
    {
        if (Base == NULL)
        {
            break;
        }

        DosHeader = (PIMAGE_DOS_HEADER)Base;
        if (DosHeader->e_magic != IMAGE_EFI_SIGNATURE)
        {
            break;
        }

        NtHeaders = (PIMAGE_NT_HEADERS64)((UINT64)Base + DosHeader->e_lfanew);
        if (NtHeaders->Signature != IMAGE_PE_SIGNATURE)
        {
            break;
        }

        NumberOfSection = NtHeaders->FileHeader.NumberOfSections;
        *SectionHeader = (PIMAGE_SECTION_HEADER)((UINT64)&NtHeaders->OptionalHeader + NtHeaders->FileHeader.SizeOfOptionalHeader);
    } while (FALSE);

    return NumberOfSection;
}
