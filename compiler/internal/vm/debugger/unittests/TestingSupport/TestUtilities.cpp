//===-- TestUtilities.cpp -------------------------------------------------===//
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

#include "TestUtilities.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ObjectYAML/yaml2obj.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/YAMLTraits.h"
#include "gtest/gtest.h"

using namespace lldb_private;

extern const char *TestMainArgv0;

std::once_flag TestUtilities::g_debugger_initialize_flag;

std::string lldb_private::PrettyPrint(const llvm::json::Value &value) {
  return llvm::formatv("{0:2}", value).str();
}

std::string lldb_private::GetInputFilePath(const llvm::Twine &name) {
  llvm::SmallString<128> result = llvm::sys::path::parent_path(TestMainArgv0);
  llvm::sys::fs::make_absolute(result);
  llvm::sys::path::append(result, "Inputs", name);
  return std::string(result.str());
}

llvm::Expected<TestFile> TestFile::fromYaml(llvm::StringRef Yaml) {
  std::string Buffer;
  llvm::raw_string_ostream OS(Buffer);
  llvm::yaml::Input YIn(Yaml);
  std::string ErrorMsg("convertYAML() failed: ");
  if (!llvm::yaml::convertYAML(YIn, OS, [&ErrorMsg](const llvm::Twine &Msg) {
        ErrorMsg += Msg.str();
      }))
    return llvm::createStringError(llvm::inconvertibleErrorCode(), ErrorMsg);
  return TestFile(std::move(Buffer));
}

llvm::Expected<TestFile> TestFile::fromYamlFile(const llvm::Twine &Name) {
  auto BufferOrError =
      llvm::MemoryBuffer::getFile(GetInputFilePath(Name), /*IsText=*/false,
                                  /*RequiresNullTerminator=*/false);
  if (!BufferOrError)
    return llvm::errorCodeToError(BufferOrError.getError());
  return fromYaml(BufferOrError.get()->getBuffer());
}

llvm::Expected<llvm::sys::fs::TempFile> TestFile::writeToTemporaryFile() {
  llvm::Expected<llvm::sys::fs::TempFile> Temp =
      llvm::sys::fs::TempFile::create("temp%%%%%%%%%%%%%%%%");
  if (!Temp)
    return Temp.takeError();
  llvm::raw_fd_ostream(Temp->FD, /*shouldClose=*/false) << Buffer;
  return std::move(*Temp);
}
