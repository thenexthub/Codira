//===-- SBCommandInterpreterTest.cpp ------------------------===----------===//
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
//===----------------------------------------------------------------------===/

// Use the umbrella header for -Wdocumentation.
#include "lldb/API/LLDB.h"

#include "TestingSupport/SubsystemRAII.h"
#include "lldb/API/SBDebugger.h"
#include "gtest/gtest.h"
#include <cstring>
#include <string>

using namespace lldb;
using namespace lldb_private;

class SBCommandInterpreterTest : public testing::Test {
protected:
  void SetUp() override {
    debugger = SBDebugger::Create(/*source_init_files=*/false);
  }

  void TearDown() override { SBDebugger::Destroy(debugger); }

  SubsystemRAII<lldb::SBDebugger> subsystems;
  SBDebugger debugger;
};

class DummyCommand : public SBCommandPluginInterface {
public:
  DummyCommand(const char *message) : m_message(message) {}

  bool DoExecute(SBDebugger dbg, char **command,
                 SBCommandReturnObject &result) override {
    result.PutCString(m_message.c_str());
    result.SetStatus(eReturnStatusSuccessFinishResult);
    return result.Succeeded();
  }

private:
  std::string m_message;
};

TEST_F(SBCommandInterpreterTest, SingleWordCommand) {
  // We first test a command without autorepeat
  DummyCommand dummy("It worked");
  SBCommandInterpreter interp = debugger.GetCommandInterpreter();
  interp.AddCommand("dummy", &dummy, /*help=*/nullptr);
  {
    SBCommandReturnObject result;
    interp.HandleCommand("dummy", result, /*add_to_history=*/true);
    EXPECT_TRUE(result.Succeeded());
    EXPECT_STREQ(result.GetOutput(), "It worked\n");
  }
  {
    SBCommandReturnObject result;
    interp.HandleCommand("", result);
    EXPECT_FALSE(result.Succeeded());
    EXPECT_STREQ(result.GetError(), "error: no auto repeat\n");
  }

  // Now we test a command with autorepeat
  interp.AddCommand("dummy_with_autorepeat", &dummy, /*help=*/nullptr,
                    /*syntax=*/nullptr, /*auto_repeat_command=*/nullptr);
  {
    SBCommandReturnObject result;
    interp.HandleCommand("dummy_with_autorepeat", result,
                         /*add_to_history=*/true);
    EXPECT_TRUE(result.Succeeded());
    EXPECT_STREQ(result.GetOutput(), "It worked\n");
  }
  {
    SBCommandReturnObject result;
    interp.HandleCommand("", result);
    EXPECT_TRUE(result.Succeeded());
    EXPECT_STREQ(result.GetOutput(), "It worked\n");
  }
}

TEST_F(SBCommandInterpreterTest, MultiWordCommand) {
  SBCommandInterpreter interp = debugger.GetCommandInterpreter();
  auto command = interp.AddMultiwordCommand("multicommand", /*help=*/nullptr);
  // We first test a subcommand without autorepeat
  DummyCommand subcommand("It worked again");
  command.AddCommand("subcommand", &subcommand, /*help=*/nullptr);
  {
    SBCommandReturnObject result;
    interp.HandleCommand("multicommand subcommand", result,
                         /*add_to_history=*/true);
    EXPECT_TRUE(result.Succeeded());
    EXPECT_STREQ(result.GetOutput(), "It worked again\n");
  }
  {
    SBCommandReturnObject result;
    interp.HandleCommand("", result);
    EXPECT_FALSE(result.Succeeded());
    EXPECT_STREQ(result.GetError(), "error: no auto repeat\n");
  }

  // We first test a subcommand with autorepeat
  command.AddCommand("subcommand_with_autorepeat", &subcommand,
                     /*help=*/nullptr, /*syntax=*/nullptr,
                     /*auto_repeat_command=*/nullptr);
  {
    SBCommandReturnObject result;
    interp.HandleCommand("multicommand subcommand_with_autorepeat", result,
                         /*add_to_history=*/true);
    EXPECT_TRUE(result.Succeeded());
    EXPECT_STREQ(result.GetOutput(), "It worked again\n");
  }
  {
    SBCommandReturnObject result;
    interp.HandleCommand("", result);
    EXPECT_TRUE(result.Succeeded());
    EXPECT_STREQ(result.GetOutput(), "It worked again\n");
  }

  DummyCommand subcommand2("It worked again 2");
  // We now test a subcommand with autorepeat of the command name
  command.AddCommand(
      "subcommand_with_custom_autorepeat", &subcommand2, /*help=*/nullptr,
      /*syntax=*/nullptr,
      /*auto_repeat_command=*/"multicommand subcommand_with_autorepeat");
  {
    SBCommandReturnObject result;
    interp.HandleCommand("multicommand subcommand_with_custom_autorepeat",
                         result, /*add_to_history=*/true);
    EXPECT_TRUE(result.Succeeded());
    EXPECT_STREQ(result.GetOutput(), "It worked again 2\n");
  }
  {
    SBCommandReturnObject result;
    interp.HandleCommand("", result);
    EXPECT_TRUE(result.Succeeded());
    EXPECT_STREQ(result.GetOutput(), "It worked again\n");
  }
}
