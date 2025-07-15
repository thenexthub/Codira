//===--- Context.cpp ------------------------------------------------------===//
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

#include "SourceKit/Core/Context.h"
#include "SourceKit/Core/LangSupport.h"
#include "SourceKit/Core/NotificationCenter.h"

using namespace SourceKit;

GlobalConfig::Settings GlobalConfig::update(
    std::optional<unsigned> CompletionMaxASTContextReuseCount,
    std::optional<unsigned> CompletionCheckDependencyInterval) {
  toolchain::sys::ScopedLock L(Mtx);
  if (CompletionMaxASTContextReuseCount.has_value())
    State.IDEInspectionOpts.MaxASTContextReuseCount =
        *CompletionMaxASTContextReuseCount;
  if (CompletionCheckDependencyInterval.has_value())
    State.IDEInspectionOpts.CheckDependencyInterval =
        *CompletionCheckDependencyInterval;
  return State;
}

GlobalConfig::Settings::IDEInspectionOptions
GlobalConfig::getIDEInspectionOpts() const {
  toolchain::sys::ScopedLock L(Mtx);
  return State.IDEInspectionOpts;
}

SourceKit::Context::Context(
    StringRef CodiraExecutablePath, StringRef RuntimeLibPath,
    toolchain::function_ref<std::unique_ptr<LangSupport>(Context &)>
        LangSupportFactoryFn,
    toolchain::function_ref<std::shared_ptr<PluginSupport>(Context &)>
        PluginSupportFactoryFn,
    bool shouldDispatchNotificationsOnMain)
    : CodiraExecutablePath(CodiraExecutablePath), RuntimeLibPath(RuntimeLibPath),
      NotificationCtr(
          new NotificationCenter(shouldDispatchNotificationsOnMain)),
      Config(new GlobalConfig()), ReqTracker(new RequestTracker()),
      SlowRequestSim(new SlowRequestSimulator(ReqTracker)) {
  // Should be called last after everything is initialized.
  CodiraLang = LangSupportFactoryFn(*this);
  Plugins = PluginSupportFactoryFn(*this);
}

SourceKit::Context::~Context() {
}
