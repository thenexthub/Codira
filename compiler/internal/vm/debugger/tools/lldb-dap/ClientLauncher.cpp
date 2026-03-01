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

#include "ClientLauncher.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/FormatVariadic.h"

using namespace lldb_dap;

std::optional<ClientLauncher::Client>
ClientLauncher::GetClientFrom(llvm::StringRef str) {
  return llvm::StringSwitch<std::optional<ClientLauncher::Client>>(str.lower())
      .Case("vscode", ClientLauncher::VSCode)
      .Case("vscode-url", ClientLauncher::VSCodeURL)
      .Default(std::nullopt);
}

std::unique_ptr<ClientLauncher>
ClientLauncher::GetLauncher(ClientLauncher::Client client) {
  switch (client) {
  case ClientLauncher::VSCode:
    return std::make_unique<VSCodeLauncher>();
  case ClientLauncher::VSCodeURL:
    return std::make_unique<VSCodeURLPrinter>();
  }
  return nullptr;
}

std::string VSCodeLauncher::URLEncode(llvm::StringRef str) {
  std::string out;
  llvm::raw_string_ostream os(out);
  for (char c : str) {
    if (std::isalnum(c) || llvm::StringRef("-_.~").contains(c))
      os << c;
    else
      os << '%' << llvm::utohexstr(c, false, 2);
  }
  return os.str();
}

std::string
VSCodeLauncher::GetLaunchURL(const std::vector<llvm::StringRef> &args) const {
  assert(!args.empty() && "empty launch args");

  std::vector<std::string> encoded_launch_args;
  for (llvm::StringRef arg : args)
    encoded_launch_args.push_back(URLEncode(arg));

  const std::string args_str = llvm::join(encoded_launch_args, "&args=");
  return llvm::formatv(
             "vscode://llvm-vs-code-extensions.lldb-dap/start?program={0}",
             args_str)
      .str();
}

llvm::Error VSCodeLauncher::Launch(const std::vector<llvm::StringRef> &args) {
  const std::string launch_url = GetLaunchURL(args);
  const std::string command =
      llvm::formatv("code --open-url {0}", launch_url).str();

  std::system(command.c_str());
  return llvm::Error::success();
}

llvm::Error VSCodeURLPrinter::Launch(const std::vector<llvm::StringRef> &args) {
  llvm::outs() << GetLaunchURL(args) << '\n';
  return llvm::Error::success();
}
