//===--- ForeignInfo.h - Declaration import information ---------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
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
