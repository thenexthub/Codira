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

#include "runtime/runtime_helpers.h"
#include "runtime/include/object_header-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/include/stack_walker.h"
#include "runtime/include/thread.h"
#include "runtime/interpreter/runtime_interface.h"
#include "runtime/mem/object_helpers.h"

namespace ark {

void PrintStackTrace()
{
    auto thread = ManagedThread::GetCurrent();
    auto walker = StackWalker::Create(thread);
    LOG(ERROR, RUNTIME) << "====================== Stack trace begin ======================";
    for (auto stack = StackWalker::Create(thread); stack.HasFrame(); stack.NextFrame()) {
        Method *method = stack.GetMethod();
        ASSERT(method != nullptr);
        LOG(ERROR, RUNTIME) << method->GetClass()->GetName() << "." << method->GetName().data << " at "
                            << method->GetLineNumberAndSourceFile(stack.GetBytecodePc());
    }
    LOG(ERROR, RUNTIME) << "====================== Stack trace end ======================";
}
}  // namespace ark
