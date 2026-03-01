/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#ifndef PANDA_COHERENCY_LINE_SIZE_H_
#define PANDA_COHERENCY_LINE_SIZE_H_

#include <cstddef>
#include "libarkbase/cpu_features.h"

namespace ark {
inline constexpr size_t COHERENCY_LINE_SIZE = CACHE_LINE_SIZE;
}  // namespace ark

#endif  // PANDA_COHERENCY_LINE_SIZE_H_
