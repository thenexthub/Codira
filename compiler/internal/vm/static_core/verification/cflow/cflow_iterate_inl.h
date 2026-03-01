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

#ifndef PANDA_VERIFIER_CFLOW_ITER_INL_H_
#define PANDA_VERIFIER_CFLOW_ITER_INL_H_

#include "libarkfile/bytecode_instruction-inl.h"
#include "libarkfile/file_items.h"
#include "libarkbase/macros.h"
#include "include/method.h"
#include "include/runtime.h"
#include "verification/verification_status.h"

#include "libarkbase/utils/logger.h"

#include <cstdint>
#include <cmath>

#include <array>
#include <functional>
#include <iomanip>
#include <limits>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <optional>

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_INST()                                                                                        \
    LOG(DEBUG, VERIFIER) << "CFLOW: " << std::hex << std::setw(sizeof(uint32_t) * 2) << std::setfill('0') \
                         << inst_.GetOffset() << std::dec << ": " << inst_

namespace ark::verifier {
#include <cflow_iterate_inl_gen.h>
}  // namespace ark::verifier

#endif  // PANDA_VERIFIER_CFLOW_ITER_INL_H_
