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
#ifndef PANDA_RUNTIME_CLASS_LINKER_EXTENSION_H_
#define PANDA_RUNTIME_CLASS_LINKER_EXTENSION_H_

#include "libarkbase/os/mutex.h"
#include "libarkfile/file.h"
#include "libarkfile/file_items.h"
#include "runtime/class_linker_context.h"
#include "runtime/include/class_root.h"
#include "runtime/include/class.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark {

class ClassLinker;
class ClassLinkerErrorHandler;

/**
 * @brief Base class for class loading extensions.
 * Plugins must extend this class in order to define language-specific operations with classes.
 * `ClassLinkerExtension` instances are owned by `ClassLinker` in a one-to-many relationship.
 */
class ClassLinkerExtension {
public:
    explicit ClassLinkerExtension(panda_file::SourceLang lang) : lang_(lang), bootContext_(this) {}

    virtual ~ClassLinkerExtension();

    bool Initialize(ClassLinker *classLinker, bool compressedStringEnabled);

    bool InitializeFinish();

    bool InitializeRoots(ManagedThread *thread);

    virtual bool InitializeArrayClass(Class *arrayClass, Class *componentClass) = 0;

    virtual bool InitializeUnionClass([[maybe_unused]] Class *unionClass,
                                      [[maybe_unused]] Span<Class *> constituentClasses)
    {
        return false;
    }

    virtual void InitializeSyntheticClass(Class *synClass) = 0;

    virtual void InitializePrimitiveClass(Class *primitiveClass) = 0;

    virtual size_t GetClassVTableSize(ClassRoot root) = 0;

    virtual size_t GetClassIMTSize(ClassRoot root) = 0;

    virtual size_t GetClassSize(ClassRoot root) = 0;

    virtual size_t GetArrayClassVTableSize() = 0;

    virtual size_t GetArrayClassIMTSize() = 0;

    virtual size_t GetArrayClassSize() = 0;

    virtual Class *CreateClass(const uint8_t *descriptor, size_t vtableSize, size_t imtSize, size_t size) = 0;

    virtual void FreeClass(Class *klass) = 0;

    virtual bool InitializeClass(Class *klass) = 0;

    virtual bool InitializeClass(Class *klass, [[maybe_unused]] ClassLinkerErrorHandler *handler)
    {
        return InitializeClass(klass);
    }

    virtual const void *GetNativeEntryPointFor(Method *method) const = 0;

    virtual bool CanThrowException(const Method *method) const = 0;

    virtual bool IsMethodNativeApi([[maybe_unused]] const Method *method) const
    {
        return false;
    }

    virtual bool IsNecessarySwitchThreadState([[maybe_unused]] const Method *method) const
    {
        return false;
    }

    virtual bool CanNativeMethodUseObjects([[maybe_unused]] const Method *method) const
    {
        return false;
    }

    virtual ClassLinkerErrorHandler *GetErrorHandler() = 0;

    virtual ClassLinkerContext *CreateApplicationClassLinkerContext(const PandaVector<PandaString> &path);

    virtual ClassLinkerContext *GetCommonContext([[maybe_unused]] Span<Class *> classes)
    {
        UNREACHABLE();
    }

    virtual std::optional<Span<Method>> GenerateProxyClassMethods([[maybe_unused]] Class *proxyKlass,
                                                                  [[maybe_unused]] Span<Method *> rawMethods)
    {
        UNREACHABLE();
    }

    virtual std::optional<PandaVector<Method *>> BuildProxyClassMethodsSpan([[maybe_unused]] ITable itable)
    {
        UNREACHABLE();
    }

    Class *GetClassRoot(ClassRoot root) const
    {
        return classRoots_[ToIndex(root)];
    }

    ClassLinkerContext *GetBootContext()
    {
        return &bootContext_;
    }

    void SetClassRoot(ClassRoot root, Class *klass)
    {
        classRoots_[ToIndex(root)] = klass;
        bootContext_.InsertClass(klass);
    }

    Class *FindLoadedClass(const uint8_t *descriptor, ClassLinkerContext *context = nullptr);

    PANDA_PUBLIC_API Class *GetClass(const uint8_t *descriptor, bool needCopyDescriptor = true,
                                     ClassLinkerContext *context = nullptr,
                                     ClassLinkerErrorHandler *errorHandler = nullptr);

    PANDA_PUBLIC_API Class *GetClass(const panda_file::File &pf, panda_file::File::EntityId id,
                                     ClassLinkerContext *context = nullptr,
                                     ClassLinkerErrorHandler *errorHandler = nullptr);

    panda_file::SourceLang GetLanguage() const
    {
        return lang_;
    }

    ClassLinker *GetClassLinker() const
    {
        return classLinker_;
    }

    bool IsInitialized() const
    {
        return classLinker_ != nullptr;
    }

    bool CanInitializeClasses()
    {
        return canInitializeClasses_;
    }

    void RecordNewRoot(mem::VisitGCRootFlags flags)
    {
        ASSERT(BitCount(flags & (mem::VisitGCRootFlags::START_RECORDING_NEW_ROOT |
                                 mem::VisitGCRootFlags::END_RECORDING_NEW_ROOT)) <= 1);
        if ((flags & mem::VisitGCRootFlags::START_RECORDING_NEW_ROOT) != 0) {
            // Atomic with seq_cst order reason: data race with record_new_class_ with requirement for sequentially
            // consistent order where threads observe all modifications in the same order
            recordNewClass_.store(true, std::memory_order_seq_cst);
        } else if ((flags & mem::VisitGCRootFlags::END_RECORDING_NEW_ROOT) != 0) {
            // Atomic with seq_cst order reason: data race with record_new_class_ with requirement for sequentially
            // consistent order where threads observe all modifications in the same order
            recordNewClass_.store(false, std::memory_order_seq_cst);
            os::memory::LockHolder lock(newClassesLock_);
            newClasses_.clear();
        }
    }

    template <class Callback>
    bool EnumerateClasses(const Callback &cb, mem::VisitGCRootFlags flags = mem::VisitGCRootFlags::ACCESS_ROOT_ALL)
    {
        ASSERT(BitCount(flags & (mem::VisitGCRootFlags::ACCESS_ROOT_ALL | mem::VisitGCRootFlags::ACCESS_ROOT_ONLY_NEW |
                                 mem::VisitGCRootFlags::ACCESS_ROOT_NONE)) == 1);
        if (((flags & mem::VisitGCRootFlags::ACCESS_ROOT_ALL) != 0) ||
            ((flags & mem::VisitGCRootFlags::ACCESS_ROOT_ONLY_NEW) != 0)) {
            os::memory::LockHolder lock(createdClassesLock_);
            for (const auto &cls : createdClasses_) {
                if (!cb(cls)) {
                    return false;
                }
            }
        }
        if ((flags & mem::VisitGCRootFlags::ACCESS_ROOT_ONLY_NEW) != 0) {
            os::memory::LockHolder lock(newClassesLock_);
            for (const auto &cls : newClasses_) {
                if (!cb(cls)) {
                    return false;
                }
            }
        }

        if ((flags & mem::VisitGCRootFlags::ACCESS_ROOT_ALL) != 0) {
            if (!bootContext_.EnumerateClasses(cb)) {
                return false;
            }

            {
                os::memory::LockHolder lock(contextsLock_);
                for (auto *ctx : contexts_) {
                    if (!ctx->EnumerateClasses(cb)) {
                        return false;
                    }
                }
            }
        }

        {
            os::memory::LockHolder lock(obsoleteClassesLock_);
            for (const auto &cls : obsoleteClasses_) {
                if (!cb(cls)) {
                    return false;
                }
            }
        }

        RecordNewRoot(flags);

        return true;
    }

    template <class ContextGetterFn>
    void RegisterContext(const ContextGetterFn &fn)
    {
        os::memory::LockHolder lock(contextsLock_);
        auto *context = fn();
        if (context != nullptr) {
            contexts_.push_back(context);
        }
    }

    template <class Callback>
    void EnumerateContexts(const Callback &cb)
    {
        if (!cb(&bootContext_)) {
            return;
        }

        os::memory::LockHolder lock(contextsLock_);
        for (auto *context : contexts_) {
            if (!cb(context)) {
                return;
            }
        }
    }

    size_t NumLoadedClasses();

    void VisitLoadedClasses(size_t flag);

    ClassLinkerContext *ResolveContext(ClassLinkerContext *context)
    {
        if (context == nullptr) {
            return &bootContext_;
        }

        return context;
    }

    void OnClassPrepared(Class *klass);

    // Saving obsolete data after hotreload to enable it to be executed
    void AddObsoleteClass(const PandaVector<ark::Class *> &classes);
    void FreeObsoleteData();

    virtual Class *FromClassObject(ObjectHeader *obj);

    virtual size_t GetClassObjectSizeFromClassSize(uint32_t size);

    virtual bool CanScalarReplaceObject([[maybe_unused]] Class *klass)
    {
        return true;
    }

    NO_COPY_SEMANTIC(ClassLinkerExtension);
    NO_MOVE_SEMANTIC(ClassLinkerExtension);

protected:
    void InitializePrimitiveClassRoot(ClassRoot root, panda_file::Type::TypeId typeId, const char *descriptor);

    void InitializeArrayClassRoot(ClassRoot root, ClassRoot componentRoot, const char *descriptor);

    void InitializeSyntheticClassRoot(ClassRoot root, const char *descriptor);

    void FreeLoadedClasses();

    Class *AddClass(Class *klass);

    // Add the class to the list, when it is just be created and not add to class linker context.
    void AddCreatedClass(Class *klass);

    // Remove class in the list, when it has been added to class linker context.
    void RemoveCreatedClass(Class *klass);

    using PandaFilePtr = std::unique_ptr<const panda_file::File>;
    virtual ClassLinkerContext *CreateApplicationClassLinkerContext(PandaVector<PandaFilePtr> &&appFiles);

private:
    class BootContext : public ClassLinkerContext {
    public:
        explicit BootContext(ClassLinkerExtension *extension)
            : ClassLinkerContext(extension->GetLanguage()), extension_(extension)
        {
        }

        bool IsBootContext() const override
        {
            return true;
        }

        Class *LoadClass(const uint8_t *descriptor, bool needCopyDescriptor,
                         ClassLinkerErrorHandler *errorHandler) override;

        void EnumeratePandaFiles(const std::function<bool(const panda_file::File &)> &cb) const override;

    private:
        ClassLinkerExtension *extension_;
    };

    class AppContext : public ClassLinkerContext {
    public:
        explicit AppContext(ClassLinkerExtension *extension, PandaVector<const panda_file::File *> &&pfList)
            : ClassLinkerContext(extension->GetLanguage()), extension_(extension), pfs_(pfList)
        {
        }

        Class *LoadClass(const uint8_t *descriptor, bool needCopyDescriptor,
                         ClassLinkerErrorHandler *errorHandler) override;

        void EnumeratePandaFiles(const std::function<bool(const panda_file::File &)> &cb) const override
        {
            for (auto &pf : pfs_) {
                if (pf == nullptr) {
                    continue;
                }
                if (!cb(*pf)) {
                    break;
                }
            }
        }

        PandaVector<std::string_view> GetPandaFilePaths() const override
        {
            PandaVector<std::string_view> filePaths;
            for (auto &pf : pfs_) {
                if (pf != nullptr) {
                    filePaths.emplace_back(pf->GetFilename());
                }
            }
            return filePaths;
        }

    private:
        ClassLinkerExtension *extension_;
        PandaVector<const panda_file::File *> pfs_;
    };

    static constexpr size_t CLASS_ROOT_COUNT = static_cast<size_t>(ClassRoot::LAST_CLASS_ROOT_ENTRY) + 1;

    virtual bool InitializeImpl(bool compressedStringEnabled) = 0;

    static constexpr size_t ToIndex(ClassRoot root)
    {
        return static_cast<size_t>(root);
    }

    ClassLinkerErrorHandler *ResolveErrorHandler(ClassLinkerErrorHandler *errorHandler)
    {
        if (errorHandler == nullptr) {
            return GetErrorHandler();
        }

        return errorHandler;
    }

    panda_file::SourceLang lang_;
    BootContext bootContext_;

    std::array<Class *, CLASS_ROOT_COUNT> classRoots_ {};
    ClassLinker *classLinker_ {nullptr};

    os::memory::RecursiveMutex contextsLock_;
    PandaVector<ClassLinkerContext *> contexts_ GUARDED_BY(contextsLock_);

    os::memory::RecursiveMutex createdClassesLock_;
    PandaVector<Class *> createdClasses_ GUARDED_BY(createdClassesLock_);

    os::memory::RecursiveMutex newClassesLock_;
    std::atomic_bool recordNewClass_ {false};
    PandaVector<Class *> newClasses_ GUARDED_BY(newClassesLock_);

    os::memory::RecursiveMutex obsoleteClassesLock_;
    PandaVector<Class *> obsoleteClasses_ GUARDED_BY(obsoleteClassesLock_);

    bool canInitializeClasses_ {false};
};

}  // namespace ark

#endif  // PANDA_RUNTIME_CLASS_LINKER_EXTENSION_H_
