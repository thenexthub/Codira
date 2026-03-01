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

#ifndef COMPILER_AOT_AOT_MANAGER_H
#define COMPILER_AOT_AOT_MANAGER_H

#include "aot_file.h"
#include "libarkfile/file.h"
#include "libarkbase/utils/arena_containers.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/method.h"
#include "libarkbase/utils/expected.h"

namespace ark::compiler {
class RuntimeInterface;

class AotManager {
    using BitSetElement = uint32_t;
    static constexpr size_t MASK_WIDTH = BITS_PER_BYTE * sizeof(BitSetElement);

public:
    explicit AotManager() = default;

    NO_MOVE_SEMANTIC(AotManager);
    NO_COPY_SEMANTIC(AotManager);
    ~AotManager() = default;

    Expected<bool, std::string> AddFile(const std::string &fileName, RuntimeInterface *runtime, uint32_t gcType,
                                        bool force = false);

    const AotFile *GetFile(const std::string &fileName) const;

    const AotPandaFile *FindPandaFile(const std::string &fileName);

    bool IsFileInAotClassContext(const std::string &fileName, bool isAotVerifyAbsPath) const;

    PandaString GetBootClassContext() const
    {
        return bootClassContext_;
    }

    void ParseClassContextToFile(std::string_view context);

    void SetBootClassContext(PandaString context, bool isArkAot)
    {
        bootClassContext_ = std::move(context);
        ParseClassContextToFile(bootClassContext_);
        UpdatePandaFilesSnapshot(isArkAot, true, false);
    }

    PandaString GetAppClassContext() const
    {
        return appClassContext_;
    }

    void SetAppClassContext(PandaString context, bool isArkAot)
    {
        appClassContext_ = std::move(context);
        ParseClassContextToFile(appClassContext_);
        UpdatePandaFilesSnapshot(isArkAot, false, true);
    }

    void VerifyClassHierarchy();

    uint32_t GetAotStringRootsCount()
    {
        // use counter to get roots count without acquiring vector's lock
        // Atomic with acquire order reason: data race with aot_string_gc_roots_count_ with dependecies on reads after
        // the load which should become visible
        return aotStringGcRootsCount_.load(std::memory_order_acquire);
    }

    void RegisterAotStringRoot(ObjectHeader **slot, bool isYoung);

    template <typename Callback>
    void VisitAotStringRoots(Callback cb, bool visitOnlyYoung)
    {
        ASSERT(aotStringGcRoots_.empty() ||
               (aotStringYoungSet_.size() - 1) == (aotStringGcRoots_.size() - 1) / MASK_WIDTH);

        if (!visitOnlyYoung) {
            for (auto root : aotStringGcRoots_) {
                cb(root);
            }
            return;
        }

        if (!hasYoungAotStringRefs_) {
            return;
        }

        // Atomic with acquire order reason: data race with aot_string_gc_roots_count_ with dependecies on reads after
        // the load which should become visible
        size_t totalRoots = aotStringGcRootsCount_.load(std::memory_order_acquire);
        for (size_t idx = 0; idx < aotStringYoungSet_.size(); idx++) {
            auto mask = aotStringYoungSet_[idx];
            if (mask == 0) {
                continue;
            }
            for (size_t offset = 0; offset < MASK_WIDTH && idx * MASK_WIDTH + offset < totalRoots; offset++) {
                if ((mask & (1ULL << offset)) != 0) {
                    cb(aotStringGcRoots_[idx * MASK_WIDTH + offset]);
                }
            }
        }
    }

    bool InAotFileRange(uintptr_t pc)
    {
        for (auto &aotFile : aotFiles_) {
            auto code = reinterpret_cast<uintptr_t>(aotFile->GetCode());
            if (pc >= code && pc < code + reinterpret_cast<uintptr_t>(aotFile->GetCodeSize())) {
                return true;
            }
        }
        return false;
    }

    bool HasAotFiles()
    {
        return !aotFiles_.empty();
    }

    void TryAddMethodToProfile(Method *method)
    {
        auto pfName = method->GetPandaFile()->GetFullFileName();
        if (profiledPandaFiles_.find(pfName) != profiledPandaFiles_.end()) {
            os::memory::LockHolder lock {profiledMethodsLock_};
            profiledMethods_.push_back(method);
        }
    }

    bool HasProfiledMethods()
    {
        os::memory::LockHolder lock {profiledMethodsLock_};
        return !profiledMethods_.empty();
    }

    PandaList<Method *>::const_iterator GetProfiledMethodsFinal() const
    {
        os::memory::LockHolder lock {profiledMethodsLock_};
        return --profiledMethods_.cend();
    }

    PandaList<Method *> &GetProfiledMethods()
    {
        return profiledMethods_;
    }

    PandaUnorderedSet<std::string_view> &GetProfiledPandaFiles()
    {
        return profiledPandaFiles_;
    }

    using PandaFileLoadData = std::pair<const panda_file::File *, ClassLinkerContext *>;

    void UpdatePandaFilesSnapshot(const panda_file::File *pf, ClassLinkerContext *ctx, bool isArkAot);
    uint32_t GetPandaFileSnapshotIndex(const std::string &fileName);
    const panda_file::File *GetPandaFileBySnapshotIndex(uint32_t index) const;

private:
    PandaVector<std::unique_ptr<AotFile>> aotFiles_;
    PandaUnorderedMap<std::string, AotPandaFile> filesMap_;
    PandaString bootClassContext_;
    PandaString appClassContext_;

    mutable os::memory::Mutex profiledMethodsLock_;
    PandaList<Method *> profiledMethods_ GUARDED_BY(profiledMethodsLock_);
    PandaUnorderedSet<std::string_view> profiledPandaFiles_;

    os::memory::RecursiveMutex aotStringRootsLock_;
    PandaVector<ObjectHeader **> aotStringGcRoots_;
    std::atomic_uint32_t aotStringGcRootsCount_ {0};
    bool hasYoungAotStringRefs_ {false};
    PandaVector<BitSetElement> aotStringYoungSet_;

    mutable os::memory::Mutex snapshotFilesLock_;
    PandaVector<std::pair<std::string_view, PandaFileLoadData>> pandaFilesSnapshot_ GUARDED_BY(snapshotFilesLock_);
    PandaUnorderedMap<std::string_view, PandaFileLoadData> pandaFilesLoaded_ GUARDED_BY(snapshotFilesLock_);

    void UpdatePandaFilesSnapshot(bool isArkAot, bool bootContext, bool appContext);
};

class AotClassContextCollector {
public:
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr char DELIMETER = ':';
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr char HASH_DELIMETER = '*';
    explicit AotClassContextCollector(PandaString *acc, bool useAbsPath = true) : acc_(acc), useAbsPath_(useAbsPath) {};
    bool operator()(const panda_file::File &pf);

    DEFAULT_MOVE_SEMANTIC(AotClassContextCollector);
    DEFAULT_COPY_SEMANTIC(AotClassContextCollector);
    ~AotClassContextCollector() = default;

private:
    PandaString *acc_;
    bool useAbsPath_;
};
}  // namespace ark::compiler

#endif  // COMPILER_AOT_AOT_MANAGER_H
