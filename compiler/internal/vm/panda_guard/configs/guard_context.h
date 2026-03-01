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

#ifndef PANDA_GUARD_CONFIGS_GUARD_CONTEXT_H
#define PANDA_GUARD_CONFIGS_GUARD_CONTEXT_H

#include <memory>

#include "generator/name_mapping.h"
#include "guard_graph_context.h"
#include "guard_name_cache.h"
#include "guard_options.h"

namespace panda::guard {

class GuardContext {
public:
    static std::shared_ptr<GuardContext> GetInstance();

    void Init(int argc, const char **argv);

    void Finalize();

    [[nodiscard]] const std::shared_ptr<GuardOptions> &GetGuardOptions() const;

    [[nodiscard]] const std::shared_ptr<NameCache> &GetNameCache() const;

    [[nodiscard]] const std::shared_ptr<NameMapping> &GetNameMapping() const;

    void CreateGraphContext(const panda_file::File &file);

    [[nodiscard]] const std::shared_ptr<GraphContext> &GetGraphContext() const;

    [[nodiscard]] bool IsDebugMode() const;

private:
    std::shared_ptr<GuardOptions> options_;

    std::shared_ptr<NameCache> nameCache_;

    std::shared_ptr<NameMapping> nameMapping_;

    std::shared_ptr<GraphContext> graphContext_;

    bool debugMode_ = false;
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_CONFIGS_GUARD_CONTEXT_H
