//===- Tester.cpp ---------------------------------------------------------===//
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
//
// This file defines the Tester class used in the MLIR Reduce tool.
//
// A Tester object is passed as an argument to the reduction passes and it is
// used to run the interestingness testing script on the different generated
// reduced variants of the test case.
//
//===----------------------------------------------------------------------===//

#include "mlir/Reducer/Tester.h"
#include "mlir/IR/Verifier.h"
#include "vm/core/Support/ToolOutputFile.h"

using namespace mlir;

Tester::Tester(StringRef scriptName, ArrayRef<std::string> scriptArgs)
    : testScript(scriptName), testScriptArgs(scriptArgs) {}

std::pair<Tester::Interestingness, size_t>
Tester::isInteresting(ModuleOp module) const {
  // The reduced module should always be vaild, or we may end up retaining the
  // error message by an invalid case. Besides, an invalid module may not be
  // able to print properly.
  if (failed(verify(module)))
    return std::make_pair(Interestingness::False, /*size=*/0);

  SmallString<128> filepath;
  int fd;

  // Print module to temporary file.
  std::error_code ec =
      toolchain::sys::fs::createTemporaryFile("mlir-reduce", "mlir", fd, filepath);

  if (ec)
    toolchain::report_fatal_error(toolchain::Twine("Error making unique filename: ") +
                             ec.message());

  toolchain::ToolOutputFile out(filepath, fd);
  module.print(out.os());
  out.os().close();

  if (out.os().has_error())
    toolchain::report_fatal_error(toolchain::Twine("Error emitting the IR to file '") +
                             filepath);

  size_t size = out.os().tell();
  return std::make_pair(isInteresting(filepath), size);
}

/// Runs the interestingness testing script on a MLIR test case file. Returns
/// true if the interesting behavior is present in the test case or false
/// otherwise.
Tester::Interestingness Tester::isInteresting(StringRef testCase) const {
  std::vector<StringRef> testerArgs;
  testerArgs.push_back(testCase);

  for (const std::string &arg : testScriptArgs)
    testerArgs.emplace_back(arg);

  testerArgs.push_back(testCase);

  std::string errMsg;
  int result = toolchain::sys::ExecuteAndWait(
      testScript, testerArgs, /*Env=*/std::nullopt, /*Redirects=*/{},
      /*SecondsToWait=*/0, /*MemoryLimit=*/0, &errMsg);

  if (result < 0)
    toolchain::report_fatal_error(
        toolchain::Twine("Error running interestingness test: ") + errMsg, false);

  if (!result)
    return Interestingness::False;

  return Interestingness::True;
}
