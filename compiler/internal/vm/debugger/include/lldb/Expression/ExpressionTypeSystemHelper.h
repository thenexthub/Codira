//===-- ExpressionTypeSystemHelper.h ---------------------------------*- C++
//-*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_EXPRESSION_EXPRESSIONTYPESYSTEMHELPER_H
#define LLDB_EXPRESSION_EXPRESSIONTYPESYSTEMHELPER_H

#include "llvm/Support/Casting.h"
#include "llvm/Support/ExtensibleRTTI.h"

namespace lldb_private {

/// \class ExpressionTypeSystemHelper ExpressionTypeSystemHelper.h
/// "lldb/Expression/ExpressionTypeSystemHelper.h"
/// A helper object that the Expression can pass to its ExpressionParser
/// to provide generic information that any type of expression will need to
/// supply.  It's only job is to support dyn_cast so that the expression parser
/// can cast it back to the requisite specific type.
///

class ExpressionTypeSystemHelper
    : public llvm::RTTIExtends<ExpressionTypeSystemHelper, llvm::RTTIRoot> {
public:
  /// LLVM RTTI support
  static char ID;

  virtual ~ExpressionTypeSystemHelper() = default;
};

} // namespace lldb_private

#endif // LLDB_EXPRESSION_EXPRESSIONTYPESYSTEMHELPER_H
