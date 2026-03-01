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
#ifndef ANI_TEST_ANI_OPTION_LOGGER_CALLBACK_H
#define ANI_TEST_ANI_OPTION_LOGGER_CALLBACK_H

#include <string>
#include <vector>

namespace ark::ets::ani::testing {

struct Message {
    int level;
    std::string component;
    std::string message;
};

std::vector<Message> g_msgs;  // NOLINT(fuchsia-statically-constructed-objects, misc-definitions-in-headers)

inline void LoggerCallback([[maybe_unused]] FILE *stream, int level, const char *component, const char *message)
{
    g_msgs.push_back({level, component, message});
}

}  // namespace ark::ets::ani::testing

#endif  // ANI_TEST_ANI_OPTION_LOGGER_CALLBACK_H
