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

#ifndef TOOLCHAIN_SOURCEKITD_DOC_STRUCTURE_ARRAY_H
#define TOOLCHAIN_SOURCEKITD_DOC_STRUCTURE_ARRAY_H

#include "sourcekitd/Internal.h"
#include "toolchain/ADT/SmallString.h"

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
                         toolchain::StringRef DisplayName, toolchain::StringRef TypeName,
                         toolchain::StringRef RuntimeName,
                         toolchain::StringRef SelectorName,
                         toolchain::ArrayRef<toolchain::StringRef> InheritedTypes,
                         toolchain::ArrayRef<std::tuple<SourceKit::UIdent, unsigned, unsigned>> Attrs);

  void addElement(SourceKit::UIdent Kind, unsigned Offset, unsigned Length);

  void endSubStructure();

  std::unique_ptr<toolchain::MemoryBuffer> createBuffer();

private:
  struct Implementation;
  Implementation &impl;
};

} // end namespace sourcekitd

#endif // TOOLCHAIN_SOURCEKITD_DOC_STRUCTURE_ARRAY_H
