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
#ifndef RUNTIME_MEM_LOCK_CONFIG_HELPER_H
#define RUNTIME_MEM_LOCK_CONFIG_HELPER_H

#include "libarkfile/file_items.h"

namespace ark {
enum LangTypeT : bool { LANG_TYPE_STATIC, LANG_TYPE_DYNAMIC };
enum MTModeT { MT_MODE_SINGLE, MT_MODE_MULTI, MT_MODE_TASK };

namespace mem {

template <class LockConfig, MTModeT MT_MODE>
class LockConfigHelper {
};

template <class LockConfigT>
class LockConfigHelper<LockConfigT, MT_MODE_MULTI> {
public:
    using Value = typename LockConfigT::CommonLock;
};

template <class LockConfigT>
class LockConfigHelper<LockConfigT, MT_MODE_TASK> {
public:
    using Value = typename LockConfigT::CommonLock;
};

template <class LockConfigT>
class LockConfigHelper<LockConfigT, MT_MODE_SINGLE> {
public:
    using Value = typename LockConfigT::DummyLock;
};
}  // namespace mem
}  // namespace ark

#endif  // RUNTIME_MEM_LOCK_CONFIG_HELPER_H
