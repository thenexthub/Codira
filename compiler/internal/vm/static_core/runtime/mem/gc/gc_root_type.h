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

#ifndef PANDA_RUNTIME_MEM_ROOT_TYPE_H
#define PANDA_RUNTIME_MEM_ROOT_TYPE_H

#include <iostream>
#include "libarkbase/macros.h"
namespace ark::mem {
enum class RootType {
    ROOT_UNKNOWN = 0,
    ROOT_CLASS,
    ROOT_FRAME,
    ROOT_THREAD,
    ROOT_CLASS_LINKER,
    ROOT_TENURED,
    ROOT_VM,
    ROOT_NATIVE_GLOBAL,
    ROOT_NATIVE_LOCAL,
    ROOT_PT_LOCAL,
    ROOT_AOT_STRING_SLOT,
    SATB_BUFFER,
    STRING_TABLE,
    CARD_TABLE,
};

PANDA_PUBLIC_API std::ostream &operator<<(std::ostream &os, RootType rootType);
}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_ROOT_TYPE_H
