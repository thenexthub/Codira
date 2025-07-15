//===--- Logging.h - --------------------------------------------*- C++ -*-===//
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

#ifndef TOOLCHAIN_SOURCEKITD_LOGGING_H
#define TOOLCHAIN_SOURCEKITD_LOGGING_H

#include "toolchain/Support/raw_ostream.h"

namespace sourcekitd {

void enableLogging(toolchain::StringRef LoggerName);

#define DEF_COLOR(NAME, COLOR)\
static const toolchain::raw_ostream::Colors NAME##Color = toolchain::raw_ostream::COLOR;
DEF_COLOR(DictKey, YELLOW)
DEF_COLOR(UID, CYAN)
#undef DEF_COLOR

class OSColor {
  toolchain::raw_ostream &OS;
  bool HasColors;
public:
  OSColor(toolchain::raw_ostream &OS, toolchain::raw_ostream::Colors Color) : OS(OS) {
    HasColors = OS.has_colors();
    if (HasColors)
      OS.changeColor(Color);
  }
  ~OSColor() {
    if (HasColors)
      OS.resetColor();
  }

  OSColor &operator<<(char C) { OS << C; return *this; }
  OSColor &operator<<(toolchain::StringRef Str) { OS << Str; return *this; }
};

} // namespace sourcekitd

#endif
