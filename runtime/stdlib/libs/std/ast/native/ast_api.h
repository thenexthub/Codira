/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#ifndef CODIRA_AST_API_H
#define CODIRA_AST_API_H

#include <cstdint>
#include <string>

extern "C" {
struct ParseRes {
    uint8_t* node;
    char* eMsg;
};

ParseRes* CODE_AST_Lex(void *fptr, const char* code);

ParseRes* CODE_AST_ParseExpr(void* fptr, const uint8_t* tokensBytes, int64_t* tokenCounter);

ParseRes* CODE_AST_ParseDecl(void* fptr, const uint8_t* tokensBytes, int64_t* tokenCounter);

ParseRes* CODE_AST_ParsePropMemberDecl(void* fptr, const uint8_t* tokensBytes);

ParseRes* CODE_AST_ParsePrimaryConstructor(void* fptr, const uint8_t* tokensBytes);

ParseRes* CODE_AST_ParsePattern(void* fptr, const uint8_t* tokensBytes, int64_t* tokenCounter);

ParseRes* CODE_AST_ParseType(void* fptr, const uint8_t* tokensBytes, int64_t* tokenCounter);

ParseRes* CODE_AST_ParseTopLevel(void* fptr, const uint8_t* tokensBytes);

ParseRes* CODE_AST_ParseAnnotationArguments(const uint8_t* tokensBytes);

bool CODE_CheckParentContext(void* fptr, char* parent, bool report);

void CODE_SetItemInfo(void* fptr, char* key, void* value, uint8_t type);

void*** CODE_GetChildMessages(void* fptr, char* children);

void CODE_CheckAddSpace(const uint8_t* tokensBytes, bool* spaceFlag);

uint8_t CODE_AST_DiagReport(void* fptr, const int* level, const uint8_t* tokensBytes,
    const char* message, const char* hint);
}

#endif /* CODIRA_AST_API_H */
