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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_STDLIB_CACHE_H
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_STDLIB_CACHE_H

#include "plugins/ets/runtime/ani/ani.h"
#include "include/mem/panda_smart_pointers.h"

namespace ark::ets {
struct StdlibCache {
    // NOLINTBEGIN(readability-identifier-naming)
    ani_module std_core;
    ani_class std_core_String_Builder;
    ani_class std_core_Console;
    ani_class escompat_Array;
    ani_method std_core_Console_error;
    ani_method std_core_String_Builder_default_ctor;
    ani_method std_core_String_Builder_append;
    ani_method std_core_String_Builder_toString;
    ani_method escompat_Array_pushOne;
    ani_variable std_core_console;
    // NOLINTEND(readability-identifier-naming)
};

PandaUniquePtr<StdlibCache> CreateStdLibCache(ani_env *env);
}  // namespace ark::ets
#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_STDLIB_CACHE_H
