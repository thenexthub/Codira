//===--- Immediate.h - Entry point for swift immediate mode -----*- C++ -*-===//
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
//
// This is the entry point to the swift immediate mode, which takes a
// source file, and runs it immediately using the JIT.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_IMMEDIATE_IMMEDIATE_H
#define SWIFT_IMMEDIATE_IMMEDIATE_H

#include <memory>
#include <string>
#include <vector>

namespace language {
  class CompilerInstance;
  class IRGenOptions;
  class SILOptions;
  class SILModule;

  // Using LLVM containers to store command-line arguments turns out
  // to be a lose, because LLVM's execution engine demands this vector
  // type.  We can flip the typedef if/when the LLVM interface
  // supports LLVM containers.
  using ProcessCmdLine = std::vector<std::string>;
  

  /// Attempt to run the script identified by the given compiler instance.
  ///
  /// \return the result returned from main(), if execution succeeded
  int RunImmediately(CompilerInstance &CI, const ProcessCmdLine &CmdLine,
                     const IRGenOptions &IRGenOpts, const SILOptions &SILOpts,
                     std::unique_ptr<SILModule> &&SM);

  int RunImmediatelyFromAST(CompilerInstance &CI);

} // end namespace language

#endif // SWIFT_IMMEDIATE_IMMEDIATE_H
