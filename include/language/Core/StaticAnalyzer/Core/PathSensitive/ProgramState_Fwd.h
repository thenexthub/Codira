//== ProgramState_Fwd.h - Incomplete declarations of ProgramState -*- C++ -*--=/
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

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_PROGRAMSTATE_FWD_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_PROGRAMSTATE_FWD_H

#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"

namespace language::Core {
namespace ento {
  class ProgramState;
  class ProgramStateManager;
  void ProgramStateRetain(const ProgramState *state);
  void ProgramStateRelease(const ProgramState *state);
}
}

namespace toolchain {
  template <> struct IntrusiveRefCntPtrInfo<const language::Core::ento::ProgramState> {
    static void retain(const language::Core::ento::ProgramState *state) {
      language::Core::ento::ProgramStateRetain(state);
    }
    static void release(const language::Core::ento::ProgramState *state) {
      language::Core::ento::ProgramStateRelease(state);
    }
  };
}

namespace language::Core {
namespace ento {
  typedef IntrusiveRefCntPtr<const ProgramState> ProgramStateRef;
}
}

#endif

