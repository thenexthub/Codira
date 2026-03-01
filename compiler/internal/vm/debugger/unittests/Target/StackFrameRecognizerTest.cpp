//===-- StackFrameRecognizerTest.cpp --------------------------------------===//
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

#include "lldb/Target/StackFrameRecognizer.h"
#include "Plugins/Platform/Linux/PlatformLinux.h"
#include "lldb/Core/Debugger.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/lldb-enumerations.h"
#include "lldb/lldb-forward.h"
#include "lldb/lldb-private-enumerations.h"
#include "lldb/lldb-private.h"
#include "llvm/Support/FormatVariadic.h"
#include "gtest/gtest.h"

using namespace lldb_private;
using namespace lldb;

namespace {
class StackFrameRecognizerTest : public ::testing::Test {
public:
  void SetUp() override {
    FileSystem::Initialize();
    HostInfo::Initialize();

    // Pretend Linux is the host platform.
    platform_linux::PlatformLinux::Initialize();
    ArchSpec arch("powerpc64-pc-linux");
    Platform::SetHostPlatform(
        platform_linux::PlatformLinux::CreateInstance(true, &arch));
  }

  void TearDown() override {
    platform_linux::PlatformLinux::Terminate();
    HostInfo::Terminate();
    FileSystem::Terminate();
  }
};

class DummyStackFrameRecognizer : public StackFrameRecognizer {
public:
  std::string GetName() override { return "Dummy StackFrame Recognizer"; }
};

void RegisterDummyStackFrameRecognizer(StackFrameRecognizerManager &manager) {
  RegularExpressionSP module_regex_sp = nullptr;
  RegularExpressionSP symbol_regex_sp(new RegularExpression("boom"));

  StackFrameRecognizerSP dummy_recognizer_sp(new DummyStackFrameRecognizer());

  manager.AddRecognizer(dummy_recognizer_sp, module_regex_sp, symbol_regex_sp,
                        Mangled::NamePreference::ePreferDemangled, false);
}

} // namespace

TEST_F(StackFrameRecognizerTest, NullModuleRegex) {
  DebuggerSP debugger_sp = Debugger::CreateInstance();
  ASSERT_TRUE(debugger_sp);

  StackFrameRecognizerManager manager;

  RegisterDummyStackFrameRecognizer(manager);

  bool any_printed = false;
  manager.ForEach([&any_printed](uint32_t recognizer_id, bool enabled,
                                 std::string name, std::string function,
                                 llvm::ArrayRef<ConstString> symbols,
                                 Mangled::NamePreference symbol_mangling,
                                 bool regexp) { any_printed = true; });

  EXPECT_TRUE(any_printed);
}
