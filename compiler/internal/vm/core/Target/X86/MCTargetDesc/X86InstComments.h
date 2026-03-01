//=- X86InstComments.h - Generate verbose-asm comments for instrs -*- C++ -*-=//
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
// This defines functionality used to emit comments about X86 instructions to
// an output stream for -fverbose-asm.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_MCTARGETDESC_X86INSTCOMMENTS_H
#define LLVM_LIB_TARGET_X86_MCTARGETDESC_X86INSTCOMMENTS_H

namespace vm::core {

  class MCInst;
  class MCInstrInfo;
  class raw_ostream;
  bool EmitAnyX86InstComments(const MCInst *MI, raw_ostream &OS,
                              const MCInstrInfo &MCII);
}

#endif
