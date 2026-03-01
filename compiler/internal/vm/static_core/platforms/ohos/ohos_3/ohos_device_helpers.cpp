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

#include "platforms/ohos/ohos_device_helpers.h"

#include "syspara/parameter.h"
#include "syspara/parameters.h"

namespace ark::ohos_device {
std::string GetHardwareModelString()
{
    return GetHardwareModel();
}

bool GetCoverageEnable()
{
    return OHOS::system::GetBoolParameter("persist.ark.static.codecoverage.enable", false);
}

std::string GetCodeCoverageOutput()
{
    return "/data/storage/el2/base/files/coverageBytecodeInfo.csv";
}
}  // namespace ark::ohos_device