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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TASKPOOL_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TASKPOOL_H

namespace ark::ets::intrinsics::taskpool {

constexpr const char *TASKPOOL_LAUNCH_MODE = "launch";
constexpr const char *TASKPOOL_EAWORKER_MODE = "eaworker";
// taskpool eaworker limit
constexpr uint32_t TASKPOOL_EAWORKER_INIT_NUM = 3;

}  // namespace ark::ets::intrinsics::taskpool

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TASKPOOL_H