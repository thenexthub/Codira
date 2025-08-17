//===- toolchain/Support/PrettyStackTrace.h - Pretty Crash Handling --*- C++ -*-===//
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
//
// This file defines the PrettyStackTraceEntry class, which is used to make
// crashes give more contextual information about what the program was doing
// when it crashed.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_PRETTYSTACKTRACE_H
#define LLVM_SUPPORT_PRETTYSTACKTRACE_H

#include <IndexStoreDB_LLVMSupport/toolchain_ADT_SmallVector.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_Compiler.h>

namespace toolchain {
  class raw_ostream;

  void EnablePrettyStackTrace();

  /// PrettyStackTraceEntry - This class is used to represent a frame of the
  /// "pretty" stack trace that is dumped when a program crashes. You can define
  /// subclasses of this and declare them on the program stack: when they are
  /// constructed and destructed, they will add their symbolic frames to a
  /// virtual stack trace.  This gets dumped out if the program crashes.
  class PrettyStackTraceEntry {
    friend PrettyStackTraceEntry *ReverseStackTrace(PrettyStackTraceEntry *);

    PrettyStackTraceEntry *NextEntry;
    PrettyStackTraceEntry(const PrettyStackTraceEntry &) = delete;
    void operator=(const PrettyStackTraceEntry &) = delete;
  public:
    PrettyStackTraceEntry();
    virtual ~PrettyStackTraceEntry();

    /// print - Emit information about this stack frame to OS.
    virtual void print(raw_ostream &OS) const = 0;

    /// getNextEntry - Return the next entry in the list of frames.
    const PrettyStackTraceEntry *getNextEntry() const { return NextEntry; }
  };

  /// PrettyStackTraceString - This object prints a specified string (which
  /// should not contain newlines) to the stream as the stack trace when a crash
  /// occurs.
  class PrettyStackTraceString : public PrettyStackTraceEntry {
    const char *Str;
  public:
    PrettyStackTraceString(const char *str) : Str(str) {}
    void print(raw_ostream &OS) const override;
  };

  /// PrettyStackTraceFormat - This object prints a string (which may use
  /// printf-style formatting but should not contain newlines) to the stream
  /// as the stack trace when a crash occurs.
  class PrettyStackTraceFormat : public PrettyStackTraceEntry {
    toolchain::SmallVector<char, 32> Str;
  public:
    PrettyStackTraceFormat(const char *Format, ...);
    void print(raw_ostream &OS) const override;
  };

  /// PrettyStackTraceProgram - This object prints a specified program arguments
  /// to the stream as the stack trace when a crash occurs.
  class PrettyStackTraceProgram : public PrettyStackTraceEntry {
    int ArgC;
    const char *const *ArgV;
  public:
    PrettyStackTraceProgram(int argc, const char * const*argv)
      : ArgC(argc), ArgV(argv) {
      EnablePrettyStackTrace();
    }
    void print(raw_ostream &OS) const override;
  };

  /// Returns the topmost element of the "pretty" stack state.
  const void *SavePrettyStackState();

  /// Restores the topmost element of the "pretty" stack state to State, which
  /// should come from a previous call to SavePrettyStackState().  This is
  /// useful when using a CrashRecoveryContext in code that also uses
  /// PrettyStackTraceEntries, to make sure the stack that's printed if a crash
  /// happens after a crash that's been recovered by CrashRecoveryContext
  /// doesn't have frames on it that were added in code unwound by the
  /// CrashRecoveryContext.
  void RestorePrettyStackState(const void *State);

} // end namespace toolchain

#endif
