//===- TemplateInstCallback.h - Template Instantiation Callback - C++ --===//
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
//===---------------------------------------------------------------------===//
//
// This file defines the TemplateInstantiationCallback class, which is the
// base class for callbacks that will be notified at template instantiations.
//
//===---------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_TEMPLATEINSTCALLBACK_H
#define LANGUAGE_CORE_SEMA_TEMPLATEINSTCALLBACK_H

#include "language/Core/Sema/Sema.h"

namespace language::Core {

/// This is a base class for callbacks that will be notified at every
/// template instantiation.
class TemplateInstantiationCallback {
public:
  virtual ~TemplateInstantiationCallback() = default;

  /// Called before doing AST-parsing.
  virtual void initialize(const Sema &TheSema) = 0;

  /// Called after AST-parsing is completed.
  virtual void finalize(const Sema &TheSema) = 0;

  /// Called when instantiation of a template just began.
  virtual void atTemplateBegin(const Sema &TheSema,
                               const Sema::CodeSynthesisContext &Inst) = 0;

  /// Called when instantiation of a template is just about to end.
  virtual void atTemplateEnd(const Sema &TheSema,
                             const Sema::CodeSynthesisContext &Inst) = 0;
};

template <class TemplateInstantiationCallbackPtrs>
void initialize(TemplateInstantiationCallbackPtrs &Callbacks,
                const Sema &TheSema) {
  for (auto &C : Callbacks) {
    if (C)
      C->initialize(TheSema);
  }
}

template <class TemplateInstantiationCallbackPtrs>
void finalize(TemplateInstantiationCallbackPtrs &Callbacks,
              const Sema &TheSema) {
  for (auto &C : Callbacks) {
    if (C)
      C->finalize(TheSema);
  }
}

template <class TemplateInstantiationCallbackPtrs>
void atTemplateBegin(TemplateInstantiationCallbackPtrs &Callbacks,
                     const Sema &TheSema,
                     const Sema::CodeSynthesisContext &Inst) {
  for (auto &C : Callbacks) {
    if (C)
      C->atTemplateBegin(TheSema, Inst);
  }
}

template <class TemplateInstantiationCallbackPtrs>
void atTemplateEnd(TemplateInstantiationCallbackPtrs &Callbacks,
                   const Sema &TheSema,
                   const Sema::CodeSynthesisContext &Inst) {
  for (auto &C : Callbacks) {
    if (C)
      C->atTemplateEnd(TheSema, Inst);
  }
}

} // namespace language::Core

#endif
