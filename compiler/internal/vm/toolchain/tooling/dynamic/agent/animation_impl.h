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

#ifndef ECMASCRIPT_TOOLING_AGENT_ANIMATION_IMPL_H
#define ECMASCRIPT_TOOLING_AGENT_ANIMATION_IMPL_H

#include "base/pt_params.h"
#include "base/pt_returns.h"
#include "dispatcher.h"

#include "libpandabase/macros.h"

namespace panda::ecmascript::tooling {
class AnimationImpl final {
public:
    AnimationImpl() = default;
    ~AnimationImpl() = default;

    class DispatcherImpl final : public DispatcherBase {
    public:
        DispatcherImpl(ProtocolChannel *channel, std::unique_ptr<AnimationImpl> animation)
            : DispatcherBase(channel), animation_(std::move(animation)) {}
        ~DispatcherImpl() override = default;
        void Disable(const DispatchRequest &request);
        std::optional<std::string> Dispatch(const DispatchRequest &request, bool crossLanguageDebug = false) override;

        enum class Method {
            DISABLE,
            UNKNOWN
        };
        Method GetMethodEnum(const std::string& method);

    private:
        NO_COPY_SEMANTIC(DispatcherImpl);
        NO_MOVE_SEMANTIC(DispatcherImpl);

        std::unique_ptr<AnimationImpl> animation_ {};
    };

private:
    NO_COPY_SEMANTIC(AnimationImpl);
    NO_MOVE_SEMANTIC(AnimationImpl);
};
}  // namespace panda::ecmascript::tooling
#endif