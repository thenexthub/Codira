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

#include "vm/core/CAS/BuiltinUnifiedCASDatabases.h"
#include "BuiltinCAS.h"
#include "vm/core/CAS/ActionCache.h"
#include "vm/core/CAS/UnifiedOnDiskCache.h"

using namespace vm::core;
using namespace vm::core::cas;

Expected<std::pair<std::unique_ptr<ObjectStore>, std::unique_ptr<ActionCache>>>
cas::createOnDiskUnifiedCASDatabases(StringRef Path) {
  std::shared_ptr<ondisk::UnifiedOnDiskCache> UniDB;
  if (Error E = builtin::createBuiltinUnifiedOnDiskCache(Path).moveInto(UniDB))
    return std::move(E);
  auto CAS = builtin::createObjectStoreFromUnifiedOnDiskCache(UniDB);
  auto AC = builtin::createActionCacheFromUnifiedOnDiskCache(std::move(UniDB));
  return std::make_pair(std::move(CAS), std::move(AC));
}

Expected<ValidationResult> cas::validateOnDiskUnifiedCASDatabasesIfNeeded(
    StringRef Path, bool CheckHash, bool AllowRecovery, bool ForceValidation,
    std::optional<StringRef> LLVMCasBinary) {
#if LLVM_ENABLE_ONDISK_CAS
  return ondisk::UnifiedOnDiskCache::validateIfNeeded(
      Path, builtin::BuiltinCASContext::getHashName(),
      sizeof(builtin::HashType), CheckHash, AllowRecovery, ForceValidation,
      LLVMCasBinary);
#else
  return createStringError(inconvertibleErrorCode(), "OnDiskCache is disabled");
#endif
}
