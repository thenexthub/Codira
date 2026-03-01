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

/**
 * @file
 *
 * This file implements the Backend related classes.
 */

#include "Codira/Driver/Backend/Backend.h"

#include "Codira/Basic/Print.h"
#include "Job.h"

namespace {
// List of extensions of output files which should not be customized.
const std::string AST_EXT = "codeo";    // *.ast file, otherwise it will shadow the real ast file.

using namespace Codira;

// Check file name for final output.
bool CheckOutputNameByTrustList(const std::string& file)
{
    auto ext = FileUtil::GetFileExtension(file);
    if (ext == AST_EXT) {
        return false;
    }

    return true;
}
}; // namespace

bool Backend::Generate()
{
    if (!GenerateToolChain() || !TC) {
        return false;
    }
    TC->InitializeLibraryPaths();
    if (!PrepareDependencyPath()) {
        return false;
    }
    // Check invalid suffix of output name for different backends
    if (!FileUtil::IsDir(driverOptions.output) &&
        !CheckOutputNameByTrustList(driverOptions.output)) {
        Errorf("file extension '.%s' is not allowed, please change it\n",
            FileUtil::GetFileExtension(driverOptions.output).c_str());
        return false;
    }
    return ProcessGeneration();
}
