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

#ifndef PANDA_INTEROP_JS_GTEST_PLUGIN_INTEROP_TEST_HELPER
#define PANDA_INTEROP_JS_GTEST_PLUGIN_INTEROP_TEST_HELPER

#include <node_api.h>
#include <string_view>

namespace ark::ets::interop::js::helper {

/**
 * @brief Execute given .abc on @param path with given entrypoint @param testName.
 * @return Returns false if an exception occured during .abc file execution, returns true otherwise.
 */
bool RunAbcFileOnArkJSVM(napi_env env, const std::string_view path, std::string_view testName);

/// @brief Empty function to call in case of linking to binary without using library functions.
void LinkWithLibraryTrick();

}  // namespace ark::ets::interop::js::helper

#endif  // PANDA_INTEROP_JS_GTEST_PLUGIN_INTEROP_TEST_HELPER
