/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#ifndef JSON_STRING_ESCAPE_H
#define JSON_STRING_ESCAPE_H

#include <stdint.h>
#include <stdbool.h>

int64_t CODE_JSON_ReplaceEscapeChar(const uint8_t* input, int64_t inputlen, uint8_t* buffer, bool htmlSafe);

int64_t CODE_JSON_WriteBufferAppendInt(uint8_t* buffer, const int64_t num);

int64_t CODE_JSON_StringEscapeCharNumGet(const uint8_t* input, int64_t strlen, bool htmlSafe);

int64_t CODE_JSON_WriteBufferAppendUint(uint8_t* buffer, const uint64_t num);

#endif
