//===--- SupplementaryOutputPaths.h ----------------------------*- C++ -*-===*//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_FRONTEND_SUPPLEMENTARYOUTPUTPATHS_H
#define SWIFT_FRONTEND_SUPPLEMENTARYOUTPUTPATHS_H

#include "language/Basic/FileTypes.h"
#include "language/Basic/LLVM.h"
#include "llvm/IR/Function.h"

#include <string>

namespace language {
struct SupplementaryOutputPaths {
#define OUTPUT(NAME, TYPE) std::string NAME;
#include "language/Basic/SupplementaryOutputPaths.def"
#undef OUTPUT

  SupplementaryOutputPaths() = default;

  /// Apply a given function for each existing (non-empty string) supplementary output
  void forEachSetOutput(llvm::function_ref<void(const std::string&)> fn) const {
#define OUTPUT(NAME, TYPE)                                                     \
  if (!NAME.empty())                                                           \
    fn(NAME);
#include "language/Basic/SupplementaryOutputPaths.def"
#undef OUTPUT
  }

  void forEachSetOutputAndType(
      llvm::function_ref<void(const std::string &, file_types::ID)> fn) const {
#define OUTPUT(NAME, TYPE)                                                     \
  if (!NAME.empty())                                                           \
    fn(NAME, file_types::ID::TYPE);
#include "language/Basic/SupplementaryOutputPaths.def"
#undef OUTPUT
  }

  bool empty() const {
    return
#define OUTPUT(NAME, TYPE) NAME.empty() &&
#include "language/Basic/SupplementaryOutputPaths.def"
#undef OUTPUT
        true;
  }
};
} // namespace language

#endif // SWIFT_FRONTEND_SUPPLEMENTARYOUTPUTPATHS_H
