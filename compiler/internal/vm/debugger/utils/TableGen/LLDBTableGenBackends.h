//===- LLDBTableGenBackends.h -----------------------------------*- C++ -*-===//
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
//
// This file contains the declarations for all of the LLDB TableGen
// backends. A "TableGen backend" is just a function.
//
// See "$LLVM_ROOT/utils/TableGen/TableGenBackends.h" for more info.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_UTILS_TABLEGEN_LLDBTABLEGENBACKENDS_H
#define LLDB_UTILS_TABLEGEN_LLDBTABLEGENBACKENDS_H

#include "llvm/ADT/StringRef.h"

namespace llvm {
class raw_ostream;
class RecordKeeper;
class Record;
} // namespace llvm

using llvm::raw_ostream;
using llvm::RecordKeeper;

namespace lldb_private {

void EmitOptionDefs(const RecordKeeper &RK, raw_ostream &OS);
void EmitPropertyDefs(const RecordKeeper &RK, raw_ostream &OS);
void EmitPropertyEnumDefs(const RecordKeeper &RK, raw_ostream &OS);
int EmitSBAPIDWARFEnum(int argc, char **argv);

} // namespace lldb_private

#endif
