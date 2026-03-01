//===- lib/MC/MCGOFFStreamer.cpp - GOFF Object Output ---------------------===//
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
// This file assembles .s files and emits GOFF .o object files.
//
//===----------------------------------------------------------------------===//

#include "vm/core/MC/MCGOFFStreamer.h"
#include "vm/core/BinaryFormat/GOFF.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCDirectives.h"
#include "vm/core/MC/MCGOFFObjectWriter.h"
#include "vm/core/MC/MCObjectStreamer.h"
#include "vm/core/MC/MCSymbolGOFF.h"
#include "vm/core/MC/TargetRegistry.h"

using namespace vm::core;

MCGOFFStreamer::MCGOFFStreamer(MCContext &Context,
                               std::unique_ptr<MCAsmBackend> MAB,
                               std::unique_ptr<MCObjectWriter> OW,
                               std::unique_ptr<MCCodeEmitter> Emitter)
    : MCObjectStreamer(Context, std::move(MAB), std::move(OW),
                       std::move(Emitter)) {}

MCGOFFStreamer::~MCGOFFStreamer() = default;

void MCGOFFStreamer::finishImpl() {
  getWriter().setRootSD(static_cast<MCSectionGOFF *>(
                            getContext().getObjectFileInfo()->getTextSection())
                            ->getParent());
  MCObjectStreamer::finishImpl();
}

GOFFObjectWriter &MCGOFFStreamer::getWriter() {
  return static_cast<GOFFObjectWriter &>(getAssembler().getWriter());
}

void MCGOFFStreamer::changeSection(MCSection *Section, uint32_t Subsection) {
  // Make sure that all section are registered in the correct order.
  SmallVector<MCSectionGOFF *> Sections;
  for (auto *S = static_cast<MCSectionGOFF *>(Section); S; S = S->getParent())
    Sections.push_back(S);
  while (!Sections.empty()) {
    auto *S = Sections.pop_back_val();
    MCObjectStreamer::changeSection(S, Sections.empty() ? Subsection : 0);
  }
}

void MCGOFFStreamer::emitLabel(MCSymbol *Symbol, SMLoc Loc) {
  MCSectionGOFF *Section =
      static_cast<MCSectionGOFF *>(getCurrentSectionOnly());
  if (Section->isPR()) {
    if (Section->getBeginSymbol() == nullptr)
      Section->setBeginSymbol(Symbol);
    else
      getContext().reportError(
          Loc, "only one symbol can be defined in a PR section.");
  }
  MCObjectStreamer::emitLabel(Symbol, Loc);
}

bool MCGOFFStreamer::emitSymbolAttribute(MCSymbol *Sym,
                                         MCSymbolAttr Attribute) {
  return static_cast<MCSymbolGOFF *>(Sym)->setSymbolAttribute(Attribute);
}

MCStreamer *toolchain::createGOFFStreamer(MCContext &Context,
                                     std::unique_ptr<MCAsmBackend> &&MAB,
                                     std::unique_ptr<MCObjectWriter> &&OW,
                                     std::unique_ptr<MCCodeEmitter> &&CE) {
  MCGOFFStreamer *S =
      new MCGOFFStreamer(Context, std::move(MAB), std::move(OW), std::move(CE));
  return S;
}
