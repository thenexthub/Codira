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

#ifndef PANDA_VERIF_CONFIG_PARSE_H_
#define PANDA_VERIF_CONFIG_PARSE_H_

#include "verification/config/config.h"

namespace ark::verifier::config {
bool ParseConfig(const char *str, Section &cfg);
}  // namespace ark::verifier::config

#endif  // PANDA_VERIF_CONFIG_PARSE_H_
