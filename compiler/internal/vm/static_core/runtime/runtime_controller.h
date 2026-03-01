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

#ifndef RUNTIME_RUNTIME_CONTROLLER_H_
#define RUNTIME_RUNTIME_CONTROLLER_H_

#include <string_view>

#include "libarkbase/macros.h"

namespace ark {

/// For application related feature.
class RuntimeController {
public:
    RuntimeController() = default;

    ~RuntimeController() = default;

    bool IsZidaneApp() const
    {
        return isZidaneApp_;
    }

    void SetZidaneApp(bool isZidaneApp)
    {
        isZidaneApp_ = isZidaneApp;
    }

    bool IsMultiFramework() const
    {
        return isMultiFramework_;
    }

    void SetMultiFramework(bool isMultiFramework)
    {
        isMultiFramework_ = isMultiFramework;
    }

    PANDA_PUBLIC_API bool CanLoadPandaFileInternal(std::string_view realPath) const;

    bool CanLoadPandaFile(const std::string &path) const;

    NO_COPY_SEMANTIC(RuntimeController);
    NO_MOVE_SEMANTIC(RuntimeController);

private:
    bool isZidaneApp_ {false};
    bool isMultiFramework_ {false};
};

}  // namespace ark

#endif
