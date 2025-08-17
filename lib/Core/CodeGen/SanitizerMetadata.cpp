//===--- SanitizerMetadata.cpp - Ignored entities for sanitizers ----------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
// Class which emits metadata consumed by sanitizer instrumentation passes.
//
//===----------------------------------------------------------------------===//
#include "SanitizerMetadata.h"
#include "CodeGenModule.h"
#include "language/Core/AST/Attr.h"
#include "language/Core/AST/Type.h"

using namespace language::Core;
using namespace CodeGen;

SanitizerMetadata::SanitizerMetadata(CodeGenModule &CGM) : CGM(CGM) {}

static bool isAsanHwasanMemTagOrTysan(const SanitizerSet &SS) {
  return SS.hasOneOf(SanitizerKind::Address | SanitizerKind::KernelAddress |
                     SanitizerKind::HWAddress | SanitizerKind::MemTag |
                     SanitizerKind::Type);
}

static SanitizerMask expandKernelSanitizerMasks(SanitizerMask Mask) {
  if (Mask & (SanitizerKind::Address | SanitizerKind::KernelAddress))
    Mask |= SanitizerKind::Address | SanitizerKind::KernelAddress;
  // Note: KHWASan doesn't support globals.
  return Mask;
}

void SanitizerMetadata::reportGlobal(toolchain::GlobalVariable *GV,
                                     SourceLocation Loc, StringRef Name,
                                     QualType Ty,
                                     SanitizerMask NoSanitizeAttrMask,
                                     bool IsDynInit) {
  SanitizerSet FsanitizeArgument = CGM.getLangOpts().Sanitize;
  if (!isAsanHwasanMemTagOrTysan(FsanitizeArgument))
    return;

  FsanitizeArgument.Mask = expandKernelSanitizerMasks(FsanitizeArgument.Mask);
  NoSanitizeAttrMask = expandKernelSanitizerMasks(NoSanitizeAttrMask);
  SanitizerSet NoSanitizeAttrSet = {NoSanitizeAttrMask &
                                    FsanitizeArgument.Mask};

  toolchain::GlobalVariable::SanitizerMetadata Meta;
  if (GV->hasSanitizerMetadata())
    Meta = GV->getSanitizerMetadata();

  Meta.NoAddress |= NoSanitizeAttrSet.hasOneOf(SanitizerKind::Address);
  Meta.NoAddress |= CGM.isInNoSanitizeList(
      FsanitizeArgument.Mask & SanitizerKind::Address, GV, Loc, Ty);

  Meta.NoHWAddress |= NoSanitizeAttrSet.hasOneOf(SanitizerKind::HWAddress);
  Meta.NoHWAddress |= CGM.isInNoSanitizeList(
      FsanitizeArgument.Mask & SanitizerKind::HWAddress, GV, Loc, Ty);

  Meta.Memtag |=
      static_cast<bool>(FsanitizeArgument.Mask & SanitizerKind::MemtagGlobals);
  Meta.Memtag &= !NoSanitizeAttrSet.hasOneOf(SanitizerKind::MemTag);
  Meta.Memtag &= !CGM.isInNoSanitizeList(
      FsanitizeArgument.Mask & SanitizerKind::MemTag, GV, Loc, Ty);

  Meta.IsDynInit = IsDynInit && !Meta.NoAddress &&
                   FsanitizeArgument.has(SanitizerKind::Address) &&
                   !CGM.isInNoSanitizeList(SanitizerKind::Address |
                                               SanitizerKind::KernelAddress,
                                           GV, Loc, Ty, "init");

  GV->setSanitizerMetadata(Meta);

  if (Ty.isNull() || !CGM.getLangOpts().Sanitize.has(SanitizerKind::Type) ||
      NoSanitizeAttrMask & SanitizerKind::Type)
    return;

  toolchain::MDNode *TBAAInfo = CGM.getTBAATypeInfo(Ty);
  if (!TBAAInfo || TBAAInfo == CGM.getTBAATypeInfo(CGM.getContext().CharTy))
    return;

  toolchain::Metadata *GlobalMetadata[] = {toolchain::ConstantAsMetadata::get(GV),
                                      TBAAInfo};

  // Metadata for the global already registered.
  if (toolchain::MDNode::getIfExists(CGM.getLLVMContext(), GlobalMetadata))
    return;

  toolchain::MDNode *ThisGlobal =
      toolchain::MDNode::get(CGM.getLLVMContext(), GlobalMetadata);
  toolchain::NamedMDNode *TysanGlobals =
      CGM.getModule().getOrInsertNamedMetadata("toolchain.tysan.globals");
  TysanGlobals->addOperand(ThisGlobal);
}

void SanitizerMetadata::reportGlobal(toolchain::GlobalVariable *GV, const VarDecl &D,
                                     bool IsDynInit) {
  if (!isAsanHwasanMemTagOrTysan(CGM.getLangOpts().Sanitize))
    return;
  std::string QualName;
  toolchain::raw_string_ostream OS(QualName);
  D.printQualifiedName(OS);

  auto getNoSanitizeMask = [](const VarDecl &D) {
    if (D.hasAttr<DisableSanitizerInstrumentationAttr>())
      return SanitizerKind::All;

    SanitizerMask NoSanitizeMask;
    for (auto *Attr : D.specific_attrs<NoSanitizeAttr>())
      NoSanitizeMask |= Attr->getMask();

    // External definitions and incomplete types get handled at the place they
    // are defined.
    if (D.hasExternalStorage() || D.getType()->isIncompleteType())
      NoSanitizeMask |= SanitizerKind::Type;

    return NoSanitizeMask;
  };

  reportGlobal(GV, D.getLocation(), QualName, D.getType(), getNoSanitizeMask(D),
               IsDynInit);
}

void SanitizerMetadata::disableSanitizerForGlobal(toolchain::GlobalVariable *GV) {
  reportGlobal(GV, SourceLocation(), "", QualType(), SanitizerKind::All);
}
