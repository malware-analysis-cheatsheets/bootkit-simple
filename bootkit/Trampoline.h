#pragma once
#include <efi.h>

// フックのテンプレート
static CONST UINT8 HookTemplate[] = {
    0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,               // jmp QWORD ptr [rip+0]
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // 0x0
};

#define TRAMPOLINE_SIZE sizeof(HookTemplate)

/**
 * @brief 関数をフックする
 * @param dst[in] フック先の関数を指定
 * @param src[in] フックされる関数の先頭アドレス
 * @param orig[in] アンフック時に戻す用
 * @retval NULL 失敗
 * @retval !=NULL フックされる関数の先頭アドレス
 */
VOID* TrampolineHook(VOID* dst, VOID* src, UINT8* orig);

/**
 * @brief フックされた関数をアンフックする
 * @param dst[in] フックされた関数の先頭アドレス
 * @param orig[in] バックアップを保持している変数
 */
VOID TrampolineUnhook(VOID* dst, VOID* orig);

/**
 * @brief パターンを見つける
 * @param base[in] 探索を開始する先頭アドレス
 * @param size[in] 探索するサイズ
 * @param pattern[in] 探索するパターン
 * @param mask[in] 探索パターンのマスク
 * @retval NULL 見つからなかった
 * @retval !=NULL 見つけた先頭アドレス
 */
VOID* FindPattern(CHAR8* base, UINTN size, CONST CHAR8* pattern, CONST CHAR8* mask);
