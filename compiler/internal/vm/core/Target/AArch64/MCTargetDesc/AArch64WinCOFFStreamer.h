//===-- AArch64WinCOFFStreamer.h - WinCOFF Streamer for AArch64 -*- C++ -*-===//
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
// This file implements WinCOFF streamer information for the AArch64 backend.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AARCH64_MCTARGETDESC_AARCH64WINCOFFSTREAMER_H
#define LLVM_LIB_TARGET_AARCH64_MCTARGETDESC_AARCH64WINCOFFSTREAMER_H

#include "AArch64TargetStreamer.h"
#include "vm/core/MC/MCWinCOFFStreamer.h"

namespace vm::core {

MCStreamer *
createAArch64WinCOFFStreamer(MCContext &Context,
                             std::unique_ptr<MCAsmBackend> &&TAB,
                             std::unique_ptr<MCObjectWriter> &&OW,
                             std::unique_ptr<MCCodeEmitter> &&Emitter);
} // end toolchain namespace

#endif
