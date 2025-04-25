//===--- DeclarationsArray.h - ----------------------------------*- C++ -*-===//
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
// This is an array used in the response to editor.open.interface requests.
// It contains all declarations, identified by their Kind, Offset, and Length,
// and optionally includes a USR, if the declaration has one.
//===----------------------------------------------------------------------===//
#ifndef LLVM_SOURCEKITD_DECLARATIONS_ARRAY_H
#define LLVM_SOURCEKITD_DECLARATIONS_ARRAY_H

#include "sourcekitd/Internal.h"

namespace sourcekitd {

VariantFunctions *getVariantFunctionsForDeclarationsArray();

/// Builds an array for declarations by kind, offset, length, and optionally USR
class DeclarationsArrayBuilder {
public:
  DeclarationsArrayBuilder();
  ~DeclarationsArrayBuilder();

  void add(SourceKit::UIdent Kind, unsigned Offset, unsigned Length,
           llvm::StringRef USR);

  bool empty() const;

  std::unique_ptr<llvm::MemoryBuffer> createBuffer();

private:
  struct Implementation;
  Implementation &Impl;
};

} // namespace sourcekitd

#endif
