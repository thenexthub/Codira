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


#include "StackMetadataHelper.h"

#include "StackMap/StackMap.h"

namespace MapleRuntime {
// Normally, the file-level line number information will be returned.
// If it returns 0, it means that the file-level line number information
// cannot be found.
uint32_t StackMetadataHelper::GetLineNumber() const
{
    StackMapBuilder stackMapBuild(funcStartAddress, reinterpret_cast<uintptr_t>(funcPC), 0, funcDesc);

    MethodMap methodMap = stackMapBuild.Build<MethodMap>();
    // If the matching stackmap information is not found by pc,
    // the line number is 0
    return methodMap.IsValid() ? methodMap.GetLineNum() : 0;
}
} // namespace MapleRuntime
