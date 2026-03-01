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

#include "include/libabckit/cpp/abckit_cpp.h"
#include "include/libabckit/cpp/headers/core/annotation_interface.h"
#include "include/libabckit/cpp/headers/core/export_descriptor.h"
#include "include/libabckit/cpp/headers/core/function.h"
#include "include/libabckit/cpp/headers/core/import_descriptor.h"
#include "include/libabckit/cpp/headers/core/namespace.h"
#include "tests/mock/check_mock.h"
#include "src/mock/mock_values.h"
#include "tests/mock/cpp_helpers_mock.h"
#include <gtest/gtest.h>

namespace libabckit::cpp_test {

class LibAbcKitCppMockCoreTestModule : public ::testing::Test {};

// Test: test-kind=mock, api=Module::GetFile, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_GetFile)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).GetFile();
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::GetTarget, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_GetTarget)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).GetTarget();
        ASSERT_TRUE(CheckMockedApi("ModuleGetTarget"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::GetName, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_GetName)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).GetName();
        ASSERT_TRUE(CheckMockedApi("AbckitStringToString"));
        ASSERT_TRUE(CheckMockedApi("ModuleGetName"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::IsExternal, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_IsExternal)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).IsExternal();
        ASSERT_TRUE(CheckMockedApi("ModuleIsExternal"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::GetClasses, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_GetClasses)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).GetClasses();
        ASSERT_TRUE(CheckMockedApi("ModuleEnumerateClasses"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::GetInterfaces, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_GetInterfaces)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).GetInterfaces();
        ASSERT_TRUE(CheckMockedApi("ModuleEnumerateInterfaces"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::GetEnums, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_GetEnums)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).GetEnums();
        ASSERT_TRUE(CheckMockedApi("ModuleEnumerateEnums"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::GetTopLevelFunctions, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_GetTopLevelFunctions)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).GetTopLevelFunctions();
        ASSERT_TRUE(CheckMockedApi("ModuleEnumerateTopLevelFunctions"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::GetFields, abc-kind=ArkTS2, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_GetFields)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).GetFields();
        ASSERT_TRUE(CheckMockedApi("ModuleEnumerateFields"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::GetAnnotationInterfaces, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_GetAnnotationInterfaces)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).GetAnnotationInterfaces();
        ASSERT_TRUE(CheckMockedApi("ModuleEnumerateAnnotationInterfaces"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::GetNamespaces, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_GetNamespaces)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).GetNamespaces();
        ASSERT_TRUE(CheckMockedApi("ModuleEnumerateNamespaces"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::GetImports, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_GetImports)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).GetImports();
        ASSERT_TRUE(CheckMockedApi("ModuleEnumerateImports"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::GetExports, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_GetExports)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).GetExports();
        ASSERT_TRUE(CheckMockedApi("ModuleEnumerateExports"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::EnumerateNamespaces, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_EnumerateNamespaces)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).EnumerateNamespaces(DEFAULT_LAMBDA(abckit::core::Namespace));
        ASSERT_TRUE(CheckMockedApi("ModuleEnumerateNamespaces"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::EnumerateTopLevelFunctions, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_EnumerateTopLevelFunctions)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).EnumerateTopLevelFunctions(DEFAULT_LAMBDA(abckit::core::Function));
        ASSERT_TRUE(CheckMockedApi("ModuleEnumerateTopLevelFunctions"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::EnumerateClasses, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_EnumerateClasses)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).EnumerateClasses(DEFAULT_LAMBDA(abckit::core::Class));
        ASSERT_TRUE(CheckMockedApi("ModuleEnumerateClasses"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::EnumerateImports, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_EnumerateImports)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).EnumerateImports(DEFAULT_LAMBDA(abckit::core::ImportDescriptor));
        ASSERT_TRUE(CheckMockedApi("ModuleEnumerateImports"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::EnumerateAnonymousFunctions, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_EnumerateAnonymousFunctions)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).EnumerateAnonymousFunctions(DEFAULT_LAMBDA(abckit::core::Function));
        ASSERT_TRUE(CheckMockedApi("ModuleEnumerateAnonymousFunctions"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::EnumerateExports, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_EnumerateExports)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).EnumerateExports(DEFAULT_LAMBDA(abckit::core::ExportDescriptor));
        ASSERT_TRUE(CheckMockedApi("ModuleEnumerateExports"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

// Test: test-kind=mock, api=Module::EnumerateAnnotationInterfaces, abc-kind=ArkTS1, category=internal, extension=cpp
TEST_F(LibAbcKitCppMockCoreTestModule, Module_EnumerateAnnotationInterfaces)
{
    ASSERT_TRUE(CheckMockedStackEmpty());
    {
        abckit::File f(DEFAULT_PATH);
        ASSERT_TRUE(CheckMockedApi("OpenAbc"));
        abckit::mock::helpers::GetMockCoreModule(f).EnumerateAnnotationInterfaces(
            DEFAULT_LAMBDA(abckit::core::AnnotationInterface));
        ASSERT_TRUE(CheckMockedApi("ModuleEnumerateAnnotationInterfaces"));
    }
    ASSERT_TRUE(CheckMockedApi("CloseFile"));
    ASSERT_TRUE(CheckMockedStackEmpty());
}

}  // namespace libabckit::cpp_test