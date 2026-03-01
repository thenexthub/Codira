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

#ifndef CODIRA_CGCOMMONDEF_H
#define CODIRA_CGCOMMONDEF_H

/**
 * @file
 *
 * To declare the common definitions that are used in CodeGen
 */

#include <string>

#include "llvm/IR/GlobalObject.h"
#include "llvm/IR/Intrinsics.h"

#include "Utils/Constants.h"
#include "Codira/Basic/Linkage.h"
#include "Codira/CHIR/IntrinsicKind.h"
#include "Codira/CHIR/Type/Type.h"

namespace Codira {
namespace CodeGen {
enum TypeInfoFields : uint8_t {
    TYPEINFO_NAME = 0,            // mangledName : i8*.
    TYPEINFO_TYPE_KIND,           // typeKind : i8
    TYPEINFO_FLAG,                // flag filled by llvm : i8
    TYPEINFO_FIELDS_NUM,          // number of member variables : i16
    TYPEINFO_SIZE,                // size of the type (unit: byte) : i32
    TYPEINFO_GC_TIB,              // which field is reference : %BitMap*
    TYPEINFO_UUID,                // unique ID of a type in a process : i32
    TYPEINFO_ALIGN,               // alignment of the type : i8
    TYPEINFO_TYPE_ARGS_NUM,       // number of generic type args : i8
    TYPEINFO_INHERITED_CLASS_NUM, // number of inherited classes : i16
    TYPEINFO_OFFSETS,             // i32*
    TYPEINFO_SOURCE_GENERIC,      // i8*
    TYPEINFO_TYPE_ARGS,           // i8
    TYPEINFO_FIELDS,              // typeInfos list of member variables : i8*
    TYPEINFO_SUPER,               // typeinfo*
    TYPEINFO_EXTENSIONDEF_PTR,    // %ExtensionDef*
    TYPEINFO_MTABLE,              // i8*
    TYPEINFO_REFLECTION,          // i8*

    TYPE_INFO_FIELDS_NUM // the number of TypeInfo's fields
};

enum GenericTypeInfoFields : uint8_t {
    GENERIC_TYPEINFO_NAME = 0,        // srcCodeIdentifier : i8*
    GENERIC_TYPEINFO_TYPE_KIND,       // typeKind : i8
    GENERIC_TYPEINFO_UPPERBOUNDS_NUM, // the number of generic upper bounds : i32
    GENERIC_TYPEINFO_UPPERBOUNDS,     // the list of upperbounds : i8*
    GENERIC_TYPE_INFO_FIELDS_NUM      // the number of TypeInfo's fields
};

enum GenericCustomTypeInfoFields : uint8_t {
    GENERIC_CUSTOM_TYPEINFO_NAME = 0,       // i8*
    GENERIC_CUSTOM_TYPEINFO_TYPE_KIND,      // typeKind : i8
    GENERIC_CUSTOM_TYPEINFO_TYPE_ARGS_NUM,  // i8
    GENERIC_CUSTOM_TYPEINFO_TYPE_ARGS,      // i8*
    GENERIC_CUSTOM_TYPEINFO_SOURCE_GENERIC, // i8*
    GENERIC_CUSTOM_TYPE_INFO_FIELDS_NUM     // the number of TypeInfo's fields
};

enum TypeTemplateFields : uint8_t {
    TYPETEMPLATE_NAME = 0,            // mangledName : i8*.
    TYPETEMPLATE_TYPE_KIND,           // typeKind : i8
    TYPETEMPLATE_FLAG,                // i8
    TYPETEMPLATE_FIELDS_NUM,          // number of member variables : i16
    TYPETEMPLATE_TYPE_ARGS_NUM,       // i8
    TYPETEMPLATE_FIELDS_FNS,          // typeInfos list of member variables : i8*
    TYPETEMPLATE_SUPER_FN,            // i8*
    TYPETEMPLATE_FINALIZER,           // i8*
    TYPETEMPLATE_REFLECTION,          // i8*
    TYPETEMPLATE_EXTENSIONDEF_PTR,    // %ExtensionDef*
    TYPETEMPLATE_INHERITED_CLASS_NUM, // i16

    TYPE_TEMPLATE_FIELDS_NUM // the number of TypeTemplate's fields
};

enum ExtensionDefFields : uint8_t {
    TYPE_PARAM_COUNT = 0,         // i32
    IS_INTERFACE_TI,              // i8
    FLAG,                         // i8
    FUNC_TABLE_SIZE,              // i16
    TARGET_TYPE,                  // i8*
    INTERFACE_FN_OR_INTERFACE_TI, // i8*
    WHERE_CONDITION_FN,           // i8*
    FUNC_TABLE,                   // i8*

    EXTENSION_DEF_FIELDS_NUM // the number of ExtensionDef's fields
};

enum ArrayBaseFields : uint8_t {
    ARRAY_BASE_ARRAY_LEN, // array length : i64

    ARRAY_BASE_FIELDS_NUM, // the number of ArrayBase's fields
};

enum BitMapFields : uint8_t {
    BITMAP_NUMS = 0, // nums of bit map : i32
    BITMAP_BITMAP,   // bitmap : [0 x i8]

    BITMAP_FIELDS_NUM // the number of BitMap's fields
};

struct IntrinsicFuncInfo {
    std::string funcName;
    std::vector<std::string> attributes;
};

struct GenericTypeAndPath {
public:
    GenericTypeAndPath(const CHIR::GenericType& gt, const std::vector<size_t>& path) : genericType(gt), path(path)
    {
    }
    const std::vector<size_t>& GetPath() const
    {
        return path;
    }
    const CHIR::GenericType* GetGenericType() const
    {
        return &genericType;
    }

private:
    const CHIR::GenericType& genericType;
    const std::vector<size_t> path;
};

const std::unordered_map<CHIR::BooleanType::TypeKind, std::pair<int64_t, int64_t>> G_SIGNED_INT_MAP = {
    {CHIR::BooleanType::TypeKind::TYPE_INT8, {std::numeric_limits<int8_t>::min(), std::numeric_limits<int8_t>::max()}},
    {CHIR::BooleanType::TypeKind::TYPE_INT16,
        {std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max()}},
    {CHIR::BooleanType::TypeKind::TYPE_INT32,
        {std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()}},
    {CHIR::BooleanType::TypeKind::TYPE_INT64,
        {std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max()}}};

const std::unordered_map<CHIR::Type::TypeKind, uint64_t> G_UNSIGNED_INT_MAP = {
    {CHIR::Type::TypeKind::TYPE_UINT8, std::numeric_limits<uint8_t>::max()},
    {CHIR::Type::TypeKind::TYPE_UINT16, std::numeric_limits<uint16_t>::max()},
    {CHIR::Type::TypeKind::TYPE_UINT32, std::numeric_limits<uint32_t>::max()},
    {CHIR::Type::TypeKind::TYPE_UINT64, std::numeric_limits<uint64_t>::max()}};

const std::set<std::string> REFLECT_INTRINSIC_FUNC = {"CODE_MCC_ApplyCODEInstanceMethod", "CODE_MCC_ApplyCODEStaticMethod",
    "CODE_MCC_ApplyCODEGenericInstanceMethod", "CODE_MCC_ApplyCODEGenericStaticMethod", "CODE_MCC_CheckMethodActualArgs",
    "CODE_MCC_GetOrCreateTypeInfoForReflect"};

const std::unordered_map<CHIR::IntrinsicKind, IntrinsicFuncInfo> INTRINSIC_KIND_TO_FUNCNAME_MAP = {
    // future
    {CHIR::IntrinsicKind::FUTURE_INIT, {"CODE_MCC_FutureInit", {FAST_NATIVE_ATTR}}},
    {CHIR::IntrinsicKind::FUTURE_IS_COMPLETE, {"CODE_MCC_FutureIsComplete", {FAST_NATIVE_ATTR}}},
    {CHIR::IntrinsicKind::FUTURE_WAIT, {"CODE_MCC_FutureWait", {}}},
    {CHIR::IntrinsicKind::FUTURE_NOTIFYALL, {"CODE_MCC_FutureNotifyAll", {FAST_NATIVE_ATTR}}},
    {CHIR::IntrinsicKind::IS_THREAD_OBJECT_INITED, {"CODE_MCC_IsThreadObjectInited", {FAST_NATIVE_ATTR}}},
    {CHIR::IntrinsicKind::GET_THREAD_OBJECT, {"CODE_MCC_GetCurrentCODEThreadObject", {FAST_NATIVE_ATTR}}},
    {CHIR::IntrinsicKind::SET_THREAD_OBJECT, {"CODE_MCC_SetCurrentCODEThreadObject", {FAST_NATIVE_ATTR}}},
    // sync
    {CHIR::IntrinsicKind::MUTEX_INIT, {"CODE_MCC_MutexInit", {FAST_NATIVE_ATTR}}},
    {CHIR::IntrinsicKind::CODE_MUTEX_LOCK, {"CODE_MCC_MutexLock", {}}},
    {CHIR::IntrinsicKind::MUTEX_TRY_LOCK, {"CODE_MCC_MutexTryLock", {FAST_NATIVE_ATTR}}},
    {CHIR::IntrinsicKind::MUTEX_CHECK_STATUS, {"CODE_MCC_MutexCheckStatus", {FAST_NATIVE_ATTR}}},
    {CHIR::IntrinsicKind::MUTEX_UNLOCK, {"CODE_MCC_MutexUnlock", {FAST_NATIVE_ATTR}}},
    {CHIR::IntrinsicKind::WAITQUEUE_INIT, {"CODE_MCC_WaitQueueInit", {FAST_NATIVE_ATTR}}},
    {CHIR::IntrinsicKind::MONITOR_INIT, {"CODE_MCC_WaitQueueForMonitorInit", {FAST_NATIVE_ATTR}}},
    {CHIR::IntrinsicKind::MOITIOR_WAIT, {"CODE_MCC_MonitorWait", {}}},
    {CHIR::IntrinsicKind::MOITIOR_NOTIFY, {"CODE_MCC_MonitorNotify", {FAST_NATIVE_ATTR}}},
    {CHIR::IntrinsicKind::MOITIOR_NOTIFY_ALL, {"CODE_MCC_MonitorNotifyAll", {FAST_NATIVE_ATTR}}},
    {CHIR::IntrinsicKind::MULTICONDITION_WAIT, {"CODE_MCC_MultiConditionMonitorWait", {}}},
    {CHIR::IntrinsicKind::MULTICONDITION_NOTIFY, {"CODE_MCC_MultiConditionMonitorNotify", {FAST_NATIVE_ATTR}}},
    {CHIR::IntrinsicKind::MULTICONDITION_NOTIFY_ALL, {"CODE_MCC_MultiConditionMonitorNotifyAll", {FAST_NATIVE_ATTR}}},
    {CHIR::IntrinsicKind::SLEEP, {"CODE_MRT_Sleep", {}}},
    // reflection
#define REFLECTION_KIND_TO_RUNTIME_FUNCTION(REFLECTION_KIND, CODE_FUNCTION, RUNTIME_FUNCTION)                            \
    {CHIR::IntrinsicKind::REFLECTION_KIND, {#RUNTIME_FUNCTION, {}}},
#include "Codira/CHIR/LLVMReflectionIntrinsics.def"
#undef REFLECTION_KIND_TO_RUNTIME_FUNCTION
};

const std::unordered_map<CHIR::Type::TypeKind, CHIR::Type::TypeKind> INTEGER_CONVERT_MAP = {
    {CHIR::Type::TypeKind::TYPE_UINT8, CHIR::Type::TypeKind::TYPE_INT8},
    {CHIR::Type::TypeKind::TYPE_UINT16, CHIR::Type::TypeKind::TYPE_INT16},
    {CHIR::Type::TypeKind::TYPE_UINT32, CHIR::Type::TypeKind::TYPE_INT32},
    {CHIR::Type::TypeKind::TYPE_UINT64, CHIR::Type::TypeKind::TYPE_INT64},
    {CHIR::Type::TypeKind::TYPE_UINT_NATIVE, CHIR::Type::TypeKind::TYPE_INT_NATIVE}};

const std::unordered_map<CHIR::IntrinsicKind, llvm::Intrinsic::ID> INTRINSIC_KIND_TO_ID_MAP = {
    // runtime
    {CHIR::IntrinsicKind::GET_MAX_HEAP_SIZE, llvm::Intrinsic::code_get_max_heap_size},
    {CHIR::IntrinsicKind::GET_ALLOCATE_HEAP_SIZE, llvm::Intrinsic::code_get_allocated_heap_size},
    {CHIR::IntrinsicKind::GET_REAL_HEAP_SIZE, llvm::Intrinsic::code_get_real_heap_size},
    {CHIR::IntrinsicKind::GET_THREAD_NUMBER, llvm::Intrinsic::code_get_thread_number},
    {CHIR::IntrinsicKind::GET_BLOCKING_THREAD_NUMBER, llvm::Intrinsic::code_get_blocking_thread_number},
    {CHIR::IntrinsicKind::GET_NATIVE_THREAD_NUMBER, llvm::Intrinsic::code_get_native_thread_number},
    {CHIR::IntrinsicKind::DUMP_CODE_HEAP_DATA, llvm::Intrinsic::code_dump_heap_data},
    {CHIR::IntrinsicKind::GET_GC_COUNT, llvm::Intrinsic::code_get_gc_count},
    {CHIR::IntrinsicKind::GET_GC_TIME_US, llvm::Intrinsic::code_get_gc_time_us},
    {CHIR::IntrinsicKind::GET_GC_FREED_SIZE, llvm::Intrinsic::code_get_gc_freed_size},
    {CHIR::IntrinsicKind::START_CODE_CPU_PROFILING, llvm::Intrinsic::code_start_cpu_profiling},
    {CHIR::IntrinsicKind::STOP_CODE_CPU_PROFILING, llvm::Intrinsic::code_stop_cpu_profiling},
    {CHIR::IntrinsicKind::INVOKE_GC, llvm::Intrinsic::code_invoke_gc},
    {CHIR::IntrinsicKind::SET_GC_THRESHOLD, llvm::Intrinsic::code_set_gc_threshold},
    {CHIR::IntrinsicKind::CROSS_ACCESS_BARRIER, llvm::Intrinsic::code_cross_access_barrier},
    {CHIR::IntrinsicKind::CREATE_EXPORT_HANDLE, llvm::Intrinsic::code_create_export_handle},
    {CHIR::IntrinsicKind::GET_EXPORTED_REF, llvm::Intrinsic::code_get_exported_ref},
    {CHIR::IntrinsicKind::REMOVE_EXPORTED_REF, llvm::Intrinsic::code_remove_exported_ref},
    // math
    {CHIR::IntrinsicKind::ABS, llvm::Intrinsic::abs}, {CHIR::IntrinsicKind::POW, llvm::Intrinsic::pow},
    {CHIR::IntrinsicKind::POWI, llvm::Intrinsic::powi}, {CHIR::IntrinsicKind::FABS, llvm::Intrinsic::fabs},
    {CHIR::IntrinsicKind::FLOOR, llvm::Intrinsic::floor}, {CHIR::IntrinsicKind::CEIL, llvm::Intrinsic ::ceil},
    {CHIR::IntrinsicKind::TRUNC, llvm::Intrinsic::trunc}, {CHIR::IntrinsicKind::SIN, llvm::Intrinsic::sin},
    {CHIR::IntrinsicKind::COS, llvm::Intrinsic::cos}, {CHIR::IntrinsicKind::EXP, llvm::Intrinsic::exp},
    {CHIR::IntrinsicKind::EXP2, llvm::Intrinsic::exp2}, {CHIR::IntrinsicKind::LOG, llvm::Intrinsic::log},
    {CHIR::IntrinsicKind::LOG2, llvm::Intrinsic::log2}, {CHIR::IntrinsicKind::LOG10, llvm::Intrinsic::log10},
    {CHIR::IntrinsicKind::SQRT, llvm::Intrinsic::sqrt}, {CHIR::IntrinsicKind::ROUND, llvm::Intrinsic::round},
    // atomic
    {CHIR::IntrinsicKind::ATOMIC_STORE, llvm::Intrinsic::code_atomic_store},
    {CHIR::IntrinsicKind::ATOMIC_COMPARE_AND_SWAP, llvm::Intrinsic::code_atomic_compare_swap},
    {CHIR::IntrinsicKind::ATOMIC_LOAD, llvm::Intrinsic::code_atomic_load},
    {CHIR::IntrinsicKind::ATOMIC_SWAP, llvm::Intrinsic::code_atomic_swap},
    {CHIR::IntrinsicKind::BLACK_BOX, llvm::Intrinsic::code_blackhole},
};
bool IsCHIRWrapper(const std::string& chirFuncName);
llvm::GlobalValue::LinkageTypes CHIRLinkage2LLVMLinkage(Linkage chirLinkage);
void AddLinkageTypeMetadata(
    llvm::GlobalObject& globalObject, llvm::GlobalValue::LinkageTypes linkageType, bool markByMD);
llvm::GlobalValue::LinkageTypes GetLinkageTypeOfGlobalObject(const llvm::GlobalObject& globalObject);
} // namespace CodeGen
} // namespace Codira

#endif // CODIRA_CGCOMMONDEF_H
