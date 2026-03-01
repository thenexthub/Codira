/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_OBJECT_REPOSITORY_H
#define PANDA_TOOLING_INSPECTOR_OBJECT_REPOSITORY_H

#include "include/tooling/debug_interface.h"
#include "include/typed_value.h"
#include "libarkbase/os/mutex.h"
#include "runtime/handle_scope.h"

#include "types/numeric_id.h"
#include "types/property_descriptor.h"

namespace ark::tooling::inspector {
/**
 * All manipulations with an object repository should be made
 * on the corresponding application thread with mutator lock held
 */

class ObjectRepository {
public:
    explicit ObjectRepository();
    ~ObjectRepository() = default;

    NO_COPY_SEMANTIC(ObjectRepository);
    NO_MOVE_SEMANTIC(ObjectRepository);

    RemoteObject CreateGlobalObject();
    RemoteObject CreateFrameObject(const PtFrame &frame, const std::map<std::string, TypedValue> &locals,
                                   std::optional<RemoteObject> &objThis);
    RemoteObject CreateObject(TypedValue value);

    std::vector<PropertyDescriptor> GetProperties(RemoteObjectId id, bool generatePreview);

private:
    static constexpr RemoteObjectId GLOBAL_OBJECT_ID = 0;

    RemoteObject CreateObject(coretypes::TaggedValue value);
    RemoteObject CreateObject(ObjectHeader *object);
    std::vector<PropertyDescriptor> GetProperties(RemoteObjectId id);

    std::optional<ObjectPreview> CreateObjectPreview(RemoteObject &remobj);

    std::unique_ptr<PtLangExt> extension_;
    HandleScope<ObjectHeader *> scope_;
    RemoteObjectId counter_ {GLOBAL_OBJECT_ID + 1};
    std::map<RemoteObjectId, std::vector<PropertyDescriptor>> frames_;
    std::map<RemoteObjectId, VMHandle<ObjectHeader>> objects_;
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_OBJECT_REPOSITORY_H
