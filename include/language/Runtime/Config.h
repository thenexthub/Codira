//===--- Config.h - Codira Language Platform Configuration -------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// Definitions of common interest in Codira.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_CONFIG_H
#define LANGUAGE_RUNTIME_CONFIG_H

#include "language/Basic/Compiler.h"
#include "language/Runtime/CMakeConfig.h"

/// LANGUAGE_RUNTIME_WEAK_IMPORT - Marks a symbol for weak import.
#if (__has_attribute(weak_import))
#define LANGUAGE_RUNTIME_WEAK_IMPORT __attribute__((weak_import))
#else
#define LANGUAGE_RUNTIME_WEAK_IMPORT
#endif

/// LANGUAGE_RUNTIME_WEAK_CHECK - Tests if a potentially weakly linked function
/// is linked into the runtime.  This is useful on Apple platforms where it is
/// possible that system functions are only available on newer versions.
#ifdef __clang__
#define LANGUAGE_RUNTIME_WEAK_CHECK(x)                                     \
  _Pragma("clang diagnostic push")                                      \
  _Pragma("clang diagnostic ignored \"-Wunguarded-availability\"")      \
  _Pragma("clang diagnostic ignored \"-Wunguarded-availability-new\"")  \
  (&x)                                                                  \
  _Pragma("clang diagnostic pop")
#else
#define LANGUAGE_RUNTIME_WEAK_CHECK(x) &x
#endif

/// LANGUAGE_RUNTIME_WEAK_USE - Use a potentially weakly imported symbol.
#ifdef __clang__
#define LANGUAGE_RUNTIME_WEAK_USE(x)                                       \
  _Pragma("clang diagnostic push")                                      \
  _Pragma("clang diagnostic ignored \"-Wunguarded-availability\"")      \
  _Pragma("clang diagnostic ignored \"-Wunguarded-availability-new\"")  \
  (x)                                                                   \
  _Pragma("clang diagnostic pop")
#else
#define LANGUAGE_RUNTIME_WEAK_USE(x) x
#endif

/// LANGUAGE_RUNTIME_LIBRARY_VISIBILITY - If a class marked with this attribute is
/// linked into a shared library, then the class should be private to the
/// library and not accessible from outside it.  Can also be used to mark
/// variables and functions, making them private to any shared library they are
/// linked into.
/// On PE/COFF targets, library visibility is the default, so this isn't needed.
#if (__has_attribute(visibility) || LANGUAGE_GNUC_PREREQ(4, 0, 0)) &&    \
    !defined(__MINGW32__) && !defined(__CYGWIN__) && !defined(_WIN32)
#define LANGUAGE_RUNTIME_LIBRARY_VISIBILITY __attribute__ ((visibility("hidden")))
#else
#define LANGUAGE_RUNTIME_LIBRARY_VISIBILITY
#endif

#define LANGUAGE_RUNTIME_ATTRIBUTE_NOINLINE LANGUAGE_ATTRIBUTE_NOINLINE
#define LANGUAGE_RUNTIME_ATTRIBUTE_ALWAYS_INLINE LANGUAGE_ATTRIBUTE_ALWAYS_INLINE
#define LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN LANGUAGE_ATTRIBUTE_NORETURN

/// LANGUAGE_RUNTIME_BUILTIN_TRAP - On compilers which support it, expands to an expression
/// which causes the program to exit abnormally.
#if __has_builtin(__builtin_trap) || LANGUAGE_GNUC_PREREQ(4, 3, 0)
# define LANGUAGE_RUNTIME_BUILTIN_TRAP __builtin_trap()
#elif defined(_MSC_VER)
// The __debugbreak intrinsic is supported by MSVC, does not require forward
// declarations involving platform-specific typedefs (unlike RaiseException),
// results in a call to vectored exception handlers, and encodes to a short
// instruction that still causes the trapping behavior we want.
# define LANGUAGE_RUNTIME_BUILTIN_TRAP __debugbreak()
#else
# define LANGUAGE_RUNTIME_BUILTIN_TRAP *(volatile int*)0x11 = 0
#endif

/// Does the current Codira platform support "unbridged" interoperation
/// with Objective-C?  If so, the implementations of various types must
/// implicitly handle Objective-C pointers.
///
/// Apple platforms support this by default.
#ifndef LANGUAGE_OBJC_INTEROP
#ifdef __APPLE__
#define LANGUAGE_OBJC_INTEROP 1
#else
#define LANGUAGE_OBJC_INTEROP 0
#endif
#endif

/// Does the current Codira platform allow information other than the
/// class pointer to be stored in the isa field?  If so, when deriving
/// the class pointer of an object, we must apply a
/// dynamically-determined mask to the value loaded from the first
/// field of the object.
///
/// According to the Objective-C ABI, this is true only for 64-bit
/// platforms.
#ifndef LANGUAGE_HAS_ISA_MASKING
#if LANGUAGE_OBJC_INTEROP && __POINTER_WIDTH__ == 64
#define LANGUAGE_HAS_ISA_MASKING 1
#else
#define LANGUAGE_HAS_ISA_MASKING 0
#endif
#endif

/// Does the current Codira platform have ISA pointers which should be opaque
/// to anyone outside the Codira runtime?  Similarly to the ISA_MASKING case
/// above, information other than the class pointer could be contained in the
/// ISA.
#ifndef LANGUAGE_HAS_OPAQUE_ISAS
#if defined(__arm__) && __ARM_ARCH_7K__ >= 2
#define LANGUAGE_HAS_OPAQUE_ISAS 1
#else
#define LANGUAGE_HAS_OPAQUE_ISAS 0
#endif
#endif

#if LANGUAGE_HAS_OPAQUE_ISAS && LANGUAGE_HAS_ISA_MASKING
#error Masking ISAs are incompatible with opaque ISAs
#endif

#if defined(__APPLE__) && defined(__LP64__) && __has_include(<malloc_type_private.h>) && LANGUAGE_STDLIB_HAS_DARWIN_LIBMALLOC
# include <TargetConditionals.h>
# if TARGET_OS_IOS && !TARGET_OS_SIMULATOR
#  define LANGUAGE_STDLIB_HAS_MALLOC_TYPE 1
# endif
#endif
#ifndef LANGUAGE_STDLIB_HAS_MALLOC_TYPE
# define LANGUAGE_STDLIB_HAS_MALLOC_TYPE 0
#endif

/// Which bits in the class metadata are used to distinguish Codira classes
/// from ObjC classes?
#ifndef LANGUAGE_CLASS_IS_LANGUAGE_MASK

// Compatibility hook libraries cannot rely on the "is language" bit being either
// value, since they must work with both OS and Xcode versions of the libraries.
// Generate a reference to a nonexistent symbol so that we get obvious linker
// errors if we try.
# if LANGUAGE_COMPATIBILITY_LIBRARY
extern uintptr_t __COMPATIBILITY_LIBRARIES_CANNOT_CHECK_THE_IS_LANGUAGE_BIT_DIRECTLY__;
#  define LANGUAGE_CLASS_IS_LANGUAGE_MASK __COMPATIBILITY_LIBRARIES_CANNOT_CHECK_THE_IS_LANGUAGE_BIT_DIRECTLY__

// Apple platforms always use 2
# elif defined(__APPLE__)
#  define LANGUAGE_CLASS_IS_LANGUAGE_MASK 2ULL

// Non-Apple platforms always use 1.
# else
#  define LANGUAGE_CLASS_IS_LANGUAGE_MASK 1ULL

# endif
#endif

// We try to avoid global constructors in the runtime as much as possible.
// These macros delimit allowed global ctors.
#if __clang__
# define LANGUAGE_ALLOWED_RUNTIME_GLOBAL_CTOR_BEGIN \
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Wglobal-constructors\"")
# define LANGUAGE_ALLOWED_RUNTIME_GLOBAL_CTOR_END \
    _Pragma("clang diagnostic pop")
#else
# define LANGUAGE_ALLOWED_RUNTIME_GLOBAL_CTOR_BEGIN
# define LANGUAGE_ALLOWED_RUNTIME_GLOBAL_CTOR_END
#endif

// Bring in visibility attribute macros
#include "language/shims/Visibility.h"

// Temporary definitions to allow compilation on clang-15.
#if defined(__cplusplus)
#define LANGUAGE_EXTERN_C extern "C"
#else
#define LANGUAGE_EXTERN_C
#endif
#define LANGUAGE_RUNTIME_EXPORT_ATTRIBUTE LANGUAGE_EXPORT_FROM_ATTRIBUTE(languageCore)

// Define mappings for calling conventions.

// Annotation for specifying a calling convention of
// a runtime function. It should be used with declarations
// of runtime functions like this:
// void runtime_function_name() LANGUAGE_CC(language)
#define LANGUAGE_CC(CC) LANGUAGE_CC_##CC

// LANGUAGE_CC(c) is the C calling convention.
#define LANGUAGE_CC_c

// LANGUAGE_CC(language) is the Codira calling convention.
// FIXME: the next comment is false.
// Functions outside the stdlib or runtime that include this file may be built
// with a compiler that doesn't support languagecall; don't define these macros
// in that case so any incorrect usage is caught.
#if __has_attribute(languagecall)
#define LANGUAGE_CC_language __attribute__((languagecall))
#define LANGUAGE_CONTEXT __attribute__((language_context))
#define LANGUAGE_ERROR_RESULT __attribute__((language_error_result))
#define LANGUAGE_INDIRECT_RESULT __attribute__((language_indirect_result))
#else
#define LANGUAGE_CC_language
#define LANGUAGE_CONTEXT
#define LANGUAGE_ERROR_RESULT
#define LANGUAGE_INDIRECT_RESULT
#endif

#if __has_attribute(language_async_context)
#define LANGUAGE_ASYNC_CONTEXT __attribute__((language_async_context))
#else
#define LANGUAGE_ASYNC_CONTEXT
#endif

// LANGUAGE_CC(languageasync) is the Codira async calling convention.
// We assume that it supports mandatory tail call elimination.
#if __has_attribute(languageasynccall)
# if __has_feature(languageasynccc) || __has_extension(languageasynccc)
#  define LANGUAGE_CC_languageasync __attribute__((languageasynccall))
# else
#  define LANGUAGE_CC_languageasync LANGUAGE_CC_language
# endif
#else
#define LANGUAGE_CC_languageasync LANGUAGE_CC_language
#endif

// LANGUAGE_CC(PreserveMost) is used in the runtime implementation to prevent
// register spills on the hot path.
// It is not safe to use for external calls; the loader's lazy function
// binding may not save all of the registers required for this convention.
#if __has_attribute(preserve_most) &&                                          \
    (defined(__aarch64__) || defined(__x86_64__))
#define LANGUAGE_CC_PreserveMost __attribute__((preserve_most))
#else
#define LANGUAGE_CC_PreserveMost
#endif

// This is the DefaultCC value used by the compiler.
// FIXME: the runtime's code does not honor DefaultCC
// so changing this value is not sufficient.
#define LANGUAGE_DEFAULT_TOOLCHAIN_CC toolchain::CallingConv::C

/// Should we use absolute function pointers instead of relative ones?
/// WebAssembly target uses it by default.
#ifndef LANGUAGE_COMPACT_ABSOLUTE_FUNCTION_POINTER
# if defined(__wasm__)
#  define LANGUAGE_COMPACT_ABSOLUTE_FUNCTION_POINTER 1
# else
#  define LANGUAGE_COMPACT_ABSOLUTE_FUNCTION_POINTER 0
# endif
#endif

// Pointer authentication.
#if __has_feature(ptrauth_calls)
#define LANGUAGE_PTRAUTH 1
#include <ptrauth.h>
#define __ptrauth_language_runtime_function_entry \
  __ptrauth(ptrauth_key_function_pointer, 1, \
            SpecialPointerAuthDiscriminators::RuntimeFunctionEntry)
#define __ptrauth_language_runtime_function_entry_with_key(__key) \
  __ptrauth(ptrauth_key_function_pointer, 1, __key)
#define __ptrauth_language_runtime_function_entry_strip(__fn) \
  ptrauth_strip(__fn, ptrauth_key_function_pointer)
#define __ptrauth_language_type_descriptor \
  __ptrauth(ptrauth_key_process_independent_data, 1, \
            SpecialPointerAuthDiscriminators::TypeDescriptor)
#define __ptrauth_language_protocol_conformance_descriptor \
  __ptrauth(ptrauth_key_process_independent_data, 1, \
            SpecialPointerAuthDiscriminators::ProtocolConformanceDescriptor)
#define __ptrauth_language_dynamic_replacement_key                                \
  __ptrauth(ptrauth_key_process_independent_data, 1,                           \
            SpecialPointerAuthDiscriminators::DynamicReplacementKey)
#define __ptrauth_language_job_invoke_function                                    \
  __ptrauth(ptrauth_key_function_pointer, 1,                                   \
            SpecialPointerAuthDiscriminators::JobInvokeFunction)
#define __ptrauth_language_task_resume_function                                   \
  __ptrauth(ptrauth_key_function_pointer, 1,                                   \
            SpecialPointerAuthDiscriminators::TaskResumeFunction)
#define __ptrauth_language_task_resume_context                                    \
  __ptrauth(ptrauth_key_process_independent_data, 1,                           \
            SpecialPointerAuthDiscriminators::TaskResumeContext)
#define __ptrauth_language_async_context_parent                                   \
  __ptrauth(ptrauth_key_process_independent_data, 1,                           \
            SpecialPointerAuthDiscriminators::AsyncContextParent)
#define __ptrauth_language_async_context_resume                                   \
  __ptrauth(ptrauth_key_function_pointer, 1,                                   \
            SpecialPointerAuthDiscriminators::AsyncContextResume)
#define __ptrauth_language_async_context_yield                                    \
  __ptrauth(ptrauth_key_function_pointer, 1,                                   \
            SpecialPointerAuthDiscriminators::AsyncContextYield)
#define __ptrauth_language_cancellation_notification_function                     \
  __ptrauth(ptrauth_key_function_pointer, 1,                                   \
            SpecialPointerAuthDiscriminators::CancellationNotificationFunction)
#define __ptrauth_language_escalation_notification_function                       \
  __ptrauth(ptrauth_key_function_pointer, 1,                                   \
            SpecialPointerAuthDiscriminators::EscalationNotificationFunction)
#define __ptrauth_language_dispatch_invoke_function                               \
  __ptrauth(ptrauth_key_process_independent_code, 1,                           \
            SpecialPointerAuthDiscriminators::DispatchInvokeFunction)
#define __ptrauth_language_accessible_function_record                             \
  __ptrauth(ptrauth_key_process_independent_data, 1,                           \
            SpecialPointerAuthDiscriminators::AccessibleFunctionRecord)
#define __ptrauth_language_objc_superclass                                        \
  __ptrauth(ptrauth_key_process_independent_data, 1,                           \
            language::SpecialPointerAuthDiscriminators::ObjCSuperclass)
#define __ptrauth_language_nonunique_extended_existential_type_shape              \
  __ptrauth(ptrauth_key_process_independent_data, 1,                           \
            SpecialPointerAuthDiscriminators::NonUniqueExtendedExistentialTypeShape)
#define language_ptrauth_sign_opaque_read_resume_function(__fn, __buffer)         \
  ptrauth_auth_and_resign(__fn, ptrauth_key_function_pointer, 0,               \
                          ptrauth_key_process_independent_code,                \
                          ptrauth_blend_discriminator(__buffer,                \
            SpecialPointerAuthDiscriminators::OpaqueReadResumeFunction))
#define language_ptrauth_sign_opaque_modify_resume_function(__fn, __buffer)       \
  ptrauth_auth_and_resign(__fn, ptrauth_key_function_pointer, 0,               \
                          ptrauth_key_process_independent_code,                \
                          ptrauth_blend_discriminator(__buffer,                \
            SpecialPointerAuthDiscriminators::OpaqueModifyResumeFunction))
#define __ptrauth_language_type_layout_string                                     \
  __ptrauth(ptrauth_key_process_independent_data, 1,                           \
            SpecialPointerAuthDiscriminators::TypeLayoutString)
#define __ptrauth_language_deinit_work_function                                   \
  __ptrauth(ptrauth_key_function_pointer, 1,                                   \
            SpecialPointerAuthDiscriminators::DeinitWorkFunction)
#define __ptrauth_language_is_global_actor_function                               \
  __ptrauth(ptrauth_key_function_pointer, 1,                                   \
            SpecialPointerAuthDiscriminators::IsCurrentGlobalActorFunction)

#if __has_attribute(ptrauth_struct)
#define language_ptrauth_struct(key, discriminator)                               \
  __attribute__((ptrauth_struct(key, discriminator)))
#else
#define language_ptrauth_struct(key, discriminator)
#endif
// Set ptrauth_struct to the same scheme as the ptrauth_struct on `from`, but
// with a modified discriminator.
#define language_ptrauth_struct_derived(from)                                     \
  language_ptrauth_struct(__builtin_ptrauth_struct_key(from),                     \
                       __builtin_ptrauth_struct_disc(from) + 1)
#else
#define LANGUAGE_PTRAUTH 0
#define __ptrauth_language_function_pointer(__typekey)
#define __ptrauth_language_class_method_pointer(__declkey)
#define __ptrauth_language_protocol_witness_function_pointer(__declkey)
#define __ptrauth_language_value_witness_function_pointer(__key)
#define __ptrauth_language_type_metadata_instantiation_function
#define __ptrauth_language_job_invoke_function
#define __ptrauth_language_task_resume_function
#define __ptrauth_language_task_resume_context
#define __ptrauth_language_async_context_parent
#define __ptrauth_language_async_context_resume
#define __ptrauth_language_async_context_yield
#define __ptrauth_language_cancellation_notification_function
#define __ptrauth_language_escalation_notification_function
#define __ptrauth_language_dispatch_invoke_function
#define __ptrauth_language_accessible_function_record
#define __ptrauth_language_objc_superclass
#define __ptrauth_language_runtime_function_entry
#define __ptrauth_language_runtime_function_entry_with_key(__key)
#define __ptrauth_language_runtime_function_entry_strip(__fn) (__fn)
#define __ptrauth_language_heap_object_destructor
#define __ptrauth_language_type_descriptor
#define __ptrauth_language_protocol_conformance_descriptor
#define __ptrauth_language_nonunique_extended_existential_type_shape
#define __ptrauth_language_dynamic_replacement_key
#define language_ptrauth_sign_opaque_read_resume_function(__fn, __buffer) (__fn)
#define language_ptrauth_sign_opaque_modify_resume_function(__fn, __buffer) (__fn)
#define __ptrauth_language_type_layout_string
#define __ptrauth_language_deinit_work_function
#define __ptrauth_language_is_global_actor_function
#define language_ptrauth_struct(key, discriminator)
#define language_ptrauth_struct_derived(from)
#endif

#ifdef __cplusplus

#define language_ptrauth_struct_context_descriptor(name)                          \
  language_ptrauth_struct(ptrauth_key_process_dependent_data,                     \
                       ptrauth_string_discriminator(#name))

/// Copy an address-discriminated signed code pointer from the source
/// to the destination.
template <class T>
LANGUAGE_RUNTIME_ATTRIBUTE_ALWAYS_INLINE static inline void
language_ptrauth_copy(T *dest, const T *src, unsigned extra, bool allowNull) {
#if LANGUAGE_PTRAUTH
  if (allowNull && *src == nullptr) {
    *dest = nullptr;
    return;
  }

  *dest = ptrauth_auth_and_resign(*src,
                                  ptrauth_key_function_pointer,
                                  ptrauth_blend_discriminator(src, extra),
                                  ptrauth_key_function_pointer,
                                  ptrauth_blend_discriminator(dest, extra));
#else
  *dest = *src;
#endif
}

/// Copy an address-discriminated signed data pointer from the source
/// to the destination.
template <class T>
LANGUAGE_RUNTIME_ATTRIBUTE_ALWAYS_INLINE
static inline void language_ptrauth_copy_data(T *dest, const T *src,
                                           unsigned extra, bool allowNull) {
#if LANGUAGE_PTRAUTH
  if (allowNull && *src == nullptr) {
    *dest = nullptr;
    return;
  }

  *dest = ptrauth_auth_and_resign(*src,
                                  ptrauth_key_process_independent_data,
                                  ptrauth_blend_discriminator(src, extra),
                                  ptrauth_key_process_independent_data,
                                  ptrauth_blend_discriminator(dest, extra));
#else
  *dest = *src;
#endif
}

/// Copy an address-discriminated signed pointer from the source
/// to the destination.
template <class T>
LANGUAGE_RUNTIME_ATTRIBUTE_ALWAYS_INLINE static inline void
language_ptrauth_copy_code_or_data(T *dest, const T *src, unsigned extra,
                                bool isCode, bool allowNull) {
  if (isCode) {
    return language_ptrauth_copy(dest, src, extra, allowNull);
  } else {
    return language_ptrauth_copy_data(dest, src, extra, allowNull);
  }
}

/// Initialize the destination with an address-discriminated signed
/// function pointer.  This does not authenticate the source value, so be
/// careful about how you construct it.
template <class T>
LANGUAGE_RUNTIME_ATTRIBUTE_ALWAYS_INLINE
static inline void language_ptrauth_init(T *dest, T value, unsigned extra) {
  // FIXME: assert that T is not a function-pointer type?
#if LANGUAGE_PTRAUTH
  *dest = ptrauth_sign_unauthenticated(value,
                                  ptrauth_key_function_pointer,
                                  ptrauth_blend_discriminator(dest, extra));
#else
  *dest = value;
#endif
}

/// Initialize the destination with an address-discriminated signed
/// data pointer.  This does not authenticate the source value, so be
/// careful about how you construct it.
template <class T>
LANGUAGE_RUNTIME_ATTRIBUTE_ALWAYS_INLINE
static inline void language_ptrauth_init_data(T *dest, T value, unsigned extra) {
  // FIXME: assert that T is not a function-pointer type?
#if LANGUAGE_PTRAUTH
  *dest = ptrauth_sign_unauthenticated(value,
                                  ptrauth_key_process_independent_data,
                                  ptrauth_blend_discriminator(dest, extra));
#else
  *dest = value;
#endif
}

/// Initialize the destination with an address-discriminated signed
/// pointer.  This does not authenticate the source value, so be
/// careful about how you construct it.
template <class T>
LANGUAGE_RUNTIME_ATTRIBUTE_ALWAYS_INLINE static inline void
language_ptrauth_init_code_or_data(T *dest, T value, unsigned extra, bool isCode) {
  if (isCode) {
    return language_ptrauth_init(dest, value, extra);
  } else {
    return language_ptrauth_init_data(dest, value, extra);
  }
}

template <typename T>
LANGUAGE_RUNTIME_ATTRIBUTE_ALWAYS_INLINE
static inline T language_auth_data_non_address(T value, unsigned extra) {
#if LANGUAGE_PTRAUTH
  // Cast to void* using a union to avoid implicit ptrauth operations when T
  // points to a type with the ptrauth_struct attribute.
  union {
    T value;
    void *voidValue;
  } converter;
  converter.value = value;
  if (converter.voidValue == nullptr)
    return nullptr;
  return (T)ptrauth_auth_data(converter.voidValue,
                              ptrauth_key_process_independent_data, extra);
#else
  return value;
#endif
}

template <typename T>
LANGUAGE_RUNTIME_ATTRIBUTE_ALWAYS_INLINE
static inline T language_sign_data_non_address(T value, unsigned extra) {
#if LANGUAGE_PTRAUTH
  // Cast from void* using a union to avoid implicit ptrauth operations when T
  // points to a type with the ptrauth_struct attribute.
  union {
    T value;
    void *voidValue;
  } converter;
  converter.voidValue = ptrauth_sign_unauthenticated(
      (void *)value, ptrauth_key_process_independent_data, extra);
  return converter.value;
#else
  return value;
#endif
}

template <typename T>
LANGUAGE_RUNTIME_ATTRIBUTE_ALWAYS_INLINE
static inline T language_strip_data(T value) {
#if LANGUAGE_PTRAUTH
  // Cast to void* using a union to avoid implicit ptrauth operations when T
  // points to a type with the ptrauth_struct attribute.
  union {
    T value;
    void *voidValue;
  } converter;
  converter.value = value;

  return (T)ptrauth_strip(converter.voidValue, ptrauth_key_process_independent_data);
#else
  return value;
#endif
}

template <typename T>
LANGUAGE_RUNTIME_ATTRIBUTE_ALWAYS_INLINE static inline T
language_auth_code(T value, unsigned extra) {
#if LANGUAGE_PTRAUTH
  return (T)ptrauth_auth_function((void *)value,
                                  ptrauth_key_process_independent_code, extra);
#else
  return value;
#endif
}

/// Does this platform support backtrace-on-crash?
#ifdef __APPLE__
#  include <TargetConditionals.h>
#  if TARGET_OS_OSX
#    define LANGUAGE_BACKTRACE_ON_CRASH_SUPPORTED 1
#    define LANGUAGE_BACKTRACE_SECTION "__DATA,language5_backtrace"
#  else
#    define LANGUAGE_BACKTRACE_ON_CRASH_SUPPORTED 0
#  endif
#elif defined(_WIN32)
#  define LANGUAGE_BACKTRACE_ON_CRASH_SUPPORTED 0
#  define LANGUAGE_BACKTRACE_SECTION ".sw5bckt"
#elif defined(__linux__) && (defined(__aarch64__) || defined(__x86_64__))
#  define LANGUAGE_BACKTRACE_ON_CRASH_SUPPORTED 1
#  define LANGUAGE_BACKTRACE_SECTION "language5_backtrace"
#else
#  define LANGUAGE_BACKTRACE_ON_CRASH_SUPPORTED 0
#endif

/// What is the system page size?
#if defined(__APPLE__) && defined(__arm64__)
  // Apple Silicon systems use a 16KB page size
  #define LANGUAGE_PAGE_SIZE 16384
#else
  // Everything else uses 4KB pages
  #define LANGUAGE_PAGE_SIZE 4096
#endif

#endif

#endif // LANGUAGE_RUNTIME_CONFIG_H
