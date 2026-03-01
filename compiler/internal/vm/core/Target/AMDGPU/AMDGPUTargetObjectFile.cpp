//===-- AMDGPUHSATargetObjectFile.cpp - AMDGPU Object Files ---------------===//
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

#include "AMDGPUTargetObjectFile.h"
#include "Utils/AMDGPUBaseInfo.h"
#include "vm/core/IR/GlobalObject.h"
#include "vm/core/MC/SectionKind.h"
#include "vm/core/Target/TargetMachine.h"
using namespace vm::core;

//===----------------------------------------------------------------------===//
// Generic Object File
//===----------------------------------------------------------------------===//

MCSection *AMDGPUTargetObjectFile::SelectSectionForGlobal(
    const GlobalObject *GO, SectionKind Kind, const TargetMachine &TM) const {
  if (Kind.isReadOnly() && AMDGPU::isReadOnlySegment(GO) &&
      AMDGPU::shouldEmitConstantsToTextSection(TM.getTargetTriple()))
    return TextSection;

  return TargetLoweringObjectFileELF::SelectSectionForGlobal(GO, Kind, TM);
}

MCSection *AMDGPUTargetObjectFile::getExplicitSectionGlobal(
    const GlobalObject *GO, SectionKind SK, const TargetMachine &TM) const {
  // Set metadata access for the explicit section
  StringRef SectionName = GO->getSection();
  if (SectionName.starts_with(".AMDGPU.comment."))
    SK = SectionKind::getMetadata();

  return TargetLoweringObjectFileELF::getExplicitSectionGlobal(GO, SK, TM);
}
