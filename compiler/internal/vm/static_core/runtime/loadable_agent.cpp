/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "runtime/include/loadable_agent.h"

#include "runtime/include/runtime.h"

namespace ark {
LibraryAgent::LibraryAgent(os::memory::Mutex &mutex, PandaString libraryPath, PandaString loadCallbackName,
                           PandaString unloadCallbackName)
    : lock_(mutex),
      libraryPath_(std::move(libraryPath)),
      loadCallbackName_(std::move(loadCallbackName)),
      unloadCallbackName_(std::move(unloadCallbackName))
{
}

bool LibraryAgent::Load()
{
    ASSERT(!handle_.IsValid());

    auto handle = os::library_loader::Load(libraryPath_);
    if (!handle) {
        LOG(ERROR, RUNTIME) << "Couldn't load library '" << libraryPath_ << "': " << handle.Error().ToString();
        return false;
    }

    auto loadCallback = os::library_loader::ResolveSymbol(handle.Value(), loadCallbackName_);
    if (!loadCallback) {
        LOG(ERROR, RUNTIME) << "Couldn't resolve '" << loadCallbackName_ << "' in '" << libraryPath_
                            << "':" << loadCallback.Error().ToString();
        return false;
    }

    auto unloadCallback = os::library_loader::ResolveSymbol(handle.Value(), unloadCallbackName_);
    if (!unloadCallback) {
        LOG(ERROR, RUNTIME) << "Couldn't resolve '" << unloadCallbackName_ << "' in '" << libraryPath_
                            << "':" << unloadCallback.Error().ToString();
        return false;
    }

    if (!CallLoadCallback(loadCallback.Value())) {
        LOG(ERROR, RUNTIME) << "'" << loadCallbackName_ << "' failed in '" << libraryPath_ << "'";
        return false;
    }

    handle_ = std::move(handle.Value());
    unloadCallback_ = unloadCallback.Value();

    return true;
}

bool LibraryAgent::Unload()
{
    ASSERT(handle_.IsValid());

    if (!CallUnloadCallback(unloadCallback_)) {
        LOG(ERROR, RUNTIME) << "'" << unloadCallbackName_ << "' failed in '" << libraryPath_ << "'";
        return false;
    }

    return true;
}
}  // namespace ark
