//===--- VariableTypeArray.h - ----------------------------------*- C++ -*-===//
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

#ifndef TOOLCHAIN_SOURCEKITD_VARIABLE_TYPE_ARRAY_H
#define TOOLCHAIN_SOURCEKITD_VARIABLE_TYPE_ARRAY_H

#include "sourcekitd/Internal.h"

namespace SourceKit {
struct VariableType;
}

namespace sourcekitd {
VariantFunctions *getVariantFunctionsForVariableTypeArray();

class VariableTypeArrayBuilder {
public:
  VariableTypeArrayBuilder(toolchain::StringRef PrintedTypes);
  ~VariableTypeArrayBuilder();

  void add(const SourceKit::VariableType &VarType);
  std::unique_ptr<toolchain::MemoryBuffer> createBuffer();
  static VariantFunctions Funcs;

private:
  struct Implementation;
  Implementation &Impl;
};

} // namespace sourcekitd

#endif
