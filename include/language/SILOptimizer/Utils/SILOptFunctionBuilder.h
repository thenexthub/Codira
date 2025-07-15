//===--- SILOptFunctionBuilder.h --------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_UTILS_SILOPTFUNCTIONBUILDER_H
#define LANGUAGE_SILOPTIMIZER_UTILS_SILOPTFUNCTIONBUILDER_H

#include "language/SIL/SILFunctionBuilder.h"
#include "language/SILOptimizer/PassManager/PassManager.h"
#include "language/SILOptimizer/PassManager/Transforms.h"

namespace language {

class SILOptFunctionBuilder {
  SILTransform &transform;
  SILFunctionBuilder builder;

public:
  SILOptFunctionBuilder(SILTransform &transform)
      : transform(transform),
        builder(*transform.getPassManager()->getModule()) {}

  template <class... ArgTys>
  SILFunction *getOrCreateSharedFunction(ArgTys &&... args) {
    SILFunction *f =
      builder.getOrCreateSharedFunction(std::forward<ArgTys>(args)...);
    notifyAddFunction(f);
    return f;
  }

  template <class... ArgTys>
  SILFunction *getOrCreateFunction(ArgTys &&... args) {
    SILFunction *f = builder.getOrCreateFunction(std::forward<ArgTys>(args)...);
    notifyAddFunction(f);
    return f;
  }

  template <class... ArgTys> SILFunction *createFunction(ArgTys &&... args) {
    SILFunction *f = builder.createFunction(std::forward<ArgTys>(args)...);
    notifyAddFunction(f);
    return f;
  }

  void eraseFunction(SILFunction *f) {
    auto &pm = getPassManager();
    pm.notifyWillDeleteFunction(f);
    pm.getModule()->eraseFunction(f);
  }

  SILModule &getModule() const { return *getPassManager().getModule(); }
  irgen::IRGenModule *getIRGenModule() const {
    return transform.getIRGenModule();
  }

private:
  SILPassManager &getPassManager() const {
    return *transform.getPassManager();
  }

  void notifyAddFunction(SILFunction *f) {
    auto &pm = getPassManager();
    pm.notifyOfNewFunction(f, &transform);
    pm.notifyAnalysisOfFunction(f);
  }
};

} // namespace language

#endif
