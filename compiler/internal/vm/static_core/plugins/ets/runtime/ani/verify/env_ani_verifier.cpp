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

#include "plugins/ets/runtime/ani/verify/env_ani_verifier.h"

#include "plugins/ets/runtime/ani/verify/ani_verifier.h"
#include "plugins/ets/runtime/ani/ani_converters.h"

namespace ark::ets::ani::verify {

EnvANIVerifier::EnvANIVerifier(ANIVerifier *verifier, const __ani_interaction_api *interactionAPI)
    : verifier_(verifier), interactionAPI_(interactionAPI)
{
    DoPushNativeFrame();
}

VEnv *EnvANIVerifier::GetEnv(ani_env *env)
{
    // NOTE: Use different ani_env * for each native frames, #28620
    return reinterpret_cast<VEnv *>(env);
}

VEnv *EnvANIVerifier::AttachThread(ani_env *env)
{
    // NOTE: Use different ani_env * for each native frames, #28620
    return reinterpret_cast<VEnv *>(env);
}

void EnvANIVerifier::DoPushNativeFrame()
{
    Frame frame {
        SubFrameType::NATIVE_FRAME, 0, {}, MakePandaUnique<ArenaAllocator>(SpaceType::SPACE_TYPE_INTERNAL), nullptr};
    frame.refsAllocator = frame.refsAllocatorHolder.get();
    frames_.push_back(std::move(frame));
}

void EnvANIVerifier::PushNativeFrame()
{
    ASSERT(!frames_.empty());
    DoPushNativeFrame();
}

std::optional<PandaString> EnvANIVerifier::PopNativeFrame()
{
    ASSERT(!frames_.empty());
    switch (frames_.back().frameType) {
        case SubFrameType::NATIVE_FRAME:
            frames_.pop_back();
            return {};
        case SubFrameType::LOCAL_SCOPE:
            return "It is necessary to call DestroyLocalScope(), after calling CreateLocalScope()";
        case SubFrameType::ESCAPE_LOCAL_SCOPE:
            return "It is necessary to call DestroyEscapeLocalScope(), after calling CreateEscapeLocalScope()";
    }
    UNREACHABLE();
    return "Verification logic was broken";
}

void EnvANIVerifier::CreateLocalScope()
{
    ASSERT(!frames_.empty());
    ArenaAllocator *allocator = frames_.back().refsAllocator;
    frames_.push_back(Frame {SubFrameType::LOCAL_SCOPE, 0, {}, {}, allocator});
}

std::optional<PandaString> EnvANIVerifier::DestroyLocalScope()
{
    ASSERT(!frames_.empty());
    switch (frames_.back().frameType) {
        case SubFrameType::NATIVE_FRAME:
            return "Illegal call DestroyLocalScope() without first calling CreateLocalScope()";
        case SubFrameType::LOCAL_SCOPE:
            frames_.pop_back();
            return {};
        case SubFrameType::ESCAPE_LOCAL_SCOPE:
            return "Illegal call DestroyLocalScope(), after calling CreateEscapeLocalScope()";
    }
    UNREACHABLE();
    return "Verification logic was broken";
}

void EnvANIVerifier::CreateEscapeLocalScope()
{
    ASSERT(!frames_.empty());
    ArenaAllocator *allocator = frames_.back().refsAllocator;
    frames_.push_back(Frame {SubFrameType::ESCAPE_LOCAL_SCOPE, 0, {}, {}, allocator});
}

std::optional<PandaString> EnvANIVerifier::DestroyEscapeLocalScope([[maybe_unused]] VRef *vref)
{
    ASSERT(!frames_.empty());
    switch (frames_.back().frameType) {
        case SubFrameType::NATIVE_FRAME:
            return "Illegal call DestroyEscapeLocalScope() without first calling CreateEscapeLocalScope()";
        case SubFrameType::LOCAL_SCOPE:
            return "Illegal call DestroyEscapeLocalScope() after calling CreateLocalScope()";
        case SubFrameType::ESCAPE_LOCAL_SCOPE: {
            // Drop top frame
            ASSERT(IsValidVerifiedRef(frames_.back(), vref));
            frames_.pop_back();
            return {};
        }
    }
    UNREACHABLE();
    return "Verification logic was broken";
}

VRef *EnvANIVerifier::AddLocalVerifiedRef(ani_ref ref)
{
    ASSERT(!frames_.empty());
    Frame &frame = frames_.back();

    InternalRef *iref = frame.refsAllocator->New<InternalRef>(ref);
    auto vref = InternalRef::CastToVRef(iref);
    [[maybe_unused]] auto ret = frame.refs.emplace(vref);
    ASSERT(ret.second);
    return vref;
}

VMethod *EnvANIVerifier::GetVerifiedMethod(ani_method method)
{
    return static_cast<VMethod *>(verifier_->AddMethod(ToInternalMethod(method)));
}

VStaticMethod *EnvANIVerifier::GetVerifiedStaticMethod(ani_static_method staticMethod)
{
    return static_cast<VStaticMethod *>(verifier_->AddMethod(ToInternalMethod(staticMethod)));
}

VFunction *EnvANIVerifier::GetVerifiedFunction(ani_function function)
{
    return static_cast<VFunction *>(verifier_->AddMethod(ToInternalMethod(function)));
}

VField *EnvANIVerifier::GetVerifiedField(ani_field field)
{
    return static_cast<VField *>(verifier_->AddField(ToInternalField(field)));
}

VStaticField *EnvANIVerifier::GetVerifiedStaticField(ani_static_field staticField)
{
    return static_cast<VStaticField *>(verifier_->AddField(ToInternalField(staticField)));
}

void EnvANIVerifier::DeleteLocalVerifiedRef(VRef *vref)
{
    ASSERT(!frames_.empty());
    Frame &frame = frames_.back();

    auto it = frame.refs.find(vref);
    ASSERT(it != frame.refs.cend());
    frame.refs.erase(it);
}

bool EnvANIVerifier::IsValidRefInCurrentFrame(VRef *vref) const
{
    ASSERT(!frames_.empty());
    for (auto it = frames_.crbegin(); it != frames_.crend(); ++it) {
        if (IsValidVerifiedRef(*it, vref)) {
            return true;
        }
        if (it->frameType == SubFrameType::NATIVE_FRAME) {
            break;
        }
    }
    if (IsValidStackRef(vref)) {
        return true;
    }
    return false;
}

bool EnvANIVerifier::IsGlobalRef(VRef *vref) const
{
    return verifier_->IsValidGlobalVerifiedRef(vref);
}

bool EnvANIVerifier::IsValidMethod(impl::VMethod *vmethod) const
{
    return verifier_->IsValidVerifiedMethod(vmethod);
}

bool EnvANIVerifier::IsValidField(impl::VField *vfield) const
{
    return verifier_->IsValidVerifiedField(vfield);
}

bool EnvANIVerifier::CanBeDeletedFromCurrentScope(VRef *vref)
{
    ASSERT(!frames_.empty());
    Frame &frame = frames_.back();
    return IsValidVerifiedRef(frame, vref);
}

/*static*/
bool EnvANIVerifier::IsValidVerifiedRef(const Frame &frame, VRef *vref)
{
    return frame.refs.find(vref) != frame.refs.cend();
}

VRef *EnvANIVerifier::AddGlobalVerifiedRef(ani_ref gref)
{
    return verifier_->AddGlobalVerifiedRef(gref);
}

void EnvANIVerifier::DeleteGlobalVerifiedRef(VRef *vgref)
{
    verifier_->DeleteGlobalVerifiedRef(vgref);
}

bool EnvANIVerifier::IsValidGlobalVerifiedRef(VRef *vgref) const
{
    return verifier_->IsValidGlobalVerifiedRef(vgref);
}

bool EnvANIVerifier::IsValidStackRef(VRef *vref) const
{
    return verifier_->IsValidStackRef(vref);
}

VResolver *EnvANIVerifier::AddGlobalVerifiedResolver(ani_resolver resolver)
{
    return verifier_->AddGlobalVerifiedResolver(resolver);
}

void EnvANIVerifier::DeleteGlobalVerifiedResolver(VResolver *vresolver)
{
    return verifier_->DeleteGlobalVerifiedResolver(vresolver);
}

bool EnvANIVerifier::IsValidGlobalVerifiedResolver(VResolver *vresolver) const
{
    return verifier_->IsValidGlobalVerifiedResolver(vresolver);
}

}  // namespace ark::ets::ani::verify
