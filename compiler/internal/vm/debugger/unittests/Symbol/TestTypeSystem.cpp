//===-- TestTypeSystem.cpp -------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "TestingSupport/SubsystemRAII.h"
#include "lldb/Core/Module.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Symbol/TypeSystem.h"
#include "gtest/gtest.h"

using namespace lldb;
using namespace lldb_private;

class TestTypeSystemMap : public testing::Test {
public:
  SubsystemRAII<FileSystem, HostInfo> subsystems;
};

TEST_F(TestTypeSystemMap, GetTypeSystemForLanguageWithInvalidModule) {
  // GetTypeSystemForLanguage called with an invalid Module.
  TypeSystemMap map;
  Module module{ModuleSpec()};
  EXPECT_THAT_EXPECTED(
      map.GetTypeSystemForLanguage(eLanguageTypeUnknown, &module,
                                   /*can_create=*/true),
      llvm::FailedWithMessage("TypeSystem for language unknown doesn't exist"));

  EXPECT_THAT_EXPECTED(
      map.GetTypeSystemForLanguage(eLanguageTypeUnknown, &module,
                                   /*can_create=*/false),
      llvm::FailedWithMessage("TypeSystem for language unknown doesn't exist"));

  EXPECT_THAT_EXPECTED(
      map.GetTypeSystemForLanguage(eLanguageTypeC, &module,
                                   /*can_create=*/true),
      llvm::FailedWithMessage("TypeSystem for language c doesn't exist"));
  EXPECT_THAT_EXPECTED(
      map.GetTypeSystemForLanguage(eLanguageTypeC, &module,
                                   /*can_create=*/false),
      llvm::FailedWithMessage("TypeSystem for language c doesn't exist"));
}

TEST_F(TestTypeSystemMap, GetTypeSystemForLanguageWithNoModule) {
  // GetTypeSystemForLanguage called with no Module.
  TypeSystemMap map;
  Module *module = nullptr;
  EXPECT_THAT_EXPECTED(
      map.GetTypeSystemForLanguage(eLanguageTypeUnknown, module,
                                   /*can_create=*/true),
      llvm::FailedWithMessage("TypeSystem for language unknown doesn't exist"));

  EXPECT_THAT_EXPECTED(
      map.GetTypeSystemForLanguage(eLanguageTypeUnknown, module,
                                   /*can_create=*/false),
      llvm::FailedWithMessage("TypeSystem for language unknown doesn't exist"));

  EXPECT_THAT_EXPECTED(
      map.GetTypeSystemForLanguage(eLanguageTypeC, module, /*can_create=*/true),
      llvm::FailedWithMessage("TypeSystem for language c doesn't exist"));
  EXPECT_THAT_EXPECTED(
      map.GetTypeSystemForLanguage(eLanguageTypeC, module,
                                   /*can_create=*/false),
      llvm::FailedWithMessage("TypeSystem for language c doesn't exist"));
}

TEST_F(TestTypeSystemMap, GetTypeSystemForLanguageWithNoTarget) {
  // GetTypeSystemForLanguage called with no Target.
  TypeSystemMap map;
  Target *target = nullptr;
  EXPECT_THAT_EXPECTED(
      map.GetTypeSystemForLanguage(eLanguageTypeUnknown, target,
                                   /*can_create=*/true),
      llvm::FailedWithMessage("TypeSystem for language unknown doesn't exist"));

  EXPECT_THAT_EXPECTED(
      map.GetTypeSystemForLanguage(eLanguageTypeUnknown, target,
                                   /*can_create=*/false),
      llvm::FailedWithMessage("TypeSystem for language unknown doesn't exist"));

  EXPECT_THAT_EXPECTED(
      map.GetTypeSystemForLanguage(eLanguageTypeC, target, /*can_create=*/true),
      llvm::FailedWithMessage("TypeSystem for language c doesn't exist"));
  EXPECT_THAT_EXPECTED(
      map.GetTypeSystemForLanguage(eLanguageTypeC, target,
                                   /*can_create=*/false),
      llvm::FailedWithMessage("TypeSystem for language c doesn't exist"));
}
