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

#include "runtime/profilesaver/profile_saver.h"

#include <cerrno>
#include <chrono>
#include <thread>

#include "runtime/include/runtime.h"
#include "libarkbase/trace/trace.h"

namespace ark {

ProfileSaver *ProfileSaver::instance_ = nullptr;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
std::thread ProfileSaver::profilerSaverDaemonThread_;

static bool CheckLocationForCompilation(const PandaString &location)
{
    return !location.empty();
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
ProfileSaver::ProfileSaver(const PandaString &outputFilename, const PandaVector<PandaString> &codePaths,
                           const PandaString &appDir)
{
    AddTrackedLocations(outputFilename, codePaths, appDir);
}

// NB! it is the caller's responsibility to pass suitable output_filename, code_paths as well as app_data_dir.
void ProfileSaver::AddTrackedLocations(const PandaString &outputFilename, const PandaVector<PandaString> &codePaths,
                                       const PandaString &appDataDir)
{
    auto it = trackedPandafileBaseLocations_.find(outputFilename);
    if (it == trackedPandafileBaseLocations_.end()) {
        trackedPandafileBaseLocations_.insert(
            std::make_pair(outputFilename, PandaSet<PandaString>(codePaths.begin(), codePaths.end())));
        if (!appDataDir.empty()) {
            appDataDirs_.insert(appDataDir);
        }
    } else {
        if (UNLIKELY(appDataDirs_.count(appDataDir) <= 0)) {
            LOG(INFO, RUNTIME) << "Cannot find app dir, bad output filename";
            return;
        }
        it->second.insert(codePaths.begin(), codePaths.end());
    }
}

void ProfileSaver::Start(const PandaString &outputFilename, const PandaVector<PandaString> &codePaths,
                         const PandaString &appDataDir)
{
    if (Runtime::GetCurrent() == nullptr) {
        LOG(ERROR, RUNTIME) << "Runtime is nullptr";
        return;
    }

    if (!Runtime::GetCurrent()->SaveProfileInfo()) {
        LOG(ERROR, RUNTIME) << "ProfileSaver is forbidden";
        return;
    }

    if (outputFilename.empty()) {
        LOG(ERROR, RUNTIME) << "Invalid output filename";
        return;
    }

    PandaVector<PandaString> codePathsToProfile;
    for (const PandaString &location : codePaths) {
        if (CheckLocationForCompilation(location)) {
            codePathsToProfile.push_back(location);
        }
    }

    if (codePathsToProfile.empty()) {
        LOG(INFO, RUNTIME) << "No code paths should be profiled.";
        return;
    }

    if (UNLIKELY(instance_ != nullptr)) {
        LOG(INFO, RUNTIME) << "Profile Saver Singleton already exists";
        instance_->AddTrackedLocations(outputFilename, codePathsToProfile, appDataDir);
        return;
    }

    LOG(INFO, RUNTIME) << "Starting dumping profile saver output file" << outputFilename;

    instance_ = new ProfileSaver(outputFilename, codePathsToProfile, appDataDir);
    profilerSaverDaemonThread_ = std::thread(ProfileSaver::RunProfileSaverThread, reinterpret_cast<void *>(instance_));
}

void ProfileSaver::Stop(bool dumpInfo)
{
    {
        os::memory::LockHolder lock(profile_saver_lock_);

        if (instance_ == nullptr) {
            LOG(ERROR, RUNTIME) << "Tried to stop a profile saver which was not started";
            return;
        }

        if (instance_->shuttingDown_) {
            LOG(ERROR, RUNTIME) << "Tried to stop the profile saver twice";
            return;
        }

        instance_->shuttingDown_ = true;

        if (dumpInfo) {
            instance_->DumpInfo();
        }
    }

    // Wait for the saver thread to stop.
    // NB! we must release profile_saver_lock_ here.
    profilerSaverDaemonThread_.join();

    // Kill the singleton ...
    delete instance_;
    instance_ = nullptr;
}

bool ProfileSaver::IsStarted()
{
    return instance_ != nullptr;
}

void ProfileSaver::DumpInfo()
{
    LOG(INFO, RUNTIME) << "ProfileSaver stoped" << '\n';
}

void *ProfileSaver::RunProfileSaverThread(void *arg)
{
    // NOLINTNEXTLINE(hicpp-use-auto, modernize-use-auto)
    ProfileSaver *profileSaver = reinterpret_cast<ProfileSaver *>(arg);
    profileSaver->Run();

    LOG(INFO, RUNTIME) << "Profile saver shutdown";
    return nullptr;
}

void ProfileSaver::Run()
{
    while (!ShuttingDown()) {
        LOG(INFO, RUNTIME) << "Step1: Time Sleeping >>>>>>> ";
        uint32_t sleepytime = Runtime::GetOptions().GetProfilesaverSleepingTimeMs();
        constexpr uint32_t TIME_SECONDS_IN_MS = 1000;
        for (int i = 0; i < static_cast<int>(sleepytime / TIME_SECONDS_IN_MS); ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (ShuttingDown()) {
                break;
            }
        }

        LOG(INFO, RUNTIME) << "Step2: tranverse the resolved class and methods >>>>>>> ";
        TranverseAndCacheResolvedClassAndMethods();

        LOG(INFO, RUNTIME) << "Step3: merge current profile file and save it back >>>>>>> ";
        MergeAndDumpProfileData();
    }
}

bool ProfileSaver::ShuttingDown()
{
    os::memory::LockHolder lock(profile_saver_lock_);
    return shuttingDown_;
}

static bool CallBackTranverseResolvedClassAndMethods(PandaSet<ExtractedResolvedClasses> &resolvedClasses,
                                                     PandaVector<ExtractedMethod> &methods, Class *klass)
{
    const panda_file::File *pandafile = klass->GetPandaFile();
    panda_file::File::EntityId classfieldid = klass->GetFileId();

    if (pandafile == nullptr) {
        LOG(INFO, RUNTIME) << "panda file is nullptr";
        return false;
    }
    LOG(INFO, RUNTIME) << "      pandafile name = " << pandafile->GetFilename() << " classname = " << klass->GetName();

    Span<Method> tmpMethods = klass->GetMethods();
    LOG(INFO, RUNTIME) << "      methods size = " << tmpMethods.size();
    for (int i = 0; i < static_cast<int>(tmpMethods.Size()); ++i) {
        Method &method = tmpMethods[i];
        if (!method.IsNative()) {
            if (method.GetHotnessCounter() < Method::GetInitialHotnessCounter()) {
                ASSERT(method.GetPandaFile() != nullptr);
                LOG(INFO, RUNTIME) << "      method pandafile name = " << method.GetPandaFile()->GetFilename();
                methods.emplace_back(ExtractedMethod(method.GetPandaFile(), method.GetFileId()));
            }
        }
    }

    ExtractedResolvedClasses tmpResolvedClasses(ConvertToString(pandafile->GetFilename()),
                                                pandafile->GetHeader()->checksum);
    LOG(INFO, RUNTIME) << "      Add class " << klass->GetName();
    auto it = resolvedClasses.find(tmpResolvedClasses);
    if (it != resolvedClasses.end()) {
        it->AddClass(classfieldid.GetOffset());
    } else {
        tmpResolvedClasses.AddClass(classfieldid.GetOffset());
        resolvedClasses.insert(tmpResolvedClasses);
    }

    return true;
}

void ProfileSaver::TranverseAndCacheResolvedClassAndMethods()
{
    trace::ScopedTrace scopedTrace(__PRETTY_FUNCTION__);
    PandaSet<ExtractedResolvedClasses> resolvedClasses;
    PandaVector<ExtractedMethod> methods;
    auto callBack = [&resolvedClasses, &methods](Class *klass) -> bool {
        return CallBackTranverseResolvedClassAndMethods(resolvedClasses, methods, klass);
    };

    // NB: classliner == nullptr
    if (Runtime::GetCurrent()->GetClassLinker() == nullptr) {
        LOG(INFO, RUNTIME) << "classliner is nullptr";
        return;
    }

    LOG(INFO, RUNTIME) << "  Step2.1: tranverse the resolved class and methods";
    Runtime::GetCurrent()->GetClassLinker()->EnumerateClasses(callBack);
    LOG(INFO, RUNTIME) << "  Step2.2: starting tracking all the pandafile locations and flush the cache";

    for (const auto &it : trackedPandafileBaseLocations_) {
        const PandaString &filename = it.first;
        const PandaSet<PandaString> &locations = it.second;

        PandaSet<ExtractedResolvedClasses> resolvedClassesForLocation;
        PandaVector<ExtractedMethod> methodsForLocation;

        LOG(INFO, RUNTIME) << "      all the locations are:";
        for (auto const &iter : locations) {
            LOG(INFO, RUNTIME) << iter << " ";
        }

        LOG(INFO, RUNTIME) << "      Methods name : ";
        for (const ExtractedMethod &ref : methods) {
            LOG(INFO, RUNTIME) << "      " << ref.pandaFile->GetFilename();
            if (locations.find(ConvertToString(ref.pandaFile->GetFilename())) != locations.end()) {
                LOG(INFO, RUNTIME) << "      bingo method!";
                methodsForLocation.push_back(ref);
            }
        }
        LOG(INFO, RUNTIME) << std::endl;
        LOG(INFO, RUNTIME) << "      Classes name";

        for (const ExtractedResolvedClasses &classes : resolvedClasses) {
            LOG(INFO, RUNTIME) << "      " << classes.GetPandaFileLocation();
            if (locations.find(classes.GetPandaFileLocation()) != locations.end()) {
                LOG(INFO, RUNTIME) << "      bingo class!";
                resolvedClassesForLocation.insert(classes);
            }
        }

        ProfileDumpInfo *info = GetOrAddCachedProfiledInfo(filename);
        LOG(INFO, RUNTIME) << "      Adding Bingo Methods and Classes";
        info->AddMethodsAndClasses(methodsForLocation, resolvedClassesForLocation);
    }
}

ProfileDumpInfo *ProfileSaver::GetOrAddCachedProfiledInfo(const PandaString &filename)
{
    auto infoIt = profileCache_.find(filename);
    if (infoIt == profileCache_.end()) {
        LOG(INFO, RUNTIME) << "      bingo profile_cache_!";
        auto ret = profileCache_.insert(std::make_pair(filename, ProfileDumpInfo()));
        ASSERT(ret.second);
        infoIt = ret.first;
    }
    return &(infoIt->second);
}

ProfileSaver::CntStats *ProfileSaver::GetOrAddCachedProfiledStatsInfo(const PandaString &filename)
{
    auto infoIt = statcache.find(filename);
    if (infoIt == statcache.end()) {
        LOG(INFO, RUNTIME) << "      bingo StatsInfo_cache_!";
        auto ret = statcache.insert(std::make_pair(filename, CntStats()));
        ASSERT(ret.second);
        infoIt = ret.first;
    }
    return &(infoIt->second);
}

void ProfileSaver::MergeAndDumpProfileData()
{
    trace::ScopedTrace scopedTrace(__PRETTY_FUNCTION__);
    for (const auto &it : trackedPandafileBaseLocations_) {
        if (ShuttingDown()) {
            return;
        }
        const PandaString &filename = it.first;
        LOG(INFO, RUNTIME) << "  Step3.1 starting merging and save the following file ***";
        LOG(INFO, RUNTIME) << "      filename = " << filename;

        ProfileDumpInfo *cachedInfo = GetOrAddCachedProfiledInfo(filename);
        CntStats *cachedStat = GetOrAddCachedProfiledStatsInfo(filename);
        ASSERT(cachedInfo->GetNumberOfMethods() >= cachedStat->GetMethodCount());
        ASSERT(cachedInfo->GetNumberOfResolvedClasses() >= cachedStat->GetClassCount());
        uint64_t deltaNumberOfMethods = cachedInfo->GetNumberOfMethods() - cachedStat->GetMethodCount();
        uint64_t deltaNumberOfClasses = cachedInfo->GetNumberOfResolvedClasses() - cachedStat->GetClassCount();
        uint64_t numthreshold = Runtime::GetOptions().GetProfilesaverDeltaNumberThreshold();
        if (deltaNumberOfMethods < numthreshold && deltaNumberOfClasses < numthreshold) {
            LOG(INFO, RUNTIME) << "      number of delta number/class not enough";
            continue;
        }

        ssize_t bytesWritten;
        if (cachedInfo->MergeAndSave(filename, &bytesWritten, true)) {
            cachedStat->SetMethodCount(cachedInfo->GetNumberOfMethods());
            cachedStat->SetClassCount(cachedInfo->GetNumberOfResolvedClasses());
        } else {
            LOG(INFO, RUNTIME) << "Could not save profiling info to " << filename;
        }
    }
}

}  // namespace ark
