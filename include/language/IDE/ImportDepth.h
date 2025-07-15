//===--- ImportDepth.h ----------------------------------------------------===//
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

#ifndef LANGUAGE_IDE_IMPORTDEPTH_H
#define LANGUAGE_IDE_IMPORTDEPTH_H

#include "language/AST/ASTContext.h"
#include "language/Basic/Toolchain.h"
#include "language/Frontend/FrontendOptions.h"
#include "toolchain/ADT/StringMap.h"

namespace language {
namespace ide {

/// A utility for calculating the import depth of a given module. Direct imports
/// have depth 1, imports of those modules have depth 2, etc.
///
/// Special modules such as Playground auxiliary sources are considered depth
/// 0.
class ImportDepth {
  toolchain::StringMap<uint8_t> depths;

public:
  ImportDepth() = default;
  ImportDepth(ASTContext &context, const FrontendOptions &frontendOptions);

  std::optional<uint8_t> lookup(StringRef module) {
    auto I = depths.find(module);
    if (I == depths.end())
      return std::nullopt;
    return I->getValue();
  }
};

} // end namespace ide
} // end namespace language

#endif // LANGUAGE_IDE_IMPORTDEPTH_H
