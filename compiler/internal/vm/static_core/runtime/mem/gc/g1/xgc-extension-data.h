/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_MEM_GC_XGC_EXTENSION_DATA_H
#define PANDA_RUNTIME_MEM_GC_XGC_EXTENSION_DATA_H

#include "runtime/mem/gc/gc_extension_data.h"

namespace ark::mem {
class XGCExtensionData : public GCExtensionData {
public:
    explicit XGCExtensionData(std::function<void(ObjectHeader *)> xobjHandler) : xobjHandler_(std::move(xobjHandler)) {}
    ~XGCExtensionData() override = default;
    NO_COPY_SEMANTIC(XGCExtensionData);
    NO_MOVE_SEMANTIC(XGCExtensionData);

    auto GetXObjectHandler() const
    {
        return xobjHandler_;
    }

private:
    std::function<void(ObjectHeader *)> xobjHandler_;
};
}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_XGC_EXTENSION_DATA_H
