//===--- Options.h - Option info & table ------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_OPTION_OPTIONS_H
#define LANGUAGE_OPTION_OPTIONS_H

#include "toolchain/Option/OptTable.h"

#include <memory>

namespace language {
namespace options {
  /// Flags specifically for Codira driver options.  Must not overlap with
  /// toolchain::opt::DriverFlag.
  enum CodiraFlags {
    FrontendOption = (1 << 4),
    NoDriverOption = (1 << 5),
    NoInteractiveOption = (1 << 6),
    NoBatchOption = (1 << 7),
    DoesNotAffectIncrementalBuild = (1 << 8),
    AutolinkExtractOption = (1 << 9),
    ModuleWrapOption = (1 << 10),
    CodiraSynthesizeInterfaceOption = (1 << 11),
    ArgumentIsPath = (1 << 12),
    ModuleInterfaceOption = (1 << 13),
    SupplementaryOutput = (1 << 14),
    CodiraSymbolGraphExtractOption = (1 << 15),
    CodiraAPIDigesterOption = (1 << 16),
    NewDriverOnlyOption = (1 << 17),
    ModuleInterfaceOptionIgnorable = (1 << 18),
    ArgumentIsFileList = (1 << 19),
    CacheInvariant = (1 << 20),
  };

  enum ID {
    OPT_INVALID = 0, // This is not an option ID.
#define OPTION(...) TOOLCHAIN_MAKE_OPT_ID(__VA_ARGS__),
#include "language/Option/Options.inc"
    LastOption
#undef OPTION
  };
} //end namespace options

  std::unique_ptr<toolchain::opt::OptTable> createCodiraOptTable();

} // end namespace language

#endif
