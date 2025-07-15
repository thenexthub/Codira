//===--- DictionaryKeys.h - -------------------------------------*- C++ -*-===//
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

#ifndef TOOLCHAIN_SOURCEKITD_LIB_API_DICTIONARYKEYS_H
#define TOOLCHAIN_SOURCEKITD_LIB_API_DICTIONARYKEYS_H

namespace SourceKit {
  class UIdent;
}

namespace sourcekitd {

#define KEY(NAME, CONTENT) extern SourceKit::UIdent Key##NAME;
#include "SourceKit/Core/ProtocolUIDs.def"

/// Used for determining the printing order of dictionary keys.
bool compareDictKeys(SourceKit::UIdent LHS, SourceKit::UIdent RHS);

}

#endif
