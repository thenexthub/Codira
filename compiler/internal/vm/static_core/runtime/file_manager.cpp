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

#include "runtime/include/file_manager.h"
#include "runtime/include/runtime.h"
#include "libarkbase/os/filesystem.h"

namespace ark {

bool FileManager::LoadAbcFile(std::string_view location, panda_file::File::OpenMode openMode)
{
    auto pf = panda_file::OpenPandaFile(location, "", openMode);
    if (pf == nullptr) {
        LOG(ERROR, PANDAFILE) << "Load panda file failed: " << location;
        return false;
    }
    auto runtime = Runtime::GetCurrent();
    if (Runtime::GetOptions().IsEnableAn() && !Runtime::GetOptions().IsArkAot()) {
        TryLoadAnFileForLocation(location);
        auto aotFile = runtime->GetClassLinker()->GetAotManager()->FindPandaFile(std::string(location));
        if (aotFile != nullptr) {
            pf->SetClassHashTable(aotFile->GetClassHashTable());
        }
    }
    runtime->GetClassLinker()->AddPandaFile(std::move(pf));
    return true;
}

bool FileManager::TryLoadAnFileForLocation(std::string_view abcPath)
{
    PandaString::size_type posStart = abcPath.find_last_of('/');
    PandaString::size_type posEnd = abcPath.find_last_of('.');
    if (posStart == std::string_view::npos || posEnd == std::string_view::npos) {
        return true;
    }
    LOG(DEBUG, PANDAFILE) << "current abc file path: " << abcPath;
    PandaString abcFilePrefix = PandaString(abcPath.substr(posStart, posEnd - posStart));

    // If set boot-an-location, load from this location first
    std::string_view anLocation = Runtime::GetOptions().GetBootAnLocation();
    if (!anLocation.empty()) {
        bool res = FileManager::TryLoadAnFileFromLocation(anLocation, abcFilePrefix, abcPath);
        if (res) {
            return true;
        }
    }

    // If load failed from boot-an-location, continue try load from location of abc
    anLocation = abcPath.substr(0, posStart);
    FileManager::TryLoadAnFileFromLocation(anLocation, abcFilePrefix, abcPath);
    return true;
}

bool FileManager::TryLoadAnFileFromLocation(std::string_view anFileLocation, PandaString &abcFilePrefix,
                                            std::string_view pandaFileLocation)
{
    const PandaString &anPathSuffix = ".an";
    PandaString anFilePath = PandaString(anFileLocation) + abcFilePrefix + anPathSuffix;

    const char *filename = anFilePath.c_str();
    if (access(filename, F_OK) != 0) {
        LOG(DEBUG, PANDAFILE) << "There is no corresponding .an file for '" << pandaFileLocation << "' in '"
                              << anFileLocation << "'";
        return false;
    }
    auto res = FileManager::LoadAnFile(anFilePath, false);
    if (res && res.Value()) {
        LOG(INFO, PANDAFILE) << "Successfully load .an file for '" << pandaFileLocation << "': '" << anFileLocation
                             << "'";
        return true;
    }
    if (!res) {
        LOG(INFO, PANDAFILE) << "Failed to load AOT file: '" << anFileLocation << "': " << res.Error();
    } else {
        LOG(INFO, PANDAFILE) << "Failed to load '" << anFileLocation << "' with unknown reason";
    }
    return false;
}

Expected<bool, std::string> FileManager::LoadAnFile(std::string_view anLocation, bool force)
{
    PandaRuntimeInterface runtimeIface;
    auto runtime = Runtime::GetCurrent();
    auto gcType = Runtime::GetGCType(Runtime::GetOptions(), plugins::RuntimeTypeToLang(runtime->GetRuntimeType()));
    ASSERT(gcType != ark::mem::GCType::INVALID_GC);
    auto realAnFilePath = os::GetAbsolutePath(anLocation);
    return runtime->GetClassLinker()->GetAotManager()->AddFile(realAnFilePath, &runtimeIface,
                                                               static_cast<uint32_t>(gcType), force);
}

}  // namespace ark
