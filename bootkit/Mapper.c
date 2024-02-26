#include "Mapper.h"
#include "Trampoline.h"
#include "Serial.h"
#include "BinDriver.h"
#include "Pe.h"

#include <efilib.h>

// ドライバのexport名
// ターゲットモジュールの元データを渡す
#define EXPORT_NAME_FROM_DRIVER "MapperDdata"

extern MAP_INFO Mapper;

// エントリーポイントへ戻れるように、
// 第3引数(r8)にエントリーポイント(ターゲットモジュール)のアドレスを保持する
CONST CHAR8 HoldBytesOfEntryPoint[] = {
    0x4C, 0x8D, 0x05, 0xF9, 0xFF, 0xFF, 0xFF,                   // lea r8, [rip - 7]
};

#define HOLDBYTES_SIZE 7
#define HOOK_SIZE TRAMPOLINE_SIZE + HOLDBYTES_SIZE

// TargetModule、エントリーポイントの元命令を保持
UINT8 OriginalTargetModule[HOOK_SIZE] = { 0 };

/**
 * @brief ドライバを静的にマップする
 * @param BaseNtoskrnl[in] ntoskrnlのベースアドレス
 * @param EntryPoint[out] マップするドライバのエントリーポイント
 * @param TargetFunction[in] 悪用ドライバのエントリーポイント
 * @retval !=EFI_SUCCESS マップに失敗
 * @retval EFI_SUCCESS   マップに成功
 */
EFI_STATUS MapDriver(VOID* BaseNtoskrnl, VOID** EntryPoint, VOID* EntryPointTarget)
{
    EFI_STATUS Status;

    UINT8* BaseDriver = NULL;
    UINT8* BufferDriver = NULL;

    do
    {
        if (BaseNtoskrnl == NULL || EntryPointTarget == NULL)
        {
            SerialPrint(L"[+] Failed to parameter\r\n");
            Status = EFI_INVALID_PARAMETER;
            break;
        }

        BaseDriver = Mapper.AllocatedBuffer;
        BufferDriver = DriverBinary;

        SerialPrint(L"[+] Driver Physical Memory = 0x%llx\r\n", BaseDriver);
        SerialPrint(L"[+] Driver Virtual Memory  = 0x%llx\r\n", BufferDriver);

        // DOS/NT/Sectionヘッダをコピー
        SerialPrint(L"[+] Copy the pe header\r\n");
        Status = PeHeader(BaseDriver, BufferDriver);
        if (EFI_ERROR(Status))
        {
            SerialPrint(L"[-] Failed to copy the pe header\r\n");
            break;
        }

        // セクションの展開
        SerialPrint(L"[+] Write the pe sections\r\n");
        Status = PeSections(BaseDriver, BufferDriver);
        if (EFI_ERROR(Status))
        {
            SerialPrint(L"[-] Failed to write the pe sections\r\n");
            break;
        }

        // イメージベースの再配置
        SerialPrint(L"[+] Relocate the image base\r\n");
        Status = PeRelocation(BaseDriver, BufferDriver);
        if (EFI_ERROR(Status))
        {
            SerialPrint(L"[-] Failed to relocate the image base\r\n");
            break;
        }

        // IAT解決
        SerialPrint(L"[+] Resolve the IAT\r\n");
    } while (FALSE);

    return Status;
}

EFI_STATUS SetupDriver(PKLDR_DATA_TABLE_ENTRY Ntoskrnl, PKLDR_DATA_TABLE_ENTRY TargetModule)
{
    EFI_STATUS Status;
    VOID* DriverEntryPoint = NULL;

    do
    {
        Status = MapDriver(Ntoskrnl->ImageBase, &DriverEntryPoint, TargetModule->EntryPoint);
        if (EFI_ERROR(Status))
        {
            SerialPrint(L"[-] Failed to map the driver\r\n");
            break;
        }

        Status = EFI_SUCCESS;
    } while (FALSE);

    return Status;
}
