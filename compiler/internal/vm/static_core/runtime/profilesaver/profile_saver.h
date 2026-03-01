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
#ifndef PANDA_RUNTIME_PROFILE_SAVER_H_
#define PANDA_RUNTIME_PROFILE_SAVER_H_

#include <pthread.h>

#include <map>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "libarkbase/macros.h"
#include "libarkbase/os/mutex.h"
#include "libarkbase/utils/logger.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/profilesaver/profile_dump_info.h"

namespace ark {

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects, readability-identifier-naming)
static os::memory::Mutex profile_saver_lock_;

/*
 * NB! we take singleton pattern, multi-threading scenario should be taken more serious considerations.
 *
 * e.g. Global Mutex Management or corresponding alternatives should better than
 * global mutex profile_saver_lock_.
 *
 */

// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions, hicpp-special-member-functions)
class ProfileSaver {
public:
    /*
     * start the profile saver daemon thread
     *
     * output_filename records the profile name, code_paths stores all the locations contain pandafile(aka *.aex)
     * app_data_dir contains the location of application package file (aka *.hap)
     */
    static void Start(const PandaString &outputFilename, const PandaVector<PandaString> &codePaths,
                      const PandaString &appDataDir);

    /*
     * stop the profile saver daemon thread.
     *
     * if dump_info == true, dumps the debug information
     */
    static void Stop(bool dumpInfo);

    /*
     * whether profile saver instance exists.
     */
    static bool IsStarted();

private:
    ProfileSaver(const PandaString &outputFilename, const PandaVector<PandaString> &codePaths,
                 const PandaString &appDir);

    void AddTrackedLocations(const PandaString &outputFilename, const PandaVector<PandaString> &codePaths,
                             const PandaString &appDataDir);

    /*
     * Callback for pthread_create, we attach/detach this thread to Runtime here
     */
    static void *RunProfileSaverThread(void *arg);

    /*
     * Run loop for the saver.
     */
    void Run();

    /*
     * Returns true if the saver is shutting down (ProfileSaver::Stop() has been called, will set this flag).
     */
    bool ShuttingDown();

    /*
     * Dump functions, we leave it stub and for test until now.
     */
    void DumpInfo();

    /*
     * Fetches the current resolved classes and methods from the ClassLinker and stores them in the profile_cache_.
     */
    void TranverseAndCacheResolvedClassAndMethods();

    /*
     * Retrieves the cached ProfileDumpInfo for the given profile filename.
     * If no entry exists, a new empty one will be created, added to the cache and then returned.
     */
    ProfileDumpInfo *GetOrAddCachedProfiledInfo(const PandaString &filename);

    /*
     * Processes the existing profiling info from the jit code cache(if exists) and returns
     * true if it needed to be saved back to disk.
     */
    void MergeAndDumpProfileData();

    static ProfileSaver *instance_;

    static std::thread profilerSaverDaemonThread_;

    PandaMap<PandaString, PandaSet<PandaString>> trackedPandafileBaseLocations_;

    PandaMap<PandaString, ProfileDumpInfo> profileCache_;

    PandaSet<PandaString> appDataDirs_;

    bool shuttingDown_ GUARDED_BY(profile_saver_lock_) {false};

    struct CntStats {
    public:
        uint64_t GetMethodCount()
        {
            return lastSaveNumberOfMethods_;
        }

        void SetMethodCount(uint64_t methodcnt)
        {
            lastSaveNumberOfMethods_ = methodcnt;
        }

        uint64_t GetClassCount()
        {
            return lastSaveNumberOfClasses_;
        }

        void SetClassCount(uint64_t classcnt)
        {
            lastSaveNumberOfClasses_ = classcnt;
        }

    private:
        uint64_t lastSaveNumberOfMethods_ {0};
        uint64_t lastSaveNumberOfClasses_ {0};
    };

    PandaMap<PandaString, CntStats> statcache;  // NOLINT(readability-identifier-naming)

    /*
     * Retrieves the cached CntStats for the given profile filename.
     * If no entry exists, a new empty one will be created, added to the cache and then returned.
     */
    CntStats *GetOrAddCachedProfiledStatsInfo(const PandaString &filename);

    NO_COPY_SEMANTIC(ProfileSaver);
    NO_MOVE_SEMANTIC(ProfileSaver);
};

}  // namespace ark

#endif
