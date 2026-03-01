//===- toolchain/IR/DiagnosticPrinter.cpp - Diagnostic Printer -------*- C++ -*-===//
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
// This file defines a diagnostic printer relying on raw_ostream.
//
//===----------------------------------------------------------------------===//

#include "vm/core/IR/DiagnosticPrinter.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/Value.h"
#include "vm/core/Support/SourceMgr.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

DiagnosticPrinter &DiagnosticPrinterRawOStream::operator<<(char C) {
  Stream << C;
  return *this;
}

DiagnosticPrinter &DiagnosticPrinterRawOStream::operator<<(unsigned char C) {
  Stream << C;
  return *this;
}

DiagnosticPrinter &DiagnosticPrinterRawOStream::operator<<(signed char C) {
  Stream << C;
  return *this;
}

DiagnosticPrinter &DiagnosticPrinterRawOStream::operator<<(StringRef Str) {
  Stream << Str;
  return *this;
}

DiagnosticPrinter &DiagnosticPrinterRawOStream::operator<<(const char *Str) {
  Stream << Str;
  return *this;
}

DiagnosticPrinter &DiagnosticPrinterRawOStream::operator<<(
    const std::string &Str) {
  Stream << Str;
  return *this;
}

DiagnosticPrinter &DiagnosticPrinterRawOStream::operator<<(unsigned long N) {
  Stream << N;
  return *this;
}
DiagnosticPrinter &DiagnosticPrinterRawOStream::operator<<(long N) {
  Stream << N;
  return *this;
}

DiagnosticPrinter &DiagnosticPrinterRawOStream::operator<<(
    unsigned long long N) {
  Stream << N;
  return *this;
}

DiagnosticPrinter &DiagnosticPrinterRawOStream::operator<<(long long N) {
  Stream << N;
  return *this;
}

DiagnosticPrinter &DiagnosticPrinterRawOStream::operator<<(const void *P) {
  Stream << P;
  return *this;
}

DiagnosticPrinter &DiagnosticPrinterRawOStream::operator<<(unsigned int N) {
  Stream << N;
  return *this;
}

DiagnosticPrinter &DiagnosticPrinterRawOStream::operator<<(int N) {
  Stream << N;
  return *this;
}

DiagnosticPrinter &DiagnosticPrinterRawOStream::operator<<(double N) {
  Stream << N;
  return *this;
}

DiagnosticPrinter &DiagnosticPrinterRawOStream::operator<<(const Twine &Str) {
  Str.print(Stream);
  return *this;
}

// IR related types.
DiagnosticPrinter &DiagnosticPrinterRawOStream::operator<<(const Value &V) {
  // Avoid printing '@' prefix for named functions.
  if (V.hasName())
    Stream << V.getName();
  else
    V.printAsOperand(Stream, /*PrintType=*/false);

  return *this;
}

DiagnosticPrinter &DiagnosticPrinterRawOStream::operator<<(const Module &M) {
  Stream << M.getModuleIdentifier();
  return *this;
}

// Other types.
DiagnosticPrinter &DiagnosticPrinterRawOStream::
operator<<(const SMDiagnostic &Diag) {
  // We don't have to print the SMDiagnostic kind, as the diagnostic severity
  // is printed by the diagnostic handler.
  Diag.print("", Stream, /*ShowColors=*/true, /*ShowKindLabel=*/false);
  return *this;
}
