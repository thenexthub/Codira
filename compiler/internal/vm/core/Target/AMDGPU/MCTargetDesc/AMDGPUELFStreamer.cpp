//===-------- AMDGPUELFStreamer.cpp - ELF Object Output -------------------===//
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

#include "AMDGPUELFStreamer.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCELFStreamer.h"
#include "vm/core/MC/MCObjectWriter.h"

using namespace vm::core;

namespace {

class AMDGPUELFStreamer : public MCELFStreamer {
public:
  AMDGPUELFStreamer(const Triple &T, MCContext &Context,
                    std::unique_ptr<MCAsmBackend> MAB,
                    std::unique_ptr<MCObjectWriter> OW,
                    std::unique_ptr<MCCodeEmitter> Emitter)
      : MCELFStreamer(Context, std::move(MAB), std::move(OW),
                      std::move(Emitter)) {}
};

} // anonymous namespace

MCELFStreamer *
toolchain::createAMDGPUELFStreamer(const Triple &T, MCContext &Context,
                              std::unique_ptr<MCAsmBackend> MAB,
                              std::unique_ptr<MCObjectWriter> OW,
                              std::unique_ptr<MCCodeEmitter> Emitter) {
  return new AMDGPUELFStreamer(T, Context, std::move(MAB), std::move(OW),
                               std::move(Emitter));
}
