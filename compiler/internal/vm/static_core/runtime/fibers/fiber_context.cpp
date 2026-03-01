/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <securec.h>
#include "libarkbase/macros.h"
#include "runtime/fibers/fiber_context.h"

namespace ark::fibers {

void CopyContext(FiberContext *dst, const FiberContext *src)
{
    ASSERT(dst != nullptr);
    ASSERT(src != nullptr);
    memcpy_s(dst, sizeof(FiberContext), src, sizeof(FiberContext));
}

}  // namespace ark::fibers