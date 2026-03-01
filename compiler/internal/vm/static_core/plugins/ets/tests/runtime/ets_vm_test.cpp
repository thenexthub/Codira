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

#include "libarkbase/macros.h"
#include <libarkfile/include/source_lang_enum.h>

#include "libarkbase/utils/logger.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "runtime/include/class_linker_extension.h"
#include "runtime/include/class_root.h"
#include "runtime/include/coretypes/class.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/runtime.h"

namespace ark::ets::test {

class EtsVMTest : public testing::Test {
public:
    EtsVMTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(false);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});

        Logger::InitializeStdLogging(Logger::Level::ERROR, 0);

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        ark::Runtime::Create(options);
    }
    ~EtsVMTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsVMTest);
    NO_MOVE_SEMANTIC(EtsVMTest);
};

static void AssertCompoundClassRoot(EtsClassLinker *classLinker, EtsClassRoot root)
{
    EtsClass *klass = classLinker->GetClassRoot(root);
    Class *coreClass = klass->GetRuntimeClass();

    ASSERT_NE(klass, nullptr);
    ASSERT_TRUE(klass->IsInitialized());
    ASSERT_TRUE(coreClass->IsInstantiable());
    ASSERT_FALSE(klass->IsArrayClass());
    ASSERT_FALSE(klass->IsPrimitive());
    ASSERT_TRUE(klass->IsClass());
    ASSERT_FALSE(klass->IsInterface());
    ASSERT_FALSE(klass->IsAbstract());
    ASSERT_FALSE(coreClass->IsProxy());
    ASSERT_EQ(coreClass->GetLoadContext(), classLinker->GetEtsClassLinkerExtension()->GetBootContext());

    if (root == EtsClassRoot::OBJECT) {
        ASSERT_TRUE(coreClass->IsObjectClass());
        ASSERT_EQ(klass->GetBase(), nullptr);
        ASSERT_EQ(klass->GetSuperClass(), nullptr);
    } else {
        ASSERT_FALSE(coreClass->IsObjectClass());
        ASSERT_NE(klass->GetBase(), nullptr);
        ASSERT_EQ(klass->GetBase(), classLinker->GetClassRoot(EtsClassRoot::OBJECT));
        ASSERT_NE(klass->GetSuperClass(), nullptr);
        ASSERT_EQ(klass->GetSuperClass(), classLinker->GetClassRoot(EtsClassRoot::OBJECT));
    }
}

static void AssertStringClassRoot(EtsClassLinker *classLinker, EtsClassRoot root)
{
    EtsClass *klass = classLinker->GetClassRoot(root);
    Class *coreClass = klass->GetRuntimeClass();

    ASSERT_NE(klass, nullptr);
    ASSERT_TRUE(klass->IsInitialized());
    ASSERT_TRUE(coreClass->IsInstantiable());
    ASSERT_FALSE(klass->IsArrayClass());
    ASSERT_FALSE(klass->IsPrimitive());
    ASSERT_TRUE(klass->IsClass());
    ASSERT_FALSE(klass->IsInterface());
    ASSERT_FALSE(klass->IsAbstract());
    ASSERT_FALSE(coreClass->IsProxy());
    ASSERT_EQ(coreClass->GetLoadContext(), classLinker->GetEtsClassLinkerExtension()->GetBootContext());

    ASSERT_TRUE(klass->IsFinal());
    ASSERT_FALSE(coreClass->IsObjectClass());
    ASSERT_NE(klass->GetBase(), nullptr);
    ASSERT_EQ(klass->GetBase(), classLinker->GetClassRoot(EtsClassRoot::STRING));
    ASSERT_NE(klass->GetSuperClass(), nullptr);
    ASSERT_EQ(klass->GetSuperClass(), classLinker->GetClassRoot(EtsClassRoot::STRING));

    switch (root) {
        case EtsClassRoot::LINE_STRING:
            ASSERT_TRUE(coreClass->IsLineStringClass());
            break;
        case EtsClassRoot::SLICED_STRING:
            ASSERT_TRUE(coreClass->IsSlicedStringClass());
            break;
        case EtsClassRoot::TREE_STRING:
            ASSERT_TRUE(coreClass->IsTreeStringClass());
            break;
        default:
            UNREACHABLE();
    }
}

static void AssertCompoundContainerClassRoot(EtsClassLinker *classLinker, EtsClassRoot root)
{
    EtsClass *klass = classLinker->GetClassRoot(root);
    Class *coreClass = klass->GetRuntimeClass();

    ASSERT_NE(klass, nullptr);
    ASSERT_TRUE(klass->IsInitialized());
    ASSERT_TRUE(coreClass->IsInstantiable());
    ASSERT_TRUE(klass->IsArrayClass());
    ASSERT_FALSE(klass->IsPrimitive());
    ASSERT_TRUE(klass->IsClass());
    ASSERT_FALSE(klass->IsInterface());
    ASSERT_TRUE(klass->IsAbstract());
    ASSERT_FALSE(coreClass->IsProxy());
    ASSERT_EQ(coreClass->GetLoadContext(), classLinker->GetEtsClassLinkerExtension()->GetBootContext());
    ASSERT_FALSE(coreClass->IsObjectClass());
    ASSERT_NE(klass->GetBase(), nullptr);
    ASSERT_EQ(klass->GetBase(), classLinker->GetClassRoot(EtsClassRoot::OBJECT));
    ASSERT_NE(klass->GetSuperClass(), nullptr);
    ASSERT_EQ(klass->GetSuperClass(), classLinker->GetClassRoot(EtsClassRoot::OBJECT));
}

static void AssertPrimitiveClassRoot(EtsClassLinker *classLinker, EtsClassRoot root)
{
    EtsClass *klass = classLinker->GetClassRoot(root);
    Class *coreClass = klass->GetRuntimeClass();

    ASSERT_NE(klass, nullptr);
    ASSERT_TRUE(klass->IsInitialized());
    ASSERT_FALSE(coreClass->IsInstantiable());
    ASSERT_FALSE(klass->IsArrayClass());
    ASSERT_TRUE(klass->IsPrimitive());
    ASSERT_TRUE(klass->IsPublic());
    ASSERT_TRUE(klass->IsFinal());
    ASSERT_FALSE(klass->IsClass());
    ASSERT_FALSE(klass->IsInterface());
    ASSERT_TRUE(klass->IsAbstract());
    ASSERT_FALSE(coreClass->IsProxy());
    ASSERT_EQ(coreClass->GetLoadContext(), classLinker->GetEtsClassLinkerExtension()->GetBootContext());
    ASSERT_EQ(klass->GetBase(), nullptr);
    ASSERT_EQ(klass->GetSuperClass(), nullptr);
}

static void AssertPrimitiveContainerClassRoot(EtsClassLinker *classLinker, EtsClassRoot root)
{
    EtsClass *klass = classLinker->GetClassRoot(root);
    Class *coreClass = klass->GetRuntimeClass();

    ASSERT_NE(klass, nullptr);
    ASSERT_TRUE(klass->IsInitialized());
    ASSERT_TRUE(coreClass->IsInstantiable());
    ASSERT_TRUE(klass->IsArrayClass());
    ASSERT_FALSE(klass->IsPrimitive());
    ASSERT_TRUE(klass->IsPublic());
    ASSERT_TRUE(klass->IsFinal());
    ASSERT_TRUE(klass->IsClass());
    ASSERT_FALSE(klass->IsInterface());
    ASSERT_TRUE(klass->IsAbstract());
    ASSERT_FALSE(coreClass->IsProxy());
    ASSERT_EQ(coreClass->GetLoadContext(), classLinker->GetEtsClassLinkerExtension()->GetBootContext());
    ASSERT_NE(klass->GetBase(), nullptr);
    ASSERT_EQ(klass->GetBase(), classLinker->GetClassRoot(EtsClassRoot::OBJECT));
    ASSERT_NE(klass->GetSuperClass(), nullptr);
    ASSERT_EQ(klass->GetSuperClass(), classLinker->GetClassRoot(EtsClassRoot::OBJECT));
}

static void AssertPrimitiveClassRoots(EtsClassLinker *classLinker)
{
    AssertPrimitiveClassRoot(classLinker, EtsClassRoot::BOOLEAN);
    AssertPrimitiveClassRoot(classLinker, EtsClassRoot::BYTE);
    AssertPrimitiveClassRoot(classLinker, EtsClassRoot::SHORT);
    AssertPrimitiveClassRoot(classLinker, EtsClassRoot::CHAR);
    AssertPrimitiveClassRoot(classLinker, EtsClassRoot::INT);
    AssertPrimitiveClassRoot(classLinker, EtsClassRoot::LONG);
    AssertPrimitiveClassRoot(classLinker, EtsClassRoot::FLOAT);
    AssertPrimitiveClassRoot(classLinker, EtsClassRoot::DOUBLE);
}

TEST_F(EtsVMTest, InitTest)
{
    auto runtime = Runtime::GetCurrent();
    ASSERT_NE(runtime, nullptr);

    PandaEtsVM *etsVm = PandaEtsVM::GetCurrent();
    ASSERT_NE(etsVm, nullptr);

    EtsClassLinker *classLinker = etsVm->GetClassLinker();
    ASSERT_NE(classLinker, nullptr);

    EtsClassLinkerExtension *ext = classLinker->GetEtsClassLinkerExtension();
    ASSERT_NE(ext, nullptr);
    ASSERT_EQ(ext->GetLanguage(), panda_file::SourceLang::ETS);
    ASSERT_TRUE(ext->IsInitialized());

    AssertCompoundClassRoot(classLinker, EtsClassRoot::OBJECT);
    AssertCompoundClassRoot(classLinker, EtsClassRoot::CLASS);
    AssertCompoundClassRoot(classLinker, EtsClassRoot::STRING);
    AssertStringClassRoot(classLinker, EtsClassRoot::LINE_STRING);
    AssertStringClassRoot(classLinker, EtsClassRoot::SLICED_STRING);
    AssertStringClassRoot(classLinker, EtsClassRoot::TREE_STRING);

    AssertCompoundContainerClassRoot(classLinker, EtsClassRoot::STRING_ARRAY);

    AssertPrimitiveClassRoots(classLinker);

    AssertPrimitiveContainerClassRoot(classLinker, EtsClassRoot::BOOLEAN_ARRAY);
    AssertPrimitiveContainerClassRoot(classLinker, EtsClassRoot::BYTE_ARRAY);
    AssertPrimitiveContainerClassRoot(classLinker, EtsClassRoot::SHORT_ARRAY);
    AssertPrimitiveContainerClassRoot(classLinker, EtsClassRoot::CHAR_ARRAY);
    AssertPrimitiveContainerClassRoot(classLinker, EtsClassRoot::INT_ARRAY);
    AssertPrimitiveContainerClassRoot(classLinker, EtsClassRoot::LONG_ARRAY);
    AssertPrimitiveContainerClassRoot(classLinker, EtsClassRoot::FLOAT_ARRAY);
    AssertPrimitiveContainerClassRoot(classLinker, EtsClassRoot::DOUBLE_ARRAY);

    EtsCoroutine *etsCoroutine = EtsCoroutine::GetCurrent();
    ASSERT_NE(etsCoroutine, nullptr);

    auto vm = etsCoroutine->GetVM();
    ASSERT_EQ(vm->GetAssociatedThread(), etsCoroutine);

    ASSERT_NE(vm, nullptr);
    ASSERT_EQ(ext->GetLanguage(), vm->GetLanguageContext().GetLanguage());

    ASSERT_NE(vm->GetHeapManager(), nullptr);
    ASSERT_NE(vm->GetMemStats(), nullptr);

    ASSERT_NE(vm->GetGC(), nullptr);
    ASSERT_NE(vm->GetGCTrigger(), nullptr);
    ASSERT_NE(vm->GetGCStats(), nullptr);
    ASSERT_NE(vm->GetReferenceProcessor(), nullptr);

    ASSERT_NE(vm->GetGlobalObjectStorage(), nullptr);
    ASSERT_NE(vm->GetStringTable(), nullptr);

    ASSERT_NE(vm->GetMonitorPool(), nullptr);
    ASSERT_NE(vm->GetThreadManager(), nullptr);
    ASSERT_NE(vm->GetRendezvous(), nullptr);

    ASSERT_NE(vm->GetCompiler(), nullptr);
    ASSERT_NE(vm->GetCompilerRuntimeInterface(), nullptr);
}

}  // namespace ark::ets::test
