//===--- SILDebugInfoGenerator.cpp - Writes a SIL file for debugging ------===//
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

#define DEBUG_TYPE "sil-based-debuginfo-gen"
#include "language/AST/SILOptions.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/SILPrintContext.h"
#include "language/SIL/SILModule.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/MemoryBuffer.h"

using namespace language;

namespace {

/// A pass for generating debug info on SIL level.
///
/// This pass is only enabled if SILOptions::SILOutputFileNameForDebugging is
/// set (i.e. if the -sil-based-debuginfo frontend option is specified).
/// The pass writes all SIL functions into one or multiple output files,
/// depending on the size of the SIL. The names of the output files are derived
/// from the main output file.
///
///     output file name = <main-output-filename>.sil_dbg_<n>.sil
///
/// Where <n> is a consecutive number. The files are stored in the same
/// same directory as the main output file.
/// The debug locations and scopes of all functions and instructions are changed
/// to point to the generated SIL output files.
/// This enables debugging and profiling on SIL level.
class SILDebugInfoGenerator : public SILModuleTransform {

  enum {
    /// To prevent extra large output files, e.g. when compiling the stdlib.
    LineLimitPerFile = 10000
  };

  /// A stream for counting line numbers.
  struct LineCountStream : public toolchain::raw_ostream {
    toolchain::raw_ostream &Underlying;
    int LineNum = 1;
    uint64_t Pos = 0;

    void write_impl(const char *Ptr, size_t Size) override {
      for (size_t Idx = 0; Idx < Size; ++Idx) {
        char c = Ptr[Idx];
        if (c == '\n')
        ++LineNum;
      }
      Underlying.write(Ptr, Size);
      Pos += Size;
    }

    uint64_t current_pos() const override { return Pos; }

    LineCountStream(toolchain::raw_ostream &Underlying) :
      toolchain::raw_ostream(/* unbuffered = */ true),
      Underlying(Underlying) { }
    
    ~LineCountStream() override {
      flush();
    }
  };

  /// A print context which records the line numbers where instructions are
  /// printed.
  struct PrintContext : public SILPrintContext {

    LineCountStream LCS;

    toolchain::DenseMap<const SILInstruction *, int> LineNums;

    void printInstructionCallBack(const SILInstruction *I) override {
      // Record the current line number of the instruction.
      LineNums[I] = LCS.LineNum;
    }

    PrintContext(toolchain::raw_ostream &OS) : SILPrintContext(LCS), LCS(OS) { }

    ~PrintContext() override { }
  };

  void run() override {
    SILModule *M = getModule();
    StringRef FileBaseName = M->getOptions().SILOutputFileNameForDebugging;
    if (FileBaseName.empty())
      return;

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "** SILDebugInfoGenerator **\n");

    std::vector<SILFunction *> PrintedFuncs;
    int FileIdx = 0;
    auto FIter = M->begin();
    while (FIter != M->end()) {

      std::string FileName;
      toolchain::raw_string_ostream NameOS(FileName);
      NameOS << FileBaseName << ".sil_dbg_" << FileIdx++ << ".sil";
      NameOS.flush();

      char *FileNameBuf = (char *)M->allocate(FileName.size() + 1, 1);
      strcpy(FileNameBuf, FileName.c_str());

      TOOLCHAIN_DEBUG(toolchain::dbgs() << "Write debug SIL file " << FileName << '\n');

      std::error_code EC;
      toolchain::raw_fd_ostream OutFile(FileName, EC,
                                   toolchain::sys::fs::OpenFlags::OF_None);
      assert(!OutFile.has_error() && !EC && "Can't write SIL debug file");

      PrintContext Ctx(OutFile);

      // Write functions until we reach the LineLimitPerFile.
      do {
        SILFunction *F = &*FIter++;
        PrintedFuncs.push_back(F);

        // Set the debug scope for the function.
        RegularLocation Loc(SILLocation::FilenameAndLocation::alloc(
             Ctx.LCS.LineNum, 1,FileNameBuf, *M));
        SILDebugScope *Scope = new (*M) SILDebugScope(Loc, F);
        F->setSILDebugScope(Scope);

        // Ensure that the function is visible for debugging.
        F->setBare(IsNotBare);

        // Print it to the output file.
        F->print(Ctx);
      } while (FIter != M->end() && Ctx.LCS.LineNum < LineLimitPerFile);

      // Set the debug locations of all instructions.
      for (SILFunction *F : PrintedFuncs) {
        const SILDebugScope *Scope = F->getDebugScope();
        for (SILBasicBlock &BB : *F) {
          for (auto iter = BB.begin(), end = BB.end(); iter != end;) {
            SILInstruction *I = &*iter;
            ++iter;
            if (isa<DebugValueInst>(I)) {
              // debug_value instructions are not needed anymore.
              // Also, keeping them might trigger a verifier error.
              I->eraseFromParent();
              continue;
            }
            if (auto *ASI = dyn_cast<AllocStackInst>(I))
              // Remove the debug variable scope enclosed
              // within the SILDebugVariable such that we won't
              // trigger a verification error.
              ASI->setDebugVarScope(nullptr);

            SILLocation Loc = I->getLoc();
            auto *filePos = SILLocation::FilenameAndLocation::alloc(
                Ctx.LineNums[I], 1, FileNameBuf, *M);
            assert(filePos->line && "no line set for instruction");
            if (Loc.is<ReturnLocation>()) {
              I->setDebugLocation(
                SILDebugLocation(ReturnLocation(filePos), Scope));
            } else if (Loc.is<ImplicitReturnLocation>()) {
              I->setDebugLocation(
                SILDebugLocation(ImplicitReturnLocation(filePos), Scope));
            } else {
              I->setDebugLocation(
                SILDebugLocation(RegularLocation(filePos), Scope));
            }
          }
        }
      }
      PrintedFuncs.clear();
    }
  }

};

} // end anonymous namespace

SILTransform *language::createSILDebugInfoGenerator() {
  return new SILDebugInfoGenerator();
}
