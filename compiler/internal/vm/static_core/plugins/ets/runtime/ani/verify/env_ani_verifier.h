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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_ENV_ANI_VERIFIER_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_ENV_ANI_VERIFIER_H

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/runtime/ani/verify/types/vref.h"
#include "plugins/ets/runtime/ani/verify/types/vmethod.h"
#include "plugins/ets/runtime/ani/verify/types/vfield.h"
#include "plugins/ets/runtime/ani/verify/types/vresolver.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/mem/panda_string.h"
#include <optional>

namespace ark::ets::ani::verify {

class VEnv;
class VRef;
class ANIVerifier;
class VResolver;

class EnvANIVerifier final {
public:
    EnvANIVerifier(ANIVerifier *verifier, const __ani_interaction_api *interactionAPI);
    ~EnvANIVerifier() = default;

    VEnv *GetEnv(ani_env *env);
    VEnv *AttachThread(ani_env *env);

    void PushNativeFrame();
    [[nodiscard]] std::optional<PandaString> PopNativeFrame();

    void CreateLocalScope();
    [[nodiscard]] std::optional<PandaString> DestroyLocalScope();
    void CreateEscapeLocalScope();
    [[nodiscard]] std::optional<PandaString> DestroyEscapeLocalScope(VRef *vref);

    VRef *AddLocalVerifiedRef(ani_ref ref);
    void DeleteLocalVerifiedRef(VRef *vref);

    VMethod *GetVerifiedMethod(ani_method method);
    VStaticMethod *GetVerifiedStaticMethod(ani_static_method staticMethod);
    VFunction *GetVerifiedFunction(ani_function function);

    VField *GetVerifiedField(ani_field field);
    VStaticField *GetVerifiedStaticField(ani_static_field staticField);

    VRef *AddGlobalVerifiedRef(ani_ref gref);
    void DeleteGlobalVerifiedRef(VRef *vgref);
    bool IsValidGlobalVerifiedRef(VRef *vgref) const;

    bool IsValidRefInCurrentFrame(VRef *vref) const;
    bool IsGlobalRef(VRef *vref) const;
    bool IsValidMethod(impl::VMethod *vmethod) const;
    bool IsValidField(impl::VField *vfield) const;

    bool IsValidInCurrentFrame(VRef *vref);
    bool CanBeDeletedFromCurrentScope(VRef *vref);

    bool IsValidGlobalVerifiedResolver(VResolver *vresolver) const;
    VResolver *AddGlobalVerifiedResolver(ani_resolver resolver);
    void DeleteGlobalVerifiedResolver(VResolver *vresolver);

    const __ani_interaction_api *GetInteractionAPI() const
    {
        return interactionAPI_;
    }

    bool IsValidStackRef(VRef *vref) const;

private:
    enum class SubFrameType {
        NATIVE_FRAME,
        LOCAL_SCOPE,
        ESCAPE_LOCAL_SCOPE,
    };

    struct Frame {
        SubFrameType frameType;
        size_t nrUndef;
        PandaSet<VRef *> refs;
        PandaUniquePtr<ArenaAllocator> refsAllocatorHolder;
        ArenaAllocator *refsAllocator;
    };

    void DoPushNativeFrame();
    static bool IsValidVerifiedRef(const Frame &frame, VRef *vref);

    ANIVerifier *verifier_ {};
    const __ani_interaction_api *interactionAPI_ {};
    PandaVector<Frame> frames_;

    NO_COPY_SEMANTIC(EnvANIVerifier);
    NO_MOVE_SEMANTIC(EnvANIVerifier);
};

}  // namespace ark::ets::ani::verify

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_ENV_ANI_VERIFIER_H
