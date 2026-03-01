//===-- UserIDResolver.h ----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_USERIDRESOLVER_H
#define LLDB_UTILITY_USERIDRESOLVER_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"
#include <mutex>
#include <optional>

namespace lldb_private {

/// An abstract interface for things that know how to map numeric user/group IDs
/// into names. It caches the resolved names to avoid repeating expensive
/// queries. The cache is internally protected by a mutex, so concurrent queries
/// are safe.
class UserIDResolver {
public:
  typedef uint32_t id_t;
  virtual ~UserIDResolver(); // anchor

  std::optional<llvm::StringRef> GetUserName(id_t uid) {
    return Get(uid, m_uid_cache, &UserIDResolver::DoGetUserName);
  }
  std::optional<llvm::StringRef> GetGroupName(id_t gid) {
    return Get(gid, m_gid_cache, &UserIDResolver::DoGetGroupName);
  }

  /// Returns a resolver which returns a failure value for each query. Useful as
  /// a fallback value for the case when we know all lookups will fail.
  static UserIDResolver &GetNoopResolver();

protected:
  virtual std::optional<std::string> DoGetUserName(id_t uid) = 0;
  virtual std::optional<std::string> DoGetGroupName(id_t gid) = 0;

private:
  using Map = llvm::DenseMap<id_t, std::optional<std::string>>;

  std::optional<llvm::StringRef>
  Get(id_t id, Map &cache,
      std::optional<std::string> (UserIDResolver::*do_get)(id_t));

  std::mutex m_mutex;
  Map m_uid_cache;
  Map m_gid_cache;
};

} // namespace lldb_private

#endif // LLDB_UTILITY_USERIDRESOLVER_H
