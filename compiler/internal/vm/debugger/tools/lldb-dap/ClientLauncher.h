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

#ifndef LLDB_TOOLS_LLDB_DAP_CLIENTLAUNCHER_H
#define LLDB_TOOLS_LLDB_DAP_CLIENTLAUNCHER_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include <vector>

namespace lldb_dap {

class ClientLauncher {
public:
  enum Client {
    VSCode,
    VSCodeURL,
  };

  virtual ~ClientLauncher() = default;
  virtual llvm::Error Launch(const std::vector<llvm::StringRef> &args) = 0;

  static std::optional<Client> GetClientFrom(llvm::StringRef str);
  static std::unique_ptr<ClientLauncher> GetLauncher(Client client);
};

class VSCodeLauncher : public ClientLauncher {
public:
  using ClientLauncher::ClientLauncher;

  llvm::Error Launch(const std::vector<llvm::StringRef> &args) override;

  std::string GetLaunchURL(const std::vector<llvm::StringRef> &args) const;
  static std::string URLEncode(llvm::StringRef str);
};

class VSCodeURLPrinter : public VSCodeLauncher {
  using VSCodeLauncher::VSCodeLauncher;

  llvm::Error Launch(const std::vector<llvm::StringRef> &args) override;
};

} // namespace lldb_dap

#endif
