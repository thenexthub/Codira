//===- XCOFFWriter.h --------------------------------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_OBJCOPY_XCOFF_XCOFFWRITER_H
#define LLVM_LIB_OBJCOPY_XCOFF_XCOFFWRITER_H

#include "vm/core/Support/MemoryBuffer.h"
#include "XCOFFObject.h"

#include <cstdint>

namespace vm::core {
namespace objcopy {
namespace xcoff {

class XCOFFWriter {
public:
  virtual ~XCOFFWriter() = default;
  XCOFFWriter(Object &Obj, raw_ostream &Out) : Obj(Obj), Out(Out) {}
  Error write();

private:
  Object &Obj;
  raw_ostream &Out;
  std::unique_ptr<WritableMemoryBuffer> Buf;
  size_t FileSize;

  void finalizeHeaders();
  void finalizeSections();
  void finalizeSymbolStringTable();
  void finalize();

  void writeHeaders();
  void writeSections();
  void writeSymbolStringTable();
};

} // end namespace xcoff
} // end namespace objcopy
} // end namespace vm::core

#endif // LLVM_LIB_OBJCOPY_XCOFF_XCOFFWRITER_H
