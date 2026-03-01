//===- Error.cpp - tblgen error handling helper routines --------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// This file contains error handling helper routines to pretty-print diagnostic
// messages from tblgen.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ADT/Twine.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/Support/Signals.h"
#include "vm/core/Support/WithColor.h"
#include "vm/core/TableGen/Error.h"
#include "vm/core/TableGen/Record.h"
#include <cstdlib>

using namespace vm::core;

SourceMgr toolchain::SrcMgr;
unsigned toolchain::ErrorsPrinted = 0;

static void PrintMessage(ArrayRef<SMLoc> Locs, SourceMgr::DiagKind Kind,
                         const Twine &Msg) {
  // Count the total number of errors printed.
  // This is used to exit with an error code if there were any errors.
  if (Kind == SourceMgr::DK_Error)
    ++ErrorsPrinted;

  SMLoc NullLoc;
  if (Locs.empty())
    Locs = NullLoc;
  SrcMgr.PrintMessage(Locs.consume_front(), Kind, Msg);
  for (SMLoc Loc : Locs)
    SrcMgr.PrintMessage(Loc, SourceMgr::DK_Note,
                        "instantiated from multiclass");
}

// Run file cleanup handlers and then exit fatally (with non-zero exit code).
[[noreturn]] inline static void fatal_exit() {
  // The following call runs the file cleanup handlers.
  sys::RunInterruptHandlers();
  std::exit(1);
}

// Functions to print notes.

void toolchain::PrintNote(const Twine &Msg) { WithColor::note() << Msg << "\n"; }

void toolchain::PrintNote(function_ref<void(raw_ostream &OS)> PrintMsg) {
  PrintMsg(WithColor::note());
}

void toolchain::PrintNote(ArrayRef<SMLoc> NoteLoc, const Twine &Msg) {
  PrintMessage(NoteLoc, SourceMgr::DK_Note, Msg);
}

// Functions to print fatal notes.

void toolchain::PrintFatalNote(const Twine &Msg) {
  PrintNote(Msg);
  fatal_exit();
}

void toolchain::PrintFatalNote(ArrayRef<SMLoc> NoteLoc, const Twine &Msg) {
  PrintNote(NoteLoc, Msg);
  fatal_exit();
}

// This method takes a Record and uses the source location
// stored in it.
void toolchain::PrintFatalNote(const Record *Rec, const Twine &Msg) {
  PrintNote(Rec->getLoc(), Msg);
  fatal_exit();
}

// This method takes a RecordVal and uses the source location
// stored in it.
void toolchain::PrintFatalNote(const RecordVal *RecVal, const Twine &Msg) {
  PrintNote(RecVal->getLoc(), Msg);
  fatal_exit();
}

// Functions to print warnings.

void toolchain::PrintWarning(const Twine &Msg) {
  WithColor::warning() << Msg << "\n";
}

void toolchain::PrintWarning(ArrayRef<SMLoc> WarningLoc, const Twine &Msg) {
  PrintMessage(WarningLoc, SourceMgr::DK_Warning, Msg);
}

void toolchain::PrintWarning(const char *Loc, const Twine &Msg) {
  SrcMgr.PrintMessage(SMLoc::getFromPointer(Loc), SourceMgr::DK_Warning, Msg);
}

// Functions to print errors.

void toolchain::PrintError(const Twine &Msg) { WithColor::error() << Msg << "\n"; }

void toolchain::PrintError(function_ref<void(raw_ostream &OS)> PrintMsg) {
  PrintMsg(WithColor::error());
}

void toolchain::PrintError(ArrayRef<SMLoc> ErrorLoc, const Twine &Msg) {
  PrintMessage(ErrorLoc, SourceMgr::DK_Error, Msg);
}

void toolchain::PrintError(const char *Loc, const Twine &Msg) {
  SrcMgr.PrintMessage(SMLoc::getFromPointer(Loc), SourceMgr::DK_Error, Msg);
}

// This method takes a Record and uses the source location
// stored in it.
void toolchain::PrintError(const Record *Rec, const Twine &Msg) {
  PrintMessage(Rec->getLoc(), SourceMgr::DK_Error, Msg);
}

// This method takes a RecordVal and uses the source location
// stored in it.
void toolchain::PrintError(const RecordVal *RecVal, const Twine &Msg) {
  PrintMessage(RecVal->getLoc(), SourceMgr::DK_Error, Msg);
}

// Functions to print fatal errors.

void toolchain::PrintFatalError(const Twine &Msg) {
  PrintError(Msg);
  fatal_exit();
}

void toolchain::PrintFatalError(function_ref<void(raw_ostream &OS)> PrintMsg) {
  PrintError(PrintMsg);
  fatal_exit();
}

void toolchain::PrintFatalError(ArrayRef<SMLoc> ErrorLoc, const Twine &Msg) {
  PrintError(ErrorLoc, Msg);
  fatal_exit();
}

// This method takes a Record and uses the source location
// stored in it.
void toolchain::PrintFatalError(const Record *Rec, const Twine &Msg) {
  PrintError(Rec->getLoc(), Msg);
  fatal_exit();
}

// This method takes a RecordVal and uses the source location
// stored in it.
void toolchain::PrintFatalError(const RecordVal *RecVal, const Twine &Msg) {
  PrintError(RecVal->getLoc(), Msg);
  fatal_exit();
}

// Check an assertion: Obtain the condition value and be sure it is true.
// If not, print a nonfatal error along with the message.
bool toolchain::CheckAssert(SMLoc Loc, const Init *Condition, const Init *Message) {
  auto *CondValue = dyn_cast_or_null<IntInit>(Condition->convertInitializerTo(
      IntRecTy::get(Condition->getRecordKeeper())));
  if (!CondValue) {
    PrintError(Loc, "assert condition must of type bit, bits, or int.");
    return true;
  }
  if (!CondValue->getValue()) {
    auto *MessageInit = dyn_cast<StringInit>(Message);
    StringRef AssertMsg = MessageInit ? MessageInit->getValue()
                                      : "(assert message is not a string)";
    PrintError(Loc, "assertion failed: " + AssertMsg);
    return true;
  }
  return false;
}

// Dump a message to stderr.
void toolchain::dumpMessage(SMLoc Loc, const Init *Message) {
  if (auto *MessageInit = dyn_cast<StringInit>(Message))
    PrintNote(Loc, MessageInit->getValue());
  else
    PrintError(Loc, "dump value is not of type string");
}
