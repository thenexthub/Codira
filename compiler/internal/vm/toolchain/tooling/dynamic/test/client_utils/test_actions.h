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

#ifndef ECMASCRIPT_TOOLING_TEST_UTILS_TEST_ACTIONS_H
#define ECMASCRIPT_TOOLING_TEST_UTILS_TEST_ACTIONS_H

#include <functional>
#include <string>
#include <vector>

namespace panda::ecmascript::tooling::test {
/*
 * Socket action.
 * {SEND} Send a message to server.
 * {RECV} Should recv a message from server.
 */
enum class SocketAction {
    SEND = 0U,
    RECV = 1U,
};

/*
 * Action matching rules.
 * {STRING_EQUAL} Expect string should equal recv string.
 * {STRING_CONTAIN} Expect string should a substring of recv string.
 * {CUSTOM_RULE} Recv string should match custom rule.
 */
enum class ActionRule {
    STRING_EQUAL,
    STRING_CONTAIN,
    CUSTOM_RULE,
};
std::ostream &operator<<(std::ostream &out, ActionRule value);

using MatchFunc = std::function<bool(const std::string&, const std::string&, bool&)>;

enum class TestCase {
    COMMON,
    SOURCE,
    WATCH,
    WATCH_OBJECT,
};

/*
 * Add some common match func here
 */
struct MatchRule {
    static MatchFunc replySuccess;
};

/*
 * Action information.
 * @param action Socket action.
 * @param rule Action matching rules.
 * @param message Expect message or send message
 * @param matchFunc Custom matching rule
 */
struct ActionInfo {
    SocketAction action;
    std::string message;
    ActionRule rule = ActionRule::STRING_CONTAIN;
    MatchFunc matchFunc = [] (auto, auto, auto) -> auto {
        return true;
    };
    TestCase event = TestCase::COMMON;

    ActionInfo(const ActionInfo&) = default;
    ActionInfo& operator=(const ActionInfo&) = default;
};

struct TestActions {
    std::vector<ActionInfo> testAction;

    TestActions() = default;
    virtual ~TestActions() = default;

    virtual std::pair<std::string, std::string> GetEntryPoint() = 0;
};
}  // namespace panda::ecmascript::tooling::test

#endif  // ECMASCRIPT_TOOLING_TEST_UTILS_TEST_ACTIONS_H
