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

#ifndef PANDA_RUNTIME_ECMASCRIPT_NATIVEPOINTER_H_
#define PANDA_RUNTIME_ECMASCRIPT_NATIVEPOINTER_H_

#include "runtime/include/language_context.h"
#include "runtime/include/object_header.h"

namespace ark::coretypes {

// Use for the requirement of ACE that wants to associated a registered C++ resource with a JSObject.
class NativePointer : public ObjectHeader {
public:
    inline void *GetExternalPointer() const
    {
        return externalPointer_;
    }

    inline void SetExternalPointer(void *externalPointer)
    {
        externalPointer_ = externalPointer;
    }

    static NativePointer *Cast(ObjectHeader *object)
    {
        return static_cast<NativePointer *>(object);
    }

    static constexpr uint32_t GetExternalPointerOffset()
    {
        return MEMBER_OFFSET(NativePointer, externalPointer_);
    }

private:
    NativePointer() : ObjectHeader() {}

    void *externalPointer_ {nullptr};
};

}  // namespace ark::coretypes

#endif  // PANDA_RUNTIME_ECMASCRIPT_NATIVEPOINTER_H_
