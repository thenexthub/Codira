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

#ifndef LIBPANDAFILE_PGO_H
#define LIBPANDAFILE_PGO_H

#include "libarkfile/file_items.h"

namespace ark::panda_file::pgo {

class ProfileOptimizer {
public:
    ProfileOptimizer() = default;
    ~ProfileOptimizer() = default;
    static std::string GetNameInfo(const std::unique_ptr<BaseItem> &item);
    void MarkProfileItem(std::unique_ptr<BaseItem> &item, bool setPgo) const;
    bool ParseProfileData();
    void ProfileGuidedRelayout(std::list<std::unique_ptr<BaseItem>> &items);
    void SetProfilePath(std::string &filePath)
    {
        profileFilePath_ = std::move(filePath);
    }

private:
    NO_COPY_SEMANTIC(ProfileOptimizer);
    NO_MOVE_SEMANTIC(ProfileOptimizer);
    std::string profileFilePath_;
    std::vector<std::pair<std::string, std::string>> profileData_;
};
}  // namespace ark::panda_file::pgo

#endif  // LIBPANDAFILE_PGO_H
