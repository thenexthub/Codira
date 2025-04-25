//===--- DocSupportAnnotationArray.h - --------------------------*- C++ -*-===//
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

#ifndef LLVM_SOURCEKITD_DOCSUPPORT_ANNOTATION_ARRAY_H
#define LLVM_SOURCEKITD_DOCSUPPORT_ANNOTATION_ARRAY_H

#include "sourcekitd/Internal.h"
#include "llvm/ADT/SmallString.h"

namespace SourceKit {
  struct DocEntityInfo;
}

namespace sourcekitd {

VariantFunctions *getVariantFunctionsForDocSupportAnnotationArray();

class DocSupportAnnotationArrayBuilder {
public:
  DocSupportAnnotationArrayBuilder();
  ~DocSupportAnnotationArrayBuilder();

  void add(const SourceKit::DocEntityInfo &Info);
  std::unique_ptr<llvm::MemoryBuffer> createBuffer();

private:
  struct Implementation;
  Implementation &Impl;
};

}

#endif
