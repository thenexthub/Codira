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

#include "runtime/include/class_linker.h"
#include "runtime/bridge/bridge.h"
#include "runtime/cha.h"
#include "runtime/class_initializer.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/field.h"
#include "runtime/include/itable_builder.h"
#include "runtime/include/method.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "libarkbase/macros.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/utils/bit_utils.h"
#include "libarkbase/utils/span.h"
#include "libarkbase/utils/utf.h"
#include "libarkfile/class_data_accessor-inl.h"
#include "libarkfile/code_data_accessor-inl.h"
#include "libarkfile/field_data_accessor-inl.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "libarkfile/modifiers.h"
#include "libarkfile/panda_cache.h"
#include "libarkfile/proto_data_accessor-inl.h"
#include "runtime/include/tooling/debug_inf.h"
#include "libarkbase/trace/trace.h"
#include "libarkbase/utils/utils.h"
#include "runtime/trace.h"

namespace ark {

using Type = panda_file::Type;
using SourceLang = panda_file::SourceLang;

void ClassLinker::AddPandaFile(std::unique_ptr<const panda_file::File> &&pf, ClassLinkerContext *context)
{
    ASSERT(pf != nullptr);

    const panda_file::File *file = pf.get();

    SCOPED_TRACE_STREAM << __FUNCTION__ << " " << file->GetFilename();

    {
        os::memory::LockHolder lock {pandaFilesLock_};
        pandaFiles_.push_back({context, std::forward<std::unique_ptr<const panda_file::File>>(pf)});
    }

    GetAotManager()->UpdatePandaFilesSnapshot(file, context, Runtime::GetOptions().IsArkAot());

    if (context == nullptr || context->IsBootContext()) {
        os::memory::LockHolder lock {bootPandaFilesLock_};
        bootPandaFiles_.push_back(file);
        if (Runtime::GetOptions().IsUseBootClassFilter()) {
            AddBootClassFilter(file);
        }
    }

    if (Runtime::GetCurrent()->IsInitialized()) {
        // LoadModule for initial boot files is called in runtime
        Runtime::GetCurrent()->GetNotificationManager()->LoadModuleEvent(file->GetFilename());
    }

    tooling::DebugInf::AddCodeMetaInfo(file);
}

void ClassLinker::FreeClassData(Class *classPtr)
{
    Span<Field> fields = classPtr->GetFields();
    if (fields.Size() > 0) {
        allocator_->Free(fields.begin());
        classPtr->SetFields(Span<Field>(), 0);
    }
    Span<Method> methods = classPtr->GetMethods();
    size_t n = methods.Size() + classPtr->GetNumCopiedMethods();
    if (n > 0) {
        mem::InternalAllocatorPtr allocator = Runtime::GetCurrent()->GetInternalAllocator();
        for (auto &method : methods) {
            // We create Profiling data in method class via InternalAllocator.
            // Therefore, we should delete it via InternalAllocator too.
            allocator->Free(method.GetProfilingData());
        }

        // Free copied method's ProfileData if it is necessary.
        for (auto &method : classPtr->GetCopiedMethods()) {
            auto id = method.GetFileId();
            auto pf = method.GetPandaFile();

            // There is a possibility that ProfilingData can be borrowed from the original method. Example:
            // 1. create a base interface class with a default method,
            // 2. call StartProfiling(),
            // 3. create a derived class and do not override the default method from the base interface,
            // 4. call StartProfiling() for the derived class.
            // Currently this example is impossible (the default method cannot be called from the interface) because
            // there is no such a language where #2 step will be executed.
            // Need more investigations to eliminate this extra check.
            Span<Method> originalMethods = method.GetClass()->GetMethods();
            auto it = std::find_if(originalMethods.begin(), originalMethods.end(),
                                   [id, pf](const auto &m) { return m.GetFileId() == id && m.GetPandaFile() == pf; });

            // Free method's ProfileData if it was set after copying of the original method.
            auto ptr = method.GetProfilingData();
            if (it == originalMethods.end() || it->GetProfilingDataWithoutCheck() != ptr) {
                allocator->Free(ptr);
            }
        }

        allocator_->Free(methods.begin());
        classPtr->SetMethods(Span<Method>(), 0, 0);
    }
    bool hasOwnItable = !classPtr->IsArrayClass();
    auto itable = classPtr->GetITable().Get();
    if (hasOwnItable && !itable.Empty()) {
        for (size_t i = 0; i < itable.Size(); i++) {
            Span<Method *> imethods = itable[i].GetMethods();
            if (!imethods.Empty()) {
                allocator_->Free(imethods.begin());
            }
        }
        allocator_->Free(itable.begin());
        classPtr->SetITable(ITable());
    }
    Span<Class *> interfaces = classPtr->GetInterfaces();
    if (!interfaces.Empty()) {
        allocator_->Free(interfaces.begin());
        classPtr->SetInterfaces(Span<Class *>());
    }
    Span<Class *> consTypes = classPtr->GetConstituentTypes();
    if (!consTypes.Empty()) {
        allocator_->Free(consTypes.begin());
        classPtr->SetConstituentTypes(Span<Class *>());
    }
}

void ClassLinker::FreeClass(Class *classPtr)
{
    FreeClassData(classPtr);
    GetExtension(classPtr->GetSourceLang())->FreeClass(classPtr);
}

ClassLinker::~ClassLinker()
{
    for (auto &copiedName : copiedNames_) {
        allocator_->Free(reinterpret_cast<void *>(const_cast<uint8_t *>(copiedName)));
    }
}

ClassLinker::ClassLinker(mem::InternalAllocatorPtr allocator,
                         std::vector<std::unique_ptr<ClassLinkerExtension>> &&extensions)
    : allocator_(allocator),
      aotManager_(MakePandaUnique<AotManager>()),
      copiedNames_(allocator->Adapter()),
      isTraceEnabled_(Runtime::GetOptions().IsEnableClassLinkerTrace()),
      bootClassFilter_(NUM_BOOT_CLASSES, FILTER_RATE)
{
    for (auto &ext : extensions) {
        extensions_[ark::panda_file::GetLangArrIndex(ext->GetLanguage())] = std::move(ext);
    }
}

void ClassLinker::ResetExtension(panda_file::SourceLang lang)
{
    extensions_[ark::panda_file::GetLangArrIndex(lang)] =
        Runtime::GetCurrent()->GetLanguageContext(lang).CreateClassLinkerExtension();
}

template <class T, class... Args>
static T *InitializeMemory(T *mem, Args... args)
{
    return new (mem) T(std::forward<Args>(args)...);
}

bool ClassLinker::Initialize(bool compressedStringEnabled)
{
    if (isInitialized_) {
        return true;
    }

    for (auto &ext : extensions_) {
        if (ext == nullptr) {
            continue;
        }

        if (!ext->Initialize(this, compressedStringEnabled)) {
            return false;
        }
    }

    isInitialized_ = true;

    return true;
}

bool ClassLinker::InitializeRoots(ManagedThread *thread)
{
    for (auto &ext : extensions_) {
        if (ext == nullptr) {
            continue;
        }

        if (!ext->InitializeRoots(thread)) {
            return false;
        }
    }

    return true;
}

using ClassEntry = std::pair<panda_file::File::EntityId, const panda_file::File *>;
using PandaFiles = PandaVector<const panda_file::File *>;

static ClassEntry FindClassInPandaFiles(const uint8_t *descriptor, const PandaFiles &pandaFiles)
{
    for (auto *pf : pandaFiles) {
        auto classId = pf->GetClassId(descriptor);
        if (classId.IsValid() && !pf->IsExternal(classId)) {
            return {classId, pf};
        }
    }

    return {};
}

Class *ClassLinker::FindLoadedClass(const uint8_t *descriptor, ClassLinkerContext *context)
{
    ASSERT(context != nullptr);
    return context->FindClass(descriptor);
}

template <class ClassDataAccessorT>
static size_t GetClassSize(ClassDataAccessorT dataAccessor, size_t vtableSize, size_t imtSize, size_t *outNumSfields)
{
    size_t num8bitSfields = 0;
    size_t num16bitSfields = 0;
    size_t num32bitSfields = 0;
    size_t num64bitSfields = 0;
    size_t numRefSfields = 0;
    size_t numTaggedSfields = 0;

    size_t numSfields = 0;

    dataAccessor.template EnumerateStaticFieldTypes([&num8bitSfields, &num16bitSfields, &num32bitSfields,
                                                     &num64bitSfields, &numRefSfields, &numTaggedSfields,
                                                     &numSfields](Type fieldType) {
        ++numSfields;

        switch (fieldType.GetId()) {
            case Type::TypeId::U1:
            case Type::TypeId::I8:
            case Type::TypeId::U8:
                ++num8bitSfields;
                break;
            case Type::TypeId::I16:
            case Type::TypeId::U16:
                ++num16bitSfields;
                break;
            case Type::TypeId::I32:
            case Type::TypeId::U32:
            case Type::TypeId::F32:
                ++num32bitSfields;
                break;
            case Type::TypeId::I64:
            case Type::TypeId::U64:
            case Type::TypeId::F64:
                ++num64bitSfields;
                break;
            case Type::TypeId::REFERENCE:
                ++numRefSfields;
                break;
            case Type::TypeId::TAGGED:
                ++numTaggedSfields;
                break;
            default:
                UNREACHABLE();
                break;
        }
    });

    *outNumSfields = numSfields;

    return Class::ComputeClassSize(vtableSize, imtSize, num8bitSfields, num16bitSfields, num32bitSfields,
                                   num64bitSfields, numRefSfields, numTaggedSfields);
}

class ClassDataAccessorWrapper {
public:
    explicit ClassDataAccessorWrapper(panda_file::ClassDataAccessor *dataAccessor = nullptr)
        : dataAccessor_(dataAccessor)
    {
    }

    template <class Callback>
    void EnumerateStaticFieldTypes(const Callback &cb) const
    {
        dataAccessor_->EnumerateFields([cb](panda_file::FieldDataAccessor &fda) {
            if (!fda.IsStatic()) {
                return;
            }

            cb(Type::GetTypeFromFieldEncoding(fda.GetType()));
        });
    }

    ~ClassDataAccessorWrapper() = default;

    DEFAULT_COPY_SEMANTIC(ClassDataAccessorWrapper);
    DEFAULT_MOVE_SEMANTIC(ClassDataAccessorWrapper);

private:
    panda_file::ClassDataAccessor *dataAccessor_;
};

ClassLinker::ClassInfo ClassLinker::CreateClassInfo(LanguageContext ctx, ClassLinkerErrorHandler *errorHandler)
{
    ClassInfo classInfo;
    classInfo.itableBuilder = ctx.CreateITableBuilder(errorHandler);
    classInfo.vtableBuilder = ctx.CreateVTableBuilder(errorHandler);
    classInfo.imtableBuilder = ctx.CreateIMTableBuilder();
    return classInfo;
}

bool ClassLinker::SetupClassInfo(ClassLinker::ClassInfo &info, panda_file::ClassDataAccessor *dataAccessor, Class *base,
                                 Span<Class *> interfaces, ClassLinkerContext *context,
                                 ClassLinkerErrorHandler *errorHandler)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(dataAccessor);

    info.vtableBuilder = ctx.CreateVTableBuilder(errorHandler);
    info.itableBuilder = ctx.CreateITableBuilder(errorHandler);
    info.imtableBuilder = ctx.CreateIMTableBuilder();

    ASSERT(info.itableBuilder != nullptr);
    if (!info.itableBuilder->Build(this, base, interfaces, dataAccessor->IsInterface())) {
        ITable::Free(allocator_, info.itableBuilder->GetITable());
        return false;
    }
    ASSERT(info.vtableBuilder != nullptr);
    if (!info.vtableBuilder->Build(dataAccessor, base, info.itableBuilder->GetITable(), context)) {
        ITable::Free(allocator_, info.itableBuilder->GetITable());
        return false;
    }
    info.imtableBuilder->Build(dataAccessor, info.itableBuilder->GetITable());

    ClassDataAccessorWrapper dataAccessorWrapper(dataAccessor);
    info.size = GetClassSize(dataAccessorWrapper, info.vtableBuilder->GetVTableSize(),
                             info.imtableBuilder->GetIMTSize(), &info.numSfields);
    return true;
}

class ClassDataAccessor {
public:
    explicit ClassDataAccessor(Span<Field> fields) : fields_(fields) {}

    template <class Callback>
    void EnumerateStaticFieldTypes(const Callback &cb) const
    {
        for (const auto &field : fields_) {
            if (!field.IsStatic()) {
                continue;
            }

            cb(field.GetType());
        }
    }

    ~ClassDataAccessor() = default;

    DEFAULT_COPY_SEMANTIC(ClassDataAccessor);
    DEFAULT_MOVE_SEMANTIC(ClassDataAccessor);

private:
    Span<Field> fields_;
};

bool ClassLinker::SetupClassInfo(ClassLinker::ClassInfo &info, Span<Method> methods, Span<Field> fields, Class *base,
                                 Span<Class *> interfaces, bool isInterface, ClassLinkerErrorHandler *errorHandler)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*base);

    info.vtableBuilder = ctx.CreateVTableBuilder(errorHandler);
    info.itableBuilder = ctx.CreateITableBuilder(errorHandler);
    info.imtableBuilder = ctx.CreateIMTableBuilder();

    ASSERT(info.itableBuilder != nullptr);
    if (!info.itableBuilder->Build(this, base, interfaces, isInterface)) {
        return false;
    }
    ASSERT(info.vtableBuilder != nullptr);
    if (!info.vtableBuilder->Build(methods, base, info.itableBuilder->GetITable(), isInterface)) {
        ITable::Free(allocator_, info.itableBuilder->GetITable());
        return false;
    }
    info.imtableBuilder->Build(info.itableBuilder->GetITable(), isInterface);

    ClassDataAccessor dataAccessor(fields);
    info.size = GetClassSize(dataAccessor, info.vtableBuilder->GetVTableSize(), info.imtableBuilder->GetIMTSize(),
                             &info.numSfields);
    return true;
}

static void LoadMethod(Method *method, panda_file::MethodDataAccessor *methodDataAccessor, Class *klass,
                       const LanguageContext &ctx, const ClassLinkerExtension *ext)
{
    const auto &pf = methodDataAccessor->GetPandaFile();
    panda_file::ProtoDataAccessor pda(pf, methodDataAccessor->GetProtoId());

    uint32_t accessFlags = methodDataAccessor->GetAccessFlags();

    auto *methodName = pf.GetStringData(methodDataAccessor->GetNameId()).data;
    if (utf::IsEqual(methodName, ctx.GetCtorName()) || utf::IsEqual(methodName, ctx.GetCctorName())) {
        accessFlags |= ACC_CONSTRUCTOR;
    }

    auto codeId = methodDataAccessor->GetCodeId();
    size_t numArgs = (methodDataAccessor->IsStatic()) ? pda.GetNumArgs() : (pda.GetNumArgs() + 1);

    if (!codeId.has_value()) {
        InitializeMemory(method, klass, &pf, methodDataAccessor->GetMethodId(), panda_file::File::EntityId(0),
                         accessFlags, numArgs, reinterpret_cast<const uint16_t *>(pda.GetShorty().Data()));

        if (methodDataAccessor->IsNative()) {
            method->SetCompiledEntryPoint(ext->GetNativeEntryPointFor(method));
        } else {
            method->SetInterpreterEntryPoint();
        }
    } else {
        InitializeMemory(method, klass, &pf, methodDataAccessor->GetMethodId(), codeId.value(), accessFlags, numArgs,
                         reinterpret_cast<const uint16_t *>(pda.GetShorty().Data()));
        method->SetCompiledEntryPoint(GetCompiledCodeToInterpreterBridge(method));
    }
}

static void MaybeLinkMethodToAotCode(Method *method, const compiler::AotClass &aotClass, size_t methodIndex)
{
    ASSERT(aotClass.IsValid());
    if (method->IsIntrinsic()) {
        return;
    }
    auto entry = aotClass.FindMethodCodeEntry(methodIndex);
    if (entry != nullptr) {
        method->SetCompiledEntryPoint(entry);
        LOG(DEBUG, AOT) << "Found AOT entrypoint ["
                        << reinterpret_cast<const void *>(aotClass.FindMethodCodeSpan(methodIndex).data()) << ":"
                        << reinterpret_cast<const void *>(aotClass.FindMethodCodeSpan(methodIndex).end())
                        << "] for method: " << method->GetFullName();

        EVENT_AOT_ENTRYPOINT_FOUND(method->GetFullName());
        ASSERT(aotClass.FindMethodHeader(methodIndex)->methodId == method->GetFileId().GetOffset());
    }
}

static void SetupCopiedMethods(Span<Method> methods, Span<const CopiedMethod> copiedMethods)
{
    size_t const numMethods = methods.size() - copiedMethods.size();

    for (size_t i = 0; i < copiedMethods.size(); i++) {
        Method *method = &methods[numMethods + i];
        InitializeMemory(method, copiedMethods[i].GetMethod());
        method->SetIsDefaultInterfaceMethod();
        switch (copiedMethods[i].GetStatus()) {
            case CopiedMethod::Status::ORDINARY:
                break;
            case CopiedMethod::Status::ABSTRACT:
                method->SetCompiledEntryPoint(GetAbstractMethodStub());
                break;
            case CopiedMethod::Status::CONFLICT:
                method->SetCompiledEntryPoint(GetDefaultConflictMethodStub());
                break;
        }
    }
}

bool ClassLinker::LoadMethods(Class *klass, ClassInfo *classInfo, panda_file::ClassDataAccessor *dataAccessor,
                              [[maybe_unused]] ClassLinkerErrorHandler *errorHandler)
{
    uint32_t numMethods = dataAccessor->GetMethodsNumber();

    uint32_t numVmethods = klass->GetNumVirtualMethods();
    uint32_t numSmethods = numMethods - numVmethods;

    auto copiedMethods = classInfo->vtableBuilder->GetCopiedMethods();
    uint32_t totalNumMethods = numMethods + copiedMethods.size();
    if (totalNumMethods == 0) {
        return true;
    }

    Span<Method> methods {allocator_->AllocArray<Method>(totalNumMethods), totalNumMethods};

    size_t smethodIdx = numVmethods;
    size_t vmethodIdx = 0;

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*klass);
    auto *ext = GetExtension(ctx);
    ASSERT(ext != nullptr);

    auto aotPfile = aotManager_->FindPandaFile(klass->GetPandaFile()->GetFullFileName());
    if (aotPfile != nullptr) {
        EVENT_AOT_LOADED_FOR_CLASS(PandaString(aotPfile->GetFileName()), PandaString(klass->GetName()));
    }

    compiler::AotClass aotClass =
        (aotPfile != nullptr) ? aotPfile->GetClass(klass->GetFileId().GetOffset()) : compiler::AotClass::Invalid();

    size_t methodIndex = 0;
    dataAccessor->EnumerateMethods([klass, &smethodIdx, &vmethodIdx, &methods, aotClass, ctx, ext,
                                    &methodIndex](panda_file::MethodDataAccessor &methodDataAccessor) {
        Method *method = methodDataAccessor.IsStatic() ? &methods[smethodIdx++] : &methods[vmethodIdx++];
        LoadMethod(method, &methodDataAccessor, klass, ctx, ext);
        if (aotClass.IsValid()) {
            MaybeLinkMethodToAotCode(method, aotClass, methodIndex);
        }
        // Instead of checking if the method is abstract before every virtual call
        // the special stub throwing AbstractMethodError is registered as compiled entry point.
        if (method->IsAbstract()) {
            method->SetCompiledEntryPoint(GetAbstractMethodStub());
        }
        methodIndex++;
    });

    SetupCopiedMethods(methods, copiedMethods);
    klass->SetMethods(methods, numVmethods, numSmethods);
    return true;
}

bool ClassLinker::LoadFields(Class *klass, panda_file::ClassDataAccessor *dataAccessor,
                             [[maybe_unused]] ClassLinkerErrorHandler *errorHandler)
{
    uint32_t numFields = dataAccessor->GetFieldsNumber();
    if (numFields == 0) {
        return true;
    }

    uint32_t numSfields = klass->GetNumStaticFields();

    Span<Field> fields {allocator_->AllocArray<Field>(numFields), numFields};

    size_t sfieldsIdx = 0;
    size_t ifieldsIdx = numSfields;
    dataAccessor->EnumerateFields(
        [klass, &sfieldsIdx, &ifieldsIdx, &fields](panda_file::FieldDataAccessor &fieldDataAccessor) {
            Field *field = fieldDataAccessor.IsStatic() ? &fields[sfieldsIdx++] : &fields[ifieldsIdx++];
            InitializeMemory(field, klass, fieldDataAccessor.GetFieldId(), fieldDataAccessor.GetAccessFlags(),
                             panda_file::Type::GetTypeFromFieldEncoding(fieldDataAccessor.GetType()));
        });

    klass->SetFields(fields, numSfields);

    return true;
}

template <bool REVERSE_LAYOUT = false>
static void LayoutFieldsWithoutAlignment(size_t size, size_t *offset, size_t *space, PandaVector<Field *> *fields)
{
    auto lastProceededElement = fields->end();
    // Iterating from beginning to end and erasing elements from the beginning of a vector
    // is required for correct field layout between language class representation in C++ code
    // and generated by methods in class linker.
    // (e.g. class String should have length field before hash field, not vice versa)
    for (auto i = fields->begin(); i != fields->end(); i++) {
        if (!(space == nullptr || *space >= size)) {
            lastProceededElement = i;
            break;
        }
        Field *field = *i;
        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (REVERSE_LAYOUT) {
            *offset -= size;
            field->SetOffset(*offset);
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            field->SetOffset(*offset);
            *offset += size;
        }
        if (space != nullptr) {
            *space -= size;
        }
    }
    fields->erase(fields->begin(), lastProceededElement);
}

static uint32_t LayoutReferenceFields(size_t size, size_t *offset, const PandaVector<Field *> &fields)
{
    uint32_t volatileFieldsNum = 0;
    // layout volatile fields firstly
    for (auto *field : fields) {
        if (field->IsVolatile()) {
            volatileFieldsNum++;
            field->SetOffset(*offset);
            *offset += size;
        }
    }
    for (auto *field : fields) {
        if (!field->IsVolatile()) {
            field->SetOffset(*offset);
            *offset += size;
        }
    }
    return volatileFieldsNum;
}

constexpr size_t SIZE_64 = sizeof(uint64_t);
constexpr size_t SIZE_32 = sizeof(uint32_t);
constexpr size_t SIZE_16 = sizeof(uint16_t);
constexpr size_t SIZE_8 = sizeof(uint8_t);

// CC-OFFNXT(G.FUN.01) solid logic
static size_t LayoutFieldsInBaseClassPadding(Class *klass, PandaVector<Field *> *taggedFields,
                                             PandaVector<Field *> *fields64, PandaVector<Field *> *fields32,
                                             PandaVector<Field *> *fields16, PandaVector<Field *> *fields8,
                                             PandaVector<Field *> *refFields, bool isStatic)
{
    size_t offset;

    if (isStatic) {
        offset = klass->GetStaticFieldsOffset();
    } else {
        offset = (klass->GetBase() != nullptr) ? klass->GetBase()->GetObjectSize() : ObjectHeader::ObjectHeaderSize();
    }

    size_t alignOffset = offset;
    if (!refFields->empty()) {
        alignOffset = AlignUp(offset, ClassHelper::OBJECT_POINTER_SIZE);
    } else if (!(fields64->empty()) || !(taggedFields->empty())) {
        alignOffset = AlignUp(offset, SIZE_64);
    } else if (!fields32->empty()) {
        alignOffset = AlignUp(offset, SIZE_32);
    } else if (!fields16->empty()) {
        alignOffset = AlignUp(offset, SIZE_16);
    }
    if (alignOffset != offset) {
        size_t endOffset = alignOffset;
        size_t padding = endOffset - offset;
        // try to put short fields of child class at end of free space of base class
        LayoutFieldsWithoutAlignment<true>(SIZE_32, &endOffset, &padding, fields32);
        LayoutFieldsWithoutAlignment<true>(SIZE_16, &endOffset, &padding, fields16);
        LayoutFieldsWithoutAlignment<true>(SIZE_8, &endOffset, &padding, fields8);
    }
    return alignOffset;
}

// CC-OFFNXT(G.FUN.01) solid logic
static size_t LayoutFields(Class *klass, PandaVector<Field *> *taggedFields, PandaVector<Field *> *fields64,
                           PandaVector<Field *> *fields32, PandaVector<Field *> *fields16,
                           PandaVector<Field *> *fields8, PandaVector<Field *> *refFields, bool isStatic)
{
    size_t offset =
        LayoutFieldsInBaseClassPadding(klass, taggedFields, fields64, fields32, fields16, fields8, refFields, isStatic);
    if (!refFields->empty()) {
        offset = AlignUp(offset, ClassHelper::OBJECT_POINTER_SIZE);
        klass->SetRefFieldsNum(refFields->size(), isStatic);
        klass->SetRefFieldsOffset(offset, isStatic);
        auto volatileNum = LayoutReferenceFields(ClassHelper::OBJECT_POINTER_SIZE, &offset, *refFields);
        klass->SetVolatileRefFieldsNum(volatileNum, isStatic);
    }

    static_assert(coretypes::TaggedValue::TaggedTypeSize() == SIZE_64,
                  "Please fix alignment of the fields of type \"TaggedValue\"");
    if (!IsAligned<SIZE_64>(offset) && (!fields64->empty() || !taggedFields->empty())) {
        size_t padding = AlignUp(offset, SIZE_64) - offset;

        LayoutFieldsWithoutAlignment(SIZE_32, &offset, &padding, fields32);
        LayoutFieldsWithoutAlignment(SIZE_16, &offset, &padding, fields16);
        LayoutFieldsWithoutAlignment(SIZE_8, &offset, &padding, fields8);

        offset += padding;
    }

    LayoutFieldsWithoutAlignment(coretypes::TaggedValue::TaggedTypeSize(), &offset, nullptr, taggedFields);
    LayoutFieldsWithoutAlignment(SIZE_64, &offset, nullptr, fields64);

    if (!IsAligned<SIZE_32>(offset) && !fields32->empty()) {
        size_t padding = AlignUp(offset, SIZE_32) - offset;

        LayoutFieldsWithoutAlignment(SIZE_16, &offset, &padding, fields16);
        LayoutFieldsWithoutAlignment(SIZE_8, &offset, &padding, fields8);

        offset += padding;
    }

    LayoutFieldsWithoutAlignment(SIZE_32, &offset, nullptr, fields32);

    if (!IsAligned<SIZE_16>(offset) && !fields16->empty()) {
        size_t padding = AlignUp(offset, SIZE_16) - offset;

        LayoutFieldsWithoutAlignment(SIZE_8, &offset, &padding, fields8);

        offset += padding;
    }

    LayoutFieldsWithoutAlignment(SIZE_16, &offset, nullptr, fields16);

    LayoutFieldsWithoutAlignment(SIZE_8, &offset, nullptr, fields8);

    return offset;
}

static inline bool PushBackFieldIfNonPrimitiveType(Field &field, PandaVector<Field *> &refFields)
{
    auto type = field.GetType();
    if (!type.IsPrimitive()) {
        refFields.push_back(&field);
        return true;
    }

    return false;
}

/* static */
// CC-OFFNXT(huge_method) solid logic
bool ClassLinker::LayoutFields(Class *klass, Span<Field> fields, bool isStatic,
                               [[maybe_unused]] ClassLinkerErrorHandler *errorHandler)
{
    // These containers must be optimized
    PandaVector<Field *> taggedFields;
    PandaVector<Field *> fields64;
    PandaVector<Field *> fields32;
    PandaVector<Field *> fields16;
    PandaVector<Field *> fields8;
    PandaVector<Field *> refFields;
    taggedFields.reserve(fields.size());
    fields64.reserve(fields.size());
    fields32.reserve(fields.size());
    fields16.reserve(fields.size());
    fields8.reserve(fields.size());
    refFields.reserve(fields.size());

    for (auto &field : fields) {
        if (PushBackFieldIfNonPrimitiveType(field, refFields)) {
            continue;
        }

        switch (field.GetType().GetId()) {
            case Type::TypeId::U1:
            case Type::TypeId::I8:
            case Type::TypeId::U8:
                fields8.push_back(&field);
                break;
            case Type::TypeId::I16:
            case Type::TypeId::U16:
                fields16.push_back(&field);
                break;
            case Type::TypeId::I32:
            case Type::TypeId::U32:
            case Type::TypeId::F32:
                fields32.push_back(&field);
                break;
            case Type::TypeId::I64:
            case Type::TypeId::U64:
            case Type::TypeId::F64:
                fields64.push_back(&field);
                break;
            case Type::TypeId::TAGGED:
                taggedFields.push_back(&field);
                break;
            default:
                UNREACHABLE();
                break;
        }
    }

    size_t size =
        ark::LayoutFields(klass, &taggedFields, &fields64, &fields32, &fields16, &fields8, &refFields, isStatic);

    if (!isStatic && !klass->IsVariableSize()) {
        klass->SetObjectSize(size);
    }

    return true;
}

bool ClassLinker::LinkMethods(Class *klass, ClassInfo *classInfo,
                              [[maybe_unused]] ClassLinkerErrorHandler *errorHandler)
{
    classInfo->vtableBuilder->UpdateClass(klass);
    ASSERT(classInfo->itableBuilder != nullptr);
    if (!classInfo->itableBuilder->Resolve(klass)) {
        return false;
    }
    classInfo->itableBuilder->UpdateClass(klass);
    classInfo->imtableBuilder->UpdateClass(klass);
    return true;
}

bool ClassLinker::LinkFields(Class *klass, ClassLinkerErrorHandler *errorHandler)
{
    if (!LayoutFields(klass, klass->GetStaticFields(), true, errorHandler)) {
        LOG(ERROR, CLASS_LINKER) << "Cannot layout static fields of class '" << klass->GetName() << "'";
        return false;
    }

    if (!LayoutFields(klass, klass->GetInstanceFields(), false, errorHandler)) {
        LOG(ERROR, CLASS_LINKER) << "Cannot layout instance fields of class '" << klass->GetName() << "'";
        return false;
    }

    return true;
}

Class *ClassLinker::LoadBaseClass(panda_file::ClassDataAccessor *cda, const LanguageContext &ctx,
                                  ClassLinkerContext *context, ClassLinkerErrorHandler *errorHandler)
{
    auto baseClassId = cda->GetSuperClassId();
    auto *ext = GetExtension(ctx);
    ASSERT(ext != nullptr);
    if (baseClassId.GetOffset() == 0) {
        return ext->GetClassRoot(ClassRoot::OBJECT);
    }

    auto &pf = cda->GetPandaFile();
    auto *baseClass = ext->GetClass(pf, baseClassId, context, errorHandler);
    if (baseClass == nullptr) {
        LOG(INFO, CLASS_LINKER) << "Cannot find base class '" << utf::Mutf8AsCString(pf.GetStringData(baseClassId).data)
                                << "' of class '" << utf::Mutf8AsCString(pf.GetStringData(cda->GetClassId()).data)
                                << "' in ctx " << context;
        return nullptr;
    }

    return baseClass;
}

std::optional<Span<Class *>> ClassLinker::LoadInterfaces(panda_file::ClassDataAccessor *cda,
                                                         ClassLinkerContext *context,
                                                         ClassLinkerErrorHandler *errorHandler)
{
    ASSERT(context != nullptr);
    size_t ifacesNum = cda->GetIfacesNumber();
    if (ifacesNum == 0) {
        return Span<Class *>(nullptr, ifacesNum);
    }

    Span<Class *> ifaces {allocator_->AllocArray<Class *>(ifacesNum), ifacesNum};

    for (size_t i = 0; i < ifacesNum; i++) {
        auto id = cda->GetInterfaceId(i);
        auto &pf = cda->GetPandaFile();
        auto *iface = GetClass(pf, id, context, errorHandler);
        if (iface == nullptr) {
            LOG(INFO, CLASS_LINKER) << "Cannot find interface '" << utf::Mutf8AsCString(pf.GetStringData(id).data)
                                    << "' of class '" << utf::Mutf8AsCString(pf.GetStringData(cda->GetClassId()).data)
                                    << "' in ctx " << context;
            ASSERT(!ifaces.Empty());
            allocator_->Free(ifaces.begin());
            return {};
        }

        ifaces[i] = iface;
    }

    return ifaces;
}

using ClassLoadingSet = std::unordered_set<uint64_t>;

// This class is required to clear static unordered_set on return
class ClassScopeStaticSetAutoCleaner {
public:
    ClassScopeStaticSetAutoCleaner() = default;
    explicit ClassScopeStaticSetAutoCleaner(ClassLoadingSet *setPtr, ClassLoadingSet **tlSetPtr)
        : setPtr_(setPtr), tlSetPtr_(tlSetPtr)
    {
    }
    ~ClassScopeStaticSetAutoCleaner()
    {
        setPtr_->clear();
        if (tlSetPtr_ != nullptr) {
            *tlSetPtr_ = nullptr;
        }
    }

    NO_COPY_SEMANTIC(ClassScopeStaticSetAutoCleaner);
    NO_MOVE_SEMANTIC(ClassScopeStaticSetAutoCleaner);

private:
    ClassLoadingSet *setPtr_ {nullptr};
    ClassLoadingSet **tlSetPtr_ {nullptr};
};

static uint64_t GetClassUniqueHash(uint32_t pandaFileHash, uint32_t classId)
{
    const uint8_t bitsToShuffle = 32;
    return (static_cast<uint64_t>(pandaFileHash) << bitsToShuffle) | static_cast<uint64_t>(classId);
}

Class *ClassLinker::LoadClass(panda_file::ClassDataAccessor *classDataAccessor, const uint8_t *descriptor,
                              Class *baseClass, Span<Class *> interfaces, ClassLinkerContext *context,
                              ClassLinkerExtension *ext, ClassLinkerErrorHandler *errorHandler)
{
    ASSERT(context != nullptr);
    ClassInfo classInfo {};
    if (!SetupClassInfo(classInfo, classDataAccessor, baseClass, interfaces, context, errorHandler)) {
        return nullptr;
    }

    ASSERT(classInfo.vtableBuilder != nullptr);
    auto *klass = ext->CreateClass(descriptor, classInfo.vtableBuilder->GetVTableSize(),
                                   classInfo.imtableBuilder->GetIMTSize(), classInfo.size);

    if (UNLIKELY(klass == nullptr)) {
        return nullptr;
    }

    klass->SetLoadContext(context);
    klass->SetBase(baseClass);
    klass->SetInterfaces(interfaces);
    klass->SetFileId(classDataAccessor->GetClassId());
    klass->SetPandaFile(&classDataAccessor->GetPandaFile());
    klass->SetAccessFlags(classDataAccessor->GetAccessFlags());

    auto &pf = classDataAccessor->GetPandaFile();
    auto classId = classDataAccessor->GetClassId();
    klass->SetClassIndex(pf.GetClassIndex(classId));
    klass->SetMethodIndex(pf.GetMethodIndex(classId));
    klass->SetFieldIndex(pf.GetFieldIndex(classId));

    klass->SetNumVirtualMethods(classInfo.vtableBuilder->GetNumVirtualMethods());
    klass->SetNumCopiedMethods(classInfo.vtableBuilder->GetCopiedMethods().size());
    klass->SetNumStaticFields(classInfo.numSfields);

    auto const onFail = [this, descriptor, klass](std::string_view msg) {
        FreeClass(klass);
        LOG(ERROR, CLASS_LINKER) << msg << " '" << descriptor << "'";
        return nullptr;
    };
    if (!LoadMethods(klass, &classInfo, classDataAccessor, errorHandler)) {
        return onFail("Cannot load methods of class");
    }
    if (!LoadFields(klass, classDataAccessor, errorHandler)) {
        return onFail("Cannot load fields of class");
    }
    if (!LinkMethods(klass, &classInfo, errorHandler)) {
        return onFail("Cannot link methods of class");
    }
    if (!LinkFields(klass, errorHandler)) {
        return onFail("Cannot link fields of class");
    }
    klass->CalcHaveNoRefsInParents();
    return klass;
}

Class *ClassLinker::LoadClass(const panda_file::File *pf, const uint8_t *descriptor, panda_file::SourceLang lang)
{
    panda_file::File::EntityId classId = pf->GetClassId(descriptor);
    if (!classId.IsValid() || pf->IsExternal(classId)) {
        return nullptr;
    }
    ClassLinkerContext *context = GetExtension(lang)->GetBootContext();
    return LoadClass(pf, classId, descriptor, context, nullptr);
}

static void OnError(ClassLinkerErrorHandler *errorHandler, ClassLinker::Error error, const PandaString &msg)
{
    if (errorHandler != nullptr) {
        errorHandler->OnError(error, msg);
    }
}

static bool TryInsertClassLoading(panda_file::File::EntityId &classId, const panda_file::File *pf,
                                  panda_file::ClassDataAccessor &classDataAccessor, ClassLoadingSet *threadLocalSet,
                                  ClassLinkerErrorHandler *errorHandler)
{
    uint32_t classIdInt = classId.GetOffset();
    uint32_t pandaFileHash = pf->GetFilenameHash();
    if (!threadLocalSet->insert(GetClassUniqueHash(pandaFileHash, classIdInt)).second) {
        const PandaString &className = utf::Mutf8AsCString(pf->GetStringData(classDataAccessor.GetClassId()).data);
        PandaString msg = "Class or interface \"" + className + "\" is its own superclass or superinterface";
        OnError(errorHandler, ClassLinker::Error::CLASS_CIRCULARITY, msg);
        return false;
    }

    return true;
}

static bool IsContextCanBeLoaded(ClassLinkerContext *context, panda_file::ClassDataAccessor &classDataAccessor,
                                 const uint8_t *descriptor, ClassLinkerErrorHandler *errorHandler)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(&classDataAccessor);
    if (ctx.GetLanguage() != context->GetSourceLang()) {
        LanguageContext currentCtx = Runtime::GetCurrent()->GetLanguageContext(context->GetSourceLang());
        PandaStringStream ss;
        ss << "Cannot load " << ctx << " class " << descriptor << " into " << currentCtx << " context";
        LOG(ERROR, CLASS_LINKER) << ss.str();
        OnError(errorHandler, ClassLinker::Error::CLASS_NOT_FOUND, ss.str());
        return false;
    }

    return true;
}

static void HandleNoExtensionError(LanguageContext &ctx, const uint8_t *descriptor,
                                   ClassLinkerErrorHandler *errorHandler)
{
    PandaStringStream ss;
    ss << "Cannot load class '" << descriptor << "' as class linker hasn't " << ctx << " language extension";
    LOG(ERROR, CLASS_LINKER) << ss.str();
    OnError(errorHandler, ClassLinker::Error::CLASS_NOT_FOUND, ss.str());
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
Class *ClassLinker::LoadClass(const panda_file::File *pf, panda_file::File::EntityId classId, const uint8_t *descriptor,
                              ClassLinkerContext *context, ClassLinkerErrorHandler *errorHandler,
                              bool addToRuntime /* = true */)
{
    ASSERT(pf != nullptr);
    ASSERT(!pf->IsExternal(classId));
    ASSERT(context != nullptr);
    panda_file::ClassDataAccessor classDataAccessor(*pf, classId);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(&classDataAccessor);
    if (!IsContextCanBeLoaded(context, classDataAccessor, descriptor, errorHandler)) {
        return nullptr;
    }

    if (!HasExtension(ctx)) {
        HandleNoExtensionError(ctx, descriptor, errorHandler);
        return nullptr;
    }

    // This set is used to find out if the class is its own superclass
    ClassLoadingSet loadingSet;
    static thread_local ClassLoadingSet *threadLocalSet = nullptr;
    ClassLoadingSet **threadLocalSetPtr = nullptr;
    if (threadLocalSet == nullptr) {
        threadLocalSet = &loadingSet;
        threadLocalSetPtr = &threadLocalSet;
    }
    ClassScopeStaticSetAutoCleaner classSetAutoCleanerOnReturn(threadLocalSet, threadLocalSetPtr);

    auto *ext = GetExtension(ctx);
    Class *baseClass = nullptr;
    bool needLoadBase = IsInitialized() || !utf::IsEqual(ctx.GetObjectClassDescriptor(), descriptor);
    if (needLoadBase) {
        if (!TryInsertClassLoading(classId, pf, classDataAccessor, threadLocalSet, errorHandler)) {
            return nullptr;
        }

        baseClass = LoadBaseClass(&classDataAccessor, ctx, context, errorHandler);
        if (baseClass == nullptr) {
            LOG(INFO, CLASS_LINKER) << "Cannot load base class of class '" << descriptor << "'";
            return nullptr;
        }
    }

    auto res = LoadInterfaces(&classDataAccessor, context, errorHandler);
    if (!res) {
        LOG(INFO, CLASS_LINKER) << "Cannot load interfaces of class '" << descriptor << "'";
        return nullptr;
    }

    auto *klass = LoadClass(&classDataAccessor, descriptor, baseClass, res.value(), context, ext, errorHandler);
    if (klass == nullptr) {
        allocator_->Free(res->Data());
        return nullptr;
    }

    auto *cha = Runtime::GetCurrent()->GetCha();
    ASSERT(cha != nullptr);
    cha->Update(klass);

    ASSERT(ext != nullptr);
    if (LIKELY(ext->CanInitializeClasses())) {
        if (!ext->InitializeClass(klass)) {
            LOG(ERROR, CLASS_LINKER) << "Language specific initialization for class '" << descriptor << "' failed";
            FreeClass(klass);
            return nullptr;
        }
        klass->SetState(Class::State::LOADED);
    }

    if (LIKELY(addToRuntime)) {
        Runtime::GetCurrent()->GetNotificationManager()->ClassLoadEvent(klass);

        auto *otherKlass = context->InsertClass(klass);
        if (otherKlass != nullptr) {
            // Someone has created the class in the other thread (increase the critical section?)
            FreeClass(klass);
            return otherKlass;
        }

        RemoveCreatedClassInExtension(klass);

        Runtime::GetCurrent()->GetNotificationManager()->ClassPrepareEvent(klass);
    }
    return klass;
}

static const uint8_t *CopyMutf8String(mem::InternalAllocatorPtr allocator, const uint8_t *descriptor)
{
    size_t size = utf::Mutf8Size(descriptor) + 1;  // + 1 - null terminate
    auto *ptr = allocator->AllocArray<uint8_t>(size);
    MemcpyUnsafe(ptr, descriptor, size);
    return ptr;
}

bool ClassLinker::LinkEntitiesAndInitClass(Class *klass, ClassInfo *classInfo, ClassLinkerExtension *ext,
                                           const uint8_t *descriptor)
{
    if (!LinkMethods(klass, classInfo, ext->GetErrorHandler())) {
        LOG(ERROR, CLASS_LINKER) << "Cannot link class methods '" << descriptor << "'";
        return false;
    }

    if (!LinkFields(klass, ext->GetErrorHandler())) {
        LOG(ERROR, CLASS_LINKER) << "Cannot link class fields '" << descriptor << "'";
        return false;
    }

    if (!ext->InitializeClass(klass)) {
        LOG(ERROR, CLASS_LINKER) << "Language specific initialization for class '" << descriptor << "' failed";
        FreeClass(klass);
        return false;
    }

    return true;
}

// CC-OFFNXT(huge_method) solid logic
Class *ClassLinker::BuildClassImpl(const uint8_t *descriptor, uint32_t accessFlags, Span<Method> methods,
                                   Span<Field> fields, Class *baseClass, Span<Class *> interfaces,
                                   ClassLinkerContext *context, ClassLinkerExtension *ext, ClassInfo classInfo)
{
    ASSERT(classInfo.vtableBuilder != nullptr);
    auto *klass = ext->CreateClass(descriptor, classInfo.vtableBuilder->GetVTableSize(),
                                   classInfo.imtableBuilder->GetIMTSize(), classInfo.size);

    if (UNLIKELY(klass == nullptr)) {
        ITable::Free(allocator_, classInfo.itableBuilder->GetITable());
        return nullptr;
    }

    klass->SetLoadContext(context);
    klass->SetBase(baseClass);
    klass->SetInterfaces(interfaces);
    klass->SetAccessFlags(accessFlags);

    klass->SetNumVirtualMethods(classInfo.vtableBuilder->GetNumVirtualMethods());
    klass->SetNumCopiedMethods(classInfo.vtableBuilder->GetCopiedMethods().size());
    klass->SetNumStaticFields(classInfo.numSfields);

    size_t numSmethods = methods.size() - klass->GetNumVirtualMethods();
    klass->SetMethods(methods, klass->GetNumVirtualMethods(), numSmethods);
    klass->SetFields(fields, klass->GetNumStaticFields());

    for (auto &method : methods) {
        method.SetClass(klass);
    }

    for (auto &field : fields) {
        field.SetClass(klass);
    }

    klass->CalcHaveNoRefsInParents();
    if (UNLIKELY(!LinkEntitiesAndInitClass(klass, &classInfo, ext, descriptor))) {
        ITable::Free(allocator_, classInfo.itableBuilder->GetITable());
        return nullptr;
    }

    klass->SetState(Class::State::LOADED);

    Runtime::GetCurrent()->GetNotificationManager()->ClassLoadEvent(klass);

    auto *otherKlass = context->InsertClass(klass);
    if (otherKlass != nullptr) {
        // Someone has created the class in the other thread (increase the critical section?)
        FreeClass(klass);
        return otherKlass;
    }

    RemoveCreatedClassInExtension(klass);
    Runtime::GetCurrent()->GetNotificationManager()->ClassPrepareEvent(klass);

    return klass;
}

Class *ClassLinker::BuildClass(const uint8_t *descriptor, bool needCopyDescriptor, uint32_t accessFlags,
                               Span<Method> methods, Span<Field> fields, Class *baseClass, Span<Class *> interfaces,
                               ClassLinkerContext *context, bool isInterface)
{
    ASSERT(context != nullptr);
    if (needCopyDescriptor) {
        descriptor = CopyMutf8String(allocator_, descriptor);
        os::memory::LockHolder lock(copiedNamesLock_);
        copiedNames_.push_front(descriptor);
    }

    auto *ext = GetExtension(baseClass->GetSourceLang());
    ASSERT(ext != nullptr);

    ClassInfo classInfo {};
    if (!SetupClassInfo(classInfo, methods, fields, baseClass, interfaces, isInterface, ext->GetErrorHandler())) {
        return nullptr;
    }

    return BuildClassImpl(descriptor, accessFlags, methods, fields, baseClass, interfaces, context, ext,
                          std::move(classInfo));
}

class ClassLinker::InterfaceProxyBuilder final {
public:
    NO_MOVE_SEMANTIC(InterfaceProxyBuilder);
    NO_COPY_SEMANTIC(InterfaceProxyBuilder);

    explicit InterfaceProxyBuilder(ClassLinkerExtension *ext, mem::InternalAllocatorPtr allocator)
        : ext_(ext), allocator_(allocator), tempProxyClass_(nullptr, ClassDeleter(ext))
    {
    }

    ~InterfaceProxyBuilder()
    {
        if (UNLIKELY(needReleaseItable_)) {
            ITable::Free(allocator_, itable_);
        }
        if (UNLIKELY(needReleaseProxyMethods_)) {
            allocator_->Delete(proxyMethods_.Data());
        }
    }

    Class *Build(LanguageContext ctx, ClassInfo classInfo, const uint8_t *descriptor, uint32_t accessFlags,
                 Span<Field> fields, Class *baseClass, Span<Class *> interfaces, ClassLinkerContext *context,
                 ClassLinkerErrorHandler *errorHandler)
    {
        ClassLinker *linker = ext_->GetClassLinker();

        bool buildItable = !classInfo.itableBuilder->Build(linker, baseClass, interfaces, false);
        itable_ = classInfo.itableBuilder->GetITable();

        if (UNLIKELY(buildItable)) {
            return nullptr;
        }

        // Since the creation of a target proxy class requires already built methods,
        // but the building of methods requires an owner class to be set,
        // it is necessary to create a temporary class with the same access flags and loading context
        // as the target proxy will have.
        if (UNLIKELY(!AllocateTemporaryProxyClass(descriptor, context, accessFlags))) {
            return nullptr;
        }
        if (UNLIKELY(!CollectMethodsFromItable(itable_))) {
            return nullptr;
        }
        if (UNLIKELY(!FilterMethods(ctx.CreateVTableBuilder(errorHandler), baseClass))) {
            return nullptr;
        }
        auto proxyMethodsOpt = AllocateProxyMethods();
        if (UNLIKELY(!proxyMethodsOpt.has_value())) {
            return nullptr;
        }
        proxyMethods_ = proxyMethodsOpt.value();
        if (UNLIKELY(!classInfo.vtableBuilder->Build(proxyMethods_, baseClass, itable_, false))) {
            return nullptr;
        }
        classInfo.imtableBuilder->Build(itable_, false);

        ClassDataAccessor dataAccessor(fields);
        classInfo.size = GetClassSize(dataAccessor, classInfo.vtableBuilder->GetVTableSize(),
                                      classInfo.imtableBuilder->GetIMTSize(), &classInfo.numSfields);

        auto *klass = linker->BuildClassImpl(descriptor, accessFlags, proxyMethods_, fields, baseClass, interfaces,
                                             context, ext_, std::move(classInfo));
        if (UNLIKELY(klass == nullptr)) {
            return nullptr;
        }

        needReleaseItable_ = false;
        needReleaseProxyMethods_ = false;
        return klass;
    }

private:
    bool AllocateTemporaryProxyClass(const uint8_t *descriptor, ClassLinkerContext *context, uint32_t accessFlags)
    {
        tempProxyClass_ =
            PandaUniquePtr<Class, ClassDeleter>(ext_->CreateClass(descriptor, 0, 0, sizeof(Class)), ClassDeleter(ext_));
        if (UNLIKELY(tempProxyClass_ == nullptr)) {
            return false;
        }
        tempProxyClass_->SetLoadContext(context);
        tempProxyClass_->SetAccessFlags(accessFlags);
        return true;
    }

    bool CollectMethodsFromItable(ITable itable)
    {
        auto methods = ext_->BuildProxyClassMethodsSpan(itable);
        if (UNLIKELY(!methods.has_value())) {
            return false;
        }

        allInterfacesMethods_ = std::move(*methods);
        return true;
    }

    bool FilterMethods(PandaUniquePtr<VTableBuilder> vtableBuilder, Class *baseClass)
    {
        PandaVector<Method *> candidates(allocator_->Adapter());
        if (!vtableBuilder->FilterProxyClassMethods(Span<Method *>(allInterfacesMethods_), &candidates, baseClass)) {
            return false;
        }
        filteredMethods_ = std::move(candidates);
        return true;
    }

    std::optional<Span<Method>> AllocateProxyMethods()
    {
        return ext_->GenerateProxyClassMethods(tempProxyClass_.get(), Span<Method *>(filteredMethods_));
    }

private:
    class ClassDeleter {
    public:
        explicit ClassDeleter(ClassLinkerExtension *ext) : ext_(ext) {}

        void operator()(Class *tempClassPtr) const
        {
            if (tempClassPtr != nullptr) {
                ext_->FreeClass(tempClassPtr);
            }
        }

    private:
        ClassLinkerExtension *ext_ {nullptr};
    };

private:
    ClassLinkerExtension *ext_ {nullptr};
    mem::InternalAllocatorPtr allocator_;

    ITable itable_;
    bool needReleaseItable_ {true};

    PandaVector<Method *> allInterfacesMethods_;

    PandaVector<Method *> filteredMethods_;

    Span<Method> proxyMethods_;
    bool needReleaseProxyMethods_ {true};

    PandaUniquePtr<Class, ClassDeleter> tempProxyClass_;
};

Class *ClassLinker::BuildProxyClass(const uint8_t *descriptor, bool needCopyDescriptor, uint32_t accessFlags,
                                    Span<Field> fields, Class *baseClass, Span<Class *> interfaces,
                                    ClassLinkerContext *context, ClassLinkerErrorHandler *errorHandler)
{
    ASSERT(context != nullptr);
    if (needCopyDescriptor) {
        descriptor = CopyMutf8String(allocator_, descriptor);
        os::memory::LockHolder lock(copiedNamesLock_);
        copiedNames_.push_front(descriptor);
    }

    auto *ext = GetExtension(baseClass->GetSourceLang());
    ASSERT(ext != nullptr);

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*baseClass);
    ClassInfo classInfo = CreateClassInfo(ctx, errorHandler);

    InterfaceProxyBuilder builder(ext, allocator_);
    return builder.Build(ctx, std::move(classInfo), descriptor, accessFlags, fields, baseClass, interfaces, context,
                         errorHandler);
}

Class *ClassLinker::CreateUnionClass(ClassLinkerExtension *ext, const uint8_t *descriptor, bool needCopyDescriptor,
                                     Span<Class *> constituentClasses, ClassLinkerContext *commonContext)
{
    if (needCopyDescriptor) {
        descriptor = CopyMutf8String(allocator_, descriptor);
        os::memory::LockHolder lock(copiedNamesLock_);
        copiedNames_.push_front(descriptor);
    }

    auto *unionClass = ext->CreateClass(descriptor, ext->GetArrayClassVTableSize(), ext->GetArrayClassIMTSize(),
                                        ext->GetArrayClassSize());

    if (UNLIKELY(unionClass == nullptr)) {
        return nullptr;
    }

    unionClass->SetLoadContext(commonContext);

    if (UNLIKELY(!ext->InitializeUnionClass(unionClass, constituentClasses))) {
        return nullptr;
    }

    return unionClass;
}

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
std::optional<Span<Class *>> ClassLinker::LoadConstituentClasses(const uint8_t *descriptor, bool needCopyDescriptor,
                                                                 ClassLinkerContext *context,
                                                                 ClassLinkerErrorHandler *errorHandler)
{
    static constexpr size_t UNION_COMPONENT_IDX = 2;
    size_t idx = UNION_COMPONENT_IDX;
    size_t elementsSize = 0;
    while (descriptor[idx] != '}') {
        auto typeSp = ClassHelper::GetUnionComponent(&(descriptor[idx]));
        elementsSize += 1;
        idx += typeSp.Size();
    }

    Span<Class *> klasses {allocator_->AllocArray<Class *>(elementsSize), elementsSize};
    size_t i = 0;
    idx = UNION_COMPONENT_IDX;
    while (descriptor[idx] != '}') {
        auto typeSp = ClassHelper::GetUnionComponent(&(descriptor[idx]));
        PandaString typeDescCopy(utf::Mutf8AsCString(typeSp.Data()), typeSp.Size());
        idx += typeSp.Size();
        const uint8_t *separateDesc = utf::CStringAsMutf8(typeDescCopy.c_str());

        Class *klass = GetClass(separateDesc, ClassHelper::IsArrayDescriptor(separateDesc) ? true : needCopyDescriptor,
                                context, errorHandler);
        if (klass == nullptr) {
            LOG(INFO, CLASS_LINKER) << "Cannot find substituent class '" << typeDescCopy << "' of union class '"
                                    << utf::Mutf8AsCString(descriptor) << "' in context " << context;
            ASSERT(!klasses.Empty());
            allocator_->Free(klasses.begin());
            return {};
        }

        klasses[i++] = klass;
    }
    ASSERT(klasses.Size() > 1);
    return klasses;
}
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

Class *ClassLinker::LoadUnionClass(const uint8_t *descriptor, bool needCopyDescriptor, ClassLinkerContext *context,
                                   ClassLinkerErrorHandler *errorHandler)
{
    auto constituentClasses = LoadConstituentClasses(descriptor, needCopyDescriptor, context, errorHandler);
    if (!constituentClasses.has_value()) {
        LOG(INFO, CLASS_LINKER) << "Cannot load constituent classes of union class '" << descriptor << "'";
        return nullptr;
    }

    ASSERT(constituentClasses.value().Size() > 1);

    auto *ext = GetExtension((*constituentClasses.value().begin())->GetSourceLang());
    ASSERT(ext != nullptr);
    auto *commonContext = ext->GetCommonContext(constituentClasses.value());
    ASSERT(commonContext != nullptr);

    if (commonContext != context) {
        auto *loadedClass = FindLoadedClass(descriptor, commonContext);
        if (loadedClass != nullptr) {
            ASSERT(!constituentClasses.value().Empty());
            allocator_->Free(constituentClasses.value().begin());
            return loadedClass;
        }
    }

    auto *unionClass = CreateUnionClass(ext, descriptor, needCopyDescriptor, constituentClasses.value(), commonContext);
    unionClass->CalcHaveNoRefsInParents();

    if (UNLIKELY(unionClass == nullptr)) {
        ASSERT(!constituentClasses.value().Empty());
        allocator_->Free(constituentClasses.value().begin());
        return nullptr;
    }

    Runtime::GetCurrent()->GetNotificationManager()->ClassLoadEvent(unionClass);

    auto *otherKlass = commonContext->InsertClass(unionClass);
    if (otherKlass != nullptr) {
        FreeClass(unionClass);
        return otherKlass;
    }

    RemoveCreatedClassInExtension(unionClass);
    Runtime::GetCurrent()->GetNotificationManager()->ClassPrepareEvent(unionClass);

    return unionClass;
}

Class *ClassLinker::CreateArrayClass(ClassLinkerExtension *ext, const uint8_t *descriptor, bool needCopyDescriptor,
                                     Class *componentClass)
{
    if (needCopyDescriptor) {
        descriptor = CopyMutf8String(allocator_, descriptor);
        os::memory::LockHolder lock(copiedNamesLock_);
        copiedNames_.push_front(descriptor);
    }

    auto *arrayClass = ext->CreateClass(descriptor, ext->GetArrayClassVTableSize(), ext->GetArrayClassIMTSize(),
                                        ext->GetArrayClassSize());

    if (UNLIKELY(arrayClass == nullptr)) {
        return nullptr;
    }

    arrayClass->SetLoadContext(componentClass->GetLoadContext());

    if (UNLIKELY(!ext->InitializeArrayClass(arrayClass, componentClass))) {
        return nullptr;
    }

    return arrayClass;
}

Class *ClassLinker::LoadArrayClass(const uint8_t *descriptor, bool needCopyDescriptor, ClassLinkerContext *context,
                                   ClassLinkerErrorHandler *errorHandler)
{
    Span<const uint8_t> sp(descriptor, 1);

    Class *componentClass = GetClass(sp.cend(), needCopyDescriptor, context, errorHandler);

    if (componentClass == nullptr) {
        return nullptr;
    }

    if (UNLIKELY(componentClass->GetType().GetId() == panda_file::Type::TypeId::VOID)) {
        OnError(errorHandler, Error::NO_CLASS_DEF, "Try to create array with void component type");
        return nullptr;
    }

    auto *ext = GetExtension(componentClass->GetSourceLang());
    ASSERT(ext != nullptr);

    auto *componentClassContext = componentClass->GetLoadContext();
    ASSERT(componentClassContext != nullptr);
    if (componentClassContext != context) {
        auto *loadedClass = FindLoadedClass(descriptor, componentClassContext);
        if (loadedClass != nullptr) {
            return loadedClass;
        }
    }

    auto *arrayClass = CreateArrayClass(ext, descriptor, needCopyDescriptor, componentClass);
    arrayClass->CalcHaveNoRefsInParents();

    if (UNLIKELY(arrayClass == nullptr)) {
        return nullptr;
    }

    Runtime::GetCurrent()->GetNotificationManager()->ClassLoadEvent(arrayClass);

    auto *otherKlass = componentClassContext->InsertClass(arrayClass);
    if (otherKlass != nullptr) {
        FreeClass(arrayClass);
        return otherKlass;
    }

    RemoveCreatedClassInExtension(arrayClass);
    Runtime::GetCurrent()->GetNotificationManager()->ClassPrepareEvent(arrayClass);

    return arrayClass;
}

static PandaString PandaFilesToString(const PandaVector<const panda_file::File *> &pandaFiles)
{
    PandaStringStream ss;
    ss << "[";

    size_t n = pandaFiles.size();
    for (size_t i = 0; i < n; i++) {
        ss << pandaFiles[i]->GetFilename();

        if (i < n - 1) {
            ss << ", ";
        }
    }

    ss << "]";
    return ss.str();
}

Class *ClassLinker::GetClass(const uint8_t *descriptor, bool needCopyDescriptor, ClassLinkerContext *context,
                             ClassLinkerErrorHandler *errorHandler /* = nullptr */)
{
    ASSERT(context != nullptr);
    ASSERT(descriptor != nullptr);
    ASSERT(!MTManagedThread::ThreadIsMTManagedThread(Thread::GetCurrent()) ||
           !PandaVM::GetCurrent()->GetGC()->IsGCRunning() || PandaVM::GetCurrent()->GetMutatorLock()->HasLock());

    ScopedTrace scopedTrace("ClassLinker::GetClass", isTraceEnabled_);

    Class *cls = FindLoadedClass(descriptor, context);
    if (cls != nullptr) {
        return cls;
    }

    if (ClassHelper::IsUnionDescriptor(descriptor)) {
        return LoadUnionClass(descriptor, needCopyDescriptor, context, errorHandler);
    }

    if (ClassHelper::IsArrayDescriptor(descriptor)) {
        return LoadArrayClass(descriptor, needCopyDescriptor, context, errorHandler);
    }

    if (context->IsBootContext()) {
        if (LookupInFilter(descriptor, errorHandler) == FilterResult::IMPOSSIBLY_HAS) {
            return nullptr;
        }

        panda_file::File::EntityId classId;
        const panda_file::File *pandaFile {nullptr};
        {
            {
                os::memory::LockHolder lock {bootPandaFilesLock_};
                std::tie(classId, pandaFile) = FindClassInPandaFiles(descriptor, bootPandaFiles_);
            }

            if (!classId.IsValid()) {
                PandaStringStream ss;
                {
                    // can't make a wider scope for lock here - will get recursion
                    os::memory::LockHolder lock {bootPandaFilesLock_};
                    ss << "Cannot find class " << descriptor
                       << " in boot panda files: " << PandaFilesToString(bootPandaFiles_);
                }
                OnError(errorHandler, Error::CLASS_NOT_FOUND, ss.str());
                return nullptr;
            }
        }

        return LoadClass(pandaFile, classId, pandaFile->GetStringData(classId).data, context, errorHandler);
    }

    return context->LoadClass(descriptor, needCopyDescriptor, errorHandler);
}

// CC-OFFNXT(huge_method[C++]) solid logic
Class *ClassLinker::GetClass(const panda_file::File &pf, panda_file::File::EntityId id, ClassLinkerContext *context,
                             ClassLinkerErrorHandler *errorHandler /* = nullptr */)
{
    ASSERT(context != nullptr);
    ASSERT(!MTManagedThread::ThreadIsMTManagedThread(Thread::GetCurrent()) ||
           !PandaVM::GetCurrent()->GetGC()->IsGCRunning() || PandaVM::GetCurrent()->GetMutatorLock()->HasLock());

    ScopedTrace scopedTrace("ClassLinker::GetClass", isTraceEnabled_);

    Class *cls = pf.GetPandaCache()->GetClassFromCache(id);
    if (cls != nullptr) {
        return cls;
    }

    const uint8_t *descriptor = pf.GetStringData(id).data;
    cls = FindLoadedClass(descriptor, context);
    if (cls != nullptr) {
        pf.GetPandaCache()->SetClassCache(id, cls);
        return cls;
    }

    if (ClassHelper::IsUnionDescriptor(descriptor)) {
        cls = LoadUnionClass(descriptor, false, context, errorHandler);
        if (LIKELY(cls != nullptr)) {
            pf.GetPandaCache()->SetClassCache(id, cls);
        }
        return cls;
    }

    if (ClassHelper::IsArrayDescriptor(descriptor)) {
        cls = LoadArrayClass(descriptor, false, context, errorHandler);
        if (LIKELY(cls != nullptr)) {
            pf.GetPandaCache()->SetClassCache(id, cls);
        }
        return cls;
    }

    if (context->IsBootContext()) {
        if (LookupInFilter(descriptor, errorHandler) == FilterResult::IMPOSSIBLY_HAS) {
            return nullptr;
        }

        const panda_file::File *pfPtr = nullptr;
        panda_file::File::EntityId extId;
        {
            os::memory::LockHolder lock {bootPandaFilesLock_};
            std::tie(extId, pfPtr) = FindClassInPandaFiles(descriptor, bootPandaFiles_);
        }

        if (!extId.IsValid()) {
            PandaStringStream ss;
            {
                // can't make a wider scope for lock here - will get recursion
                os::memory::LockHolder lock {bootPandaFilesLock_};
                ss << "Cannot find class " << descriptor
                   << " in boot panda files: " << PandaFilesToString(bootPandaFiles_);
            }
            OnError(errorHandler, Error::CLASS_NOT_FOUND, ss.str());
            return nullptr;
        }

        cls = LoadClass(pfPtr, extId, descriptor, context, errorHandler);
        if (LIKELY(cls != nullptr)) {
            pf.GetPandaCache()->SetClassCache(id, cls);
        }
        return cls;
    }

    return context->LoadClass(descriptor, false, errorHandler);
}

Method *ClassLinker::GetMethod(const panda_file::File &pf, panda_file::File::EntityId id,
                               ClassLinkerContext *context /* = nullptr */,
                               ClassLinkerErrorHandler *errorHandler /* = nullptr */)
{
    Method *method = pf.GetPandaCache()->GetMethodFromCache(id);
    if (method != nullptr) {
        return method;
    }
    panda_file::MethodDataAccessor methodDataAccessor(pf, id);

    auto classId = methodDataAccessor.GetClassId();
    if (context == nullptr) {
        panda_file::ClassDataAccessor classDataAccessor(pf, classId);
        auto lang = classDataAccessor.GetSourceLang();
        if (!lang) {
            LOG(INFO, CLASS_LINKER) << "Cannot resolve language context for class_id " << classId << " in file "
                                    << pf.GetFilename();
            return nullptr;
        }
        auto *extension = GetExtension(lang.value());
        context = extension->GetBootContext();
    }

    Class *klass = GetClass(pf, classId, context, errorHandler);

    if (klass == nullptr) {
        auto className = pf.GetStringData(classId).data;
        LOG(INFO, CLASS_LINKER) << "Cannot find class '" << className << "' in ctx " << context;
        return nullptr;
    }
    method = GetMethod(klass, methodDataAccessor, errorHandler);
    if (LIKELY(method != nullptr)) {
        pf.GetPandaCache()->SetMethodCache(id, method);
    }
    return method;
}

Method *ClassLinker::GetMethod(const Method &caller, panda_file::File::EntityId id,
                               ClassLinkerErrorHandler *errorHandler /* = nullptr */)
{
    auto *pf = caller.GetPandaFile();
    ASSERT(pf != nullptr);
    Method *method = pf->GetPandaCache()->GetMethodFromCache(id);
    if (method != nullptr) {
        return method;
    }

    panda_file::MethodDataAccessor methodDataAccessor(*pf, id);
    auto classId = methodDataAccessor.GetClassId();

    auto *context = caller.GetClass()->GetLoadContext();
    auto *ext = GetExtension(caller.GetClass()->GetSourceLang());
    ASSERT(ext != nullptr);
    Class *klass = ext->GetClass(*pf, classId, context, errorHandler);

    if (klass == nullptr) {
        auto className = pf->GetStringData(classId).data;
        LOG(INFO, CLASS_LINKER) << "Cannot find class '" << className << "' in ctx " << context;
        return nullptr;
    }

    method = GetMethod(klass, methodDataAccessor, (errorHandler == nullptr) ? ext->GetErrorHandler() : errorHandler);
    if (LIKELY(method != nullptr)) {
        pf->GetPandaCache()->SetMethodCache(id, method);
    }
    return method;
}

Method *ClassLinker::GetMethod(const Class *klass, const panda_file::MethodDataAccessor &methodDataAccessor,
                               ClassLinkerErrorHandler *errorHandler)
{
    Method *method;
    auto id = methodDataAccessor.GetMethodId();
    const auto &pf = methodDataAccessor.GetPandaFile();

    bool isStatic = methodDataAccessor.IsStatic();
    if (!methodDataAccessor.IsExternal() && klass->GetPandaFile() == &pf) {
        if (klass->IsInterface()) {
            method = isStatic ? klass->GetStaticInterfaceMethod(id) : klass->GetVirtualInterfaceMethod(id);
        } else {
            method = isStatic ? klass->GetStaticClassMethod(id) : klass->GetVirtualClassMethod(id);
        }

        if (method == nullptr) {
            Method::Proto proto(pf, methodDataAccessor.GetProtoId());
            PandaStringStream ss;
            ss << "Cannot find method '" << methodDataAccessor.GetName().data << " " << proto.GetSignature(true)
               << "' in class '" << klass->GetName() << "'";
            OnError(errorHandler, Error::METHOD_NOT_FOUND, ss.str());
            return nullptr;
        }

        return method;
    }

    auto name = methodDataAccessor.GetName();
    Method::Proto proto(pf, methodDataAccessor.GetProtoId());
    if (klass->IsInterface()) {
        method = isStatic ? klass->GetStaticInterfaceMethodByName(name, proto)
                          : klass->GetVirtualInterfaceMethodByName(name, proto);
    } else {
        method =
            isStatic ? klass->GetStaticClassMethodByName(name, proto) : klass->GetVirtualClassMethodByName(name, proto);
        if (method == nullptr && klass->IsAbstract()) {
            method = klass->GetInterfaceMethod(name, proto);
        }
    }

    if (method == nullptr) {
        PandaStringStream ss;
        ss << "Cannot find method '" << methodDataAccessor.GetName().data << " " << proto.GetSignature(true)
           << "' in class '" << klass->GetName() << "'";
        OnError(errorHandler, Error::METHOD_NOT_FOUND, ss.str());
        return nullptr;
    }

    LOG_IF(method->IsStatic() != methodDataAccessor.IsStatic(), FATAL, CLASS_LINKER)
        << "Expected ACC_STATIC for method " << name.data << " in class " << klass->GetName()
        << " does not match loaded value";

    return method;
}

Field *ClassLinker::GetFieldById(Class *klass, const panda_file::FieldDataAccessor &fieldDataAccessor,
                                 ClassLinkerErrorHandler *errorHandler, bool isStatic)
{
    auto &pf = fieldDataAccessor.GetPandaFile();
    auto id = fieldDataAccessor.GetFieldId();

    Field *field = isStatic ? klass->FindStaticFieldById(id) : klass->FindInstanceFieldById(id);

    if (field == nullptr) {
        PandaStringStream ss;
        ss << "Cannot find field '" << pf.GetStringData(fieldDataAccessor.GetNameId()).data << "' in class '"
           << klass->GetName() << "'";
        OnError(errorHandler, Error::FIELD_NOT_FOUND, ss.str());
        return nullptr;
    }

    pf.GetPandaCache()->SetFieldCache(id, field);
    return field;
}

Field *ClassLinker::GetFieldBySignature(Class *klass, const panda_file::FieldDataAccessor &fieldDataAccessor,
                                        ClassLinkerErrorHandler *errorHandler, bool isStatic)
{
    auto &pf = fieldDataAccessor.GetPandaFile();
    auto id = fieldDataAccessor.GetFieldId();
    auto fieldName = pf.GetStringData(fieldDataAccessor.GetNameId());
    auto fieldType = panda_file::Type::GetTypeFromFieldEncoding(fieldDataAccessor.GetType());
    auto filter = [&fieldDataAccessor, &fieldType, &fieldName, &id, &pf](const Field &fld) {
        if (fieldType == fld.GetType() && fieldName == fld.GetName()) {
            if (!fieldType.IsReference()) {
                return true;
            }

            // compare field class type
            if (&pf == fld.GetPandaFile() && id == fld.GetFileId()) {
                return true;
            }
            auto typeId = panda_file::FieldDataAccessor::GetTypeId(*fld.GetPandaFile(), fld.GetFileId());
            if (pf.GetStringData(panda_file::File::EntityId(fieldDataAccessor.GetType())) ==
                fld.GetPandaFile()->GetStringData(typeId)) {
                return true;
            }
        }
        return false;
    };
    Field *field = isStatic ? klass->FindStaticField(filter) : klass->FindInstanceField(filter);

    if (field == nullptr) {
        PandaStringStream ss;
        ss << "Cannot find field '" << fieldName.data << "' in class '" << klass->GetName() << "'";
        OnError(errorHandler, Error::FIELD_NOT_FOUND, ss.str());
        return nullptr;
    }

    pf.GetPandaCache()->SetFieldCache(id, field);
    return field;
}

Field *ClassLinker::GetField(const panda_file::File &pf, panda_file::File::EntityId id, bool isStatic,
                             ClassLinkerContext *context /* = nullptr */,
                             ClassLinkerErrorHandler *errorHandler /* = nullptr */)
{
    Field *field = pf.GetPandaCache()->GetFieldFromCache(id);
    if (field != nullptr) {
        return field;
    }
    panda_file::FieldDataAccessor fieldDataAccessor(pf, id);

    Class *klass = GetClass(pf, fieldDataAccessor.GetClassId(), context, errorHandler);

    if (klass == nullptr) {
        auto className = pf.GetStringData(fieldDataAccessor.GetClassId()).data;
        ASSERT(className != nullptr);
        LOG(INFO, CLASS_LINKER) << "Cannot find class '" << className << "' in ctx " << context;
        return nullptr;
    }

    if (!fieldDataAccessor.IsExternal() && klass->GetPandaFile() == &pf) {
        field = GetFieldById(klass, fieldDataAccessor, errorHandler, isStatic);
    } else {
        field = GetFieldBySignature(klass, fieldDataAccessor, errorHandler, isStatic);
    }
    return field;
}

bool ClassLinker::InitializeClass(ManagedThread *thread, Class *klass)
{
    ASSERT(klass != nullptr);
    if (klass->IsInitialized()) {
        return true;
    }

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*klass);
    return ctx.InitializeClass(this, thread, klass);
}

size_t ClassLinker::NumLoadedClasses()
{
    size_t sum = 0;

    for (auto &ext : extensions_) {
        if (ext == nullptr) {
            continue;
        }

        sum += ext->NumLoadedClasses();
    }

    return sum;
}

void ClassLinker::VisitLoadedClasses(size_t flag)
{
    for (auto &ext : extensions_) {
        if (ext == nullptr) {
            continue;
        }
        ext->VisitLoadedClasses(flag);
    }
}

Field *ClassLinker::GetField(const Method &caller, panda_file::File::EntityId id, bool isStatic,
                             ClassLinkerErrorHandler *errorHandler /* = nullptr */)
{
    Field *field = caller.GetPandaFile()->GetPandaCache()->GetFieldFromCache(id);
    if (field != nullptr) {
        return field;
    }
    auto *ext = GetExtension(caller.GetClass()->GetSourceLang());
    ASSERT(ext != nullptr);
    field = GetField(*caller.GetPandaFile(), id, isStatic, caller.GetClass()->GetLoadContext(),
                     (errorHandler == nullptr) ? ext->GetErrorHandler() : errorHandler);
    if (LIKELY(field != nullptr)) {
        caller.GetPandaFile()->GetPandaCache()->SetFieldCache(id, field);
    }
    return field;
}

Field *ClassLinker::GetField(Class *klass, const panda_file::FieldDataAccessor &fda, bool isStatic,
                             ClassLinkerErrorHandler *errorHandler)
{
    if (klass == nullptr) {
        return nullptr;
    }
    Field *field {nullptr};
    auto pf = &fda.GetPandaFile();
    if (!fda.IsExternal() && (klass->GetPandaFile() == pf)) {
        field = GetFieldById(klass, fda, errorHandler, isStatic);
    } else {
        field = GetFieldBySignature(klass, fda, errorHandler, isStatic);
    }
    if (LIKELY(field != nullptr)) {
        pf->GetPandaCache()->SetFieldCache(fda.GetFieldId(), field);
    }
    return field;
}

void ClassLinker::RemoveCreatedClassInExtension(Class *klass)
{
    if (klass == nullptr) {
        return;
    }
    auto ext = GetExtension(klass->GetSourceLang());
    if (ext != nullptr) {
        ext->OnClassPrepared(klass);
    }
}

void ClassLinker::TryReLinkAotCodeForBoot(const panda_file::File *pf, const compiler::AotPandaFile *aotPfile,
                                          panda_file::SourceLang language)
{
    auto bootContext = GetExtension(language)->GetBootContext();

    bootContext->EnumerateClasses([pf, aotPfile](Class *klass) {
        ASSERT(aotPfile != nullptr);
        compiler::AotClass aotClass = aotPfile->GetClass(klass->GetFileId().GetOffset());
        if (!aotClass.IsValid()) {
            return true;
        }

        LOG(DEBUG, RUNTIME) << "TryReLinkAotCode() for boot class: " << klass->GetName();
        Span<Method> methods = klass->GetMethods();
        panda_file::ClassDataAccessor cda(*pf, klass->GetFileId());
        size_t methodIdx = 0;
        size_t smethodIdx = klass->GetNumVirtualMethods();
        size_t vmethodIdx = 0;

        cda.EnumerateMethods(
            [&smethodIdx, &vmethodIdx, &methods, &methodIdx, aotClass](panda_file::MethodDataAccessor &mda) {
                Method &method = mda.IsStatic() ? methods[smethodIdx++] : methods[vmethodIdx++];
                MaybeLinkMethodToAotCode(&method, aotClass, methodIdx);
                methodIdx++;
            });
        return true;
    });
}

void ClassLinker::AddBootClassFilter(const panda_file::File *bootPandaFile)
{
    os::memory::LockHolder lock {bootClassFilterLock_};
    auto classes = bootPandaFile->GetClasses();
    LOG(DEBUG, CLASS_LINKER) << "number of class in [" << bootPandaFile->GetFilename() << "] is " << classes.size();

    for (unsigned int classe : classes) {
        auto classId = panda_file::File::EntityId(classe);
        const uint8_t *className = bootPandaFile->GetStringData(classId).data;
        bootClassFilter_.Add(className);
    }
}

ClassLinker::FilterResult ClassLinker::LookupInFilter(const uint8_t *descriptor, ClassLinkerErrorHandler *errorHandler)
{
    if (!Runtime::GetOptions().IsUseBootClassFilter()) {
        return FilterResult::DISABLED;
    }

    bool found = true;
    {
        os::memory::LockHolder lock {bootClassFilterLock_};
        found = bootClassFilter_.PossiblyContains(descriptor);
    }

    if (!found) {
        if (errorHandler == nullptr) {
            return FilterResult::IMPOSSIBLY_HAS;
        }

        PandaStringStream s;
        s << "Cannot find class " << descriptor << " in boot class bloom filter";
        OnError(errorHandler, Error::CLASS_NOT_FOUND, s.str());
        return FilterResult::IMPOSSIBLY_HAS;
    }

    return FilterResult::POSSIBLY_HAS;
}
}  // namespace ark
