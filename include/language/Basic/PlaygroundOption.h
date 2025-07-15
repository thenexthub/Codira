//===--- PlaygroundOption.h - Playground Transform Options -----*- C++ -*-===//
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

#ifndef LANGUAGE_BASIC_PLAYGROUND_OPTIONS_H
#define LANGUAGE_BASIC_PLAYGROUND_OPTIONS_H

#include "toolchain/ADT/SmallSet.h"
#include "toolchain/ADT/StringRef.h"
#include <optional>

namespace language {

/// Enumeration describing all of the available playground options.
enum class PlaygroundOption {
#define PLAYGROUND_OPTION(OptionName, Description, DefaultOn, HighPerfOn) \
  OptionName,
#include "language/Basic/PlaygroundOptions.def"
};

constexpr unsigned numPlaygroundOptions() {
  enum PlaygroundOptions {
#define PLAYGROUND_OPTION(OptionName, Description, DefaultOn, HighPerfOn) \
    OptionName,
#include "language/Basic/PlaygroundOptions.def"
    NumPlaygroundOptions
  };
  return NumPlaygroundOptions;
}

/// Return the name of the given playground option.
toolchain::StringRef getPlaygroundOptionName(PlaygroundOption option);

/// Get the playground option that corresponds to a given name, if there is one.
std::optional<PlaygroundOption> getPlaygroundOption(toolchain::StringRef name);

/// Set of enabled playground options.
typedef toolchain::SmallSet<PlaygroundOption, 8> PlaygroundOptionSet;

}

#endif // LANGUAGE_BASIC_PLAYGROUND_OPTIONS_H
