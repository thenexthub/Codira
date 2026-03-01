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
#ifndef PANDA_RUNTIME_MEM_GC_GC_EXTENSION_DATA_H
#define PANDA_RUNTIME_MEM_GC_GC_EXTENSION_DATA_H

#include "libarkbase/macros.h"
#include "runtime/include/language_config.h"

namespace ark::mem {

// Base class for all GC language-specific data holders.
// Can be extended for different language types.
class GCExtensionData {
public:
    GCExtensionData() = default;
    virtual ~GCExtensionData() = default;
    NO_COPY_SEMANTIC(GCExtensionData);
    NO_MOVE_SEMANTIC(GCExtensionData);

#ifndef NDEBUG
    LangTypeT GetLangType()
    {
        return type_;
    }

protected:
    void SetLangType(LangTypeT type)
    {
        type_ = type;
    }

private:
    // Used for assertions in inherited classes to ensure that
    // language extension got the corresponding type of data
    LangTypeT type_ {LANG_TYPE_STATIC};
#endif  // NDEBUG
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_EXTENSION_DATA_H
