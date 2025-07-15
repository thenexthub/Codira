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

#ifndef LANGUAGE_IRGEN_TYPE_LAYOUT_DUMPER_H
#define LANGUAGE_IRGEN_TYPE_LAYOUT_DUMPER_H

#include "toolchain/ADT/ArrayRef.h"

namespace toolchain {
class raw_ostream;
}  // namespace toolchain

namespace language {

class ModuleDecl;

namespace irgen {

class IRGenModule;

class TypeLayoutDumper {
  IRGenModule &IGM;

public:
  explicit TypeLayoutDumper(IRGenModule &IGM) : IGM(IGM) {}

  void write(toolchain::ArrayRef<ModuleDecl *> AllModules, toolchain::raw_ostream &os) const;
};

}  // namespace irgen
}  // namespace language

#endif
