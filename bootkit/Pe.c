#include "Pe.h"


#define DosHeader(p) ((PIMAGE_DOS_HEADER)p)
#define NtHeaders(p) ((PIMAGE_NT_HEADERS64)((ULONGLONG)p + DosHeader(p)->e_lfanew))
#define SectionHeader(p) ((PIMAGE_SECTION_HEADER)IMAGE_FIRST_SECTION(NtHeaders(p)))

#define FileHeader(p) NtHeaders(p)->FileHeader
#define OptionalHeader(p) NtHeaders(p)->OptionalHeader
#define DataDirectory(p) (OptionalHeader(p)).DataDirectory
#define Signature(p) NtHeaders(p)->Signature

#define MAX(a, b) (((a) < (b)) ? (b) : (a))

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

// =====================================================================================
EFI_STATUS PeHeader(VOID* Dst, VOID* Src)
{
    EFI_STATUS Status;

    INT32 Size = 0;

    do
    {
        if (Dst == NULL || Src == NULL)
        {
            Status = EFI_INVALID_PARAMETER;
            break;
        }

        if (DosHeader(Src)->e_magic != IMAGE_DOS_SIGNATURE ||
            NtHeaders(Src)->Signature != IMAGE_PE_SIGNATURE)
        {
            Status = EFI_UNSUPPORTED;
            break;
        }

        Size = OptionalHeader(Src).SizeOfHeaders;
        SerialPrint(L"[+] Size of Pe headers: 0x%llx\r\n", Size);
        
        CopyMem(Dst, Src, Size);

        Status = EFI_SUCCESS;
    } while (FALSE);

    return Status;
}

EFI_STATUS PeSections(VOID* Dst, VOID* Src)
{
    EFI_STATUS Status;
    PIMAGE_SECTION_HEADER SectionHeader = NULL;
    VOID* RawData = NULL;
    VOID* VA = NULL;
    INT32 Size = 0;
    INT32 NumberOfSections = 0;

    do
    {
        if (Dst == NULL || Src == NULL)
        {
            Status = EFI_INVALID_PARAMETER;
            break;
        }

        NumberOfSections = GetSectionHeader(Src, &SectionHeader);
        if (NumberOfSections == 0 || SectionHeader == NULL)
        {
            Status = EFI_NOT_FOUND;
            break;
        }
        SerialPrint(L"[+]      -> NumberOfSections = %d\r\n", NumberOfSections);

        for (INT32 i = 0; i < NumberOfSections; i++)
        {
            SerialPrint(L"[+]      -> Section = %c%c%c%c%c%c%c%c\r\n",
                SectionHeader[i].Name[0],
                SectionHeader[i].Name[1],
                SectionHeader[i].Name[2],
                SectionHeader[i].Name[3],
                SectionHeader[i].Name[4],
                SectionHeader[i].Name[5],
                SectionHeader[i].Name[6],
                SectionHeader[i].Name[7]);
            RawData = (VOID*)RVA_TO_VA(Src, SectionHeader[i].PointerToRawData);
            SerialPrint(L"[+]           -> RawData = %llx\r\n", RawData);
            VA = (VOID*)RVA_TO_VA(Dst, SectionHeader[i].VirtualAddress);
            SerialPrint(L"[+]           -> VA = 0x%llx\r\n", VA);
            Size = MAX(SectionHeader[i].SizeOfRawData, SectionHeader[i].Misc.VirtualSize);
            SerialPrint(L"[+]           -> size = 0x%x\r\n", Size);

            CopyMem(VA, RawData, Size);
        }

        Status = EFI_SUCCESS;
    } while (FALSE);

    return Status;
}
