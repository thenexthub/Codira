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

#include <stack>
#include <string>
#include <iostream>
#include "logger.h"

#include "./check_mock.h"

std::stack<std::string> g_calledFuncs;

bool CheckMockedApi(const std::string &apiName)
{
    if (g_calledFuncs.empty()) {
        return false;
    }
    auto apiStr = std::move(g_calledFuncs.top());
    LIBABCKIT_LOG(DEBUG) << "TOP NAME " << apiStr << "\n";
    g_calledFuncs.pop();
    return apiStr == apiName;
}

bool CheckMockedStackEmpty()
{
    if (!g_calledFuncs.empty()) {
        std::cerr << "Stack remains: " << std::endl;
        while (!g_calledFuncs.empty()) {
            std::cerr << g_calledFuncs.top() << std::endl;
            g_calledFuncs.pop();
        }
        return false;
    }
    return g_calledFuncs.empty();
}
