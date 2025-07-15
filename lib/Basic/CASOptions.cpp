//===--- CASOptions.cpp - CAS & caching options ---------------------------===//
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
//
//  This file defines the CASOptions class, which provides various
//  CAS and caching flags.
//
//===----------------------------------------------------------------------===//

#include "language/Basic/CASOptions.h"

using namespace language;

void CASOptions::enumerateCASConfigurationFlags(
      toolchain::function_ref<void(toolchain::StringRef)> Callback) const {
  if (EnableCaching) {
    Callback("-cache-compile-job");
    if (!CASOpts.CASPath.empty()) {
      Callback("-cas-path");
      Callback(CASOpts.CASPath);
    }
    if (!CASOpts.PluginPath.empty()) {
      Callback("-cas-plugin-path");
      Callback(CASOpts.PluginPath);
      for (auto Opt : CASOpts.PluginOptions) {
        Callback("-cas-plugin-option");
        Callback((toolchain::Twine(Opt.first) + "=" + Opt.second).str());
      }
    }
  }
}
