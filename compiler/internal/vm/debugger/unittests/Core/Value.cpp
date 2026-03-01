//===----------------------------------------------------------------------===//
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

#include "lldb/Core/Value.h"
#include "Plugins/Platform/MacOSX/PlatformMacOSX.h"
#include "Plugins/TypeSystem/Clang/TypeSystemClang.h"
#include "TestingSupport/SubsystemRAII.h"
#include "TestingSupport/Symbol/ClangTestUtils.h"

#include "lldb/Utility/DataExtractor.h"

#include "gtest/gtest.h"

using namespace lldb_private;
using namespace lldb_private::clang_utils;

TEST(ValueTest, GetValueAsData) {
  SubsystemRAII<FileSystem, HostInfo, PlatformMacOSX> subsystems;
  auto holder = std::make_unique<clang_utils::TypeSystemClangHolder>("test");
  auto *clang = holder->GetAST();

  Value v(Scalar(42));
  DataExtractor extractor;

  // no compiler type
  Status status = v.GetValueAsData(nullptr, extractor, nullptr);
  ASSERT_TRUE(status.Fail());

  // with compiler type
  v.SetCompilerType(clang->GetBasicType(lldb::BasicType::eBasicTypeChar));

  status = v.GetValueAsData(nullptr, extractor, nullptr);
  ASSERT_TRUE(status.Success());
}
