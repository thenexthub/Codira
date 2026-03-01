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

#include <gtest/gtest.h>

#include "assembly-parser.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/mem/object_helpers-inl.h"

namespace ark::mem {
inline std::string Separator()
{
#ifdef _WIN32
    return "\\";
#else
    return "/";
#endif
}

class StaticAnalyzerTest : public testing::Test {
public:
    StaticAnalyzerTest()
    {
        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        options.SetGcType("epsilon");
        options.SetGcTriggerType("debug-never");
        auto execPath = ark::os::file::File::GetExecutablePath();
        std::string pandaStdLib =
            execPath.Value() + Separator() + ".." + Separator() + "pandastdlib" + Separator() + "pandastdlib.bin";
        options.SetBootPandaFiles({pandaStdLib});

        Runtime::Create(options);
    }

    ~StaticAnalyzerTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(StaticAnalyzerTest);
    NO_MOVE_SEMANTIC(StaticAnalyzerTest);

    coretypes::String *AllocString()
    {
        Runtime *runtime = Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        ScopedManagedCodeThread s(MTManagedThread::GetCurrent());
        return coretypes::String::CreateEmptyString(ctx, runtime->GetPandaVM());
    }

    coretypes::Array *AllocStringArray(size_t length)
    {
        Runtime *runtime = Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        SpaceType spaceType = SpaceType::SPACE_TYPE_OBJECT;
        auto *klass = runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_STRING);
        ScopedManagedCodeThread s(MTManagedThread::GetCurrent());
        return coretypes::Array::Create(klass, length, spaceType);
    }
};

TEST_F(StaticAnalyzerTest, TestArray)
{
    coretypes::Array *array = AllocStringArray(2);
    ASSERT_NE(nullptr, array);
    ObjectHeader *expected = AllocString();
    ASSERT_NE(nullptr, expected);
    array->Set(0U, expected);  // SUPPRESS_CSA
    // SUPPRESS_CSA_NEXTLINE
    array->Set(1U, expected);

    size_t count = 0;
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto handler = [array, &count, expected](ObjectHeader *obj, ObjectHeader *ref, uint32_t offset, bool isVolatile) {
        ++count;
        EXPECT_EQ(array, obj);
        EXPECT_EQ(expected, ref);
        EXPECT_EQ(ref, ObjectAccessor::GetObject<true>(obj, offset));
        EXPECT_FALSE(isVolatile);
        return true;
    };
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    GCStaticObjectHelpers::TraverseAllObjectsWithInfo<false>(array, handler);
    ASSERT_EQ(2U, count);
}

class ClearA {
    NO_COPY_SEMANTIC(ClearA);
    NO_MOVE_SEMANTIC(ClearA);

public:
    ClearA() = default;
    virtual ObjectHeader *VirtualAllocString([[maybe_unused]] StaticAnalyzerTest &sat)
    {
        return nullptr;
    }

    virtual ~ClearA() = default;
};

class TriggeredB : public ClearA {
    NO_COPY_SEMANTIC(TriggeredB);
    NO_MOVE_SEMANTIC(TriggeredB);

    ObjectHeader *VirtualAllocString(StaticAnalyzerTest &sat) override
    {
        return sat.AllocString();
    }

public:
    TriggeredB() = default;
    ~TriggeredB() override = default;
};

TEST_F(StaticAnalyzerTest, TestVirtualFuncs)
{
    coretypes::Array *array = AllocStringArray(2);
    ASSERT_NE(nullptr, array);
    std::unique_ptr<ClearA> ca = std::make_unique<TriggeredB>();
    ObjectHeader *expected = ca->VirtualAllocString(*this);
    ASSERT_NE(nullptr, expected);
    array->Set(0U, expected);  // SUPPRESS_CSA
}
}  // namespace ark::mem
