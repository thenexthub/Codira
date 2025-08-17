//===- RISCVIntrinsicManager.h - RISC-V Intrinsic Handler -------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
// This file defines the RISCVIntrinsicManager, which handles RISC-V vector
// intrinsic functions.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_RISCVINTRINSICMANAGER_H
#define LANGUAGE_CORE_SEMA_RISCVINTRINSICMANAGER_H

#include <cstdint>

namespace language::Core {
class LookupResult;
class IdentifierInfo;
class Preprocessor;

namespace sema {
class RISCVIntrinsicManager {
public:
  enum class IntrinsicKind : uint8_t { RVV, SIFIVE_VECTOR, ANDES_VECTOR };

  virtual ~RISCVIntrinsicManager() = default;

  virtual void InitIntrinsicList() = 0;

  // Create RISC-V intrinsic and insert into symbol table and return true if
  // found, otherwise return false.
  virtual bool CreateIntrinsicIfFound(LookupResult &LR, IdentifierInfo *II,
                                      Preprocessor &PP) = 0;
};
} // end namespace sema
} // end namespace language::Core

#endif
