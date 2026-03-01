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

#include <cstdint>

#include "libarkfile/file.h"
#include "runtime/include/class.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/native_pointer.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/field.h"
#include "runtime/include/hclass.h"
#include "runtime/include/method.h"
#include "runtime/include/mtmanaged_thread.h"
#include "runtime/jit/profiling_data.h"
#include "runtime/mem/tlab.h"
#include "libarkbase/utils/cframe_layout.h"
#include "runtime/include/flattened_string_cache.h"

#include "plugins_defines.h"

namespace ark {
// NOLINTBEGIN(hicpp-no-assembler)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFINE_VALUE(name, value)                                                         \
    void AsmDefinition_##name()                                                           \
    {                                                                                     \
        asm volatile("\n.ascii \"^^" #name " %0^^\"" ::"i"(static_cast<int64_t>(value))); \
    }
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFINE_VALUE_WITH_TYPE(name, value, type)                                                   \
    void AsmDefinition_##name()                                                                     \
    {                                                                                               \
        asm volatile("\n.ascii \"^^" #name " %0 " #type "^^\"" ::"i"(static_cast<int64_t>(value))); \
    }
#include "asm_defines.def"
// NOLINTEND(hicpp-no-assembler)

}  // namespace ark
