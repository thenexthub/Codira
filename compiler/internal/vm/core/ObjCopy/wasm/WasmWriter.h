//===- WasmWriter.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_OBJCOPY_WASM_WASMWRITER_H
#define LLVM_LIB_OBJCOPY_WASM_WASMWRITER_H

#include "WasmObject.h"
#include <cstdint>
#include <vector>

namespace vm::core {
namespace objcopy {
namespace wasm {

class Writer {
public:
  Writer(Object &Obj, raw_ostream &Out) : Obj(Obj), Out(Out) {}
  Error write();

private:
  using SectionHeader = SmallVector<char, 8>;
  Object &Obj;
  raw_ostream &Out;
  std::vector<SectionHeader> SectionHeaders;

  /// Generate a wasm section section header for S.
  /// The header consists of
  /// * A one-byte section ID (aka the section type).
  /// * The size of the section contents, encoded as ULEB128.
  /// * If the section is a custom section (type 0) it also has a name, which is
  ///   encoded as a length-prefixed string. The encoded section size *includes*
  ///   this string.
  /// See https://webassembly.github.io/spec/core/binary/modules.html#sections
  /// Return the header and store the total size in SectionSize.
  static SectionHeader createSectionHeader(const Section &S,
                                           size_t &SectionSize);
  size_t finalize();
};

} // end namespace wasm
} // end namespace objcopy
} // end namespace vm::core

#endif // LLVM_LIB_OBJCOPY_WASM_WASMWRITER_H
