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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_CALL_STACKS_H
#define PANDA_PLUGINS_ETS_RUNTIME_CALL_STACKS_H

#include "libarkbase/macros.h"
#include "libarkbase/utils/span.h"

namespace ark::ets {

class EtsCallStack {
public:
    EtsCallStack() = default;
    NO_COPY_SEMANTIC(EtsCallStack);
    NO_MOVE_SEMANTIC(EtsCallStack);
    virtual ~EtsCallStack() = default;

    struct Record {
        Record(void *etsCurFrame, char const *codeDescr) : etsFrame(etsCurFrame), descr(codeDescr) {}

        void *etsFrame {};     // NOLINT(misc-non-private-member-variables-in-classes)
        char const *descr {};  // NOLINT(misc-non-private-member-variables-in-classes)
    };

    /// The method returns record of current managed frame
    virtual Record *Current() = 0;

    /// The method returns all records of managed call stack
    virtual Span<Record> GetRecords() = 0;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_CALL_STACKS_H