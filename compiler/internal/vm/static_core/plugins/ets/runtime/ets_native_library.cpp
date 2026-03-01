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

#include "plugins/ets/runtime/ets_native_library.h"

namespace ark::ets {
Expected<EtsNativeLibrary, os::Error> EtsNativeLibrary::Load(const PandaString &name)
{
    auto handle = os::library_loader::Load(name);
    if (!handle) {
        return Unexpected(handle.Error());
    }

    return EtsNativeLibrary(name, std::move(handle.Value()));
}

EtsNativeLibrary::EtsNativeLibrary(PandaString name, os::library_loader::LibraryHandle &&handle)
    : name_(std::move(name)), handle_(std::move(handle))
{
}
}  // namespace ark::ets
