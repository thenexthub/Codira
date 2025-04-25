//===--- TypeLayoutDumper.h - Tool to dump fixed type layouts ---*- C++ -*-===//
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
//
//  A tool to dump fixed-size type layouts in YAML format.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_IRGEN_TYPE_LAYOUT_DUMPER_H
#define SWIFT_IRGEN_TYPE_LAYOUT_DUMPER_H

#include "llvm/ADT/ArrayRef.h"

namespace llvm {
class raw_ostream;
}  // namespace llvm

namespace language {

class ModuleDecl;

namespace irgen {

class IRGenModule;

class TypeLayoutDumper {
  IRGenModule &IGM;

public:
  explicit TypeLayoutDumper(IRGenModule &IGM) : IGM(IGM) {}

  void write(llvm::ArrayRef<ModuleDecl *> AllModules, llvm::raw_ostream &os) const;
};

}  // namespace irgen
}  // namespace language

#endif
