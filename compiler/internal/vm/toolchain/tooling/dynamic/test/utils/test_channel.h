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

#ifndef ECMASCRIPT_TOOLING_TEST_UTILS_TEST_CHANNEL_H
#define ECMASCRIPT_TOOLING_TEST_UTILS_TEST_CHANNEL_H

#include "tooling/dynamic/protocol_channel.h"

namespace panda::ecmascript::tooling {
class TestChannel : public ProtocolChannel {
public:
    TestChannel() = default;
    virtual ~TestChannel() override = default;

    void WaitForDebugger() override
    {
        std::cout << "TestChannel: WaitForDebugger" << std::endl;
    }
    void RunIfWaitingForDebugger() override
    {
        std::cout << "TestChannel: RunIfWaitingForDebugger" << std::endl;
    }
    void SendResponse([[maybe_unused]] const DispatchRequest &request,
                      [[maybe_unused]] const DispatchResponse &response,
                      const PtBaseReturns &result) override
    {
        std::string str = result.ToJson()->Stringify();
        std::cout << "TestChannel: SendResponse:\n" << str << std::endl;
    }
    void SendNotification(const PtBaseEvents &events) override
    {
        std::string str = events.ToJson()->Stringify();
        std::cout << "TestChannel: SendNotification:\n" << str << std::endl;
    }

private:
    NO_COPY_SEMANTIC(TestChannel);
    NO_MOVE_SEMANTIC(TestChannel);
};
}  // namespace panda::ecmascript::tooling

#endif  // ECMASCRIPT_TOOLING_TEST_UTILS_TEST_CHANNEL_H