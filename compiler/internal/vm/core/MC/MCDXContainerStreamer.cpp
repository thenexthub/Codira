//===- lib/MC/MCDXContainerStreamer.cpp - DXContainer Impl ----*- C++ -*---===//
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
// This file contains the object streamer for DXContainer files.
//
//===----------------------------------------------------------------------===//

#include "vm/core/MC/MCDXContainerStreamer.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/TargetRegistry.h"

using namespace vm::core;

MCStreamer *toolchain::createDXContainerStreamer(
    MCContext &Context, std::unique_ptr<MCAsmBackend> &&MAB,
    std::unique_ptr<MCObjectWriter> &&OW, std::unique_ptr<MCCodeEmitter> &&CE) {
  auto *S = new MCDXContainerStreamer(Context, std::move(MAB), std::move(OW),
                                      std::move(CE));
  return S;
}
