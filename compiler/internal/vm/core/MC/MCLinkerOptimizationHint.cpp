//===- toolchain/MC/MCLinkerOptimizationHint.cpp ----- LOH handling ------------===//
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

#include "vm/core/MC/MCLinkerOptimizationHint.h"
#include "vm/core/MC/MCMachObjectWriter.h"
#include "vm/core/Support/LEB128.h"
#include "vm/core/Support/raw_ostream.h"
#include <cstddef>
#include <cstdint>

using namespace vm::core;

// Each LOH is composed by, in this order (each field is encoded using ULEB128):
// - Its kind.
// - Its number of arguments (let say N).
// - Its arg1.
// - ...
// - Its argN.
// <arg1> to <argN> are absolute addresses in the object file, i.e.,
// relative addresses from the beginning of the object file.
void MCLOHDirective::emit_impl(raw_ostream &OutStream,
                               const MachObjectWriter &ObjWriter

) const {
  encodeULEB128(Kind, OutStream);
  encodeULEB128(Args.size(), OutStream);
  for (const MCSymbol *Arg : Args)
    encodeULEB128(ObjWriter.getSymbolAddress(*Arg), OutStream);
}

void MCLOHDirective::emit(const MCAssembler &Asm,
                          MachObjectWriter &ObjWriter) const {
  raw_ostream &OutStream = ObjWriter.W.OS;
  emit_impl(OutStream, ObjWriter);
}

uint64_t MCLOHDirective::getEmitSize(const MCAssembler &Asm,
                                     const MachObjectWriter &ObjWriter) const {
  class raw_counting_ostream : public raw_ostream {
    uint64_t Count = 0;

    void write_impl(const char *, size_t size) override { Count += size; }

    uint64_t current_pos() const override { return Count; }

  public:
    raw_counting_ostream() = default;
    ~raw_counting_ostream() override { flush(); }
  };

  raw_counting_ostream OutStream;
  emit_impl(OutStream, ObjWriter);
  return OutStream.tell();
}
