//===--- DocStructureArray.h - ----------------------------------*- C++ -*-===//
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

#ifndef LLVM_SOURCEKITD_DOC_STRUCTURE_ARRAY_H
#define LLVM_SOURCEKITD_DOC_STRUCTURE_ARRAY_H

#include "sourcekitd/Internal.h"
#include "llvm/ADT/SmallString.h"

namespace sourcekitd {

VariantFunctions *getVariantFunctionsForDocStructureArray();
VariantFunctions *getVariantFunctionsForDocStructureElementArray();
VariantFunctions *getVariantFunctionsForInheritedTypesArray();
VariantFunctions *getVariantFunctionsForAttributesArray();

class DocStructureArrayBuilder {
public:
  DocStructureArrayBuilder();
  ~DocStructureArrayBuilder();

  void beginSubStructure(unsigned Offset, unsigned Length,
                         SourceKit::UIdent Kind, SourceKit::UIdent AccessLevel,
                         SourceKit::UIdent SetterAccessLevel,
                         unsigned NameOffset, unsigned NameLength,
                         unsigned BodyOffset, unsigned BodyLength,
                         unsigned DocOffset, unsigned DocLength,
                         llvm::StringRef DisplayName, llvm::StringRef TypeName,
                         llvm::StringRef RuntimeName,
                         llvm::StringRef SelectorName,
                         llvm::ArrayRef<llvm::StringRef> InheritedTypes,
                         llvm::ArrayRef<std::tuple<SourceKit::UIdent, unsigned, unsigned>> Attrs);

  void addElement(SourceKit::UIdent Kind, unsigned Offset, unsigned Length);

  void endSubStructure();

  std::unique_ptr<llvm::MemoryBuffer> createBuffer();

private:
  struct Implementation;
  Implementation &impl;
};

} // end namespace sourcekitd

#endif // LLVM_SOURCEKITD_DOC_STRUCTURE_ARRAY_H
