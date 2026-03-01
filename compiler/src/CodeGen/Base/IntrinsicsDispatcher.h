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

/**
 * @file
 *
 * This file declares generate Intrinsic APIs for codegen.
 */

#ifndef CODIRA_INTRINSICS_DISPATCHER_H
#define CODIRA_INTRINSICS_DISPATCHER_H

#include "llvm/IR/Value.h"

#include "CGModule.h"
#include "IRBuilder.h"

namespace Codira {
namespace CHIR {
class Intrinsic;
} // namespace CHIR
namespace CodeGen {
class IRBuilder2;
class CHIRIntrinsicWrapper;

llvm::Value* GenerateIntrinsic(IRBuilder2& irBuilder, const CHIRIntrinsicWrapper& intrinsic);

// `CGIntrinsicKind` is used to classify the syscall CHIR node (see GetCGIntrinsicKind())
// and map to specific generate function (see GenerateIntrinsic())
enum class CGIntrinsicKind {
    UNSAFE_MARK,
    CPOINTER_INIT,
    CSTRING_INIT,
    INOUT_PARAM,
    NATIVE_CALL,
    ARRAY,
    ARRAY_SLICE, // Array slice intrinsics
    VECTOR,
    SOURCE,
    OVERFLOW_APPLY, // 'OVERFLOW' is already in math.h
    REFLECT,
    BUILTIN,
    SYNC,
    MATH,
    STACK_TRACE,
    THREAD_INFO,
    IDENTITY_HASHCODE,
    FUTURE,
    NET,
    FFI_JAVA,
    INTEROP,
    VARRAY,
    RUNTIME,
    EXCEPTION_CATCH,
    PREINITIALIZE,
    UNKNOWN
};

inline bool IsOverflowIntrinsic(const CHIR::IntrinsicKind intrinsicKind)
{
    return (intrinsicKind >= CHIR::IntrinsicKind::OVERFLOW_CHECKED_ADD &&
        intrinsicKind <= CHIR::IntrinsicKind::OVERFLOW_WRAPPING_NEG);
}

inline bool IsArrayIntrinsic(const CHIR::IntrinsicKind intrinsicKind)
{
    return (intrinsicKind >= CHIR::IntrinsicKind::ARRAY_BUILT_IN_COPY_TO &&
        intrinsicKind <= CHIR::IntrinsicKind::ARRAY_CLONE) ||
        intrinsicKind == CHIR::IntrinsicKind::ARRAY_INIT;
}

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
inline bool IsVectorIntrinsic(const CHIR::IntrinsicKind intrinsicKind)
{
    return intrinsicKind >= CHIR::IntrinsicKind::VECTOR_COMPARE_32 &&
        intrinsicKind <= CHIR::IntrinsicKind::VECTOR_INDEX_BYTE_32;
}

inline bool IsPreInitializeIntrinsic(const CHIR::IntrinsicKind intrinsicKind)
{
    return intrinsicKind == CHIR::IntrinsicKind::PREINITIALIZE;
}
#endif

inline bool IsSourceIntrinsic(const CHIR::IntrinsicKind intrinsicKind)
{
    return (intrinsicKind == CHIR::IntrinsicKind::SOURCE_FILE || intrinsicKind == CHIR::IntrinsicKind::SOURCE_LINE);
}

const std::unordered_set<CHIR::IntrinsicKind> BUILTIN_FUNC_SET = {
    /// This comment is used to keep the code neat under format.
    CHIR::IntrinsicKind::OBJECT_REFEQ,
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    CHIR::IntrinsicKind::FUNC_REFEQ,
    CHIR::IntrinsicKind::RAW_ARRAY_REFEQ,
#endif
    CHIR::IntrinsicKind::OBJECT_ZERO_VALUE,
    CHIR::IntrinsicKind::ARRAY_ACQUIRE_RAW_DATA,
    CHIR::IntrinsicKind::ARRAY_RELEASE_RAW_DATA,
    CHIR::IntrinsicKind::CPOINTER_GET_POINTER_ADDRESS,
    CHIR::IntrinsicKind::CPOINTER_READ,
    CHIR::IntrinsicKind::CPOINTER_WRITE,
    CHIR::IntrinsicKind::CPOINTER_ADD,
    CHIR::IntrinsicKind::CSTRING_CONVERT_CSTR_TO_PTR,
    CHIR::IntrinsicKind::BIT_CAST,
    CHIR::IntrinsicKind::SIZE_OF,
    CHIR::IntrinsicKind::ALIGN_OF,
    CHIR::IntrinsicKind::OBJECT_AS,
    CHIR::IntrinsicKind::IS_NULL,
    CHIR::IntrinsicKind::GET_TYPE_FOR_TYPE_PARAMETER,
    CHIR::IntrinsicKind::IS_SUBTYPE_TYPES,
};

inline bool IsBuiltinIntrinsic(const CHIR::IntrinsicKind intrinsicKind)
{
    return BUILTIN_FUNC_SET.find(intrinsicKind) != BUILTIN_FUNC_SET.end();
}

inline bool IsFutureIntrinsic(const CHIR::IntrinsicKind intrinsicKind)
{
    return CHIR::IntrinsicKind::FUTURE_INIT <= intrinsicKind &&
        intrinsicKind <= CHIR::IntrinsicKind::SET_THREAD_OBJECT;
}

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
inline bool IsSyncIntrinsic(const CHIR::IntrinsicKind intrinsicKind)
{
    bool isSleep = intrinsicKind == CHIR::IntrinsicKind::SLEEP;

    return isSleep ||
        (intrinsicKind >= CHIR::IntrinsicKind::ATOMIC_LOAD &&
            intrinsicKind <= CHIR::IntrinsicKind::MULTICONDITION_NOTIFY_ALL);
}
#endif

inline bool IsMathIntrinsic(const CHIR::IntrinsicKind intrinsicKind)
{
    return (intrinsicKind >= CHIR::IntrinsicKind::ABS && intrinsicKind <= CHIR::IntrinsicKind::POWI);
}

inline bool IsStackTraceIntrinsic(const CHIR::IntrinsicKind intrinsicKind)
{
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    return intrinsicKind == CHIR::IntrinsicKind::FILL_IN_STACK_TRACE ||
        intrinsicKind == CHIR::IntrinsicKind::DECODE_STACK_TRACE;
#endif
}

inline bool IsThreadInfoIntrinsic(const CHIR::IntrinsicKind intrinsicKind)
{
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    return intrinsicKind == CHIR::IntrinsicKind::DUMP_CURRENT_THREAD_INFO ||
        intrinsicKind == CHIR::IntrinsicKind::DUMP_ALL_THREADS_INFO;
#endif
}

inline bool IsReflectIntrinsic(const CHIR::IntrinsicKind intrinsicKind)
{
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    return (intrinsicKind > CHIR::IntrinsicKind::REFLECTION_INTRINSIC_START_FLAG &&
        intrinsicKind < CHIR::IntrinsicKind::REFLECTION_INTRINSIC_END_FLAG);
#endif
}

inline bool IsVArrayIntrinsic(const CHIR::IntrinsicKind intrinsicKind)
{
    return (intrinsicKind == CHIR::IntrinsicKind::VARRAY_SET || intrinsicKind == CHIR::IntrinsicKind::VARRAY_GET);
}

inline bool IsRuntimeIntrinsic(const CHIR::IntrinsicKind intrinsicKind)
{
    return CHIR::IntrinsicKind::INVOKE_GC <= intrinsicKind &&
        intrinsicKind <= CHIR::IntrinsicKind::GET_NATIVE_THREAD_NUMBER;
}

inline bool IsExceptionCatchIntrinsic(const CHIR::IntrinsicKind intrinsicKind)
{
    return intrinsicKind == CHIR::IntrinsicKind::BEGIN_CATCH;
}

} // namespace CodeGen
} // namespace Codira
#endif // CODIRA_INTRINSICS_DISPATCHER_H
