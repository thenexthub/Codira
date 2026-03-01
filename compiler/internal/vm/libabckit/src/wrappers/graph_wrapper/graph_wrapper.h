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

#ifndef LIBABCKIT_SRC_WRAPPERS_GRAPH_WRAPPER_H
#define LIBABCKIT_SRC_WRAPPERS_GRAPH_WRAPPER_H

#include "libabckit/c/metadata_core.h"
#include "libabckit/c/statuses.h"
#include "libabckit/src/ir_impl.h"

#include <cstdint>
#include <cstddef>
#include <string>

#include "libabckit/src/wrappers/abcfile_wrapper.h"
#include "libabckit/src/wrappers/pandasm_wrapper.h"

namespace libabckit {

class GraphWrapper {
public:
    static void CreateGraphWrappers(AbckitGraph *graph);
    static AbckitGraph *BuildGraphDynamic(FileWrapper *pf, AbckitIrInterface *irInterface, AbckitFile *file,
                                          uint32_t methodOffset);
    static void *BuildCodeDynamic(AbckitGraph *graph, const std::string &funcName);
    static void DestroyGraphDynamic(AbckitGraph *graph);
};

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_WRAPPERS_GRAPH_WRAPPER_H
