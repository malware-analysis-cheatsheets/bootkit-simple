#pragma once
#include <efi.h>

#define SERIAL_PORT_0 0x3F8 /* COM1, ttyS0 */
#define SERIAL_PORT_1 0x2F8 /* COM2, ttyS1 */
#define SERIAL_PORT_2 0x3E8 /* COM3, ttyS2 */
#define SERIAL_PORT_3 0x2E8 /* COM4, ttyS3 */

#define SERIAL_PORT_NUM SERIAL_PORT_0
#define SERIAL_BAUDRATE 115200

VOID SerialPortInitialize(UINT16 Port, UINTN Baudrate);

VOID SerialPortWrite(UINT16 Port, UINT8 Data);
UINT8 SerialPortRead(UINT16 Port);

/**
 * @brief シリアルポートに文字列を出力する
 * @param fmt[in] 出力文字列(フォーマット)
 * @retval 出力文字数
 */
INTN SerialPrint(IN CONST CHAR16* fmt, ...);
