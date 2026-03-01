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

#include "runtime/handle_scope-inl.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/monitor_object_lock.h"
#include "runtime/monitor.h"
#include "libarkfile/panda_cache.h"

#include "runtime/hotreload/hotreload.h"
#include "libarkbase/utils/utils.h"

namespace ark::hotreload {

static Error GetHotreloadErrorByFlag(uint32_t flag)
{
    if (flag == ChangesFlags::F_NO_STRUCT_CHANGES) {
        return Error::NONE;
    }
    if ((flag & ChangesFlags::F_ACCESS_FLAGS) != 0U) {
        return Error::CLASS_MODIFIERS;
    }
    if ((flag & ChangesFlags::F_FIELDS_TYPE) != 0U || (flag & ChangesFlags::F_FIELDS_AMOUNT) != 0U) {
        return Error::FIELD_CHANGED;
    }
    if ((flag & ChangesFlags::F_INHERITANCE) != 0U || (flag & ChangesFlags::F_INTERFACES) != 0U) {
        return Error::HIERARCHY_CHANGED;
    }
    if ((flag & ChangesFlags::F_METHOD_SIGN) != 0U) {
        return Error::METHOD_SIGN;
    }
    if ((flag & ChangesFlags::F_METHOD_ADDED) != 0U) {
        return Error::METHOD_ADDED;
    }
    if ((flag & ChangesFlags::F_METHOD_DELETED) != 0U) {
        return Error::METHOD_DELETED;
    }

    return Error::UNSUPPORTED_CHANGES;
}

// ---------------------------------------------------------
// ------------------ Hotreload Class API ------------------
// ---------------------------------------------------------

const panda_file::File *ArkHotreloadBase::ReadAndOwnPandaFileFromFile(const char *location)
{
    ASSERT_MANAGED_CODE();
    ASSERT(thread_ == ManagedThread::GetCurrent());

    std::unique_ptr<const panda_file::File> pf = panda_file::OpenPandaFile(location);
    const panda_file::File *ptr = pf.get();
    pandaFiles_.push_back(std::move(pf));
    return ptr;
}

const panda_file::File *ArkHotreloadBase::ReadAndOwnPandaFileFromMemory(const void *buffer, size_t buffSize)
{
    ASSERT_MANAGED_CODE();
    ASSERT(thread_ == ManagedThread::GetCurrent());

    std::unique_ptr<const panda_file::File> pf = panda_file::OpenPandaFileFromMemory(buffer, buffSize);
    auto ptr = pf.get();
    pandaFiles_.push_back(std::move(pf));
    return ptr;
}

Error ArkHotreloadBase::ProcessHotreload()
{
    ASSERT_MANAGED_CODE();
    ASSERT(thread_ == ManagedThread::GetCurrent());

    Error err = ValidateClassesHotreloadPossibility();
    if (err != Error::NONE) {
        return err;
    }

    for (auto &hCls : classes_) {
        auto changesType = RecognizeHotreloadType(&hCls);
        if (changesType == Type::STRUCTURAL) {
            LOG(ERROR, HOTRELOAD) << "Class " << hCls.className_
                                  << " has structural changes. Structural changes is unsafe for hotreload.";
            return GetHotreloadErrorByFlag(hCls.fChanges);
        }
    }

    /*
     * On this point all classes were verified and hotreload recognized like possible
     * From now all classes should be reloaded anyway and hotreload cannot be reversed
     */
    {
        ScopedSuspendAllThreadsRunning ssat(PandaVM::GetCurrent()->GetRendezvous());
        ASSERT_MANAGED_CODE();
        if (Runtime::GetCurrent()->IsJitEnabled()) {
            /*
             * In case JIT is enabled while hotreloading we must suspend it and clear compiled methods
             * Currently JIT is not running with hotreload
             */
            UNREACHABLE();
        }
        ASSERT(thread_->GetVM()->GetThreadManager() != nullptr);
        PandaVM::GetCurrent()->GetThreadManager()->EnumerateThreads([this](ManagedThread *thread) {
            (void)this;  // [[maybe_unused]] in lambda capture list is not possible
            ASSERT(thread->GetThreadLang() == lang_);
            thread->GetInterpreterCache()->Clear();
            return true;
        });
        for (auto &hCls : classes_) {
            ReloadClassNormal(&hCls);
        }

        auto classLinker = Runtime::GetCurrent()->GetClassLinker();
        // Updating VTable should be before adding obsolete classes for performance reason
        UpdateVtablesInRuntimeClasses(classLinker);
        AddLoadedPandaFilesToRuntime(classLinker);
        AddObsoleteClassesToRuntime(classLinker);

        LangSpecificHotreloadPart();
    }
    return Error::NONE;
}

ArkHotreloadBase::ArkHotreloadBase(ManagedThread *mthread, panda_file::SourceLang lang) : lang_(lang), thread_(mthread)
{
    /*
     * Scoped object that switch to managed code is language dependent
     * So is should be constructed in superclass
     */
    ASSERT_NATIVE_CODE();
    ASSERT(thread_ != nullptr);
    ASSERT(thread_ == ManagedThread::GetCurrent());
    ASSERT(lang_ == thread_->GetThreadLang());
}

/* virtual */
ArkHotreloadBase::~ArkHotreloadBase()
{
    ASSERT_NATIVE_CODE();
    ASSERT(thread_ == ManagedThread::GetCurrent());

    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    for (auto &hCls : classes_) {
        if (hCls.tmpClass != nullptr) {
            classLinker->FreeClass(hCls.tmpClass);
        }
    }
}

// ---------------------------------------------------------
// ----------------------- Validators ----------------------
// ---------------------------------------------------------

Error ArkHotreloadBase::ValidateClassesHotreloadPossibility()
{
    ASSERT_MANAGED_CODE();
    ASSERT(thread_ == ManagedThread::GetCurrent());

    auto err = this->LangSpecificValidateClasses();
    if (err != Error::NONE) {
        return err;
    }

    for (const auto &hCls : classes_) {
        Error returnErr = ValidateClassForHotreload(hCls);
        if (returnErr != Error::NONE) {
            return returnErr;
        }
    }

    return Error::NONE;
}

std::optional<uint32_t> GetLockOwnerThreadId(Class *cls)
{
    if (Monitor::HoldsLock(cls->GetManagedObject()) != 0U) {
        return Monitor::GetLockOwnerOsThreadID(cls->GetManagedObject());
    }
    return {};
}

Error ArkHotreloadBase::ValidateClassForHotreload(const ClassContainment &hCls)
{
    Class *clazz = hCls.tmpClass;
    Class *runtimeClass = hCls.loadedClass;
    if (clazz == nullptr) {
        LOG(ERROR, HOTRELOAD) << "Class " << hCls.className_ << " are failed to be initialized";
        return Error::INTERNAL;
    }

    /*
     * Check if class is not resolved
     * In case class have a lock holded by the same thread it will lead to deadlock
     * In case class is not initialized we should initialize it before hotreloading
     */
    if (!runtimeClass->IsInitialized()) {
        if (GetLockOwnerThreadId(runtimeClass) == thread_->GetId()) {
            LOG(ERROR, HOTRELOAD) << "Class lock " << hCls.className_ << " are already owned by this thread.";
            return Error::INTERNAL;
        }
        auto classLinker = Runtime::GetCurrent()->GetClassLinker();
        if (!classLinker->InitializeClass(thread_, runtimeClass)) {
            LOG(ERROR, HOTRELOAD) << "Class " << hCls.className_ << " cannot be initialized in runtime.";
            return Error::INTERNAL;
        }
    }

    if (clazz->IsInterface()) {
        LOG(ERROR, HOTRELOAD) << "Class " << hCls.className_ << " is an interface class. Cannot modify.";
        return Error::CLASS_UNMODIFIABLE;
    }
    if (clazz->IsProxy()) {
        LOG(ERROR, HOTRELOAD) << "Class " << hCls.className_ << " is a proxy class. Cannot modify.";
        return Error::CLASS_UNMODIFIABLE;
    }
    if (clazz->IsArrayClass()) {
        LOG(ERROR, HOTRELOAD) << "Class " << hCls.className_ << " is an array class. Cannot modify.";
        return Error::CLASS_UNMODIFIABLE;
    }
    if (clazz->IsStringClass()) {
        LOG(ERROR, HOTRELOAD) << "Class " << hCls.className_ << " is a string class. Cannot modify.";
        return Error::CLASS_UNMODIFIABLE;
    }
    if (clazz->IsPrimitive()) {
        LOG(ERROR, HOTRELOAD) << "Class " << hCls.className_ << " is a primitive class. Cannot modify.";
        return Error::CLASS_UNMODIFIABLE;
    }

    return Error::NONE;
}

Type ArkHotreloadBase::RecognizeHotreloadType(ClassContainment *hCls)
{
    /*
     * Checking for the changes:
     *  - Inheritance changed
     *  - Access flags changed
     *  - Field added/deleted
     *  - Field type changed
     *  - Method added/deleted
     *  - Method signature changed
     *
     * In case there are any of changes above Type is Structural
     * Otherwise it's normal changes
     */
    if (InheritanceChangesCheck(hCls) == Type::STRUCTURAL) {
        return Type::STRUCTURAL;
    }
    if (FlagsChangesCheck(hCls) == Type::STRUCTURAL) {
        return Type::STRUCTURAL;
    }
    if (FieldChangesCheck(hCls) == Type::STRUCTURAL) {
        return Type::STRUCTURAL;
    }
    if (MethodChangesCheck(hCls) == Type::STRUCTURAL) {
        return Type::STRUCTURAL;
    }

    return Type::NORMAL;
}

Type ArkHotreloadBase::InheritanceChangesCheck(ClassContainment *hCls)
{
    Class *tmpClass = hCls->tmpClass;
    Class *runtimeClass = hCls->loadedClass;
    if (tmpClass->GetBase() != runtimeClass->GetBase()) {
        hCls->fChanges |= ChangesFlags::F_INHERITANCE;
        return Type::STRUCTURAL;
    }

    auto newIfaces = tmpClass->GetInterfaces();
    auto oldIfaces = runtimeClass->GetInterfaces();
    if (newIfaces.size() != oldIfaces.size()) {
        hCls->fChanges |= ChangesFlags::F_INTERFACES;
        return Type::STRUCTURAL;
    }

    PandaUnorderedSet<Class *> ifaces;
    for (auto iface : oldIfaces) {
        ifaces.insert(iface);
    }
    for (auto iface : newIfaces) {
        if (ifaces.find(iface) == ifaces.end()) {
            hCls->fChanges |= ChangesFlags::F_INTERFACES;
            return Type::STRUCTURAL;
        }
        ifaces.erase(iface);
    }
    if (!ifaces.empty()) {
        hCls->fChanges |= ChangesFlags::F_INTERFACES;
        return Type::STRUCTURAL;
    }

    return Type::NORMAL;
}

Type ArkHotreloadBase::FlagsChangesCheck(ClassContainment *hCls)
{
    Class *tmpClass = hCls->tmpClass;
    Class *runtimeClass = hCls->loadedClass;

    // NOTE(m.strizhak) research that maybe there are flags that can be changed keeping normal type
    if (tmpClass->GetRuntimeFlags() != runtimeClass->GetRuntimeFlags()) {
        hCls->fChanges |= ChangesFlags::F_ACCESS_FLAGS;
        return Type::STRUCTURAL;
    }

    if (tmpClass->GetAccessFlags() != runtimeClass->GetAccessFlags()) {
        hCls->fChanges |= ChangesFlags::F_ACCESS_FLAGS;
        return Type::STRUCTURAL;
    }

    return Type::NORMAL;
}

Type ArkHotreloadBase::FieldChangesCheck(ClassContainment *hCls)
{
    Class *tmpClass = hCls->tmpClass;
    Class *runtimeClass = hCls->loadedClass;

    auto oldFields = runtimeClass->GetFields();
    auto newFields = tmpClass->GetFields();
    if (newFields.size() != oldFields.size()) {
        hCls->fChanges |= ChangesFlags::F_FIELDS_AMOUNT;
        return Type::STRUCTURAL;
    }

    FieldIdTable fieldIdTable;
    PandaUnorderedMap<PandaString, Field *> fieldsTable;
    for (auto &oldField : oldFields) {
        PandaString fieldName(utf::Mutf8AsCString(oldField.GetName().data));
        fieldsTable.insert({fieldName, &oldField});
    }

    for (auto &newField : newFields) {
        PandaString fieldName(utf::Mutf8AsCString(newField.GetName().data));
        auto oldIt = fieldsTable.find(fieldName);
        if (oldIt == fieldsTable.end()) {
            hCls->fChanges |= ChangesFlags::F_FIELDS_AMOUNT;
            return Type::STRUCTURAL;
        }

        if (oldIt->second->GetAccessFlags() != newField.GetAccessFlags() ||
            oldIt->second->GetTypeId() != newField.GetTypeId()) {
            hCls->fChanges |= ChangesFlags::F_FIELDS_TYPE;
            return Type::STRUCTURAL;
        }

        fieldIdTable[fieldsTable[fieldName]->GetFileId()] = newField.GetFileId();
        fieldsTable.erase(fieldName);
    }

    fieldsTables_[runtimeClass] = std::move(fieldIdTable);

    if (!fieldsTable.empty()) {
        hCls->fChanges |= ChangesFlags::F_FIELDS_AMOUNT;
        return Type::STRUCTURAL;
    }

    return Type::NORMAL;
}

static inline uint32_t GetFileAccessFlags(const Method &method)
{
    return method.GetAccessFlags() & ACC_FILE_MASK;
}

Type ArkHotreloadBase::MethodChangesCheck(ClassContainment *hCls)
{
    Class *tmpClass = hCls->tmpClass;
    Class *runtimeClass = hCls->loadedClass;

    auto oldMethods = runtimeClass->GetMethods();
    auto newMethods = tmpClass->GetMethods();
    if (newMethods.size() > oldMethods.size() ||
        tmpClass->GetNumVirtualMethods() > runtimeClass->GetNumVirtualMethods()) {
        hCls->fChanges |= ChangesFlags::F_METHOD_ADDED;
        return Type::STRUCTURAL;
    }

    if (newMethods.size() < oldMethods.size() ||
        tmpClass->GetNumVirtualMethods() < runtimeClass->GetNumVirtualMethods()) {
        hCls->fChanges |= ChangesFlags::F_METHOD_DELETED;
        return Type::STRUCTURAL;
    }

    for (auto &newMethod : newMethods) {
        bool isNameFound = false;
        bool isExactFound = false;
        for (auto &oldMethod : oldMethods) {
            PandaString oldName = utf::Mutf8AsCString(oldMethod.GetName().data);
            PandaString newName = utf::Mutf8AsCString(newMethod.GetName().data);
            if (oldName != newName) {
                continue;
            }
            isNameFound = true;
            if (oldMethod.GetProto() == newMethod.GetProto() &&
                GetFileAccessFlags(oldMethod) == GetFileAccessFlags(newMethod)) {
                methodsTable_[&oldMethod] = &newMethod;
                isExactFound = true;
                break;
            }
        }

        if (isNameFound) {
            if (isExactFound) {
                continue;
            }
            hCls->fChanges |= ChangesFlags::F_METHOD_SIGN;
            return Type::STRUCTURAL;
        }
        hCls->fChanges |= ChangesFlags::F_METHOD_ADDED;
        return Type::STRUCTURAL;
    }
    return Type::NORMAL;
}

// ---------------------------------------------------------
// ----------------------- Reloaders -----------------------
// ---------------------------------------------------------

// This method is used under assert. So in release build it's unused
[[maybe_unused]] static bool VerifyClassConsistency(const Class *cls)
{
    ASSERT(cls);

    const auto pf = cls->GetPandaFile();
    auto methods = cls->GetMethods();
    auto fields = cls->GetFields();
    const uint8_t *descriptor = cls->GetDescriptor();

    if (cls->GetFileId().GetOffset() != pf->GetClassId(descriptor).GetOffset()) {
        return false;
    }

    for (const auto &method : methods) {
        panda_file::MethodDataAccessor mda(*pf, method.GetFileId());
        panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
        if (mda.GetName() != method.GetName()) {
            return false;
        }
        if (method.GetClass() != cls) {
            return false;
        }
    }

    for (const auto &field : fields) {
        panda_file::FieldDataAccessor fda(*pf, field.GetFileId());
        if (field.GetClass() != cls) {
            return false;
        }
    }

    return true;
}

static void UpdateClassPtrInMethods(Span<Method> methods, Class *cls)
{
    for (auto &method : methods) {
        method.SetClass(cls);
    }
}

static void UpdateClassPtrInFields(Span<Field> fields, Class *cls)
{
    for (auto &field : fields) {
        field.SetClass(cls);
    }
}

static void UpdatePandaFileInClass(Class *runtimeClass, const panda_file::File *pf)
{
    const uint8_t *descriptor = runtimeClass->GetDescriptor();
    panda_file::File::EntityId classId = pf->GetClassId(descriptor);
    runtimeClass->SetPandaFile(pf);
    runtimeClass->SetFileId(classId);
    runtimeClass->SetClassIndex(pf->GetClassIndex(classId));
    runtimeClass->SetMethodIndex(pf->GetMethodIndex(classId));
    runtimeClass->SetFieldIndex(pf->GetFieldIndex(classId));
}

/*
 * Obsolete methods should be saved by temporary class 'cause it might be continue executing
 * Updating class pointers in methods to keep it consistent with class and panda file
 */
static void UpdateMethods(Class *runtimeClass, Class *tmpClass)
{
    auto newMethods = tmpClass->GetMethodsWithCopied();
    auto oldMethods = runtimeClass->GetMethodsWithCopied();
    uint32_t numVmethods = tmpClass->GetNumVirtualMethods();
    uint32_t numCmethods = tmpClass->GetNumCopiedMethods();
    uint32_t numSmethods = newMethods.size() - numVmethods - numCmethods;
    UpdateClassPtrInMethods(newMethods, runtimeClass);
    UpdateClassPtrInMethods(oldMethods, tmpClass);
    runtimeClass->SetMethods(newMethods, numVmethods, numSmethods);
    tmpClass->SetMethods(oldMethods, numVmethods, numSmethods);
}

/*
 * Obsolete fields should be saved by temporary class 'cause it might be used by obselete methods
 * Updating class pointers in fields to keep it consistent with class and panda file
 */
static void UpdateFields(Class *runtimeClass, Class *tmpClass)
{
    auto newFields = tmpClass->GetFields();
    auto oldFields = runtimeClass->GetFields();
    uint32_t numSfields = tmpClass->GetNumStaticFields();
    UpdateClassPtrInFields(newFields, runtimeClass);
    UpdateClassPtrInFields(oldFields, tmpClass);
    runtimeClass->SetFields(newFields, numSfields);
    tmpClass->SetFields(oldFields, numSfields);
}

static void UpdateIfaces(Class *runtimeClass, Class *tmpClass)
{
    auto newIfaces = tmpClass->GetInterfaces();
    auto oldIfaces = runtimeClass->GetInterfaces();
    runtimeClass->SetInterfaces(newIfaces);
    tmpClass->SetInterfaces(oldIfaces);
}

/*
 * Tables is being copied to runtime class to be possible to resolve virtual calls
 * No need to copy tables to temporary class 'cause new virtual calls should be resolved from runtime class
 */
static void UpdateTables(Class *runtimeClass, Class *tmpClass)
{
    ASSERT(tmpClass->GetIMTSize() == runtimeClass->GetIMTSize());
    ASSERT(tmpClass->GetVTableSize() == runtimeClass->GetVTableSize());
    ITable oldItable = runtimeClass->GetITable();
    Span<Method *> oldVtable = runtimeClass->GetVTable();
    Span<Method *> newVtable = tmpClass->GetVTable();
    Span<Method *> oldImt = runtimeClass->GetIMT();
    Span<Method *> newImt = tmpClass->GetIMT();
    runtimeClass->SetITable(tmpClass->GetITable());
    tmpClass->SetITable(oldItable);
    if (!oldVtable.empty()) {
        MemcpyUnsafe(oldVtable.begin(), newVtable.begin(), oldVtable.size() * sizeof(void *));
    }
    if (!oldImt.empty()) {
        MemcpyUnsafe(oldImt.begin(), newImt.begin(), oldImt.size() * sizeof(void *));
    }
}

void ArkHotreloadBase::ReloadClassNormal(const ClassContainment *hCls)
{
    ASSERT(thread_->GetVM()->GetThreadManager() != nullptr);
    ASSERT(!thread_->GetVM()->GetThreadManager()->IsRunningThreadExist());

    /*
     * Take class loading lock
     * Clear interpreter cache
     * Update runtime class header:
     *   - Panda file and its id
     *   - methods
     *   - fields
     *   - ifaces
     *   - tables
     *
     * Then adding obsolete classes to special area in class linker
     * to be able to continue executing obsolete methods after hotreloading
     */
    Class *tmpClass = hCls->tmpClass;
    Class *runtimeClass = hCls->loadedClass;

    // Locking class
    HandleScope<ObjectHeader *> scope(thread_);
    VMHandle<ObjectHeader> managedClassObjHandle(thread_, runtimeClass->GetManagedObject());
    ::ark::ObjectLock lock(managedClassObjHandle.GetPtr());
    const panda_file::File *newPf = hCls->pf;
    const panda_file::File *oldPf = runtimeClass->GetPandaFile();

    oldPf->GetPandaCache()->Clear();

    reloadedClasses_.insert(runtimeClass);
    UpdatePandaFileInClass(tmpClass, oldPf);
    UpdatePandaFileInClass(runtimeClass, newPf);
    UpdateMethods(runtimeClass, tmpClass);
    UpdateFields(runtimeClass, tmpClass);
    UpdateIfaces(runtimeClass, tmpClass);
    UpdateTables(runtimeClass, tmpClass);

    ASSERT(VerifyClassConsistency(runtimeClass));
    ASSERT(VerifyClassConsistency(tmpClass));
}

void ArkHotreloadBase::UpdateVtablesInRuntimeClasses(ClassLinker *classLinker)
{
    ASSERT(thread_->GetVM()->GetThreadManager() != nullptr);
    ASSERT(!thread_->GetVM()->GetThreadManager()->IsRunningThreadExist());

    auto updateVtable = [this](Class *cls) {
        auto vtable = cls->GetVTable();

        if (reloadedClasses_.find(cls) != reloadedClasses_.end()) {
            // This table is already actual
            return true;
        }

        for (auto &methodPtr : vtable) {
            if (methodsTable_.find(methodPtr) != methodsTable_.end()) {
                methodPtr = methodsTable_[methodPtr];
            }
        }
        return true;
    };
    classLinker->GetExtension(lang_)->EnumerateClasses(updateVtable);
}

void ArkHotreloadBase::AddLoadedPandaFilesToRuntime(ClassLinker *classLinker)
{
    ASSERT(thread_->GetVM()->GetThreadManager() != nullptr);
    ASSERT(!thread_->GetVM()->GetThreadManager()->IsRunningThreadExist());

    for (auto &ptrPf : pandaFiles_) {
        classLinker->AddPandaFile(std::move(ptrPf));
    }
    pandaFiles_.clear();
}

void ArkHotreloadBase::AddObsoleteClassesToRuntime(ClassLinker *classLinker)
{
    ASSERT(thread_->GetVM()->GetThreadManager() != nullptr);
    ASSERT(!thread_->GetVM()->GetThreadManager()->IsRunningThreadExist());

    /*
     * Sending all classes in one vector to avoid holding lock for every single class
     * It should be faster because all threads are still stopped
     */
    PandaVector<Class *> obsoleteClasses;
    for (const auto &hCls : classes_) {
        if (hCls.tmpClass != nullptr) {
            ASSERT(hCls.tmpClass->GetSourceLang() == lang_);
            obsoleteClasses.push_back(hCls.tmpClass);
        }
    }
    classLinker->GetExtension(lang_)->AddObsoleteClass(obsoleteClasses);
    classes_.clear();
}

}  // namespace ark::hotreload
