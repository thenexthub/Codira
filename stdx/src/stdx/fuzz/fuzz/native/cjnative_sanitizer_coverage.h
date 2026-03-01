/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#ifndef CODE_SANCOV_STANDARD_CODENATIVE_SANITIZER_COVERAGE_H
#define CODE_SANCOV_STANDARD_CODENATIVE_SANITIZER_COVERAGE_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) void __sanitizer_cov_trace_pc_guard(uint32_t* guard);

__attribute__((weak)) void __sanitizer_cov_trace_pc_guard_init(uint32_t* start, uint32_t* stop);

__attribute__((weak)) void __sanitizer_cov_8bit_counters_init(uint8_t* start, uint8_t* stop);

__attribute__((weak)) void __sanitizer_cov_bool_flag_init(bool* start, bool* stop);

struct PCTableEntry {
    uintptr_t PC, PCFlags;
};

void __sanitizer_cov_pcs_init(const uintptr_t* pcs_beg, const uintptr_t* pcs_end);

// Return 1 if success. Return 0 if pc is not cangjie fake pc.
int __sanitizer_cangjie_symbolize_pc(void* pc, const char* fmt, char* outBuf, size_t outBufSize);

// Return 1 if success. Return 0 if pc is not cangjie fake pc.
int __sanitizer_cangjie_get_module_and_offset_for_pc(void* pc, char* modulePath, size_t modulePathLen, void** pcOffset);

void __sanitizer_cov_trace_cmp8(uint64_t arg1, uint64_t arg2);

void __sanitizer_cov_trace_const_cmp8(uint64_t arg1, uint64_t arg2);

void __sanitizer_cov_trace_cmp4(uint32_t arg1, uint32_t arg2);

void __sanitizer_cov_trace_const_cmp4(uint32_t arg1, uint32_t arg2);

void __sanitizer_cov_trace_cmp2(uint16_t arg1, uint16_t arg2);

void __sanitizer_cov_trace_const_cmp2(uint16_t arg1, uint16_t arg2);

void __sanitizer_cov_trace_cmp1(uint8_t arg1, uint8_t arg2);

void __sanitizer_cov_trace_const_cmp1(uint8_t arg1, uint8_t arg2);

void __sanitizer_cov_trace_switch(uint64_t val, uint64_t* cases);

void __sanitizer_weak_hook_memcmp(void* called_pc, const void* s1, const void* s2, size_t n, int result);

void __sanitizer_weak_hook_strcasecmp(void* called_pc, const char* s1, const char* s2, int result);

void __sanitizer_weak_hook_strncmp(void* called_pc, const char* s1, const char* s2, size_t n, int result);

void __sanitizer_weak_hook_strcmp(void* called_pc, const char* s1, const char* s2, int result);

#ifdef __cplusplus
}
#endif

#endif // CODE_SANCOV_STANDARD_CODENATIVE_SANITIZER_COVERAGE_H
