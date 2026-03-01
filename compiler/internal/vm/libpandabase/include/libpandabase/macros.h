/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef LIBPANDABASE_MACROS_H
#define LIBPANDABASE_MACROS_H

#include <cassert>
#include <iostream>
#include "os/stacktrace.h"
#include "utils/debug.h"
#include "panda_visibility.h"

// Inline (disabled for DEBUG)
#ifndef NDEBUG
#define ALWAYS_INLINE                                 // NOLINT(cppcoreguidelines-macro-usage)
#else                                                 // NDEBUG
#define ALWAYS_INLINE __attribute__((always_inline))  // NOLINT(cppcoreguidelines-macro-usage)
#endif                                                // !NDEBUG

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_INLINE __attribute__((noinline))

#ifdef __clang__
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_OPTIMIZE [[clang::optnone]]
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_OPTIMIZE __attribute__((optimize("O0")))
#endif

#ifdef __clang__
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FIELD_UNUSED __attribute__((__unused__))
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define FIELD_UNUSED
#endif

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNUSED_VAR(var) (void)(var)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MEMBER_OFFSET(T, F) offsetof(T, F)

#if defined(__cplusplus)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_COPY_CTOR(TypeName) TypeName(const TypeName &) = delete

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_COPY_OPERATOR(TypeName) void operator=(const TypeName &) = delete

// Disabling copy ctor and copy assignment operator.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_COPY_SEMANTIC(TypeName) \
    NO_COPY_CTOR(TypeName);        \
    NO_COPY_OPERATOR(TypeName)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_MOVE_CTOR(TypeName)                   \
    /* NOLINTNEXTLINE(misc-macro-parentheses) */ \
    TypeName(TypeName &&) = delete

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_MOVE_OPERATOR(TypeName)               \
    /* NOLINTNEXTLINE(misc-macro-parentheses) */ \
    TypeName &operator=(TypeName &&) = delete

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NO_MOVE_SEMANTIC(TypeName) \
    NO_MOVE_CTOR(TypeName);        \
    NO_MOVE_OPERATOR(TypeName)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_MOVE_CTOR(TypeName)              \
    /* NOLINTNEXTLINE(misc-macro-parentheses) */ \
    TypeName(TypeName &&) = default

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_MOVE_OPERATOR(TypeName)          \
    /* NOLINTNEXTLINE(misc-macro-parentheses) */ \
    TypeName &operator=(TypeName &&) = default

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_MOVE_SEMANTIC(TypeName) \
    DEFAULT_MOVE_CTOR(TypeName);        \
    DEFAULT_MOVE_OPERATOR(TypeName)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_COPY_CTOR(TypeName) TypeName(const TypeName &) = default

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_COPY_OPERATOR(TypeName)          \
    /* NOLINTNEXTLINE(misc-macro-parentheses) */ \
    TypeName &operator=(const TypeName &) = default

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_COPY_SEMANTIC(TypeName) \
    DEFAULT_COPY_CTOR(TypeName);        \
    DEFAULT_COPY_OPERATOR(TypeName)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_NOEXCEPT_MOVE_CTOR(TypeName)     \
    /* NOLINTNEXTLINE(misc-macro-parentheses) */ \
    TypeName(TypeName &&) noexcept = default

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_NOEXCEPT_MOVE_OPERATOR(TypeName) \
    /* NOLINTNEXTLINE(misc-macro-parentheses) */ \
    TypeName &operator=(TypeName &&) noexcept = default

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_NOEXCEPT_MOVE_SEMANTIC(TypeName) \
    DEFAULT_NOEXCEPT_MOVE_CTOR(TypeName);        \
    DEFAULT_NOEXCEPT_MOVE_OPERATOR(TypeName)

#endif  // defined(__cplusplus)

#define LIKELY(exp) (__builtin_expect((exp) != 0, true))     // NOLINT(cppcoreguidelines-macro-usage)
#define UNLIKELY(exp) (__builtin_expect((exp) != 0, false))  // NOLINT(cppcoreguidelines-macro-usage)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ABORT_AND_UNREACHABLE()                                      \
    do {                                                             \
        std::cerr << "This line should be unreachable" << std::endl; \
        std::abort();                                                \
        __builtin_unreachable();                                     \
    } while (0)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK(expr)                                                                                           \
    do {                                                                                                      \
        if (UNLIKELY(!(expr))) {                                                                              \
            std::cerr << "CHECK FAILED: " << #expr;                                                           \
            std::cerr << "          IN: " << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << std::endl; \
            panda::PrintStack(std::cerr);                                                                     \
            std::abort();                                                                                     \
        }                                                                                                     \
    } while (0)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_NOT_NULL(ptr) CHECK((ptr) != nullptr)

#if !defined(NDEBUG)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_FAIL(expr) panda::debug::AssertionFail(expr, __FILE__, __LINE__, __FUNCTION__)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_OP(lhs, op, rhs)                                                                                \
    do {                                                                                                       \
        auto __lhs = lhs;                                                                                      \
        auto __rhs = rhs;                                                                                      \
        if (UNLIKELY(!(__lhs op __rhs))) {                                                                     \
            std::cerr << "CHECK FAILED: " << #lhs << " " #op " " #rhs << std::endl;                            \
            std::cerr << "      VALUES: " << __lhs << " " #op " " << __rhs << std::endl;                       \
            std::cerr << "          IN: " << __FILE__ << ":" << __LINE__ << ": " << __FUNCTION__ << std::endl; \
            panda::PrintStack(std::cerr);                                                                      \
            std::abort();                                                                                      \
        }                                                                                                      \
    } while (0)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_LE(lhs, rhs) ASSERT_OP(lhs, <=, rhs)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_LT(lhs, rhs) ASSERT_OP(lhs, <, rhs)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_GE(lhs, rhs) ASSERT_OP(lhs, >=, rhs)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_GT(lhs, rhs) ASSERT_OP(lhs, >, rhs)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_EQ(lhs, rhs) ASSERT_OP(lhs, ==, rhs)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_NE(lhs, rhs) ASSERT_OP(lhs, !=, rhs)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT(cond)         \
    if (UNLIKELY(!(cond))) { \
        ASSERT_FAIL(#cond);  \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_DO(cond, func)                              \
    do {                                                   \
        if (auto cond_val = cond; UNLIKELY(!(cond_val))) { \
            func;                                          \
            ASSERT_FAIL(#cond);                            \
        }                                                  \
    } while (0)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_PRINT(cond, message)                        \
    do {                                                   \
        if (auto cond_val = cond; UNLIKELY(!(cond_val))) { \
            std::cerr << message << std::endl;             \
            ASSERT_FAIL(#cond);                            \
        }                                                  \
    } while (0)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_RETURN(cond) assert(cond)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNREACHABLE()                                                                            \
    do {                                                                                         \
        ASSERT_PRINT(false, "This line should be unreachable"); /* NOLINT(misc-static-assert) */ \
        __builtin_unreachable();                                                                 \
    } while (0)
#else                                                     // NDEBUG
#define ASSERT(cond) static_cast<void>(0)                 // NOLINT(cppcoreguidelines-macro-usage)
#define ASSERT_DO(cond, func) static_cast<void>(0)        // NOLINT(cppcoreguidelines-macro-usage)
#define ASSERT_PRINT(cond, message) static_cast<void>(0)  // NOLINT(cppcoreguidelines-macro-usage)
#define ASSERT_RETURN(cond) static_cast<void>(cond)       // NOLINT(cppcoreguidelines-macro-usage)
#define UNREACHABLE() ABORT_AND_UNREACHABLE()             // NOLINT(cppcoreguidelines-macro-usage)
#define ASSERT_OP(lhs, op, rhs)                           // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_LE(lhs, rhs)                                // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_LT(lhs, rhs)                                // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_GE(lhs, rhs)                                // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_GT(lhs, rhs)                                // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_EQ(lhs, rhs)                                // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_NE(lhs, rhs)                                // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#endif                                                    // !NDEBUG

// Due to the impossibility of using asserts in constexpr methods
// we need an extra version of UNREACHABLE macro that can be used in such situations
#define UNREACHABLE_CONSTEXPR() ABORT_AND_UNREACHABLE()  // NOLINT(cppcoreguidelines-macro-usage)

#define MERGE_WORDS_X(A, B) A##B               // NOLINT(cppcoreguidelines-macro-usage)
#define MERGE_WORDS(A, B) MERGE_WORDS_X(A, B)  // NOLINT(cppcoreguidelines-macro-usage)

// for clang
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
#define USE_THREAD_SANITIZER
#endif
#endif
// for gnu compiler
#if defined(__SANITIZE_THREAD__)
#define USE_THREAD_SANITIZER
#endif

#if defined(USE_THREAD_SANITIZER)
#if __GNUC__ < 8 && !defined(__clang__)
// gcc < 8.0 has a bug with attributes: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=78204
// Note, clang also defines __GNUC__ macro, so, check __clang also
#define NO_THREAD_SANITIZE __attribute__((no_sanitize_thread))  // NOLINT(cppcoreguidelines-macro-usage)
#else
#define NO_THREAD_SANITIZE __attribute__((no_sanitize("thread")))  // NOLINT(cppcoreguidelines-macro-usage)
#endif
// Do not inline no_sanitize functions to avoid gcc bug with missed annotations
#define ALWAYS_INLINE_NO_TSAN NO_THREAD_SANITIZE
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TSAN_ANNOTATE_HAPPENS_BEFORE(addr) AnnotateHappensBefore(__FILE__, __LINE__, reinterpret_cast<void *>(addr))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TSAN_ANNOTATE_HAPPENS_AFTER(addr) AnnotateHappensAfter(__FILE__, __LINE__, reinterpret_cast<void *>(addr))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TSAN_ANNOTATE_IGNORE_WRITES_BEGIN() AnnotateIgnoreWritesBegin(__FILE__, __LINE__)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TSAN_ANNOTATE_IGNORE_WRITES_END() AnnotateIgnoreWritesEnd(__FILE__, __LINE__)
extern "C" void AnnotateHappensBefore(const char *file, int line, const volatile void *cv);
extern "C" void AnnotateHappensAfter(const char *file, int line, const volatile void *cv);
extern "C" void AnnotateIgnoreWritesBegin(const char *file, int line);
extern "C" void AnnotateIgnoreWritesEnd(const char *file, int line);
// Attribute works in clang 14 or higher versions, currently used only for INTRUSIVE_TESTING
#define DISABLE_THREAD_SANITIZER_INSTRUMENTATION __attribute__((disable_sanitizer_instrumentation))
#else
#define NO_THREAD_SANITIZE
#define ALWAYS_INLINE_NO_TSAN ALWAYS_INLINE
#define TSAN_ANNOTATE_HAPPENS_BEFORE(addr)
#define TSAN_ANNOTATE_HAPPENS_AFTER(addr)
#define TSAN_ANNOTATE_IGNORE_WRITES_BEGIN()
#define TSAN_ANNOTATE_IGNORE_WRITES_END()
#endif

// TSAN does not support memory fences. To avoid false positive reports we can use atomic loads instead
// Ref: https://doc.rust-lang.org/src/alloc/sync.rs.html#58-74
#if defined(USE_THREAD_SANITIZER)
// CC-OFFNXT(G.PRE.02) code readability
#define ATOMIC_THREAD_FENCE_ACQUIRE(expr) expr.load(std::memory_order_acquire) // NOLINT(cppcoreguidelines-macro-usage)
#else
// CC-OFFNXT(G.PRE.02) code readability
#define ATOMIC_THREAD_FENCE_ACQUIRE(expr) std::atomic_thread_fence(std::memory_order_acquire) // NOLINT(cppcoreguidelines-macro-usage)
#endif

// for clang
#if defined(__has_feature)
#if __has_feature(undefined_behavior_sanitizer)
#define USE_UB_SANITIZER
#endif
#endif
// for gnu compiler
#if defined(__SANITIZE_UNDEFINED__)
#define USE_UB_SANITIZER
#endif

#if defined(USE_UB_SANITIZER)
#if __GNUC__ < 8 && !defined(__clang__)
// gcc < 8.0 has a bug with attributes: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=78204
// Note, clang also defines __GNUC__ macro, so, check __clang also
#define NO_UB_SANITIZE __attribute__((no_sanitize_undefined))  // NOLINT(cppcoreguidelines-macro-usage)
#else
#define NO_UB_SANITIZE __attribute__((no_sanitize("undefined")))  // NOLINT(cppcoreguidelines-macro-usage)
#endif
#else
#define NO_UB_SANITIZE
#endif  // USE_UB_SANITIZER

#ifndef __SANITIZE_HWADDRESS__
// for clang
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define USE_ADDRESS_SANITIZER
#endif
#endif
// for gnu compiler
#if defined(__SANITIZE_ADDRESS__)
#define USE_ADDRESS_SANITIZER
#endif
#else
#define NO_ADDRESS_SANITIZE
#endif

#ifdef USE_ADDRESS_SANITIZER
#if __GNUC__ < 8 && !defined(__clang__)
// gcc < 8.0 has a bug with attributes: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=78204
// Note, clang also defines __GNUC__ macro, so, check __clang also
#define NO_ADDRESS_SANITIZE __attribute__((no_sanitize_address))  // NOLINT(cppcoreguidelines-macro-usage)
#else
#define NO_ADDRESS_SANITIZE __attribute__((no_sanitize("address")))  // NOLINT(cppcoreguidelines-macro-usage)
#endif
#else
#define NO_ADDRESS_SANITIZE
#endif  // USE_ADDRESS_SANITIZER

// std::exit performs static object destrution which is not suitable for us, but in ohos we still need it for now
// eventually we will come up with better solution
#ifndef PANDA_BUILD_IN_OHOS_TREE
// CC-OFFNXT(G.PRE.02) code readability
#define PROCESS_EXIT(code) std::quick_exit(code) // NOLINT(cppcoreguidelines-macro-usage)
#else
// CC-OFFNXT(G.PRE.02) code readability
#define PROCESS_EXIT(code) std::exit(code) // NOLINT(cppcoreguidelines-macro-usage)
#endif

#if defined(__has_include)
#if __has_include(<filesystem>)
#define USE_STD_FILESYSTEM
#endif
#endif

// WEAK_SYMBOLS_FOR_LTO is defined if compiling arkbase_lto and unset otherwise
#ifdef WEAK_SYMBOLS_FOR_LTO
#define WEAK_FOR_LTO_START _Pragma("clang attribute push(__attribute__((weak)), apply_to = function)")
#define WEAK_FOR_LTO_END _Pragma("clang attribute pop")
#else
#define WEAK_FOR_LTO_START
#define WEAK_FOR_LTO_END
#endif

#ifndef NDEBUG
#define CONSTEXPR_IN_RELEASE
#else
#define CONSTEXPR_IN_RELEASE constexpr
#endif

#if defined(PANDA_TARGET_UNIX)
#define PANDA_STACK_SIZE 8 * 1024 * 1024
#elif defined(PANDA_TARGET_WINDOWS)
#define PANDA_STACK_SIZE 10 * 1024 * 1024
#else
#define PANDA_STACK_SIZE 0
#endif

#endif  // LIBPANDABASE_MACROS_H
