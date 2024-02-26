#pragma once
#include <efi.h>


#define BYTE UINT8
#define CHAR CHAR8
#define UCHAR UINT8
#define USHORT UINT16
#define WORD UINT16
#define LONG INT32
#define NTSTATUS INT32
#define ULONG UINT32
#define DWORD UINT32
#define ULONG_PTR UINT32*
#define SIZE_T ULONG_PTR
#define ULONGLONG UINT64
#define PVOID VOID*
#define HANDLE VOID*


#define IMAGE_EFI_SIGNATURE 0x5A4D
#define IMAGE_DOS_SIGNATURE 0x5A4D
#define IMAGE_PE_SIGNATURE  0x4550

#define SECTION_NAME_SIZE 8


#define RVA_TO_VA(base, rva) ((UINT64)base + rva)


typedef struct _IMAGE_DOS_HEADER
{
    USHORT e_magic;                                                         //0x0
    USHORT e_cblp;                                                          //0x2
    USHORT e_cp;                                                            //0x4
    USHORT e_crlc;                                                          //0x6
    USHORT e_cparhdr;                                                       //0x8
    USHORT e_minalloc;                                                      //0xa
    USHORT e_maxalloc;                                                      //0xc
    USHORT e_ss;                                                            //0xe
    USHORT e_sp;                                                            //0x10
    USHORT e_csum;                                                          //0x12
    USHORT e_ip;                                                            //0x14
    USHORT e_cs;                                                            //0x16
    USHORT e_lfarlc;                                                        //0x18
    USHORT e_ovno;                                                          //0x1a
    USHORT e_res[4];                                                        //0x1c
    USHORT e_oemid;                                                         //0x24
    USHORT e_oeminfo;                                                       //0x26
    USHORT e_res2[10];                                                      //0x28
    LONG e_lfanew;                                                          //0x3c
}IMAGE_DOS_HEADER, * PIMAGE_DOS_HEADER;

//0x14 bytes (sizeof)
typedef struct _IMAGE_FILE_HEADER
{
    USHORT Machine;                                                         //0x0
    USHORT NumberOfSections;                                                //0x2
    ULONG TimeDateStamp;                                                    //0x4
    ULONG PointerToSymbolTable;                                             //0x8
    ULONG NumberOfSymbols;                                                  //0xc
    USHORT SizeOfOptionalHeader;                                            //0x10
    USHORT Characteristics;                                                 //0x12
}IMAGE_FILE_HEADER, * PIMAGE_FILE_HEADER;

//0x8 bytes (sizeof)
typedef struct _IMAGE_DATA_DIRECTORY
{
    ULONG VirtualAddress;                                                   //0x0
    ULONG Size;                                                             //0x4
}IMAGE_DATA_DIRECTORY, * PIMAGE_DATA_DIRECTORY;

//0xf0 bytes (sizeof)
typedef struct _IMAGE_OPTIONAL_HEADER64
{
    USHORT Magic;                                                           //0x0
    UCHAR MajorLinkerVersion;                                               //0x2
    UCHAR MinorLinkerVersion;                                               //0x3
    ULONG SizeOfCode;                                                       //0x4
    ULONG SizeOfInitializedData;                                            //0x8
    ULONG SizeOfUninitializedData;                                          //0xc
    ULONG AddressOfEntryPoint;                                              //0x10
    ULONG BaseOfCode;                                                       //0x14
    ULONGLONG ImageBase;                                                    //0x18
    ULONG SectionAlignment;                                                 //0x20
    ULONG FileAlignment;                                                    //0x24
    USHORT MajorOperatingSystemVersion;                                     //0x28
    USHORT MinorOperatingSystemVersion;                                     //0x2a
    USHORT MajorImageVersion;                                               //0x2c
    USHORT MinorImageVersion;                                               //0x2e
    USHORT MajorSubsystemVersion;                                           //0x30
    USHORT MinorSubsystemVersion;                                           //0x32
    ULONG Win32VersionValue;                                                //0x34
    ULONG SizeOfImage;                                                      //0x38
    ULONG SizeOfHeaders;                                                    //0x3c
    ULONG CheckSum;                                                         //0x40
    USHORT Subsystem;                                                       //0x44
    USHORT DllCharacteristics;                                              //0x46
    ULONGLONG SizeOfStackReserve;                                           //0x48
    ULONGLONG SizeOfStackCommit;                                            //0x50
    ULONGLONG SizeOfHeapReserve;                                            //0x58
    ULONGLONG SizeOfHeapCommit;                                             //0x60
    ULONG LoaderFlags;                                                      //0x68
    ULONG NumberOfRvaAndSizes;                                              //0x6c
    struct _IMAGE_DATA_DIRECTORY DataDirectory[16];                         //0x70
}IMAGE_OPTIONAL_HEADER64, * PIMAGE_OPTIONAL_HEADER64;

//0x108 bytes (sizeof)
typedef struct _IMAGE_NT_HEADERS64
{
    ULONG Signature;                                                        //0x0
    struct _IMAGE_FILE_HEADER FileHeader;                                   //0x4
    struct _IMAGE_OPTIONAL_HEADER64 OptionalHeader;                         //0x18
}IMAGE_NT_HEADERS64, * PIMAGE_NT_HEADERS64;

//0x28 bytes (sizeof)
typedef struct _IMAGE_SECTION_HEADER
{
    UCHAR Name[8];                                                          //0x0
    union
    {
        ULONG PhysicalAddress;                                              //0x8
        ULONG VirtualSize;                                                  //0x8
    } Misc;                                                                 //0x8
    ULONG VirtualAddress;                                                   //0xc
    ULONG SizeOfRawData;                                                    //0x10
    ULONG PointerToRawData;                                                 //0x14
    ULONG PointerToRelocations;                                             //0x18
    ULONG PointerToLinenumbers;                                             //0x1c
    USHORT NumberOfRelocations;                                             //0x20
    USHORT NumberOfLinenumbers;                                             //0x22
    ULONG Characteristics;                                                  //0x24
}IMAGE_SECTION_HEADER, * PIMAGE_SECTION_HEADER;

/**
 * @brief イメージがメモリに読み込まれるときのイメージのサイズ
 * @param Base[in] peのベースアドレス
 * @retval イメージのサイズ
 */
INT32 GetImageSize(VOID* Base);

/**
 * @brief セクションヘッダの情報
 * @param Base[in] peのベースアドレス
 * @param SectionHeader[out] セクションヘッダの最初のアドレス
 * @retval セクションの数
 */
INT32 GetSectionHeader(VOID* Base, PIMAGE_SECTION_HEADER* SectionHeader);


// =====================================================================================
/**
 * @breif PEのヘッダーを書き込む
 * @param Dst[in] 書き込み先のベースアドレス
 * @param Src[in] PEのベースアドレス
 */
EFI_STATUS PeHeader(VOID* Dst, VOID* Src);

/**
 * @breif 各セクションをメモリ展開する
 * @param Dst[in] 書き込み先のベースアドレス
 * @param Src[in] PEのベースアドレス
 */
EFI_STATUS PeSections(VOID* Dst, VOID* Src);
