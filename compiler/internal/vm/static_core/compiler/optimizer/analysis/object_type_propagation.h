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

#ifndef PANDA_OBJECT_TYPE_PROPAGATION_H
#define PANDA_OBJECT_TYPE_PROPAGATION_H

#include "optimizer/analysis/live_in_analysis.h"
#include "optimizer/pass.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_visitor.h"

namespace ark::compiler {
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ObjectTypePropagation final : public Analysis {
public:
    explicit ObjectTypePropagation(Graph *graph);
    NO_MOVE_SEMANTIC(ObjectTypePropagation);
    NO_COPY_SEMANTIC(ObjectTypePropagation);
    ~ObjectTypePropagation() override = default;

    const char *GetPassName() const override
    {
        return "ObjectTypePropagation";
    }

    bool RunImpl() override;
};
}  // namespace ark::compiler

#endif  // PANDA_OBJECT_TYPE_PROPAGATION_H
