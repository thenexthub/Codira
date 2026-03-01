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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <sstream>
#include <type_traits>
#include <vector>

#include "assembly-parser.h"
#include "libarkbase/mem/pool_manager.h"
#include "libarkbase/utils/utf.h"
#include "libarkfile/bytecode_emitter.h"
#include "libarkfile/file.h"
#include "libarkfile/file_items.h"
#include "libarkfile/value.h"
#include "runtime/bridge/bridge.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "runtime/interpreter/frame.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/internal_allocator.h"
#include "runtime/core/core_class_linker_extension.h"
#include "runtime/tests/class_linker_test_extension.h"
#include "runtime/tests/interpreter/test_interpreter.h"
#include "runtime/tests/interpreter/test_runtime_interface.h"
#include "runtime/include/coretypes/dyn_objects.h"
#include "runtime/include/hclass.h"
#include "runtime/handle_base-inl.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/coretypes/native_pointer.h"

namespace ark::interpreter::test {

using DynClass = ark::coretypes::DynClass;
using DynObject = ark::coretypes::DynObject;

class InterpreterTestResolveCtorClass : public testing::Test {
public:
    InterpreterTestResolveCtorClass()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetRunGcInPlace(true);
        options.SetVerifyCallStack(false);
        options.SetGcType("epsilon");
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~InterpreterTestResolveCtorClass() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(InterpreterTestResolveCtorClass);
    NO_MOVE_SEMANTIC(InterpreterTestResolveCtorClass);

private:
    ark::MTManagedThread *thread_;
};

TEST_F(InterpreterTestResolveCtorClass, ResolveCtorClass)
{
    auto pf = ark::panda_file::File::Open("../bin-gtests/pre-build/interpreter_test_resolve_ctor_class.abc");
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    auto *ext = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    Class *klass = ext->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("R2"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("foo"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    Value v = method->Invoke(ManagedThread::GetCurrent(), args.data());
    ASSERT_FALSE(ManagedThread::GetCurrent()->HasPendingException());

    auto *ret = v.GetAs<ObjectHeader *>();
    ASSERT_NE(ret, nullptr);

    ASSERT_EQ(ret->ClassAddr<ark::Class>()->GetName(), "R1");
}

}  // namespace ark::interpreter::test