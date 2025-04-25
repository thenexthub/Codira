//===--- Index.h - Swift Indexing -------------------------------*- C++ -*-===//
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

#ifndef SWIFT_INDEX_INDEX_H
#define SWIFT_INDEX_INDEX_H

#include "language/Index/IndexDataConsumer.h"

namespace language {
class ModuleDecl;
class SourceFile;
class DeclContext;

namespace index {

void indexDeclContext(DeclContext *DC, IndexDataConsumer &consumer);
void indexSourceFile(SourceFile *SF, IndexDataConsumer &consumer);
void indexModule(ModuleDecl *module, IndexDataConsumer &consumer);
bool printDisplayName(const swift::ValueDecl *D, llvm::raw_ostream &OS);

} // end namespace index
} // end namespace language

#endif // SWIFT_INDEX_INDEX_H
