/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "external_array_buffer.h"

#include <limits>
#include <memory>

namespace {

class FinalizationInfo final {
public:
    static std::unique_ptr<FinalizationInfo> Create(AniFinalizer finalizer, void *data, void *hint)
    {
        // CC-OFFNXT(G.RES.09) private constructor
        return std::unique_ptr<FinalizationInfo>(new FinalizationInfo(finalizer, data, hint));
    }

    static void Finalize(void *selfPtr)
    {
        auto *self = reinterpret_cast<FinalizationInfo *>(selfPtr);
        self->finalizer_(self->data_, self->hint_);
        ReleaseInfo(self);
    }

    static void ReleaseInfo(FinalizationInfo *self)
    {
        delete self;
    }

private:
    explicit FinalizationInfo(AniFinalizer finalizer, void *data, void *hint)
        : finalizer_(finalizer), data_(data), hint_(hint)
    {
    }

private:
    AniFinalizer finalizer_;
    void *data_;
    void *hint_;
};

}  // anonymous namespace

static bool IsAligned(void *data)
{
    return reinterpret_cast<uintptr_t>(data) % alignof(uintptr_t) == 0;
}

extern "C" ani_status CreateExternalArrayBuffer(ani_env *env, void *externalData, ani_size length,
                                                ani_arraybuffer *resultBuffer)
{
    if (env == nullptr || externalData == nullptr || resultBuffer == nullptr ||
        length > static_cast<size_t>(std::numeric_limits<ani_int>::max())) {
        return ANI_INVALID_ARGS;
    }

    if (!IsAligned(externalData)) {
        return ANI_INVALID_ARGS;
    }

    ani_class cls {};
    if (auto status = env->FindClass("std.core.ArrayBuffer", &cls); status != ANI_OK) {
        return status;
    }
    ani_method ctor {};
    if (auto status = env->Class_FindMethod(cls, "<ctor>", "li:", &ctor); status != ANI_OK) {
        return status;
    }

    ani_object result;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
    auto status =
        env->Object_New(cls, ctor, &result, reinterpret_cast<ani_long>(externalData), static_cast<ani_int>(length));
    if (status != ANI_OK) {
        return status;
    }
    *resultBuffer = static_cast<ani_arraybuffer>(result);
    return ANI_OK;
}

// CC-OFFNXT(G.FUN.01-CPP) public API
extern "C" ani_status CreateFinalizableArrayBuffer(ani_env *env, void *externalData, ani_size length,
                                                   AniFinalizer finalizer, void *finalizerHint,
                                                   ani_arraybuffer *resultBuffer)
{
    if (env == nullptr || externalData == nullptr || finalizer == nullptr || resultBuffer == nullptr ||
        length > static_cast<size_t>(std::numeric_limits<ani_int>::max())) {
        return ANI_INVALID_ARGS;
    }

    if (!IsAligned(externalData)) {
        return ANI_INVALID_ARGS;
    }

    ani_class cls {};
    if (auto status = env->FindClass("std.core.ArrayBuffer", &cls); status != ANI_OK) {
        return status;
    }
    ani_method ctor {};
    if (auto status = env->Class_FindMethod(cls, "<ctor>", "lill:", &ctor); status != ANI_OK) {
        return status;
    }
    auto finalizationInfo = FinalizationInfo::Create(finalizer, externalData, finalizerHint);
    if (finalizationInfo == nullptr) {
        // Technically this is not managed OOM, so return `ANI_ERROR` here
        return ANI_ERROR;
    }

    ani_object result;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
    auto status = env->Object_New(cls, ctor, &result, reinterpret_cast<ani_long>(externalData),
                                  static_cast<ani_int>(length), reinterpret_cast<ani_long>(FinalizationInfo::Finalize),
                                  reinterpret_cast<ani_long>(finalizationInfo.get()));
    if (status != ANI_OK) {
        return status;
    }
    (void)finalizationInfo.release();
    *resultBuffer = static_cast<ani_arraybuffer>(result);
    return ANI_OK;
}

extern "C" ani_status DetachArrayBuffer(ani_env *env, ani_arraybuffer buffer)
{
    if (env == nullptr) {
        return ANI_INVALID_ARGS;
    }

    ani_long finalizationInfoPtr = ANI_FALSE;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
    auto status = env->Object_CallMethodByName_Long(buffer, "detach", ":l", &finalizationInfoPtr);
    if (status != ANI_OK) {
        return status;
    }
    if (finalizationInfoPtr == 0) {
        return ANI_INVALID_ARGS;
    }
    FinalizationInfo::ReleaseInfo(reinterpret_cast<FinalizationInfo *>(finalizationInfoPtr));
    return ANI_OK;
}

extern "C" ani_status IsDetachedArrayBuffer(ani_env *env, ani_arraybuffer buffer, ani_boolean *result)
{
    if (env == nullptr || result == nullptr) {
        return ANI_INVALID_ARGS;
    }

    ani_boolean isDetached = ANI_FALSE;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
    auto status = env->Object_CallMethodByName_Boolean(buffer, "%%get-detached", ":z", &isDetached);
    if (status != ANI_OK) {
        return status;
    }
    *result = isDetached;
    return ANI_OK;
}
