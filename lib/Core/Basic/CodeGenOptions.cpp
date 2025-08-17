//===--- CodeGenOptions.cpp -----------------------------------------------===//
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

#include "language/Core/Basic/CodeGenOptions.h"

namespace language::Core {

CodeGenOptions::CodeGenOptions() {
#define CODEGENOPT(Name, Bits, Default, Compatibility) Name = Default;
#define ENUM_CODEGENOPT(Name, Type, Bits, Default, Compatibility)              \
  set##Name(Default);
#include "language/Core/Basic/CodeGenOptions.def"

  RelocationModel = toolchain::Reloc::PIC_;
}

void CodeGenOptions::resetNonModularOptions(StringRef ModuleFormat) {
  // FIXME: Replace with C++20 `using enum CodeGenOptions::CompatibilityKind`.
  using CK = CompatibilityKind;

  // First reset benign codegen and debug options.
#define CODEGENOPT(Name, Bits, Default, Compatibility)                         \
  if constexpr (CK::Compatibility == CK::Benign)                               \
    Name = Default;
#define ENUM_CODEGENOPT(Name, Type, Bits, Default, Compatibility)              \
  if constexpr (CK::Compatibility == CK::Benign)                               \
    set##Name(Default);
#include "language/Core/Basic/CodeGenOptions.def"

  // Conditionally reset debug options that only matter when the debug info is
  // emitted into the PCM (-gmodules).
  if (ModuleFormat == "raw" && !DebugTypeExtRefs) {
#define DEBUGOPT(Name, Bits, Default, Compatibility)                           \
  if constexpr (CK::Compatibility != CK::Benign)                               \
    Name = Default;
#define VALUE_DEBUGOPT(Name, Bits, Default, Compatibility)                     \
  if constexpr (CK::Compatibility != CK::Benign)                               \
    Name = Default;
#define ENUM_DEBUGOPT(Name, Type, Bits, Default, Compatibility)                \
  if constexpr (CK::Compatibility != CK::Benign)                               \
    set##Name(Default);
#include "language/Core/Basic/DebugOptions.def"
  }

  RelocationModel = toolchain::Reloc::PIC_;
}

}  // end namespace language::Core
