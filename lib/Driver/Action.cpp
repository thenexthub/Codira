//===--- Action.cpp - Abstract compilation steps --------------------------===//
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

#include "language/Driver/Action.h"

#include "toolchain/ADT/STLExtras.h"
#include "toolchain/Support/ErrorHandling.h"

using namespace language::driver;
using namespace toolchain::opt;

const char *Action::getClassName(Kind AC) {
  switch (AC) {
  case Kind::Input:  return "input";
  case Kind::CompileJob:  return "compile";
  case Kind::InterpretJob:  return "interpret";
  case Kind::BackendJob:  return "backend";
  case Kind::MergeModuleJob:  return "merge-module";
  case Kind::ModuleWrapJob:  return "modulewrap";
  case Kind::AutolinkExtractJob:  return "language-autolink-extract";
  case Kind::REPLJob:  return "repl";
  case Kind::DynamicLinkJob:  return "link";
  case Kind::StaticLinkJob:  return "static-link";
  case Kind::GenerateDSYMJob:  return "generate-dSYM";
  case Kind::VerifyDebugInfoJob:  return "verify-debug-info";
  case Kind::GeneratePCHJob:  return "generate-pch";
  case Kind::VerifyModuleInterfaceJob: return "verify-module-interface";
  }

  toolchain_unreachable("invalid class");
}

void InputAction::anchor() {}

void JobAction::anchor() {}

void CompileJobAction::anchor() {}

void InterpretJobAction::anchor() {}

void BackendJobAction::anchor() {}

void MergeModuleJobAction::anchor() {}

void ModuleWrapJobAction::anchor() {}

void AutolinkExtractJobAction::anchor() {}

void REPLJobAction::anchor() {}

void DynamicLinkJobAction::anchor() {}

void StaticLinkJobAction::anchor() {}

void GenerateDSYMJobAction::anchor() {}

void VerifyDebugInfoJobAction::anchor() {}

void GeneratePCHJobAction::anchor() {}

void VerifyModuleInterfaceJobAction::anchor() {}
