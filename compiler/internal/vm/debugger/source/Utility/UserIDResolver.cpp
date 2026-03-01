//===-- UserIDResolver.cpp ------------------------------------------------===//
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

#include "lldb/Utility/UserIDResolver.h"
#include "llvm/Support/ManagedStatic.h"
#include <optional>

using namespace lldb_private;

UserIDResolver::~UserIDResolver() = default;

std::optional<llvm::StringRef> UserIDResolver::Get(
    id_t id, Map &cache,
    std::optional<std::string> (UserIDResolver::*do_get)(id_t)) {

  std::lock_guard<std::mutex> guard(m_mutex);
  auto iter_bool = cache.try_emplace(id, std::nullopt);
  if (iter_bool.second)
    iter_bool.first->second = (this->*do_get)(id);
  if (iter_bool.first->second)
    return llvm::StringRef(*iter_bool.first->second);
  return std::nullopt;
}

namespace {
class NoopResolver : public UserIDResolver {
protected:
  std::optional<std::string> DoGetUserName(id_t uid) override {
    return std::nullopt;
  }

  std::optional<std::string> DoGetGroupName(id_t gid) override {
    return std::nullopt;
  }
};
} // namespace

static llvm::ManagedStatic<NoopResolver> g_noop_resolver;

UserIDResolver &UserIDResolver::GetNoopResolver() { return *g_noop_resolver; }
