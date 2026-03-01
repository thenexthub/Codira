//===--- TargetRegistry.cpp - Target registration -------------------------===//
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

#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCInstPrinter.h"
#include "vm/core/MC/MCObjectStreamer.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/Support/raw_ostream.h"
#include <cassert>
#include <vector>
using namespace vm::core;

// Clients are responsible for avoid race conditions in registration.
static Target *FirstTarget = nullptr;

MCStreamer *Target::createMCObjectStreamer(
    const Triple &T, MCContext &Ctx, std::unique_ptr<MCAsmBackend> TAB,
    std::unique_ptr<MCObjectWriter> OW, std::unique_ptr<MCCodeEmitter> Emitter,
    const MCSubtargetInfo &STI) const {
  MCStreamer *S = nullptr;
  switch (T.getObjectFormat()) {
  case Triple::UnknownObjectFormat:
    llvm_unreachable("Unknown object format");
  case Triple::COFF:
    assert((T.isOSWindows() || T.isUEFI()) &&
           "only Windows and UEFI COFF are supported");
    S = COFFStreamerCtorFn(Ctx, std::move(TAB), std::move(OW),
                           std::move(Emitter));
    break;
  case Triple::MachO:
    if (MachOStreamerCtorFn)
      S = MachOStreamerCtorFn(Ctx, std::move(TAB), std::move(OW),
                              std::move(Emitter));
    else
      S = createMachOStreamer(Ctx, std::move(TAB), std::move(OW),
                              std::move(Emitter), false);
    break;
  case Triple::ELF:
    if (ELFStreamerCtorFn)
      S = ELFStreamerCtorFn(T, Ctx, std::move(TAB), std::move(OW),
                            std::move(Emitter));
    else
      S = createELFStreamer(Ctx, std::move(TAB), std::move(OW),
                            std::move(Emitter));
    break;
  case Triple::Wasm:
    S = createWasmStreamer(Ctx, std::move(TAB), std::move(OW),
                           std::move(Emitter));
    break;
  case Triple::GOFF:
    S = createGOFFStreamer(Ctx, std::move(TAB), std::move(OW),
                           std::move(Emitter));
    break;
  case Triple::XCOFF:
    S = XCOFFStreamerCtorFn(T, Ctx, std::move(TAB), std::move(OW),
                            std::move(Emitter));
    break;
  case Triple::SPIRV:
    S = createSPIRVStreamer(Ctx, std::move(TAB), std::move(OW),
                            std::move(Emitter));
    break;
  case Triple::DXContainer:
    S = createDXContainerStreamer(Ctx, std::move(TAB), std::move(OW),
                                  std::move(Emitter));
    break;
  }
  if (ObjectTargetStreamerCtorFn)
    ObjectTargetStreamerCtorFn(*S, STI);
  return S;
}

MCStreamer *Target::createAsmStreamer(MCContext &Ctx,
                                      std::unique_ptr<formatted_raw_ostream> OS,
                                      std::unique_ptr<MCInstPrinter> IP,
                                      std::unique_ptr<MCCodeEmitter> CE,
                                      std::unique_ptr<MCAsmBackend> TAB) const {
  MCInstPrinter *Printer = IP.get();
  formatted_raw_ostream &OSRef = *OS;
  MCStreamer *S;
  if (AsmStreamerCtorFn)
    S = AsmStreamerCtorFn(Ctx, std::move(OS), std::move(IP), std::move(CE),
                          std::move(TAB));
  else
    S = toolchain::createAsmStreamer(Ctx, std::move(OS), std::move(IP),
                                std::move(CE), std::move(TAB));

  createAsmTargetStreamer(*S, OSRef, Printer);
  return S;
}

iterator_range<TargetRegistry::iterator> TargetRegistry::targets() {
  return make_range(iterator(FirstTarget), iterator());
}

const Target *TargetRegistry::lookupTarget(StringRef ArchName,
                                           Triple &TheTriple,
                                           std::string &Error) {
  // Allocate target machine.  First, check whether the user has explicitly
  // specified an architecture to compile for. If so we have to look it up by
  // name, because it might be a backend that has no mapping to a target triple.
  const Target *TheTarget = nullptr;
  if (!ArchName.empty()) {
    auto I = find_if(targets(),
                     [&](const Target &T) { return ArchName == T.getName(); });

    if (I == targets().end()) {
      Error = ("invalid target '" + ArchName + "'.").str();
      return nullptr;
    }

    TheTarget = &*I;

    // Adjust the triple to match (if known), otherwise stick with the
    // given triple.
    Triple::ArchType Type = Triple::getArchTypeForLLVMName(ArchName);
    if (Type != Triple::UnknownArch)
      TheTriple.setArch(Type);
  } else {
    // Get the target specific parser.
    std::string TempError;
    TheTarget = TargetRegistry::lookupTarget(TheTriple, TempError);
    if (!TheTarget) {
      Error = "unable to get target for '" + TheTriple.getTriple() +
              "', see --version and --triple.";
      return nullptr;
    }
  }

  return TheTarget;
}

const Target *TargetRegistry::lookupTarget(const Triple &TT,
                                           std::string &Error) {
  // Provide special warning when no targets are initialized.
  if (targets().begin() == targets().end()) {
    Error = "Unable to find target for this triple (no targets are registered)";
    return nullptr;
  }
  Triple::ArchType Arch = TT.getArch();
  auto ArchMatch = [&](const Target &T) { return T.ArchMatchFn(Arch); };
  auto I = find_if(targets(), ArchMatch);

  if (I == targets().end()) {
    Error =
        "No available targets are compatible with triple \"" + TT.str() + "\"";
    return nullptr;
  }

  auto J = std::find_if(std::next(I), targets().end(), ArchMatch);
  if (J != targets().end()) {
    Error = std::string("Cannot choose between targets \"") + I->Name +
            "\" and \"" + J->Name + "\"";
    return nullptr;
  }

  return &*I;
}

void TargetRegistry::RegisterTarget(Target &T, const char *Name,
                                    const char *ShortDesc,
                                    const char *BackendName,
                                    Target::ArchMatchFnTy ArchMatchFn,
                                    bool HasJIT) {
  assert(Name && ShortDesc && ArchMatchFn &&
         "Missing required target information!");

  // Check if this target has already been initialized, we allow this as a
  // convenience to some clients.
  if (T.Name)
    return;

  // Add to the list of targets.
  T.Next = FirstTarget;
  FirstTarget = &T;

  T.Name = Name;
  T.ShortDesc = ShortDesc;
  T.BackendName = BackendName;
  T.ArchMatchFn = ArchMatchFn;
  T.HasJIT = HasJIT;
}

static int TargetArraySortFn(const std::pair<StringRef, const Target *> *LHS,
                             const std::pair<StringRef, const Target *> *RHS) {
  return LHS->first.compare(RHS->first);
}

void TargetRegistry::printRegisteredTargetsForVersion(raw_ostream &OS) {
  std::vector<std::pair<StringRef, const Target*> > Targets;
  size_t Width = 0;
  for (const auto &T : TargetRegistry::targets()) {
    Targets.push_back(std::make_pair(T.getName(), &T));
    Width = std::max(Width, Targets.back().first.size());
  }
  array_pod_sort(Targets.begin(), Targets.end(), TargetArraySortFn);

  OS << "\n";
  OS << "  Registered Targets:\n";
  for (const auto &Target : Targets) {
    OS << "    " << Target.first;
    OS.indent(Width - Target.first.size())
        << " - " << Target.second->getShortDescription() << '\n';
  }
  if (Targets.empty())
    OS << "    (none)\n";
}
