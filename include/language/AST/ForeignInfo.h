//===--- ForeignInfo.h - Declaration import information ---------*- C++ -*-===//
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
// This file defines the ForeignInfo structure, which includes
// structural information about how a foreign API's physical type
// maps into the Swift type system.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_FOREIGN_INFO_H
#define SWIFT_FOREIGN_INFO_H

#include "language/AST/ForeignAsyncConvention.h"
#include "language/AST/ForeignErrorConvention.h"
#include "language/AST/Decl.h"

namespace language {

struct ForeignInfo {
  ImportAsMemberStatus self;
  std::optional<ForeignErrorConvention> error;
  std::optional<ForeignAsyncConvention> async;
};

} // end namespace language

#endif
