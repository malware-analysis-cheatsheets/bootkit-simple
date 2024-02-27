#include "Pe.h"
#include "Serial.h"

#include <efilib.h>


#define DosHeader(p) ((PIMAGE_DOS_HEADER)p)
#define NtHeaders(p) ((PIMAGE_NT_HEADERS64)((ULONGLONG)p + DosHeader(p)->e_lfanew))
#define SectionHeader(p) ((PIMAGE_SECTION_HEADER)IMAGE_FIRST_SECTION(NtHeaders(p)))

#define FileHeader(p) NtHeaders(p)->FileHeader
#define OptionalHeader(p) NtHeaders(p)->OptionalHeader
#define DataDirectory(p) (OptionalHeader(p)).DataDirectory
#define Signature(p) NtHeaders(p)->Signature

#define MAX(a, b) (((a) < (b)) ? (b) : (a))

UINT64 GetEntryPoint(VOID* Base)
{
    return OptionalHeader(Base).AddressOfEntryPoint;
}

INT32 GetImageSize(VOID* Base)
{
    if (Base == NULL)
    {
        return 0;
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

char* strstr(const char* str1, const char* str2)
{
    unsigned int i, j, k;
    for (i = j = k = 0; str2[j] != '\0'; i++)
    {
        if (str1[i] == '\0')return '\0';
        for (j = 0, k = i; str2[j] != '\0' && str1[i + j] == str2[j]; j++);
    }
    return (char*)&str1[k];
}

UINT64 GetExport(CHAR8* Base, CHAR8* ExportName, BOOLEAN IsStrStr)
{
    UINT64 Status = 0;

    PIMAGE_DOS_HEADER DosHeader = NULL;
    PIMAGE_NT_HEADERS64 NtHeaders = NULL;
    ULONG ExportRva = 0;
    PIMAGE_EXPORT_DIRECTORY Exports = NULL;
    ULONG* NameRva = NULL;
    CHAR8* Func = NULL;
    ULONG* FuncRva = NULL;
    WORD* OrdinalRva = NULL;

    do
    {
        if (Base == NULL || ExportName == NULL)
        {
            break;
        }

        DosHeader = (PIMAGE_DOS_HEADER)Base;
        if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE)
        {
            break;
        }

        NtHeaders = (PIMAGE_NT_HEADERS64)RVA_TO_VA(Base, DosHeader->e_lfanew);
        if (NtHeaders->Signature != IMAGE_PE_SIGNATURE)
        {
            break;
        }

        ExportRva = NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        if (!ExportRva)
        {
            break;
        }

        Exports = (PIMAGE_EXPORT_DIRECTORY)RVA_TO_VA(Base, ExportRva);
        NameRva = (ULONG*)RVA_TO_VA(Base, Exports->AddressOfNames);

        for (ULONG i = 0; i < Exports->NumberOfNames; i++)
        {
            Func = (CHAR8*)RVA_TO_VA(Base, NameRva[i]);

            if (IsStrStr)
            {
                if (strstr(Func, ExportName))
                {
                    FuncRva = (ULONG*)RVA_TO_VA(Base, Exports->AddressOfFunctions);
                    OrdinalRva = (WORD*)RVA_TO_VA(Base, Exports->AddressOfNameOrdinals);

                    Status = RVA_TO_VA(Base, FuncRva[OrdinalRva[i]]);
                    break;
                }
            }
            else
            {
                if (strcmpa(Func, ExportName) == 0)
                {
                    FuncRva = (ULONG*)RVA_TO_VA(Base, Exports->AddressOfFunctions);
                    OrdinalRva = (WORD*)RVA_TO_VA(Base, Exports->AddressOfNameOrdinals);

                    Status = RVA_TO_VA(Base, FuncRva[OrdinalRva[i]]);
                    break;
                }
            }
        }
    } while (FALSE);

    return Status;
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

EFI_STATUS PeRelocation(VOID* Dst, VOID* Src)
{
    EFI_STATUS Status;

    UINT64 Delta = 0;
    PIMAGE_DATA_DIRECTORY Reloc = NULL;
    VOID* BaseReloc = NULL;
    PIMAGE_DATA_DIRECTORY Relocation = NULL;
    PBASE_RELOCATION_BLOCK RelocBlock = NULL;
    UINT32 RelocCount = 0;
    PBASE_RELOCATION_ENTRY RelocEntry = NULL;
    DWORD* RelocRva = NULL;
    UINT64* Patch = NULL;
    UINT32 RelocSizeCounter = 0;

    do
    {
        if (Dst == NULL || Src == NULL)
        {
            Status = EFI_INVALID_PARAMETER;
            break;
        }

        Delta = (UINT64)Dst - OptionalHeader(Src).ImageBase;
        SerialPrint(L"[+]      -> Delta = 0x%llx\r\n", Delta);

        Relocation = &(DataDirectory(Src)[IMAGE_DIRECTORY_ENTRY_BASERELOC]);
        BaseReloc = (VOID*)RVA_TO_VA(Dst, Relocation->VirtualAddress);
        while (RelocSizeCounter < Relocation->Size)
        {
            RelocBlock = (PBASE_RELOCATION_BLOCK)((UINT64)BaseReloc + RelocSizeCounter);
            RelocSizeCounter += sizeof(*RelocBlock);

            RelocCount = (RelocBlock->BlockSize - sizeof(*RelocBlock)) / sizeof(*RelocEntry);
            SerialPrint(L"[+]           -> entries = 0x%x\r\n", RelocCount);

            RelocEntry = (PBASE_RELOCATION_ENTRY)((UINT64)BaseReloc + RelocSizeCounter);
            for (UINT32 EntryCounter = 0; EntryCounter < RelocCount; EntryCounter++)
            {
                RelocSizeCounter += sizeof(*RelocEntry);

                switch (RelocEntry[EntryCounter].Type)
                {
                case IMAGE_REL_BASED_ABSOLUTE:
                    continue;
                case IMAGE_REL_BASED_DIR64:
                    Patch = (UINT64*)RVA_TO_VA(Dst, RelocBlock->PageAddress + RelocEntry[EntryCounter].Offset);
                    SerialPrint(L"[+]                -> 0x%llx : 0x%llx => 0x%llx\r\n", Patch, *Patch, (*Patch + Delta));
                    *Patch += Delta;
                default:
                    break;
                }
            }
        }

        Status = EFI_SUCCESS;
    } while (FALSE);

    return Status;
}

EFI_STATUS PeIat(VOID* Dst, VOID* Src, VOID* Ntoskrnl)
{
    EFI_STATUS Status = EFI_SUCCESS;

    PIMAGE_DATA_DIRECTORY Imports = NULL;
    PIMAGE_IMPORT_DESCRIPTOR ImportDescriptor = NULL;
    CHAR8* LibName = NULL;
    VOID* Library = NULL;
    PIMAGE_THUNK_DATA64 Thunk = NULL;
    UINT64 Func = 0;
    CHAR8* FuncOrdinal = NULL;
    PIMAGE_IMPORT_BY_NAME FuncName = NULL;

    do
    {
        if (Dst == NULL || Src == NULL)
        {
            Status = EFI_INVALID_PARAMETER;
            break;
        }

        Imports = &DataDirectory(Src)[IMAGE_DIRECTORY_ENTRY_IMPORT];
        ImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)RVA_TO_VA(Dst, Imports->VirtualAddress);

        while (ImportDescriptor->Name != 0)
        {
            LibName = (CHAR8*)RVA_TO_VA(Dst, ImportDescriptor->Name);
            if (strcmpa(LibName, "ntoskrnl.exe") != 0)
            {
                Status = EFI_NOT_FOUND;
                break;
            }

            SerialPrint(L"[+]      -> Name = ");
            for (UINT32 i = 0; i < strlena(LibName); i++)
            {
                if (LibName[i] == 0)
                {
                    break;
                }

                SerialPrint(L"%c", LibName[i]);
            }
            SerialPrint(L"\r\n");

            Thunk = (PIMAGE_THUNK_DATA64)RVA_TO_VA(Dst, ImportDescriptor->FirstThunk);
            while (Thunk->u1.AddressOfData != 0)
            {
                Func = 0;

                FuncName = (PIMAGE_IMPORT_BY_NAME)RVA_TO_VA(Dst, Thunk->u1.AddressOfData);
                Func = GetExport(Ntoskrnl, FuncName->Name, FALSE);
                SerialPrint(L"[+]           -> 0x%llx\r\n", Func);

                Thunk->u1.Function = Func;

                Thunk++;
            }

            ImportDescriptor++;
        }
    } while (FALSE);

    return Status;
}
