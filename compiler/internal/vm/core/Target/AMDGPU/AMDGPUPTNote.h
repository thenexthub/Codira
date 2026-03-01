//===-- AMDGPUNoteType.h - AMDGPU ELF PT_NOTE section info-------*- C++ -*-===//
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
/// \file
///
/// Enums and constants for AMDGPU PT_NOTE sections.
///
//
//===----------------------------------------------------------------------===//
//
#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUPTNOTE_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUPTNOTE_H

namespace vm::core {
namespace AMDGPU {

namespace ElfNote {

const char SectionName[] = ".note";

const char NoteNameV2[] = "AMD";
const char NoteNameV3[] = "AMDGPU";

} // End namespace ElfNote
} // End namespace AMDGPU
} // End namespace vm::core
#endif // LLVM_LIB_TARGET_AMDGPU_AMDGPUPTNOTE_H
