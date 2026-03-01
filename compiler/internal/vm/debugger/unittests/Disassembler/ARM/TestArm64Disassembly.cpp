//===-- TestArm64Disassembly.cpp ------------------------------------------===//

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

#include "gtest/gtest.h"

#include "lldb/Core/Address.h"
#include "lldb/Core/Disassembler.h"
#include "lldb/Utility/ArchSpec.h"
#include "lldb/Target/ExecutionContext.h"

#include "Plugins/Disassembler/LLVMC/DisassemblerLLVMC.h"
#include "llvm/Support/TargetSelect.h"

using namespace lldb;
using namespace lldb_private;

namespace {
class TestArm64Disassembly : public testing::Test {
public:
  static void SetUpTestCase();
  static void TearDownTestCase();

  //  virtual void SetUp() override { }
  //  virtual void TearDown() override { }

protected:
};

void TestArm64Disassembly::SetUpTestCase() {
  llvm::InitializeAllTargets();
  llvm::InitializeAllAsmPrinters();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllDisassemblers();
  DisassemblerLLVMC::Initialize();
}

void TestArm64Disassembly::TearDownTestCase() {
  DisassemblerLLVMC::Terminate();
}
} // namespace

TEST_F(TestArm64Disassembly, TestArmv81Instruction) {
  ArchSpec arch("arm64-apple-ios");

  const unsigned num_of_instructions = 2;
  uint8_t data[] = {
      0xff, 0x43, 0x00, 0xd1, // 0xd10043ff :  sub    sp, sp, #0x10
      0x62, 0x7c, 0xa1, 0xc8, // 0xc8a17c62 :  cas    x1, x2, [x3] (cas defined in ARM v8.1 & newer)
  };

  DisassemblerSP disass_sp;
  Address start_addr(0x100);
  disass_sp = Disassembler::DisassembleBytes(
      arch, nullptr, nullptr, nullptr, nullptr, start_addr, &data, sizeof(data),
      num_of_instructions, false);

  // If we failed to get a disassembler, we can assume it is because
  // the llvm we linked against was not built with the ARM target,
  // and we should skip these tests without marking anything as failing.

  if (disass_sp) {
    const InstructionList inst_list (disass_sp->GetInstructionList());
    EXPECT_EQ (num_of_instructions, inst_list.GetSize());

    InstructionSP inst_sp;
    const char *mnemonic;
    ExecutionContext exe_ctx (nullptr, nullptr, nullptr);
    inst_sp = inst_list.GetInstructionAtIndex (0);
    mnemonic = inst_sp->GetMnemonic(&exe_ctx);
    ASSERT_STREQ ("sub", mnemonic);

    inst_sp = inst_list.GetInstructionAtIndex (1);
    mnemonic = inst_sp->GetMnemonic(&exe_ctx);
    ASSERT_STREQ ("cas", mnemonic);
  }
}
