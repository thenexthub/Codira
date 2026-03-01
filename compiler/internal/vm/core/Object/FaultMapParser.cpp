//===----------------------- FaultMapParser.cpp ---------------------------===//
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

#include "vm/core/Object/FaultMapParser.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/Format.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

void printFaultType(FaultMapParser::FaultKind FT, raw_ostream &OS) {
  switch (FT) {
  default:
    llvm_unreachable("unhandled fault type!");
  case FaultMapParser::FaultingLoad:
    OS << "FaultingLoad";
    break;
  case FaultMapParser::FaultingLoadStore:
    OS << "FaultingLoadStore";
    break;
  case FaultMapParser::FaultingStore:
    OS << "FaultingStore";
    break;
  }
}

raw_ostream &
toolchain::operator<<(raw_ostream &OS,
                 const FaultMapParser::FunctionFaultInfoAccessor &FFI) {
  OS << "Fault kind: ";
  printFaultType((FaultMapParser::FaultKind)FFI.getFaultKind(), OS);
  OS << ", faulting PC offset: " << FFI.getFaultingPCOffset()
     << ", handling PC offset: " << FFI.getHandlerPCOffset();
  return OS;
}

raw_ostream &toolchain::operator<<(raw_ostream &OS,
                              const FaultMapParser::FunctionInfoAccessor &FI) {
  OS << "FunctionAddress: " << format_hex(FI.getFunctionAddr(), 8)
     << ", NumFaultingPCs: " << FI.getNumFaultingPCs() << "\n";
  for (unsigned I = 0, E = FI.getNumFaultingPCs(); I != E; ++I)
    OS << FI.getFunctionFaultInfoAt(I) << "\n";
  return OS;
}

raw_ostream &toolchain::operator<<(raw_ostream &OS, const FaultMapParser &FMP) {
  OS << "Version: " << format_hex(FMP.getFaultMapVersion(), 2) << "\n";
  OS << "NumFunctions: " << FMP.getNumFunctions() << "\n";

  if (FMP.getNumFunctions() == 0)
    return OS;

  FaultMapParser::FunctionInfoAccessor FI;

  for (unsigned I = 0, E = FMP.getNumFunctions(); I != E; ++I) {
    FI = (I == 0) ? FMP.getFirstFunctionInfo() : FI.getNextFunctionInfo();
    OS << FI;
  }

  return OS;
}
