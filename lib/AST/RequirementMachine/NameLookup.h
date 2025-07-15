//===--- NameLookup.h - Name lookup utilities -------------------*- C++ -*-===//
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

#ifndef LANGUAGE_RQM_NAMELOOKUP_H
#define LANGUAGE_RQM_NAMELOOKUP_H

#include "toolchain/ADT/SmallVector.h"

namespace language {

class Identifier;
class NominalTypeDecl;
class Type;
class TypeDecl;

namespace rewriting {

void lookupConcreteNestedType(
    Type baseType,
    Identifier name,
    toolchain::SmallVectorImpl<TypeDecl *> &concreteDecls);

void lookupConcreteNestedType(
    NominalTypeDecl *decl,
    Identifier name,
    toolchain::SmallVectorImpl<TypeDecl *> &concreteDecls);

TypeDecl *findBestConcreteNestedType(
    toolchain::SmallVectorImpl<TypeDecl *> &concreteDecls);

} // end namespace rewriting

} // end namespace language

#endif