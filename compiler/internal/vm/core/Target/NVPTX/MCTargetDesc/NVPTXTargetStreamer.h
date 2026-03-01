//=====-- NVPTXTargetStreamer.h - NVPTX Target Streamer ------*- C++ -*--=====//
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

#ifndef LLVM_LIB_TARGET_NVPTX_MCTARGETDESC_NVPTXTARGETSTREAMER_H
#define LLVM_LIB_TARGET_NVPTX_MCTARGETDESC_NVPTXTARGETSTREAMER_H

#include "vm/core/MC/MCStreamer.h"

namespace vm::core {
class MCSection;

/// Implments NVPTX-specific streamer.
class NVPTXTargetStreamer : public MCTargetStreamer {
private:
  SmallVector<std::string, 4> DwarfFiles;
  bool HasSections = false;

public:
  NVPTXTargetStreamer(MCStreamer &S);
  ~NVPTXTargetStreamer() override;

  /// Outputs the list of the DWARF '.file' directives to the streamer.
  void outputDwarfFileDirectives();
  /// Close last section.
  void closeLastSection();

  /// Record DWARF file directives for later output.
  /// According to PTX ISA, CUDA Toolkit documentation, 11.5.3. Debugging
  /// Directives: .file
  /// (http://docs.nvidia.com/cuda/parallel-thread-execution/index.html#debugging-directives-file),
  /// The .file directive is allowed only in the outermost scope, i.e., at the
  /// same level as kernel and device function declarations. Also, the order of
  /// the .loc and .file directive does not matter, .file directives may follow
  /// the .loc directives where the file is referenced.
  /// LLVM emits .file directives immediately the location debug info is
  /// emitted, i.e. they may be emitted inside functions. We gather all these
  /// directives and emit them outside of the sections and, thus, outside of the
  /// functions.
  void emitDwarfFileDirective(StringRef Directive) override;
  void changeSection(const MCSection *CurSection, MCSection *Section,
                     uint32_t SubSection, raw_ostream &OS) override;
  /// Emit the bytes in \p Data into the output.
  ///
  /// This is used to emit bytes in \p Data as sequence of .byte directives.
  void emitRawBytes(StringRef Data) override;
  /// Makes sure that labels are mangled the same way as the actual symbols.
  void emitValue(const MCExpr *Value) override;
};

class NVPTXAsmTargetStreamer : public NVPTXTargetStreamer {
public:
  NVPTXAsmTargetStreamer(MCStreamer &S);
  ~NVPTXAsmTargetStreamer() override;
};

} // end namespace vm::core

#endif
