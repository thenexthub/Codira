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

#ifndef PANDA_GUARD_CONFIGS_GUARD_GRAPH_CONTEXT_H
#define PANDA_GUARD_CONFIGS_GUARD_GRAPH_CONTEXT_H

#include "libpandafile/file.h"

namespace panda::guard {

class GraphContext {
public:
    explicit GraphContext(const panda_file::File &file) : file_(file) {};

    void Init();

    static void Finalize();

    [[nodiscard]] const panda_file::File &GetAbcFile() const;

    /**
     * Get the MethodPtr corresponding to the function,
     * which is the offset value of the function in the abc file
     */
    [[nodiscard]] uint32_t FillMethodPtr(const std::string &recordName, const std::string &rawName) const;

private:
    [[nodiscard]] std::string GetStringById(const panda_file::File::EntityId &entity_id) const;

    const panda_file::File &file_;

    std::map<std::string, uint32_t> recordNameMap_ {};
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_CONFIGS_GUARD_GRAPH_CONTEXT_H
