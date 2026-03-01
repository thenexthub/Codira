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
#ifndef PANDA_RUNTIME_CLASS_LINKER_CONTEXT_H_
#define PANDA_RUNTIME_CLASS_LINKER_CONTEXT_H_

#include <atomic>
#include "libarkbase/macros.h"
#include "libarkbase/mem/object_pointer.h"
#include "libarkbase/os/mutex.h"
#include "libarkbase/utils/bit_utils.h"
#include "mem/refstorage/reference.h"
#include "runtime/include/class.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_root.h"
#include "runtime/mem/object_helpers.h"

namespace ark {

class ClassLinker;
class ClassLinkerErrorHandler;

/// @brief Wrapper around a recursive mutex and a condition variable
class ClassMutexHandler {  // NOLINT(cppcoreguidelines-special-member-functions)
public:
    ClassMutexHandler() = default;

    void Lock() ACQUIRE(clsHandlerMtx_)
    {
        clsHandlerMtx_.Lock();
    }

    void Unlock() RELEASE(clsHandlerMtx_)
    {
        clsHandlerMtx_.Unlock();
    }

    void Wait()
    {
        clsHandlerCondVar_.Wait(&clsHandlerMtx_);
    }

    void Signal()
    {
        clsHandlerCondVar_.Signal();
    }

    void SignalAll()
    {
        clsHandlerCondVar_.SignalAll();
    }

    NO_COPY_SEMANTIC(ClassMutexHandler);
    NO_MOVE_SEMANTIC(ClassMutexHandler);

private:
    os::memory::RecursiveMutex clsHandlerMtx_;
    os::memory::ConditionVariable clsHandlerCondVar_;
};

/**
 * @brief Base class implementing logic of load context for runtime classes.
 * Plugins can extend this class in order to define language-specific class loading algorithms
 * in method `LoadClass`.
 * `ClassLinkerContext` instances are owned by `ClassLinkerExtension` in a one-to-many relationship.
 */
class ClassLinkerContext {
public:
    explicit ClassLinkerContext(panda_file::SourceLang lang) : lang_(lang) {}

    Class *FindClass(const uint8_t *descriptor)
    {
        os::memory::LockHolder lock(classesLock_);
        auto it = loadedClasses_.find(descriptor);
        if (it != loadedClasses_.cend()) {
            return it->second;
        }

        return nullptr;
    }

    virtual bool IsBootContext() const
    {
        return false;
    }

    panda_file::SourceLang GetSourceLang()
    {
        return lang_;
    }

    virtual Class *LoadClass([[maybe_unused]] const uint8_t *descriptor, [[maybe_unused]] bool needCopyDescriptor,
                             [[maybe_unused]] ClassLinkerErrorHandler *errorHandler)
    {
        return nullptr;
    }

    Class *InsertClass(Class *klass)
    {
        os::memory::LockHolder lock(classesLock_);
        auto *otherKlass = FindClass(klass->GetDescriptor());
        if (otherKlass != nullptr) {
            return otherKlass;
        }

        ASSERT(klass->GetSourceLang() == lang_);
        loadedClasses_.insert({klass->GetDescriptor(), klass});
        os::memory::LockHolder<os::memory::Mutex> lockHolder(mapLock_);
        mutexTable_[klass];
        return nullptr;
    }

    void RemoveClass(Class *klass)
    {
        os::memory::LockHolder lock(classesLock_);
        loadedClasses_.erase(klass->GetDescriptor());
        os::memory::LockHolder<os::memory::Mutex> lockHolder(mapLock_);
        mutexTable_.erase(klass);
    }

    template <class Callback>
    bool EnumerateClasses(const Callback &cb)
    {
        os::memory::LockHolder lock(classesLock_);
        for (const auto &v : loadedClasses_) {
            if (!cb(v.second)) {
                return false;
            }
        }
        return true;
    }

    virtual void EnumeratePandaFiles(const std::function<bool(const panda_file::File &)> & /* cb */) const {}

    /// @brief Enumerate panda files in chained contexts
    virtual void EnumeratePandaFilesInChain(const std::function<bool(const panda_file::File &)> &cb) const
    {
        EnumeratePandaFiles(cb);
    }

    size_t NumLoadedClasses()
    {
        os::memory::LockHolder lock(classesLock_);
        return loadedClasses_.size();
    }

    void VisitLoadedClasses(size_t flag)
    {
        os::memory::LockHolder lock(classesLock_);
        for (auto &loadedClass : loadedClasses_) {
            auto classPtr = loadedClass.second;
            classPtr->DumpClass(GET_LOG_STREAM(ERROR, RUNTIME), flag);
        }
    }

    void VisitGCRoots(const GCRootVisitor &cb)
    {
        for (auto &root : roots_) {
            cb({mem::RootType::ROOT_CLASS_LINKER, &root});
        }
    }

    bool AddGCRoot(ObjectHeader *obj)
    {
        os::memory::LockHolder lock(classesLock_);
        for (auto root : roots_) {
            if (root == obj) {
                return false;
            }
        }

        roots_.emplace_back(obj);
        return true;
    }

    virtual PandaVector<std::string_view> GetPandaFilePaths() const
    {
        return PandaVector<std::string_view>();
    }

    virtual void Dump(std::ostream &os)
    {
        os << "|Class loader :\"" << this << "\" "
           << "|Loaded Classes:" << NumLoadedClasses() << "\n";
    }

    virtual bool FindClassLoaderParent([[maybe_unused]] ClassLinkerContext *parent)
    {
        parent = nullptr;
        return false;
    }

    ClassMutexHandler *GetClassMutexHandler(Class *cls)
    {
        os::memory::LockHolder<os::memory::Mutex> lockHolder(mapLock_);
        return &mutexTable_.at(cls);
    }

    ClassLinkerContext() = default;
    virtual ~ClassLinkerContext() = default;

    NO_COPY_SEMANTIC(ClassLinkerContext);
    NO_MOVE_SEMANTIC(ClassLinkerContext);

private:
    // Dummy fix of concurrency issues to evaluate degradation
    os::memory::RecursiveMutex classesLock_;
    PandaUnorderedMap<const uint8_t *, Class *, utf::Mutf8Hash, utf::Mutf8Equal> loadedClasses_
        GUARDED_BY(classesLock_);
    os::memory::Mutex mapLock_;
    PandaUnorderedMap<Class *, ClassMutexHandler> mutexTable_;
    PandaVector<ObjectHeader *> roots_;
    panda_file::SourceLang lang_ {panda_file::SourceLang::PANDA_ASSEMBLY};
};

}  // namespace ark

#endif  // PANDA_RUNTIME_CLASS_LINKER_CONTEXT_H_
