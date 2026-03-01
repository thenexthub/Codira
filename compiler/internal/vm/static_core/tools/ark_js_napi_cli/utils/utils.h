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

#ifndef ARK_JS_NAPI_CLI_UTILS_H
#define ARK_JS_NAPI_CLI_UTILS_H

#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"

#include <string>
#include <vector>

namespace panda::utils {
// CC-OFFNXT(G.FUN.01): public API
void GetAsset(const std::string &uri, uint8_t **buff, size_t *buffSize, std::vector<uint8_t> &content, std::string &ami,
              bool &useSecureMem, void **mapper, bool isRestricted);

}  // namespace panda::utils

#endif  // ARK_JS_NAPI_CLI_UTILS_H
