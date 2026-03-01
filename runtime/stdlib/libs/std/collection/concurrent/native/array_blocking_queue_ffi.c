/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "securec.h"

#ifdef __arm__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Watomic-alignment"
#endif

extern uint8_t* CodeStdSyncStateInit(int64_t capacity)
{
    if (capacity <= 0) {
        return NULL;
    }
    uint8_t* ptr = (uint8_t*)malloc((size_t)capacity);
    if (ptr == NULL) {
        return NULL;
    }
    (void)memset_s(ptr, (size_t)capacity, 0, (size_t)capacity);
    return ptr;
}

extern uint8_t CodeStdSyncStateLoad(uint8_t* ptr, int64_t index)
{
    _Atomic uint8_t* atmPtr = (_Atomic uint8_t*)ptr + index;
    return atomic_load_explicit(atmPtr, memory_order_acquire);
}

extern void CodeStdSyncStateStore(uint8_t* ptr, int64_t index, uint8_t state)
{
    _Atomic uint8_t* atmPtr = (_Atomic uint8_t*)ptr + index;
    atomic_store_explicit(atmPtr, state, memory_order_release);
}

extern bool CodeStdSyncStateCas(uint8_t* ptr, int index, uint8_t expected, uint8_t desired)
{
    _Atomic uint8_t* atmPtr = (_Atomic uint8_t*)ptr + index;
    return atomic_compare_exchange_strong_explicit(
        atmPtr, &expected, desired, memory_order_acq_rel, memory_order_acquire);
}

#ifdef __arm__
#pragma GCC diagnostic pop
#endif
