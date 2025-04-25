//===--- PrettyStackTrace.cpp - Defines Driver crash prettifiers ----------===//
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

#include "language/Driver/PrettyStackTrace.h"

#include "language/Basic/FileTypes.h"
#include "language/Driver/Action.h"
#include "language/Driver/Job.h"
#include "llvm/Option/Arg.h"
#include "llvm/Support/raw_ostream.h"

using namespace language::driver;

void PrettyStackTraceDriverAction::print(llvm::raw_ostream &out) const {
  out << "While " << Description << " for driver Action "
      << TheAction->getClassName() << " of type "
      << file_types::getTypeName(TheAction->getType());
  if (auto *IA = dyn_cast<InputAction>(TheAction)) {
    out << " = ";
    IA->getInputArg().print(out);
  }
  out << '\n';
}

void PrettyStackTraceDriverJob::print(llvm::raw_ostream &out) const {
  out << "While " << Description << " for driver Job ";
  TheJob->printSummary(out);
  out << '\n';
}

void PrettyStackTraceDriverCommandOutput::print(llvm::raw_ostream &out) const {
  out << "While " << Description << " for driver CommandOutput\n";
  TheCommandOutput->print(out);
  out << '\n';
}

void PrettyStackTraceDriverCommandOutputAddition::print(
    llvm::raw_ostream &out) const {
  out << "While adding " << Description << " output named " << NewOutputName
      << " of type " << file_types::getTypeName(NewOutputType) << " for input "
      << PrimaryInput << " to driver CommandOutput\n";
  TheCommandOutput->print(out);
  out << '\n';
}
