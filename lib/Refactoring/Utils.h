//===----------------------------------------------------------------------===//
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

#ifndef LANGUAGE_REFACTORING_UTILS_H
#define LANGUAGE_REFACTORING_UTILS_H

#include "language/AST/ASTContext.h"
#include "language/AST/Decl.h"
#include "language/Basic/Toolchain.h"

namespace language {
namespace refactoring {
StringRef correctNameInternal(ASTContext &Ctx, StringRef Name,
                              ArrayRef<ValueDecl *> AllVisibles);

StringRef correctNewDeclName(SourceLoc Loc, DeclContext *DC, StringRef Name);

/// If \p NTD is a protocol, return all the protocols it inherits from. If it's
/// a type, return all the protocols it conforms to.
SmallVector<ProtocolDecl *, 2> getAllProtocols(NominalTypeDecl *NTD);
}
} // namespace language

#endif
