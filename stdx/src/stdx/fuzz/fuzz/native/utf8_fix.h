/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#ifndef CODE_FUZZ_UTF8_FIX
#define CODE_FUZZ_UTF8_FIX

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * Repair the input data and try to repair it to legal UTF8 as much as possible.
 * The corresponding CodePoint range after UTF8 decoding is [0x0000, 0xD7FF], [0xE000, 0x10FFFF]
 * The execution process will convert illegal bytes into legal bytes,
 * so the input data may be modified. The return value can be used to intercept a legal UTF8 array.
 * Agreement 1: The repaired UTF8 does not end with \0.
 * Convention 2: \0 is allowed in UTF, and functions such as strlen are prohibited.
 *
 * @param data Data to be repaired
 * @param data_len Length of data to be repaired
 * @return The length of the repaired array is less than or equal to the length of the data to be repaired.
 * When the last few bytes cannot form UTF8, the return value will be less than the input data length.
 */
int64_t CODE_fix_to_utf8(uint8_t *data, int64_t data_len);

#ifdef __cplusplus
}
#endif
#endif  // CODE_FUZZ_UTF8_FIX
