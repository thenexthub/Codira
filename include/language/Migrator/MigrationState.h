//===--- MigrationState.h - Migration State ---------------------*- C++ -*-===//
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
// This class is an explicit container for a state during migration, its input
// and output text, as well as what created this state.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_MIGRATOR_MIGRATIONSTATE_H
#define SWIFT_MIGRATOR_MIGRATIONSTATE_H

#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/StringRef.h"

namespace language {

class SourceManager;

namespace migrator {

enum class MigrationKind {
  /// The start state of the migrator.
  Start,

  /// A syntactic pass. This is generic for now because there isn't a good
  /// way to separate out and compose syntactic passes until lib/Syntax is
  /// integrated.
  Syntactic,

  /// The compiler has made several passes over the input file and we
  /// applied the suggested fix-its we deemed appropriate.
  CompilerFixits,
};

struct MigrationState : public llvm::ThreadSafeRefCountedBase<MigrationState> {
  MigrationKind Kind;
  SourceManager &SrcMgr;
  unsigned InputBufferID;
  unsigned OutputBufferID;

  MigrationState(const MigrationKind Kind, SourceManager &SrcMgr,
                 const unsigned InputBufferID, const unsigned OutputBufferID)
    : Kind(Kind), SrcMgr(SrcMgr),
      InputBufferID(InputBufferID),
      OutputBufferID(OutputBufferID) {}

  MigrationKind getKind() const {
    return Kind;
  }

  std::string getInputText() const;

  unsigned getInputBufferID() const {
    return InputBufferID;
  }

  std::string getOutputText() const;

  unsigned getOutputBufferID() const {
    return OutputBufferID;
  }

  /// Write all relevant information about the state to OutDir, such as the
  /// input file, output file, replacements, syntax trees, etc.
  bool print(size_t StateNumber, StringRef OutDir) const;

  bool noChangesOccurred() const {
    return InputBufferID == OutputBufferID;
  }

  static llvm::IntrusiveRefCntPtr<MigrationState>
  start(SourceManager &SrcMgr, const unsigned InputBufferID) {
    return llvm::IntrusiveRefCntPtr<MigrationState> {
      new MigrationState {
        MigrationKind::Start, SrcMgr, InputBufferID, InputBufferID
      }
    };
  }

  static llvm::IntrusiveRefCntPtr<MigrationState>
  make(MigrationKind Kind, SourceManager &SrcMgr, const unsigned InputBufferID,
       const unsigned OutputBufferID) {
    return llvm::IntrusiveRefCntPtr<MigrationState> {
      new MigrationState {
        Kind,
        SrcMgr,
        InputBufferID,
        // The input is the output here, because nothing happened yet.
        OutputBufferID
      }
    };
  }
};

}
}
#endif // SWIFT_MIGRATOR_MIGRATIONSTATE_H
