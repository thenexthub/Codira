//===--- CXXStdlibKind.h ----------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_BASIC_CXX_STDLIB_KIND_H
#define LANGUAGE_BASIC_CXX_STDLIB_KIND_H

#include "toolchain/Support/ErrorHandling.h"
#include <stdint.h>
#include <string>

namespace language {

enum class CXXStdlibKind : uint8_t {
  Unknown = 0,

  /// libc++ is the default C++ stdlib on Darwin platforms. It is also supported
  /// on Linux when explicitly requested via `-Xcc -stdlib=libc++` flag.
  Libcxx = 1,

  /// libstdc++ is the default C++ stdlib on most Linux distributions.
  Libstdcxx = 2,

  /// msvcprt is used when targeting Windows.
  Msvcprt = 3,
};

inline std::string to_string(CXXStdlibKind kind) {
  switch (kind) {
  case CXXStdlibKind::Unknown:
    return "unknown C++ stdlib";
  case CXXStdlibKind::Libcxx:
    return "libc++";
  case CXXStdlibKind::Libstdcxx:
    return "libstdc++";
  case CXXStdlibKind::Msvcprt:
    return "msvcprt";
  }
  toolchain_unreachable("unhandled CXXStdlibKind");
}

} // namespace language

#endif // LANGUAGE_BASIC_CXX_STDLIB_KIND_H
