#pragma once
#include <efi.h>

typedef struct _MAP_INFO
{
    VOID* AllocatedBuffer;
    EFI_STATUS AllocatedBufferStatus;
}MAP_INFO, * PMAP_INFO;

typedef struct _UNICODE_STRING
{
    UINT16 Length;
    UINT16 MaximumLength;
    CHAR16* Buffer;
} UNICODE_STRING;

typedef struct _KLDR_DATA_TABLE_ENTRY
{
    LIST_ENTRY InLoadOrderLinks;
    VOID* ExceptionTable;
    UINT32 ExceptionTableSize;
    VOID* GpValue;
    VOID* NonPagedDebugInfo;
    VOID* ImageBase;
    VOID* EntryPoint;
    UINT32 SizeOfImage;
    UNICODE_STRING FullImageName;
    UNICODE_STRING BaseImageName;
} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;

/**
 * @breif ドライバーのマップ処理
 * @param Ntoskrnl[in] ntoskrnl情報
 * @param TargetModule[in] 悪用ドライバ情報
 * @retval EFI_SUCCESS マップに成功
 */
EFI_STATUS SetupDriver(PKLDR_DATA_TABLE_ENTRY Ntoskrnl, PKLDR_DATA_TABLE_ENTRY TargetModule);
