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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_MEM_ROOT_PROVIDER_H
#define PANDA_PLUGINS_ETS_RUNTIME_MEM_ROOT_PROVIDER_H

#include "libarkbase/mem/mem.h"

namespace ark::mem {

/// @class provides common interface for gc root processing
class RootProvider {
public:
    RootProvider() = default;
    NO_COPY_SEMANTIC(RootProvider);
    NO_MOVE_SEMANTIC(RootProvider);
    virtual ~RootProvider() = default;

    /**
     * Provides interface for root visiting during GC pause
     * @param visitor gc roor visitor
     */
    virtual void VisitRoots(const GCRootVisitor &visitor) = 0;

    /// Provides interface for object updating in references during GC pause phase
    virtual void UpdateRefs(const GCRootUpdater &gcRootUpdater) = 0;
};

}  // namespace ark::mem

#endif  // PANDA_PLUGINS_ETS_RUNTIME_MEM_ROOT_PROVIDER_H
