/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <memory.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#ifdef __arm__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Watomic-alignment"
#pragma GCC diagnostic ignored "-Wshift-count-overflow"
#endif

// MinGW specifically does not support attribute((weak)) properly
#ifndef __MINGW64__
#define ATTRIBUTE_WEAK __attribute__((weak))
#else
#define ATTRIBUTE_WEAK
#endif // __MINGW64__p

#if defined(__has_feature)
#if __has_feature(coverage_sanitizer)
// compatibility with GCC and MinGW
#define __SANITIZE_ADDRESS__
#endif // __has_feature(coverage_sanitizer)
#endif // defined(__has_feature)

#if defined(__SANITIZE_ADDRESS__)
// all functions marked with this are our API available to Codira code
#define MICROFUZZ_API __attribute__((no_sanitize("coverage")))
// all functions marked with this are coverage sanitizer callbacks
#define SANCOV_CALLBACK __attribute__((no_sanitize("coverage"))) ATTRIBUTE_WEAK
#else
// all functions marked with this are our API available to Codira code
#define MICROFUZZ_API
// all functions marked with this are coverage sanitizer callbacks
#define SANCOV_CALLBACK ATTRIBUTE_WEAK
#endif

typedef uint8_t UInt8;
typedef uint16_t UInt16;
typedef uint32_t UInt32;
typedef uint64_t UInt64;
typedef float Float32;
typedef double Float64;

#define DECLARE_ENTRY_TYPE(TYPE)                                                                                       \
    typedef struct {                                                                                                   \
        TYPE left;                                                                                                     \
        TYPE right;                                                                                                    \
    } TYPE##Entry

DECLARE_ENTRY_TYPE(UInt8);
DECLARE_ENTRY_TYPE(UInt16);
DECLARE_ENTRY_TYPE(UInt32);
DECLARE_ENTRY_TYPE(UInt64);
DECLARE_ENTRY_TYPE(Float32);
DECLARE_ENTRY_TYPE(Float64);

// initialized to true once on creation callback
// if this variable is false, it means that coverage sanitizer was probably not enabled
static atomic_bool g_initialized = false;
// enable/disable tracing, controlled by the API
static atomic_bool g_enableTrace = false;
// is set to true once new coverage is encountered, can be reset by the API
static atomic_bool g_hasNewCoverage = false;

#define CMP_PC_MAP_SIZE (1 << 18)
// pc-based coverage storage for comparison ops, local part for one test run
static UInt8 g_cmpPcCounterMap[CMP_PC_MAP_SIZE] = {0};
// pc-based coverage storage for comparison ops, persisted between test runs
static UInt8 g_cmpPcCounterMapInterrun[CMP_PC_MAP_SIZE] = {0};
// hamming distance between values compared
static UInt8 g_cmpPcHammingDistMap[CMP_PC_MAP_SIZE] = {255};
// absolute distance between values compared
static UInt8 g_cmpPcAbsDistmap[CMP_PC_MAP_SIZE] = {255};

// edge storage size, initialized by the coverage sanitizer
static size_t g_coverageMapSize = 0;
// maximum possible size for edge storage
#define MAX_COVERAGE_MAP_SIZE (1 << 20)
// edge-based coverage storage, local part for one test run
static UInt8 g_coverageMap[MAX_COVERAGE_MAP_SIZE];
// edge-based coverage storage, persisted between test runs
static UInt8 g_globalCoverageMap[MAX_COVERAGE_MAP_SIZE];

// below this comment TORC = Table of Recent Compares
// TORC size
#define RECENT_VALUE_MAP_SIZE (1 << 13)
// TORC storages for comparison operations, stored in pairs
static UInt8 g_cmpRecentValuesUInt8[RECENT_VALUE_MAP_SIZE / sizeof(UInt8)] = {0};
static UInt16 g_cmpRecentValuesUInt16[RECENT_VALUE_MAP_SIZE / sizeof(UInt16)] = {0};
static UInt32 g_cmpRecentValuesUInt32[RECENT_VALUE_MAP_SIZE / sizeof(UInt32)] = {0};
static UInt64 g_cmpRecentValuesUInt64[RECENT_VALUE_MAP_SIZE / sizeof(UInt64)] = {0};
static Float32 g_cmpRecentValuesFloat32[RECENT_VALUE_MAP_SIZE / sizeof(Float32)] = {0};
static Float64 g_cmpRecentValuesFloat64[RECENT_VALUE_MAP_SIZE / sizeof(Float64)] = {0};

static void (*g_debugCallback)(const char*, const char*, UInt64, UInt64) = 0;

#define STORE(atomic, value) atomic_store_explicit(&atomic, value, memory_order_relaxed)
#define LOAD(atomic) atomic_load_explicit(&atomic, memory_order_relaxed)
#define CAS(atomic, old, new) atomic_compare_exchange_strong(&atomic, &old, new)

#define PC_TO_INDEX(pc) (size_t)(((pc) >> 32) ^ (pc))
#define CLAMP_UINT8(x) ((x) > 255 ? ((UInt8)255) : ((UInt8)(x)))
#define DIST_UINT8(x, y) CLAMP_UINT8((x) > (y) ? ((x) - (y)) : ((y) - (x)))
#define POOR_RANDOM(field) (field = UINT64_C(6364136223846793005) * field + UINT64_C(1442695040888963407))
#define DEBUG(message, type, v1, v2)                                                                                   \
    if (g_debugCallback) {                                                                                             \
        bool traceEnabled = true;                                                                                      \
        if (CAS(g_enableTrace, traceEnabled, false)) {                                                                 \
            g_debugCallback(message, #type, (UInt64)(v1), (UInt64)(v2));                                               \
            STORE(g_enableTrace, true);                                                                                \
        }                                                                                                              \
    }

MICROFUZZ_API void CODE_MICROFUZZ_ENABLE_TRACE(void)
{
    STORE(g_enableTrace, true);
}

MICROFUZZ_API void CODE_MICROFUZZ_DISABLE_TRACE(void)
{
    STORE(g_enableTrace, false);
}

MICROFUZZ_API bool CODE_MICROFUZZ_TRACE_ENABLED(void)
{
    return LOAD(g_enableTrace);
}

MICROFUZZ_API bool CODE_MICROFUZZ_IS_INITIALIZED(void)
{
    return LOAD(g_initialized);
}

MICROFUZZ_API bool CODE_MICROFUZZ_HAS_NEW_COVERAGE(void)
{
    return LOAD(g_hasNewCoverage);
}

MICROFUZZ_API void CODE_MICROFUZZ_RESET_HAS_NEW_COVERAGE(void)
{
    STORE(g_hasNewCoverage, false);
}

MICROFUZZ_API void CODE_MICROFUZZ_SET_DBG_CALLBACK(void (*callback)(const char*, const char*, UInt64, UInt64))
{
    g_debugCallback = callback;
}

// The operations below are macros for several reasons:
// 1) They are generic
// 2) functions can be accidentally instrumented, we don't want that
// 3) we depend heavily on the call site and don't want extra functions in the stack
// Process comparison operation: update counter map and distance maps
#define SANITIZER_COV_TRACE_CMP_TRACE(TYPE, ARG1, ARG2)                                                                \
    {                                                                                                                  \
        uintptr_t pc = (uintptr_t)__builtin_return_address(0);                                                         \
        size_t index = PC_TO_INDEX(pc) % CMP_PC_MAP_SIZE;                                                              \
        UInt8 newCounter = ++g_cmpPcCounterMap[index];                                                                 \
        UInt8 newHamming = (UInt8)__builtin_popcountll(ARG1 ^ ARG2);                                                   \
        UInt8 newAbs = DIST_UINT8(ARG1, ARG2);                                                                         \
                                                                                                                       \
        UInt8 oldCounter = g_cmpPcCounterMapInterrun[index];                                                           \
        if (oldCounter < newCounter) {                                                                                 \
            g_cmpPcCounterMapInterrun[index] = newCounter;                                                             \
            g_cmpPcHammingDistMap[index] = newHamming;                                                                 \
            g_cmpPcAbsDistmap[index] = newAbs;                                                                         \
            DEBUG("new coverage (pc) on ", TYPE, ARG1, ARG2);                                                          \
            STORE(g_hasNewCoverage, true);                                                                             \
        } else if (oldCounter == newCounter) {                                                                         \
            UInt8 hamming = g_cmpPcHammingDistMap[index];                                                              \
            UInt8 abs = g_cmpPcAbsDistmap[index];                                                                      \
            if (newHamming < hamming) {                                                                                \
                g_cmpPcHammingDistMap[index] = newHamming;                                                             \
                DEBUG("new coverage (Hamming) on ", TYPE, ARG1, ARG2);                                                 \
                STORE(g_hasNewCoverage, true);                                                                         \
            }                                                                                                          \
            if (newAbs < abs) {                                                                                        \
                g_cmpPcAbsDistmap[index] = newAbs;                                                                     \
                DEBUG("new coverage (abs) on ", TYPE, ARG1, ARG2);                                                     \
                STORE(g_hasNewCoverage, true);                                                                         \
            }                                                                                                          \
        }                                                                                                              \
    }

// Store two recently compared values
#define STORE_RECENT_CMP(TYPE, ARG1, ARG2)                                                                             \
    {                                                                                                                  \
        if (ARG1 != ARG2) {                                                                                            \
            static UInt64 cmpRecentWritePrng = 800 * (UInt64)sizeof(TYPE);                                             \
            size_t index = (POOR_RANDOM(cmpRecentWritePrng) % (RECENT_VALUE_MAP_SIZE / sizeof(TYPE)) / 2) * 2;         \
            g_cmpRecentValues##TYPE[index] = ARG1;                                                                     \
            g_cmpRecentValues##TYPE[index + 1] = ARG2;                                                                 \
        }                                                                                                              \
    }

// Get (pseudo-)random value from TORC for type TYPE
#define RANDOM_RECENT_CMP(TYPE, RESULT)                                                                                \
    {                                                                                                                  \
        static UInt64 cmpRecentReadPrng = 0xdead * (UInt64)sizeof(TYPE);                                               \
        const size_t totalSize = RECENT_VALUE_MAP_SIZE / sizeof(TYPE);                                                 \
        size_t index = POOR_RANDOM(cmpRecentReadPrng) % totalSize;                                                     \
        while (index < 2 * totalSize) {                                                                                \
            TYPE candidate = g_cmpRecentValues##TYPE[index % totalSize];                                               \
            if (candidate != 0) {                                                                                      \
                size_t oddIndex = (index / 2) * 2 % totalSize;                                                         \
                (RESULT)->left = g_cmpRecentValues##TYPE[oddIndex];                                                    \
                (RESULT)->right = g_cmpRecentValues##TYPE[oddIndex + 1];                                               \
                return;                                                                                                \
            }                                                                                                          \
            index++;                                                                                                   \
        }                                                                                                              \
        return;                                                                                                        \
    }

// Get (pseudo-)random value from TORC for type TYPE compared to value KEY
#define RANDOM_RECENT_CMP_FOR_KEY(TYPE, KEY, RESULT)                                                                   \
    {                                                                                                                  \
        static UInt64 cmpRecentReadPrng = 0xbeef * (UInt64)sizeof(TYPE);                                               \
        const size_t totalSize = RECENT_VALUE_MAP_SIZE / sizeof(TYPE);                                                 \
        size_t index = POOR_RANDOM(cmpRecentReadPrng) % totalSize;                                                     \
        while (index < 2 * totalSize) {                                                                                \
            TYPE candidate = g_cmpRecentValues##TYPE[index % totalSize];                                               \
            if (candidate == KEY) {                                                                                    \
                size_t oddIndex = (index / 2) * 2 % totalSize;                                                         \
                (RESULT)->left = g_cmpRecentValues##TYPE[oddIndex];                                                    \
                (RESULT)->right = g_cmpRecentValues##TYPE[oddIndex + 1];                                               \
                return;                                                                                                \
            }                                                                                                          \
            index++;                                                                                                   \
        }                                                                                                              \
        return;                                                                                                        \
    }

MICROFUZZ_API void CODE_MICROFUZZ_RANDOM_RECENT_CMP_VALUE_UINT8(UInt8Entry* result)
{
    RANDOM_RECENT_CMP(UInt8, result);
}

MICROFUZZ_API void CODE_MICROFUZZ_RANDOM_RECENT_CMP_VALUE_UINT16(UInt16Entry* result)
{
    RANDOM_RECENT_CMP(UInt16, result);
}

MICROFUZZ_API void CODE_MICROFUZZ_RANDOM_RECENT_CMP_VALUE_UINT32(UInt32Entry* result)
{
    RANDOM_RECENT_CMP(UInt32, result);
}

MICROFUZZ_API void CODE_MICROFUZZ_RANDOM_RECENT_CMP_VALUE_UINT64(UInt64Entry* result)
{
    RANDOM_RECENT_CMP(UInt64, result);
}

MICROFUZZ_API void CODE_MICROFUZZ_RANDOM_RECENT_CMP_VALUE_FLOAT(Float32Entry* result)
{
    RANDOM_RECENT_CMP(Float32, result);
}

MICROFUZZ_API void CODE_MICROFUZZ_RANDOM_RECENT_CMP_VALUE_DOUBLE(Float64Entry* result)
{
    RANDOM_RECENT_CMP(Float64, result);
}

MICROFUZZ_API void CODE_MICROFUZZ_RANDOM_RECENT_CMP_VALUE_FOR_KEY_UINT8(UInt8 key, UInt8Entry* result)
{
    RANDOM_RECENT_CMP_FOR_KEY(UInt8, key, result);
}

MICROFUZZ_API void CODE_MICROFUZZ_RANDOM_RECENT_CMP_VALUE_FOR_KEY_UINT16(UInt16 key, UInt16Entry* result)
{
    RANDOM_RECENT_CMP_FOR_KEY(UInt16, key, result);
}

MICROFUZZ_API void CODE_MICROFUZZ_RANDOM_RECENT_CMP_VALUE_FOR_KEY_UINT32(UInt32 key, UInt32Entry* result)
{
    RANDOM_RECENT_CMP_FOR_KEY(UInt32, key, result);
}

MICROFUZZ_API void CODE_MICROFUZZ_RANDOM_RECENT_CMP_VALUE_FOR_KEY_UINT64(UInt64 key, UInt64Entry* result)
{
    RANDOM_RECENT_CMP_FOR_KEY(UInt64, key, result);
}

MICROFUZZ_API void CODE_MICROFUZZ_RANDOM_RECENT_CMP_VALUE_FOR_KEY_FLOAT(Float32 key, Float32Entry* result)
{
    RANDOM_RECENT_CMP_FOR_KEY(Float32, key, result);
}

MICROFUZZ_API void CODE_MICROFUZZ_RANDOM_RECENT_CMP_VALUE_FOR_KEY_DOUBLE(Float64 key, Float64Entry* result)
{
    RANDOM_RECENT_CMP_FOR_KEY(Float64, key, result);
}

// don't use standard memset because it may be instrumented
// if we call this macro only in no-sanitize functions, it will not be instrumented
// even if standard memset is inserted by compiler optimization
#define MEMSET_M(ARR, VALUE) \
    for (size_t i = 0; i < sizeof(ARR) / sizeof(ARR[0]); ++i) { ARR[i] = VALUE; }

MICROFUZZ_API void CODE_MICROFUZZ_RESET_LOCAL_COVERAGE_INFO(void)
{
    STORE(g_hasNewCoverage, false);
    for (size_t i = 0; i < g_coverageMapSize; ++i) {
        g_coverageMap[i] = 0;
    }
    MEMSET_M(g_cmpPcCounterMap, 0);
}

MICROFUZZ_API void CODE_MICROFUZZ_RESET_ALL_COVERAGE_INFO(void)
{
    CODE_MICROFUZZ_RESET_LOCAL_COVERAGE_INFO();
    for (size_t i = 0; i < g_coverageMapSize; ++i) {
        g_globalCoverageMap[i] = 0;
    }
    MEMSET_M(g_cmpPcCounterMapInterrun, 0);
    MEMSET_M(g_cmpPcHammingDistMap, 255);
    MEMSET_M(g_cmpPcAbsDistmap, 255);
    MEMSET_M(g_cmpRecentValuesUInt8, 0);
    MEMSET_M(g_cmpRecentValuesUInt16, 0);
    MEMSET_M(g_cmpRecentValuesUInt32, 0);
    MEMSET_M(g_cmpRecentValuesUInt64, 0);
    MEMSET_M(g_cmpRecentValuesFloat32, 0.0f);
    MEMSET_M(g_cmpRecentValuesFloat64, 0.0);
}

MICROFUZZ_API Float64 CODE_MICROFUZZ_GLOBAL_EDGE_COVERAGE_EST(void)
{
    Float64 result = 0.0;
    for (size_t i = 0; i < g_coverageMapSize; ++i) {
        result += ((g_globalCoverageMap[i] != 0) ? 1 : 0);
    }
    result /= g_coverageMapSize;
    return result;
}

MICROFUZZ_API Float32 CODE_MICROFUZZ_FLIP_BIT_FLOAT(Float32 value, UInt8 bit)
{
    UInt64 floatBits = *(UInt32*)(UInt8*)(&value);
    floatBits = floatBits ^ (1 << (bit % (sizeof(Float32) * CHAR_BIT)));
    return *(Float32*)(UInt8*)(&floatBits);
}

MICROFUZZ_API Float64 CODE_MICROFUZZ_FLIP_BIT_DOUBLE(Float64 value, UInt8 bit)
{
    UInt64 floatBits = *(UInt64*)(UInt8*)(&value);
    floatBits = floatBits ^ (1 << (bit % (sizeof(Float64) * CHAR_BIT)));
    return *(Float64*)(UInt8*)(&floatBits);
}

/**
 * functions below are callbacks that will be invoked by coverage sanitizer
 */

SANCOV_CALLBACK void __sanitizer_cov_pcs_init(
    __attribute__((unused)) const uintptr_t* pcs_beg, __attribute__((unused)) const uintptr_t* pcs_end)
{
    STORE(g_initialized, true);
    return;
}

SANCOV_CALLBACK void __sanitizer_cov_trace_const_cmp1(UInt8 arg1, UInt8 arg2)
{
    if (!LOAD(g_enableTrace)) {
        return;
    }

    SANITIZER_COV_TRACE_CMP_TRACE(UInt8, arg1, arg2);
    STORE_RECENT_CMP(UInt8, arg1, arg2);
}

SANCOV_CALLBACK void __sanitizer_cov_trace_const_cmp2(UInt16 arg1, UInt16 arg2)
{
    if (!LOAD(g_enableTrace)) {
        return;
    }

    SANITIZER_COV_TRACE_CMP_TRACE(UInt16, arg1, arg2);
    STORE_RECENT_CMP(UInt16, arg1, arg2);
}

SANCOV_CALLBACK void __sanitizer_cov_trace_const_cmp4(UInt32 arg1, UInt32 arg2)
{
    if (!LOAD(g_enableTrace)) {
        return;
    }

    SANITIZER_COV_TRACE_CMP_TRACE(UInt32, arg1, arg2);
    STORE_RECENT_CMP(UInt32, arg1, arg2);
}

SANCOV_CALLBACK void __sanitizer_cov_trace_const_cmp8(UInt64 arg1, UInt64 arg2)
{
    if (!LOAD(g_enableTrace)) {
        return;
    }

    SANITIZER_COV_TRACE_CMP_TRACE(UInt64, arg1, arg2);
    STORE_RECENT_CMP(UInt64, arg1, arg2);
}

SANCOV_CALLBACK void __sanitizer_cov_trace_cmp1(UInt8 arg1, UInt8 arg2)
{
    if (!LOAD(g_enableTrace)) {
        return;
    }

    SANITIZER_COV_TRACE_CMP_TRACE(UInt8, arg1, arg2);
    STORE_RECENT_CMP(UInt8, arg1, arg2);
}

SANCOV_CALLBACK void __sanitizer_cov_trace_cmp2(UInt16 arg1, UInt16 arg2)
{
    if (!LOAD(g_enableTrace)) {
        return;
    }

    SANITIZER_COV_TRACE_CMP_TRACE(UInt16, arg1, arg2);
    STORE_RECENT_CMP(UInt16, arg1, arg2);
}

SANCOV_CALLBACK void __sanitizer_cov_trace_cmp4(UInt32 arg1, UInt32 arg2)
{
    if (!LOAD(g_enableTrace)) {
        return;
    }

    SANITIZER_COV_TRACE_CMP_TRACE(UInt32, arg1, arg2);
    STORE_RECENT_CMP(UInt32, arg1, arg2);
}

SANCOV_CALLBACK void __sanitizer_cov_trace_cmp8(UInt64 arg1, UInt64 arg2)
{
    if (!LOAD(g_enableTrace)) {
        return;
    }

    SANITIZER_COV_TRACE_CMP_TRACE(UInt64, arg1, arg2);
    STORE_RECENT_CMP(UInt64, arg1, arg2);
}

// the following functions are defined here for reference: they exist in clang or gcc sancov,
// but are not currently implemented for Codira
SANCOV_CALLBACK void __sanitizer_cov_trace_switch(
    __attribute__((unused)) UInt64 val, __attribute__((unused)) UInt64* cases)
{
}

SANCOV_CALLBACK void __sanitizer_weak_hook_strcasecmp(__attribute__((unused)) void* p,
    __attribute__((unused)) const char* s1, __attribute__((unused)) const char* s2, __attribute__((unused)) int result)
{
}

SANCOV_CALLBACK void __sanitizer_weak_hook_memcmp(__attribute__((unused)) void* p,
    __attribute__((unused)) const void* s1, __attribute__((unused)) const void* s2, __attribute__((unused)) size_t n,
    __attribute__((unused)) int result)
{
}

SANCOV_CALLBACK void __sanitizer_weak_hook_strncmp(__attribute__((unused)) void* p,
    __attribute__((unused)) const char* s1, __attribute__((unused)) const char* s2, __attribute__((unused)) size_t n,
    __attribute__((unused)) int result)
{
}

SANCOV_CALLBACK void __sanitizer_weak_hook_strcmp(__attribute__((unused)) void* p,
    __attribute__((unused)) const char* s1, __attribute__((unused)) const char* s2, __attribute__((unused)) int result)
{
}

SANCOV_CALLBACK void __sanitizer_weak_hook_strncasecmp(__attribute__((unused)) void* caller_pc,
    __attribute__((unused)) const char* s1, __attribute__((unused)) const char* s2, __attribute__((unused)) size_t n,
    __attribute__((unused)) int result)
{
}

// floating comparison operations: currently not supported by cangjie sancov
// but should be supported in the future

SANCOV_CALLBACK void __sanitizer_cov_trace_cmpf(Float32 arg1, Float32 arg2)
{
    if (!LOAD(g_enableTrace)) {
        return;
    }
    STORE_RECENT_CMP(Float32, arg1, arg2);
}

SANCOV_CALLBACK void __sanitizer_cov_trace_cmpd(Float64 arg1, Float64 arg2)
{
    if (!LOAD(g_enableTrace)) {
        return;
    }
    STORE_RECENT_CMP(Float64, arg1, arg2);
}

// the rest of the functions are initialization hooks of sancov, largely
// copied from initialization functions used in other fuzzers that use other implementations of
// sancov

SANCOV_CALLBACK void __sanitizer_cov_trace_pc_guard_init(UInt32* start, UInt32* stop)
{
    if (start == stop || *start != 0) {
        return; // Initialize only once.
    }
    STORE(g_initialized, true);
    UInt64 size = 0; // Counter for the guards.
    for (UInt32* x = start; x < stop; x++) {
        *x = (++size) % MAX_COVERAGE_MAP_SIZE; // Guards should start from 1.
    }
    if (size > MAX_COVERAGE_MAP_SIZE) {
        size = MAX_COVERAGE_MAP_SIZE;
    }
    for (size_t i = 0; i < size; ++i) {
        g_coverageMap[i] = 0;
    }
    for (size_t i = 0; i < size; ++i) {
        g_globalCoverageMap[i] = 0;
    }
    g_coverageMapSize = size;
}

/**
 * PC guard individual initialization callback, this is called for each individual counter
 */
SANCOV_CALLBACK void __sanitizer_cov_trace_pc_guard(UInt32* guard)
{
    if (*guard == 0) {
        return; // Duplicate the guard check.
    }
    if (!LOAD(g_enableTrace)) {
        return;
    }

    UInt8 newCoverage = ++g_coverageMap[*guard - 1];
    if (newCoverage > g_globalCoverageMap[*guard - 1]) {
        DEBUG("new coverage on ", guard, *guard, 0);
        g_globalCoverageMap[*guard - 1] = newCoverage;
        STORE(g_hasNewCoverage, true);
    }
}

/**
 * PC guard initialization callback, this is called once and constructs the edge counters
 */
SANCOV_CALLBACK UInt32* __code_sancov_pc_guard_ctor(UInt64 edgeCount)
{
    UInt32* p = (UInt32*)calloc(edgeCount, sizeof(UInt32));
    if (p != NULL) {
        __sanitizer_cov_trace_pc_guard_init(p, p + edgeCount);
    }
    return p;
}

/**
 * This callback is needed to achieve compatibility with fuzz library, doesn't do anything
 */
SANCOV_CALLBACK int LLVMFuzzerRunDriver(__attribute__((unused)) int* argc, __attribute__((unused)) char*** argv,
    __attribute__((unused)) int (*userCb)(const UInt8* data, size_t size))
{
    return 0;
}

#ifdef __arm__
#pragma GCC diagnostic pop
#endif
