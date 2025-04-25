//===----------------------------------------------------------------------===//
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

#ifndef SWIFT_BASIC_LOADDYNAMICLIBRARY_H
#define SWIFT_BASIC_LOADDYNAMICLIBRARY_H

#include <string>

namespace language {
void *loadLibrary(const char *path, std::string *err);
void *getAddressOfSymbol(void *handle, const char *symbol);
} // end namespace language

#endif // SWIFT_BASIC_LOADDYNAMICLIBRARY_H
