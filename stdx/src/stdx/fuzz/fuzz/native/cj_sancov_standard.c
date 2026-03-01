/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include "code_sancov_standard.h"
#include "fake_stacktrace_printer.h"
#include "codenative_sanitizer_coverage.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MODULE_CNT 4096 // the same as module count of libfuzzer
struct FakeRegion g_reigons[MAX_MODULE_CNT];
int g_numReignon = 0;

uint8_t* __code_sancov_8bit_counters_ctor(uint64_t edgeCount)
{
    uint8_t* p = (uint8_t*)calloc(edgeCount, sizeof(uint8_t));
    if (p != NULL) {
        __sanitizer_cov_8bit_counters_init(p, p + edgeCount);
    }
    return p;
}

uint32_t* __code_sancov_pc_guard_ctor(uint64_t edgeCount)
{
    uint32_t* p = (uint32_t*)calloc(edgeCount, sizeof(uint32_t));
    if (p != NULL) {
        __sanitizer_cov_trace_pc_guard_init(p, p + edgeCount);
    }
    return p;
}

void __code_sancov_pcs_init(const int8_t* packageName, uint64_t n, const int8_t** funcNameTable,
    const int8_t** fileNameTable, const uint64_t* lineNumberTable)
{
    if (g_numReignon >= MAX_MODULE_CNT || n == 0) {
        return;
    }

    struct PCTableEntry* entrys = malloc(n * sizeof(struct PCTableEntry));
    if (entrys == NULL) {
        return;
    }
    struct FakeInfo* infos = malloc(n * sizeof(struct FakeInfo));
    if (infos == NULL) {
        free(entrys);
        return;
    }

    for (uint64_t i = 0; i < n; i++) {
        struct FakeInfo* info = &infos[i];
        entrys[i].PC = (uintptr_t)info;
        info->PC = &(entrys[i].PC);
        info->packageName = (const char*)packageName;
        info->fileName = (const char*)fileNameTable[i];
        info->funcName = (const char*)funcNameTable[i];
        info->lineNumber = lineNumberTable[i];
        if (i == 0 || strcmp((const char*)funcNameTable[i], (const char*)funcNameTable[i - 1]) != 0) {
            entrys[i].PCFlags = 1;
        } else {
            entrys[i].PCFlags = 0;
        }
    }

    g_reigons[g_numReignon++] = (struct FakeRegion){.start = (uintptr_t)infos, .end = (uintptr_t)(infos + n)};
    __sanitizer_cov_pcs_init((const uintptr_t*)entrys, (const uintptr_t*)(entrys + n));
}

static bool check_fake_regions(uintptr_t pc)
{
    for (int i = 0; i < g_numReignon; i++) {
        if (g_reigons[i].start <= pc && pc < g_reigons[i].end) {
            return true;
        }
    }
    return false;
}

// libfuzzer invokes `TracePC::GetNextInstructionPc` at each pc, we should recovery it.
// reference: compiler-rt/lib/fuzzer/FuzzerTracePC.cpp
static inline uintptr_t recovery_pc(uintptr_t pc)
{
#if defined(__mips__)
    return pc - 8; // libfuzzer plus 8 in GetNextInstructionPc for mips
#elif defined(__powerpc__) || defined(__sparc__) || defined(__arm__) || defined(__aarch64__)
    return pc - 4; // libfuzzer plus 4 in GetNextInstructionPc for arm64
#else
    return pc - 1; // libfuzzer plus 1 in GetNextInstructionPc for others
#endif
}

int __sanitizer_cangjie_symbolize_pc(void* pc, const char* fmt, char* outBuf, size_t outBufSize)
{
    uintptr_t fake_pc = recovery_pc((uintptr_t)pc);
    if (!check_fake_regions(fake_pc)) {
        return 0;
    }
    const struct FakeInfo* info = (const struct FakeInfo*)fake_pc;
    fake_info_formatter(info, fmt, outBuf, outBufSize);
    return 1;
}

int __sanitizer_cangjie_get_module_and_offset_for_pc(void* pc, char* modulePath, size_t modulePathLen, void** pcOffset)
{
    uintptr_t fake_pc = recovery_pc((uintptr_t)pc);
    if (!check_fake_regions(fake_pc)) {
        return 0;
    }
    const struct FakeInfo* info = (const struct FakeInfo*)fake_pc;
    (void)(info);
    (void)(pcOffset);
    // Not implement, require SanCovPass information
    if (modulePathLen > 0) {
        modulePath[0] = 0;
    }
    return 1;
}

void __code_sanitizer_weak_hook_memcmp(const void* s1, const void* s2, size_t n)
{
    __sanitizer_weak_hook_memcmp(__builtin_return_address(0), s1, s2, n, 1);
}

void __code_sanitizer_weak_hook_strcasecmp(const char* s1, const char* s2)
{
    __sanitizer_weak_hook_strcasecmp(__builtin_return_address(0), s1, s2, 1);
}

void __code_sanitizer_weak_hook_strncmp(const char* s1, const char* s2, size_t n)
{
    __sanitizer_weak_hook_strncmp(__builtin_return_address(0), s1, s2, n, 1);
}

void __code_sanitizer_weak_hook_strcmp(const char* s1, const char* s2)
{
    __sanitizer_weak_hook_strcmp(__builtin_return_address(0), s1, s2, 1);
}
