/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "default_config.h"

namespace ark::verifier::config {

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
const char VERIFIER_DEBUG_DEFAULT_CONFIG[] =
    "debug {\n"
    "  options {\n"
    "    verifier {\n"
    "      allow {\n"
    "        undefined-method\n"
    "      }\n"
    "    }\n"
    "  }\n"
    "  method_options {\n"
    "    verifier {\n"
    "      default {\n"
    "        check {\n"
    "          cflow, resolve-id, typing, absint\n"
    "        }\n"
    "      }\n"
    "    }\n"
    "  }\n"
    "}\n";

}  // namespace ark::verifier::config
