//===- toolchain/CodeGen/PseudoProbePrinter.cpp - Pseudo Probe Emission -------===//
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
// This file contains support for writing pseudo probe info into asm files.
//
//===----------------------------------------------------------------------===//

#include "PseudoProbePrinter.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/IR/DebugInfoMetadata.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/PseudoProbe.h"
#include "vm/core/MC/MCPseudoProbe.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/ProfileData/SampleProf.h"

#ifndef NDEBUG
#include "vm/core/IR/Module.h"
#include "vm/core/Support/WithColor.h"
#endif

using namespace vm::core;

#ifndef NDEBUG
// Deprecated with ThinLTO. For some modules compiled with ThinLTO, certain
// pseudo probe descriptors may not be imported, resulting in false positive
// warning.
static cl::opt<bool> VerifyGuidExistence(
    "pseudo-probe-verify-guid-existence-in-desc",
    cl::desc("Verify whether GUID exists in the .pseudo_probe_desc."),
    cl::Hidden, cl::init(false));
#endif

void PseudoProbeHandler::emitPseudoProbe(uint64_t Guid, uint64_t Index,
                                         uint64_t Type, uint64_t Attr,
                                         const DILocation *DebugLoc) {
  // Gather all the inlined-at nodes.
  // When it's done ReversedInlineStack looks like ([66, B], [88, A])
  // which means, Function A inlines function B at calliste with a probe id 88,
  // and B inlines C at probe 66 where C is represented by Guid.
  SmallVector<InlineSite, 8> ReversedInlineStack;
  auto *InlinedAt = DebugLoc ? DebugLoc->getInlinedAt() : nullptr;
  while (InlinedAt) {
    auto Name = InlinedAt->getSubprogramLinkageName();
    // Strip Coroutine suffixes from CoroSplit Pass, since pseudo probes are
    // generated in an earlier stage.
    Name = FunctionSamples::getCanonicalCoroFnName(Name);
    // Use caching to avoid redundant md5 computation for build speed.
    uint64_t &CallerGuid = NameGuidMap[Name];
    if (!CallerGuid)
      CallerGuid = Function::getGUIDAssumingExternalLinkage(Name);
#ifndef NDEBUG
    if (VerifyGuidExistence)
      verifyGuidExistenceInDesc(CallerGuid, Name);
#endif
    uint64_t CallerProbeId = PseudoProbeDwarfDiscriminator::extractProbeIndex(
        InlinedAt->getDiscriminator());
    ReversedInlineStack.emplace_back(CallerGuid, CallerProbeId);
    InlinedAt = InlinedAt->getInlinedAt();
  }
  uint64_t Discriminator = 0;
  // For now only block probes have FS discriminators. See
  // MIRFSDiscriminator.cpp for more details.
  if (EnableFSDiscriminator && DebugLoc &&
      (Type == (uint64_t)PseudoProbeType::Block))
    Discriminator = DebugLoc->getDiscriminator();
  assert((EnableFSDiscriminator || Discriminator == 0) &&
         "Discriminator should not be set in non-FSAFDO mode");
  SmallVector<InlineSite, 8> InlineStack(toolchain::reverse(ReversedInlineStack));
  Asm->OutStreamer->emitPseudoProbe(Guid, Index, Type, Attr, Discriminator,
                                    InlineStack, Asm->CurrentFnSym);
#ifndef NDEBUG
  if (VerifyGuidExistence)
    verifyGuidExistenceInDesc(
        Guid, DebugLoc ? DebugLoc->getSubprogramLinkageName() : "");
#endif
}

#ifndef NDEBUG
void PseudoProbeHandler::verifyGuidExistenceInDesc(uint64_t Guid,
                                                   StringRef FuncName) {
  NamedMDNode *Desc = Asm->MF->getFunction().getParent()->getNamedMetadata(
      PseudoProbeDescMetadataName);
  assert(Desc && "pseudo probe does not exist");

  // Keep DescGuidSet up to date.
  for (size_t I = DescGuidSet.size(), E = Desc->getNumOperands(); I != E; ++I) {
    const auto *MD = cast<MDNode>(Desc->getOperand(I));
    auto *ID = mdconst::extract<ConstantInt>(MD->getOperand(0));
    DescGuidSet.insert(ID->getZExtValue());
  }

  if (!DescGuidSet.contains(Guid))
    WithColor::warning() << "Guid:" << Guid << " Name:" << FuncName
                         << " does not exist in pseudo probe desc\n";
}
#endif
