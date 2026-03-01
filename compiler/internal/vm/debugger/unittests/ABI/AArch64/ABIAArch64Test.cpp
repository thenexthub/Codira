//===-- ABIAArch64Test.cpp ------------------------------------------------===//

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

#include "Plugins/ABI/AArch64/ABIMacOSX_arm64.h"
#include "Plugins/ABI/AArch64/ABISysV_arm64.h"
#include "Utility/ARM64_DWARF_Registers.h"
#include "Utility/ARM64_ehframe_Registers.h"
#include "lldb/Target/DynamicRegisterInfo.h"
#include "lldb/Utility/ArchSpec.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "gtest/gtest.h"
#include <vector>

using namespace lldb_private;
using namespace lldb;

class ABIAArch64TestFixture : public testing::TestWithParam<llvm::StringRef> {
public:
  static void SetUpTestCase();
  static void TearDownTestCase();

  //  virtual void SetUp() override { }
  //  virtual void TearDown() override { }

protected:
};

void ABIAArch64TestFixture::SetUpTestCase() {
  LLVMInitializeAArch64TargetInfo();
  LLVMInitializeAArch64TargetMC();
  ABISysV_arm64::Initialize();
  ABIMacOSX_arm64::Initialize();
}

void ABIAArch64TestFixture::TearDownTestCase() {
  ABISysV_arm64::Terminate();
  ABIMacOSX_arm64::Terminate();
  llvm::llvm_shutdown();
}

TEST_P(ABIAArch64TestFixture, AugmentRegisterInfo) {
  ABISP abi_sp = ABI::FindPlugin(ProcessSP(), ArchSpec(GetParam()));
  ASSERT_TRUE(abi_sp);
  using Register = DynamicRegisterInfo::Register;

  Register pc;
  pc.name = ConstString("pc");
  pc.alt_name = ConstString();
  pc.set_name = ConstString("GPR");
  std::vector<Register> regs{pc};

  abi_sp->AugmentRegisterInfo(regs);

  ASSERT_EQ(regs.size(), 1U);
  Register new_pc = regs[0];
  EXPECT_EQ(new_pc.name, pc.name);
  EXPECT_EQ(new_pc.set_name, pc.set_name);
  EXPECT_EQ(new_pc.regnum_ehframe, arm64_ehframe::pc);
  EXPECT_EQ(new_pc.regnum_dwarf, arm64_dwarf::pc);
}

INSTANTIATE_TEST_SUITE_P(ABIAArch64Tests, ABIAArch64TestFixture,
                         testing::Values("aarch64-pc-linux",
                                         "arm64-apple-macosx"));
