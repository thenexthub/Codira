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

#include "runtime/compiler.h"

#include "intrinsics.h"
#include "libarkfile/bytecode_instruction.h"
#include "libarkfile/type_helper.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/runtime_interface.h"
#include "runtime/cha.h"
#include "runtime/jit/profiling_data.h"
#include "runtime/include/class_linker-inl.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/field.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/coretypes/native_pointer.h"
#include "runtime/mem/heap_manager.h"
#include "runtime/mem/refstorage/reference.h"
#include "compiler/inplace_task_runner.h"
#include "compiler/background_task_runner.h"

namespace ark {

#ifdef PANDA_COMPILER_DEBUG_INFO
namespace compiler {
void CleanJitDebugCode();
}  // namespace compiler
#endif

#include <get_intrinsics.inl>

class ErrorHandler : public ClassLinkerErrorHandler {
    void OnError([[maybe_unused]] ClassLinker::Error error, [[maybe_unused]] const PandaString &message) override {}
};

bool Compiler::IsCompilationExpired(Method *method, bool isOsr)
{
    return (isOsr && GetOsrCode(method) != nullptr) || (!isOsr && method->HasCompiledCode());
}

/// Intrinsics fast paths are supported only for G1 GC.
bool PandaRuntimeInterface::IsGcValidForFastPath(SourceLanguage lang) const
{
    auto runtime = Runtime::GetCurrent();
    if (lang == SourceLanguage::INVALID) {
        ASSERT(ManagedThread::GetCurrent() != nullptr);
        lang = ManagedThread::GetCurrent()->GetThreadLang();
    }
    auto gcType = runtime->GetGCType(runtime->GetOptions(), lang);
    return gcType == mem::GCType::G1_GC;
}

uint32_t PandaRuntimeInterface::GetAOTBinaryFileSnapshotIndexForMethod(MethodPtr method) const
{
    const auto *pfile = const_cast<panda_file::File *>(MethodCast(method)->GetPandaFile());
    return panda_file::File::FILE_INDEX_BASE_OFFSET +
           Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->GetPandaFileSnapshotIndex(
               pfile->GetFullFileName());
}

compiler::RuntimeInterface::BinaryFilePtr PandaRuntimeInterface::GetAOTBinaryFileBySnapshotIndex(uint32_t index) const
{
    ASSERT(index != panda_file::File::INVALID_FILE_INDEX);
    index -= panda_file::File::FILE_INDEX_BASE_OFFSET;
    auto pfile = Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->GetPandaFileBySnapshotIndex(index);
    return const_cast<BinaryFilePtr>(reinterpret_cast<const void *>(pfile));
}

uint32_t PandaRuntimeInterface::GetAOTBinaryFileSnapshotIndex(BinaryFilePtr file) const
{
    return panda_file::File::FILE_INDEX_BASE_OFFSET +
           Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->GetPandaFileSnapshotIndex(
               reinterpret_cast<const panda_file::File *>(file)->GetFullFileName());
}

compiler::RuntimeInterface::MethodId PandaRuntimeInterface::ResolveMethodIndex(MethodPtr parentMethod,
                                                                               MethodIndex index) const
{
    return MethodCast(parentMethod)->GetClass()->ResolveMethodIndex(index).GetOffset();
}

compiler::RuntimeInterface::FieldId PandaRuntimeInterface::ResolveFieldIndex(MethodPtr parentMethod,
                                                                             FieldIndex index) const
{
    return MethodCast(parentMethod)->GetClass()->ResolveFieldIndex(index).GetOffset();
}

compiler::RuntimeInterface::IdType PandaRuntimeInterface::ResolveTypeIndex(MethodPtr parentMethod,
                                                                           TypeIndex index) const
{
    return MethodCast(parentMethod)->GetClass()->ResolveClassIndex(index).GetOffset();
}

compiler::RuntimeInterface::MethodPtr PandaRuntimeInterface::GetMethodById(MethodPtr parentMethod, MethodId id) const
{
    return GetMethod(parentMethod, id);
}

compiler::RuntimeInterface::MethodId PandaRuntimeInterface::GetMethodId(MethodPtr method) const
{
    return MethodCast(method)->GetFileId().GetOffset();
}

compiler::RuntimeInterface::IntrinsicId PandaRuntimeInterface::GetIntrinsicId(MethodPtr method) const
{
    return GetIntrinsicEntryPointId(MethodCast(method)->GetIntrinsic());
}

uint64_t PandaRuntimeInterface::GetUniqMethodId(MethodPtr method) const
{
    return MethodCast(method)->GetUniqId();
}

compiler::RuntimeInterface::MethodPtr PandaRuntimeInterface::ResolveVirtualMethod(ClassPtr cls, MethodPtr method) const
{
    ASSERT(method != nullptr);
    return ClassCast(cls)->ResolveVirtualMethod(MethodCast(method));
}

compiler::RuntimeInterface::MethodPtr PandaRuntimeInterface::ResolveInterfaceMethod(ClassPtr cls,
                                                                                    MethodPtr method) const
{
    ASSERT(method != nullptr);
    return ClassCast(cls)->ResolveVirtualMethod(MethodCast(method));
}

compiler::RuntimeInterface::IdType PandaRuntimeInterface::GetMethodReturnTypeId(MethodPtr method) const
{
    auto *pf = MethodCast(method)->GetPandaFile();
    panda_file::ProtoDataAccessor pda(*pf,
                                      panda_file::MethodDataAccessor::GetProtoId(*pf, MethodCast(method)->GetFileId()));
    return pda.GetReferenceType(0).GetOffset();
}

compiler::RuntimeInterface::IdType PandaRuntimeInterface::GetMethodArgReferenceTypeId(MethodPtr method,
                                                                                      uint16_t num) const
{
    auto *pf = MethodCast(method)->GetPandaFile();
    panda_file::ProtoDataAccessor pda(*pf,
                                      panda_file::MethodDataAccessor::GetProtoId(*pf, MethodCast(method)->GetFileId()));
    return pda.GetReferenceType(num).GetOffset();
}

compiler::RuntimeInterface::ClassPtr PandaRuntimeInterface::GetClass(MethodPtr method, IdType id) const
{
    auto *caller = MethodCast(method);
    Class *loadedClass = Runtime::GetCurrent()->GetClassLinker()->GetLoadedClass(
        *caller->GetPandaFile(), panda_file::File::EntityId(id), caller->GetClass()->GetLoadContext());
    if (LIKELY(loadedClass != nullptr)) {
        return loadedClass;
    }
    ErrorHandler handler;
    ScopedMutatorLock lock;
    return Runtime::GetCurrent()->GetClassLinker()->GetClass(*caller->GetPandaFile(), panda_file::File::EntityId(id),
                                                             caller->GetClass()->GetLoadContext(), &handler);
}

compiler::RuntimeInterface::ClassPtr PandaRuntimeInterface::GetStringClass(MethodPtr method, uint32_t *typeId) const
{
    auto *caller = MethodCast(method);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*caller);
    auto classPtr = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::STRING);
    if (typeId != nullptr) {
        *typeId = classPtr->GetFileId().GetOffset();
    }
    return classPtr;
}

compiler::RuntimeInterface::ClassPtr PandaRuntimeInterface::GetLineStringClass(MethodPtr method, uint32_t *typeId) const
{
    auto *caller = MethodCast(method);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*caller);
    auto classPtr = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::LINE_STRING);
    if (typeId != nullptr) {
        *typeId = classPtr->GetFileId().GetOffset();
    }
    return classPtr;
}

compiler::RuntimeInterface::ClassPtr PandaRuntimeInterface::GetNumberClass(MethodPtr method, const char *name,
                                                                           uint32_t *typeId) const
{
    ScopedMutatorLock lock;
    auto *caller = MethodCast(method);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*caller);
    const uint8_t *classDescriptor = utf::CStringAsMutf8(name);
    auto classLinker = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx);
    auto *classPtr = classLinker->GetClass(classDescriptor, false, classLinker->GetBootContext(), nullptr);
    ASSERT(classPtr != nullptr);
    *typeId = classPtr->GetFileId().GetOffset();
    return classPtr;
}

compiler::RuntimeInterface::ClassPtr PandaRuntimeInterface::GetArrayU16Class(MethodPtr method) const
{
    auto *caller = MethodCast(method);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*caller);
    return Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_U16);
}

compiler::RuntimeInterface::ClassPtr PandaRuntimeInterface::GetArrayU8Class(MethodPtr method) const
{
    ScopedMutatorLock lock;
    auto *caller = MethodCast(method);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*caller);
    return Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_U8);
}

compiler::ClassType PandaRuntimeInterface::GetClassType(ClassPtr klassPtr) const
{
    if (klassPtr == nullptr) {
        return compiler::ClassType::UNRESOLVED_CLASS;
    }
    auto klass = ClassCast(klassPtr);
    if (klass->IsObjectClass()) {
        return compiler::ClassType::OBJECT_CLASS;
    }
    if (klass->IsInterface()) {
        return compiler::ClassType::INTERFACE_CLASS;
    }
    if (klass->IsArrayClass()) {
        auto componentClass = klass->GetComponentType();
        ASSERT(componentClass != nullptr);
        if (componentClass->IsObjectClass()) {
            return compiler::ClassType::ARRAY_OBJECT_CLASS;
        }
        if (componentClass->IsPrimitive()) {
            return compiler::ClassType::FINAL_CLASS;
        }
        return compiler::ClassType::ARRAY_CLASS;
    }
    if (klass->IsUnionClass()) {
        return compiler::ClassType::UNION_CLASS;
    }
    if (klass->IsFinal()) {
        return compiler::ClassType::FINAL_CLASS;
    }
    return compiler::ClassType::OTHER_CLASS;
}

compiler::ClassType PandaRuntimeInterface::GetClassType(MethodPtr method, IdType id) const
{
    if (method == nullptr) {
        return compiler::ClassType::UNRESOLVED_CLASS;
    }
    return GetClassType(GetClass(method, id));
}

bool PandaRuntimeInterface::IsArrayClass(MethodPtr method, IdType id) const
{
    panda_file::File::EntityId cid(id);
    auto *pf = MethodCast(method)->GetPandaFile();
    return ClassHelper::IsArrayDescriptor(pf->GetStringData(cid).data);
}

bool PandaRuntimeInterface::IsStringClass(MethodPtr method, IdType id) const
{
    auto cls = GetClass(method, id);
    if (cls == nullptr) {
        return false;
    }
    return ClassCast(cls)->IsStringClass();
}

bool PandaRuntimeInterface::IsStringClass(ClassPtr klass) const
{
    if (klass == nullptr) {
        return false;
    }
    return ClassCast(klass)->IsStringClass();
}

compiler::RuntimeInterface::ClassPtr PandaRuntimeInterface::GetArrayElementClass(ClassPtr cls) const
{
    ASSERT(ClassCast(cls)->IsArrayClass());
    return ClassCast(cls)->GetComponentType();
}

bool PandaRuntimeInterface::CheckStoreArray(ClassPtr arrayCls, ClassPtr strCls) const
{
    ASSERT(arrayCls != nullptr);
    auto *elementClass = ClassCast(arrayCls)->GetComponentType();
    if (elementClass == nullptr) {
        return false;
    }
    if (strCls == nullptr) {
        return elementClass->IsObjectClass();
    }
    ASSERT(strCls != nullptr);
    return elementClass->IsAssignableFrom(ClassCast(strCls));
}

bool PandaRuntimeInterface::IsAssignableFrom(ClassPtr cls1, ClassPtr cls2) const
{
    ASSERT(cls1 != nullptr);
    ASSERT(cls2 != nullptr);
    return ClassCast(cls1)->IsAssignableFrom(ClassCast(cls2));
}

bool PandaRuntimeInterface::IsInterfaceMethod(MethodPtr parentMethod, MethodId id) const
{
    ErrorHandler handler;
    auto *method = GetMethod(parentMethod, id);
    ASSERT(method != nullptr);
    return (method->GetClass()->IsInterface() && !method->IsDefaultInterfaceMethod());
}

bool PandaRuntimeInterface::IsMethodNativeApi(MethodPtr method) const
{
    auto *pandaMethod = MethodCast(method);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*pandaMethod);
    return Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->IsMethodNativeApi(pandaMethod);
}

bool PandaRuntimeInterface::CanThrowException(MethodPtr method) const
{
    auto *pandaMethod = MethodCast(method);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*pandaMethod);
    return Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->CanThrowException(pandaMethod);
}

bool PandaRuntimeInterface::IsNecessarySwitchThreadState(MethodPtr method) const
{
    auto *pandaMethod = MethodCast(method);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*pandaMethod);
    return Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->IsNecessarySwitchThreadState(pandaMethod);
}

uint8_t PandaRuntimeInterface::GetStackReferenceMask() const
{
    return static_cast<uint8_t>(mem::Reference::ObjectType::STACK);
}

bool PandaRuntimeInterface::CanNativeMethodUseObjects(MethodPtr method) const
{
    auto *pandaMethod = MethodCast(method);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*pandaMethod);
    return Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->CanNativeMethodUseObjects(pandaMethod);
}

uint32_t PandaRuntimeInterface::FindCatchBlock(MethodPtr m, ClassPtr cls, uint32_t pc) const
{
    ScopedMutatorLock lock;
    return MethodCast(m)->FindCatchBlockInPandaFile(ClassCast(cls), pc);
}

bool PandaRuntimeInterface::IsInterfaceMethod(MethodPtr method) const
{
    return (MethodCast(method)->GetClass()->IsInterface() && !MethodCast(method)->IsDefaultInterfaceMethod());
}

bool PandaRuntimeInterface::HasNativeException(MethodPtr method) const
{
    if (!MethodCast(method)->IsNative()) {
        return false;
    }
    return CanThrowException(method);
}

bool PandaRuntimeInterface::IsMethodExternal(MethodPtr parentMethod, MethodPtr calleeMethod) const
{
    if (calleeMethod == nullptr) {
        return true;
    }
    return MethodCast(parentMethod)->GetPandaFile() != MethodCast(calleeMethod)->GetPandaFile();
}

bool PandaRuntimeInterface::IsClassExternal(MethodPtr method, ClassPtr calleeClass) const
{
    if (calleeClass == nullptr) {
        return true;
    }
    return MethodCast(method)->GetPandaFile() != ClassCast(calleeClass)->GetPandaFile();
}

compiler::DataType::Type PandaRuntimeInterface::GetMethodReturnType(MethodPtr parentMethod, MethodId id) const
{
    auto *pf = MethodCast(parentMethod)->GetPandaFile();
    panda_file::MethodDataAccessor mda(*pf, panda_file::File::EntityId(id));
    panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
    return ToCompilerType(panda_file::GetEffectiveType(pda.GetReturnType()));
}

compiler::DataType::Type PandaRuntimeInterface::GetMethodArgumentType(MethodPtr parentMethod, MethodId id,
                                                                      size_t index) const
{
    auto *pf = MethodCast(parentMethod)->GetPandaFile();
    panda_file::MethodDataAccessor mda(*pf, panda_file::File::EntityId(id));
    panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
    return ToCompilerType(panda_file::GetEffectiveType(pda.GetArgType(index)));
}

size_t PandaRuntimeInterface::GetMethodArgumentsCount(MethodPtr parentMethod, MethodId id) const
{
    auto *pf = MethodCast(parentMethod)->GetPandaFile();
    panda_file::MethodDataAccessor mda(*pf, panda_file::File::EntityId(id));
    panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
    return pda.GetNumArgs();
}

bool PandaRuntimeInterface::IsMethodStatic(MethodPtr parentMethod, MethodId id) const
{
    auto *pf = MethodCast(parentMethod)->GetPandaFile();
    panda_file::MethodDataAccessor mda(*pf, panda_file::File::EntityId(id));
    return mda.IsStatic();
}

bool PandaRuntimeInterface::IsMethodStatic(MethodPtr method) const
{
    return MethodCast(method)->IsStatic();
}

bool PandaRuntimeInterface::IsMethodStaticConstructor(MethodPtr method) const
{
    return MethodCast(method)->IsStaticConstructor();
}

bool PandaRuntimeInterface::IsMemoryBarrierRequired(MethodPtr method) const
{
    if (!MethodCast(method)->IsInstanceConstructor()) {
        return false;
    }
    for (auto &field : MethodCast(method)->GetClass()->GetFields()) {
        // We insert memory barrier after call to constructor to ensure writes
        // to final fields will be visible after constructor finishes
        // Static fields are initialized in runtime entrypoints like InitializeClass,
        // so barrier is not needed here if they are final
        if (field.IsFinal() && !field.IsStatic()) {
            return true;
        }
    }
    return false;
}

PandaRuntimeInterface::MethodPtr PandaRuntimeInterface::GetMethodAsIntrinsic(MethodPtr parentMethod, MethodId id) const
{
    Method *caller = MethodCast(parentMethod);
    auto *pf = caller->GetPandaFile();
    panda_file::MethodDataAccessor mda(*pf, panda_file::File::EntityId(id));

    auto *className = pf->GetStringData(mda.GetClassId()).data;
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();

    // Support intrinsics only from boot context
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*caller);
    auto *klass = classLinker->FindLoadedClass(className, classLinker->GetExtension(ctx)->GetBootContext());

    // Class should be loaded during intrinsics initialization
    if (klass == nullptr) {
        return nullptr;
    }

    auto name = pf->GetStringData(mda.GetNameId());
    bool isArrayClone = ClassHelper::IsArrayDescriptor(className) &&
                        (utf::CompareMUtf8ToMUtf8(name.data, utf::CStringAsMutf8("clone")) == 0);
    Method::Proto proto(*pf, mda.GetProtoId());
    auto *method = klass->GetDirectMethod(name.data, proto);
    if (method == nullptr) {
        if (isArrayClone) {
            method = klass->GetClassMethod(name.data, proto);
        } else {
            return nullptr;
        }
    }

    return IsMethodIntrinsic(method) ? method : nullptr;
}

std::string PandaRuntimeInterface::GetBytecodeString(MethodPtr method, uintptr_t pc) const
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    BytecodeInstruction inst(MethodCast(method)->GetInstructions() + pc);
    std::stringstream ss;
    ss << inst;
    return ss.str();
}

PandaRuntimeInterface::FieldPtr PandaRuntimeInterface::ResolveField(PandaRuntimeInterface::MethodPtr m, size_t id,
                                                                    bool isStatic, bool allowExternal,
                                                                    uint32_t *pclassId)
{
    auto method = MethodCast(m);
    auto pfile = method->GetPandaFile();
    auto *field = GetField(method, id, isStatic);
    if (field == nullptr) {
        return nullptr;
    }
    auto klass = field->GetClass();
    if (pfile == field->GetPandaFile() || allowExternal) {
        if (pclassId != nullptr) {
            *pclassId = klass->GetFileId().GetOffset();
        }
        return field;
    }

    auto classId = GetClassIdWithinFile(m, klass);
    if (classId != 0) {
        if (pclassId != nullptr) {
            *pclassId = classId;
        }
        return field;
    }
    return nullptr;
}

template <typename T>
void FillLiteralArrayData(const panda_file::File *pfile, pandasm::LiteralArray *litArray,
                          const panda_file::LiteralTag &tag, const panda_file::LiteralDataAccessor::LiteralValue &value)
{
    panda_file::File::EntityId id(std::get<uint32_t>(value));
    auto sp = pfile->GetSpanFromId(id);
    auto len = panda_file::helpers::Read<sizeof(uint32_t)>(&sp);

    for (size_t i = 0; i < len; i++) {
        pandasm::LiteralArray::Literal lit;
        lit.tag = tag;
        lit.value = bit_cast<T>(panda_file::helpers::Read<sizeof(T)>(&sp));
        litArray->literals.push_back(lit);
    }
}

static void FillLiteralArrayTyped(const panda_file::File *pfile, pandasm::LiteralArray &litArray,
                                  const panda_file::LiteralTag &tag,
                                  const panda_file::LiteralDataAccessor::LiteralValue &value)
{
    switch (tag) {
        case panda_file::LiteralTag::ARRAY_U1: {
            FillLiteralArrayData<bool>(pfile, &litArray, tag, value);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I8:
        case panda_file::LiteralTag::ARRAY_U8: {
            FillLiteralArrayData<uint8_t>(pfile, &litArray, tag, value);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I16:
        case panda_file::LiteralTag::ARRAY_U16: {
            FillLiteralArrayData<uint16_t>(pfile, &litArray, tag, value);
            break;
        }
        // in the case of ARRAY_STRING, the array stores strings ids
        case panda_file::LiteralTag::ARRAY_STRING:
        case panda_file::LiteralTag::ARRAY_I32:
        case panda_file::LiteralTag::ARRAY_U32: {
            FillLiteralArrayData<uint32_t>(pfile, &litArray, tag, value);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I64:
        case panda_file::LiteralTag::ARRAY_U64: {
            FillLiteralArrayData<uint64_t>(pfile, &litArray, tag, value);
            break;
        }
        case panda_file::LiteralTag::ARRAY_F32: {
            FillLiteralArrayData<float>(pfile, &litArray, tag, value);
            break;
        }
        case panda_file::LiteralTag::ARRAY_F64: {
            FillLiteralArrayData<double>(pfile, &litArray, tag, value);
            break;
        }
        case panda_file::LiteralTag::TAGVALUE:
        case panda_file::LiteralTag::ACCESSOR:
        case panda_file::LiteralTag::NULLVALUE: {
            break;
        }
        default: {
            UNREACHABLE();
            break;
        }
    }
}

ark::pandasm::LiteralArray PandaRuntimeInterface::GetLiteralArray(MethodPtr m, LiteralArrayId id) const
{
    auto method = MethodCast(m);
    auto pfile = method->GetPandaFile();
    id = pfile->GetLiteralArrays()[id];
    pandasm::LiteralArray litArray;

    panda_file::LiteralDataAccessor litArrayAccessor(*pfile, pfile->GetLiteralArraysId());

    litArrayAccessor.EnumerateLiteralVals(
        panda_file::File::EntityId(id),
        [&litArray, pfile](const panda_file::LiteralDataAccessor::LiteralValue &value,
                           const panda_file::LiteralTag &tag) { FillLiteralArrayTyped(pfile, litArray, tag, value); });

    return litArray;
}

std::optional<RuntimeInterface::IdType> PandaRuntimeInterface::FindClassIdInFile(MethodPtr method, ClassPtr cls) const
{
    auto klass = ClassCast(cls);
    auto pfile = MethodCast(method)->GetPandaFile();
    auto className = klass->GetName();
    PandaString storage;
    auto classId = pfile->GetClassId(ClassHelper::GetDescriptor(utf::CStringAsMutf8(className.c_str()), &storage));
    if (classId.IsValid() && className == ClassHelper::GetName(pfile->GetStringData(classId).data)) {
        return std::optional<RuntimeInterface::IdType>(classId.GetOffset());
    }
    return std::nullopt;
}

RuntimeInterface::IdType PandaRuntimeInterface::GetClassIdWithinFile(MethodPtr method, ClassPtr cls) const
{
    auto classId = FindClassIdInFile(method, cls);
    return classId ? classId.value() : 0;
}

RuntimeInterface::IdType PandaRuntimeInterface::GetLiteralArrayClassIdWithinFile(
    PandaRuntimeInterface::MethodPtr method, panda_file::LiteralTag tag) const
{
    ErrorHandler handler;
    auto ctx = Runtime::GetCurrent()->GetLanguageContext(*MethodCast(method));
    auto cls = Runtime::GetCurrent()->GetClassRootForLiteralTag(
        *Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx), tag);

    auto pfile = MethodCast(method)->GetPandaFile();
    auto className = cls->GetName();
    for (decltype(auto) classRawId : pfile->GetClasses()) {
        auto classId = panda_file::File::EntityId(classRawId);
        if (classId.IsValid() && className == ClassHelper::GetName(pfile->GetStringData(classId).data)) {
            return classId.GetOffset();
        }
    }
    UNREACHABLE();
}

bool PandaRuntimeInterface::CanUseTlabForClass(ClassPtr klass) const
{
    auto cls = ClassCast(klass);
    return !Thread::GetCurrent()->GetVM()->GetHeapManager()->IsObjectFinalized(cls) && !cls->IsVariableSize() &&
           cls->IsInstantiable();
}

size_t PandaRuntimeInterface::GetTLABMaxSize() const
{
    return Thread::GetCurrent()->GetVM()->GetHeapManager()->GetTLABMaxAllocSize();
}

bool PandaRuntimeInterface::CanScalarReplaceObject(ClassPtr klass) const
{
    auto cls = ClassCast(klass);
    auto ctx = Runtime::GetCurrent()->GetLanguageContext(*cls);
    return Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->CanScalarReplaceObject(cls);
}

Method *PandaRuntimeInterface::GetMethod(MethodPtr caller, RuntimeInterface::IdType id) const
{
    auto *methodFromCache =
        MethodCast(caller)->GetPandaFile()->GetPandaCache()->GetMethodFromCache(panda_file::File::EntityId(id));
    if (LIKELY(methodFromCache != nullptr)) {
        return methodFromCache;
    }
    ErrorHandler errorHandler;
    ScopedMutatorLock lock;
    return Runtime::GetCurrent()->GetClassLinker()->GetMethod(*MethodCast(caller), panda_file::File::EntityId(id),
                                                              &errorHandler);
}

Field *PandaRuntimeInterface::GetField(MethodPtr method, RuntimeInterface::IdType id, bool isStatic) const
{
    auto *field =
        MethodCast(method)->GetPandaFile()->GetPandaCache()->GetFieldFromCache(panda_file::File::EntityId(id));
    if (LIKELY(field != nullptr)) {
        return field;
    }
    ErrorHandler errorHandler;
    ScopedMutatorLock lock;
    return Runtime::GetCurrent()->GetClassLinker()->GetField(*MethodCast(method), panda_file::File::EntityId(id),
                                                             isStatic, &errorHandler);
}

PandaRuntimeInterface::ClassPtr PandaRuntimeInterface::ResolveType(PandaRuntimeInterface::MethodPtr method,
                                                                   size_t id) const
{
    return GetClass(method, id);
}

bool PandaRuntimeInterface::IsClassInitialized(uintptr_t klass) const
{
    return TypeCast(klass)->IsInitialized();
}

uintptr_t PandaRuntimeInterface::GetManagedType(uintptr_t klass) const
{
    return reinterpret_cast<uintptr_t>(TypeCast(klass)->GetManagedObject());
}

compiler::DataType::Type PandaRuntimeInterface::GetFieldType(FieldPtr field) const
{
    return ToCompilerType(FieldCast(field)->GetType());
}

compiler::DataType::Type PandaRuntimeInterface::GetArrayComponentType(ClassPtr klass) const
{
    return ToCompilerType(ClassCast(klass)->GetComponentType()->GetType());
}

compiler::DataType::Type PandaRuntimeInterface::GetFieldTypeById(MethodPtr parentMethod, IdType id) const
{
    auto *pf = MethodCast(parentMethod)->GetPandaFile();
    panda_file::FieldDataAccessor fda(*pf, panda_file::File::EntityId(id));
    return ToCompilerType(panda_file::Type::GetTypeFromFieldEncoding(fda.GetType()));
}

compiler::RuntimeInterface::IdType PandaRuntimeInterface::GetFieldValueTypeId(MethodPtr method, IdType id) const
{
    auto *pf = MethodCast(method)->GetPandaFile();
    auto typeId = panda_file::FieldDataAccessor::GetTypeId(*pf, panda_file::File::EntityId(id));
    return typeId.GetOffset();
}

RuntimeInterface::ClassPtr PandaRuntimeInterface::GetClassForField(FieldPtr field) const
{
    return FieldCast(field)->GetClass();
}

uint32_t PandaRuntimeInterface::GetArrayElementSize(MethodPtr method, IdType id) const
{
    auto *pf = MethodCast(method)->GetPandaFile();
    auto *descriptor = pf->GetStringData(panda_file::File::EntityId(id)).data;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ASSERT(descriptor[0] == '[');
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return Class::GetTypeSize(panda_file::Type::GetTypeIdBySignature(static_cast<char>(descriptor[1])));
}

uint32_t PandaRuntimeInterface::GetMaxArrayLength(ClassPtr klass) const
{
    if (ClassCast(klass)->IsArrayClass()) {
        return INT32_MAX / ClassCast(klass)->GetComponentSize();
    }
    return INT32_MAX;
}

uintptr_t PandaRuntimeInterface::GetPointerToConstArrayData(MethodPtr method, IdType id) const
{
    auto *pf = MethodCast(method)->GetPandaFile();
    return Runtime::GetCurrent()->GetPointerToConstArrayData(*pf, pf->GetLiteralArrays()[id]);
}

size_t PandaRuntimeInterface::GetOffsetToConstArrayData(MethodPtr method, IdType id) const
{
    auto *pf = MethodCast(method)->GetPandaFile();
    auto offset = Runtime::GetCurrent()->GetPointerToConstArrayData(*pf, pf->GetLiteralArrays()[id]) -
                  reinterpret_cast<uintptr_t>(pf->GetBase());
    return static_cast<size_t>(offset);
}

size_t PandaRuntimeInterface::GetFieldOffset(FieldPtr field) const
{
    if (!HasFieldMetadata(field)) {
        return reinterpret_cast<uintptr_t>(field) >> 1U;
    }
    return FieldCast(field)->GetOffset();
}

RuntimeInterface::FieldPtr PandaRuntimeInterface::GetFieldByOffset(size_t offset) const
{
    ASSERT(MinimumBitsToStore(offset) < std::numeric_limits<uintptr_t>::digits);
    return reinterpret_cast<FieldPtr>((offset << 1U) | 1U);
}

uintptr_t PandaRuntimeInterface::GetFieldClass(FieldPtr field) const
{
    return reinterpret_cast<uintptr_t>(FieldCast(field)->GetClass());
}

bool PandaRuntimeInterface::IsFieldVolatile(FieldPtr field) const
{
    return FieldCast(field)->IsVolatile();
}

bool PandaRuntimeInterface::IsFieldFinal(FieldPtr field) const
{
    return FieldCast(field)->IsFinal();
}

bool PandaRuntimeInterface::IsFieldReadonly(FieldPtr field) const
{
    return FieldCast(field)->IsReadonly();
}

bool PandaRuntimeInterface::HasFieldMetadata(FieldPtr field) const
{
    return (reinterpret_cast<uintptr_t>(field) & 1U) == 0;
}

double PandaRuntimeInterface::GetStaticFieldFloatValue(FieldPtr fieldPtr) const
{
    auto *field = FieldCast(fieldPtr);
    auto type = GetFieldType(fieldPtr);
    auto klass = field->GetClass();
    ASSERT(compiler::DataType::IsFloatType(type));
    switch (compiler::DataType::ShiftByType(type, Arch::NONE)) {
        case 2U:
            return klass->GetFieldPrimitive<float>(*field);
        case 3U:
            return klass->GetFieldPrimitive<double>(*field);
        default:
            UNREACHABLE();
    }
}

uint64_t PandaRuntimeInterface::GetStaticFieldIntegerValue(FieldPtr fieldPtr) const
{
    auto *field = FieldCast(fieldPtr);
    auto type = GetFieldType(fieldPtr);
    auto klass = field->GetClass();
    ASSERT(compiler::DataType::GetCommonType(type) == compiler::DataType::INT64);
    // NB: must be sign-extended for signed types at call-site
    switch (compiler::DataType::ShiftByType(type, Arch::NONE)) {
        case 0U:
            return klass->GetFieldPrimitive<uint8_t>(*field);
        case 1U:
            return klass->GetFieldPrimitive<uint16_t>(*field);
        case 2U:
            return klass->GetFieldPrimitive<uint32_t>(*field);
        case 3U:
            return klass->GetFieldPrimitive<uint64_t>(*field);
        default:
            UNREACHABLE();
    }
}

RuntimeInterface::FieldId PandaRuntimeInterface::GetFieldId(FieldPtr field) const
{
    return FieldCast(field)->GetFileId().GetOffset();
}

ark::mem::BarrierType PandaRuntimeInterface::GetPreReadType() const
{
    return Thread::GetCurrent()->GetBarrierSet()->GetPreReadType();
}

ark::mem::BarrierType PandaRuntimeInterface::GetPreType() const
{
    return Thread::GetCurrent()->GetBarrierSet()->GetPreType();
}

ark::mem::BarrierType PandaRuntimeInterface::GetPostType() const
{
    return Thread::GetCurrent()->GetBarrierSet()->GetPostType();
}

ark::mem::BarrierOperand PandaRuntimeInterface::GetBarrierOperand(ark::mem::BarrierPosition barrierPosition,
                                                                  std::string_view operandName) const
{
    return Thread::GetCurrent()->GetBarrierSet()->GetBarrierOperand(barrierPosition, operandName);
}

uint32_t PandaRuntimeInterface::GetFunctionTargetOffset([[maybe_unused]] Arch arch) const
{
    // NOTE(wengchangcheng): return offset of method in JSFunction
    return 0;
}

uint32_t PandaRuntimeInterface::GetNativePointerTargetOffset(Arch arch) const
{
    return cross_values::GetCoretypesNativePointerExternalPointerOffset(arch);
}

void ClassHierarchyAnalysisWrapper::AddDependency(PandaRuntimeInterface::MethodPtr callee,
                                                  RuntimeInterface::MethodPtr caller)
{
    Runtime::GetCurrent()->GetCha()->AddDependency(MethodCast(callee), MethodCast(caller));
}

/// With 'no-async-jit' compilation inside of c2i bridge can forced and it can trigger GC
bool PandaRuntimeInterface::HasSafepointDuringCall() const
{
#ifdef PANDA_PRODUCT_BUILD
    return false;
#else
    if (Runtime::GetOptions().IsArkAot()) {
        return false;
    }
    return Runtime::GetOptions().IsNoAsyncJit();
#endif
}

RuntimeInterface::ThreadPtr PandaRuntimeInterface::CreateCompilerThread()
{
    ASSERT(Thread::GetCurrent() != nullptr);
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    return allocator->New<Thread>(PandaVM::GetCurrent(), Thread::ThreadType::THREAD_TYPE_COMPILER);
}

void PandaRuntimeInterface::DestroyCompilerThread(ThreadPtr thread)
{
    ASSERT(thread != nullptr);
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    allocator->Delete(static_cast<Thread *>(thread));
}

InlineCachesWrapper::CallKind InlineCachesWrapper::GetClasses(PandaRuntimeInterface::MethodPtr m, uintptr_t pc,
                                                              ArenaVector<RuntimeInterface::ClassPtr> *classes)
{
    ASSERT(classes != nullptr);
    classes->clear();
    auto method = static_cast<Method *>(m);
    auto profilingData = method->GetProfilingData();
    if (profilingData == nullptr) {
        return CallKind::UNKNOWN;
    }
    auto ic = profilingData->FindInlineCache(pc);
    if (ic == nullptr) {
        return CallKind::UNKNOWN;
    }
    auto icClasses = ic->GetClassesCopy();
    classes->insert(classes->end(), icClasses.begin(), icClasses.end());
    if (classes->empty()) {
        return CallKind::UNKNOWN;
    }
    if (classes->size() == 1) {
        return CallKind::MONOMORPHIC;
    }
    if (CallSiteInlineCache::IsMegamorphic(reinterpret_cast<Class *>((*classes)[0]))) {
        return CallKind::MEGAMORPHIC;
    }
    return CallKind::POLYMORPHIC;
}

bool UnresolvedTypesWrapper::AddTableSlot(RuntimeInterface::MethodPtr method, uint32_t typeId, SlotKind kind)
{
    std::pair<uint32_t, UnresolvedTypesInterface::SlotKind> key {typeId, kind};
    if (slots_.find(method) == slots_.end()) {
        slots_[method][key] = 0;
        return true;
    }
    auto &table = slots_.at(method);
    if (table.find(key) == table.end()) {
        table[key] = 0;
        return true;
    }
    return false;
}

uintptr_t UnresolvedTypesWrapper::GetTableSlot(RuntimeInterface::MethodPtr method, uint32_t typeId, SlotKind kind) const
{
    ASSERT(slots_.find(method) != slots_.end());
    auto &table = slots_.at(method);
    ASSERT(table.find({typeId, kind}) != table.end());
    return reinterpret_cast<uintptr_t>(&table.at({typeId, kind}));
}

bool Compiler::CompileMethod(Method *method, uintptr_t bytecodeOffset, bool osr, TaggedValue func)
{
    if (method->IsAbstract()) {
        return false;
    }

    if (osr && GetOsrCode(method) != nullptr) {
        ASSERT(method == ManagedThread::GetCurrent()->GetCurrentFrame()->GetMethod());
        ASSERT(method->HasCompiledCode());
        return OsrEntry(bytecodeOffset, GetOsrCode(method));
    }
    // In case if some thread raise compilation when another already compiled it, we just exit.
    if (method->HasCompiledCode() && !osr) {
        return false;
    }
    bool ctxOsr = method->HasCompiledCode() ? osr : false;
    if (method->AtomicSetCompilationStatus(ctxOsr ? Method::COMPILED : Method::NOT_COMPILED, Method::WAITING)) {
        ASSERT(ManagedThread::GetCurrent() != nullptr);
        CompilerTask ctx {method, ctxOsr, ManagedThread::GetCurrent()->GetVM()};
        AddTask(std::move(ctx), func);
    }
    if (noAsyncJit_) {
        // Change thread status to release mutator lock, in case JIT thread needs to invalidate compiled code
        ScopedChangeThreadStatus s(ManagedThread::GetCurrent(), ThreadStatus::IS_COMPILER_WAITING);

        auto status = method->GetCompilationStatus();
        for (; (status == Method::WAITING) || (status == Method::COMPILATION);
             status = method->GetCompilationStatus()) {
            if (compilerWorker_ == nullptr || compilerWorker_->IsWorkerJoined()) {
                // JIT thread is destroyed, wait makes no sence
                return false;
            }
            auto thread = MTManagedThread::GetCurrent();
            // NOTE(asoldatov): Remove this workaround for invoking compiler from ECMA VM
            // Issue: #20680
            if (thread != nullptr) {
                static constexpr uint64_t SLEEP_MS = 10;
                thread->TimedWait(ThreadStatus::IS_COMPILER_WAITING, SLEEP_MS, 0);
            }
        }
    }
    return false;
}

template <compiler::TaskRunnerMode RUNNER_MODE>
void Compiler::CompileMethodLocked(compiler::CompilerTaskRunner<RUNNER_MODE> taskRunner)
{
    os::memory::LockHolder lock(compilationLock_);
    StartCompileMethod<RUNNER_MODE>(std::move(taskRunner));
}

template <compiler::TaskRunnerMode RUNNER_MODE>
void Compiler::StartCompileMethod(compiler::CompilerTaskRunner<RUNNER_MODE> taskRunner)
{
    ASSERT(runtimeIface_ != nullptr);
    auto &taskCtx = taskRunner.GetContext();
    auto *method = taskCtx.GetMethod();

    method->ResetHotnessCounter();

    if (IsCompilationExpired(method, taskCtx.IsOsr())) {
        ASSERT(!noAsyncJit_);
        compiler::CompilerTaskRunner<RUNNER_MODE>::EndTask(std::move(taskRunner), false);
        return;
    }

    mem::MemStatsType *memStats = taskCtx.GetVM()->GetMemStats();

    auto allocator = std::make_unique<ark::ArenaAllocator>(ark::SpaceType::SPACE_TYPE_COMPILER, memStats);
    auto localAllocator = std::make_unique<ark::ArenaAllocator>(ark::SpaceType::SPACE_TYPE_COMPILER, memStats, true);

    if constexpr (RUNNER_MODE == compiler::BACKGROUND_MODE) {
        taskCtx.SetAllocator(std::move(allocator));
        taskCtx.SetLocalAllocator(std::move(localAllocator));
    } else {
        taskCtx.SetAllocator(allocator.get());
        taskCtx.SetLocalAllocator(localAllocator.get());
    }

    taskRunner.AddFinalize([](compiler::CompilerContext<RUNNER_MODE> &compilerCtx) {
        auto *compiledMethod = compilerCtx.GetMethod();
        auto isCompiled = compilerCtx.GetCompilationStatus();
        if (isCompiled) {
            // Check that method was not deoptimized
            compiledMethod->AtomicSetCompilationStatus(Method::COMPILATION, Method::COMPILED);
            return;
        }
        // If deoptimization occurred during OSR compilation, the compilation returns false.
        // For the case we need reset compiation status
        if (compilerCtx.IsOsr()) {
            compiledMethod->SetCompilationStatus(Method::NOT_COMPILED);
            return;
        }
        // Failure during compilation, should we retry later?
        compiledMethod->SetCompilationStatus(Method::FAILED);
    });

    compiler::JITCompileMethod<RUNNER_MODE>(runtimeIface_, codeAllocator_, &gdbDebugInfoAllocator_, jitStats_,
                                            std::move(taskRunner));
}

void Compiler::JoinWorker()
{
    if (compilerWorker_ != nullptr) {
        compilerWorker_->JoinWorker();
    }
#ifdef PANDA_COMPILER_DEBUG_INFO
    if (!Runtime::GetOptions().IsArkAot() && compiler::g_options.IsCompilerEmitDebugInfo()) {
        compiler::CleanJitDebugCode();
    }
#endif
}

ObjectPointerType PandaRuntimeInterface::GetNonMovableString(MethodPtr method, StringId id) const
{
    auto vm = Runtime::GetCurrent()->GetPandaVM();
    auto pf = MethodCast(method)->GetPandaFile();
    return ToObjPtrType(vm->GetNonMovableString(*pf, panda_file::File::EntityId {id}));
}

bool PandaRuntimeInterface::CanUseStringFlatCheck() const
{
    if (!IsUseAllStrings()) {
        // No need StringFlatCheck since TreeString is disabled
        return false;
    }

    auto ctx = Runtime::GetCurrent()->GetPandaVM()->GetLanguageContext();
    // If StringFlatCheck is inserted it replaces all usages.
    // It can break code in case of reference comparison.
    // So value equality semantic is required for operator '=='
    return ctx.HasValueEqualitySemantic();
}

bool PandaRuntimeInterface::IsUseAllStrings() const
{
    return Runtime::GetOptions().IsUseAllStrings();
}

#ifndef PANDA_PRODUCT_BUILD
// Exists only for tests purposes, must not be used in release builds
uint8_t CompileMethodImpl(coretypes::String *fullMethodName, SourceLanguage sourceLang)
{
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(ManagedThread::GetCurrent());
    VMHandle<coretypes::String> handle(ManagedThread::GetCurrent(), fullMethodName);
    ClassLinkerContext *ctx = nullptr;
    auto walker = StackWalker::Create(ManagedThread::GetCurrent());
    if (LIKELY(walker.HasFrame())) {
        ctx = walker.GetMethod()->GetClass()->GetLoadContext();
    } else {
        ClassLinkerExtension *ext = Runtime::GetCurrent()->GetClassLinker()->GetExtension(sourceLang);
        ctx = ext->GetBootContext();
    }
    ASSERT(ctx->GetSourceLang() == sourceLang);
    return CompileMethodImpl(handle.GetPtr(), ctx);
}

uint8_t CompileMethodImpl(coretypes::String *fullMethodName, ClassLinkerContext *ctx)
{
    auto name = ConvertToString(fullMethodName);
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();

    size_t pos = name.find_last_of("::");
    if (pos == std::string_view::npos) {
        return 1;
    }
    auto className = PandaString(name.substr(0, pos - 1));
    auto methodName = PandaString(name.substr(pos + 1));

    PandaString descriptor;
    auto classNameBytes = ClassHelper::GetDescriptor(utf::CStringAsMutf8(className.c_str()), &descriptor);
    auto methodNameBytes = utf::CStringAsMutf8(methodName.c_str());

    Class *cls = classLinker->GetClass(classNameBytes, true, ctx);
    if (cls == nullptr) {
        static constexpr uint8_t CLASS_IS_NULL = 2;
        return CLASS_IS_NULL;
    }

    auto method = cls->GetDirectMethod(methodNameBytes);
    if (method == nullptr) {
        static constexpr uint8_t METHOD_IS_NULL = 3;
        return METHOD_IS_NULL;
    }

    if (method->IsAbstract()) {
        static constexpr uint8_t ABSTRACT_ERROR = 4;
        return ABSTRACT_ERROR;
    }
    if (method->HasCompiledCode()) {
        return 0;
    }
    auto *compiler = Runtime::GetCurrent()->GetPandaVM()->GetCompiler();
    auto status = method->GetCompilationStatus();
    for (; (status != Method::COMPILED) && (status != Method::FAILED); status = method->GetCompilationStatus()) {
        if (status == Method::NOT_COMPILED) {
            ASSERT(!method->HasCompiledCode());
            compiler->CompileMethod(method, 0, false, TaggedValue::Hole());
        }
        // NOTE(asoldatov): Remove this workaround for invoking compiler from ECMA VM
        // Issue: #20680
        auto thread = MTManagedThread::GetCurrent();
        if (thread != nullptr) {
            static constexpr uint64_t SLEEP_MS = 10;
            thread->TimedWait(ThreadStatus::IS_COMPILER_WAITING, SLEEP_MS, 0);
        }
    }
    static constexpr uint8_t COMPILATION_FAILED = 5;
    return (status == Method::COMPILED ? 0 : COMPILATION_FAILED);
}
#endif  // PANDA_PRODUCT_BUILD

template void Compiler::CompileMethodLocked<compiler::BACKGROUND_MODE>(
    compiler::CompilerTaskRunner<compiler::BACKGROUND_MODE>);
template void Compiler::CompileMethodLocked<compiler::INPLACE_MODE>(
    compiler::CompilerTaskRunner<compiler::INPLACE_MODE>);
template void Compiler::StartCompileMethod<compiler::BACKGROUND_MODE>(
    compiler::CompilerTaskRunner<compiler::BACKGROUND_MODE>);
template void Compiler::StartCompileMethod<compiler::INPLACE_MODE>(
    compiler::CompilerTaskRunner<compiler::INPLACE_MODE>);

}  // namespace ark
