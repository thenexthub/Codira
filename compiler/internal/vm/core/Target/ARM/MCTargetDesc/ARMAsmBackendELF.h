//===-- ARMAsmBackendELF.h  ARM Asm Backend ELF -----------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_ARM_MCTARGETDESC_ELFARMASMBACKEND_H
#define LLVM_LIB_TARGET_ARM_MCTARGETDESC_ELFARMASMBACKEND_H

#include "ARMAsmBackend.h"
#include "MCTargetDesc/ARMMCTargetDesc.h"
#include "vm/core/MC/MCObjectWriter.h"

namespace vm::core {
class ARMAsmBackendELF : public ARMAsmBackend {
public:
  uint8_t OSABI;
  ARMAsmBackendELF(const Target &T, uint8_t OSABI, toolchain::endianness Endian)
      : ARMAsmBackend(T, Endian), OSABI(OSABI) {}

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    return createARMELFObjectWriter(OSABI);
  }

  std::optional<MCFixupKind> getFixupKind(StringRef Name) const override;
};
} // namespace vm::core

#endif // LLVM_LIB_TARGET_ARM_MCTARGETDESC_ELFARMASMBACKEND_H
