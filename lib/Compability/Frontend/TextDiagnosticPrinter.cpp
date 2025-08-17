//===--- TextDiagnosticPrinter.cpp - Diagnostic Printer -------------------===//
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
// This diagnostic client prints out their diagnostic messages.
//
//===----------------------------------------------------------------------===//
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Frontend/TextDiagnosticPrinter.h"
#include "language/Compability/Frontend/TextDiagnostic.h"
#include "language/Core/Basic/DiagnosticOptions.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language::Compability::frontend;

TextDiagnosticPrinter::TextDiagnosticPrinter(raw_ostream &diagOs,
                                             language::Core::DiagnosticOptions &diags)
    : os(diagOs), diagOpts(diags) {}

TextDiagnosticPrinter::~TextDiagnosticPrinter() {}

// For remarks only, print the remark option and pass name that was used to a
// raw_ostream. This also supports warnings from invalid remark arguments
// provided.
static void printRemarkOption(toolchain::raw_ostream &os,
                              language::Core::DiagnosticsEngine::Level level,
                              const language::Core::Diagnostic &info) {
  toolchain::StringRef opt =
      info.getDiags()->getDiagnosticIDs()->getWarningOptionForDiag(
          info.getID());
  if (!opt.empty()) {
    // We still need to check if the level is a Remark since, an unknown option
    // warning could be printed i.e. [-Wunknown-warning-option]
    os << " [" << (level == language::Core::DiagnosticsEngine::Remark ? "-R" : "-W")
       << opt;
    toolchain::StringRef optValue = info.getFlagValue();
    if (!optValue.empty())
      os << "=" << optValue;
    os << ']';
  }
}

// For remarks only, if we are receiving a message of this format
// [file location with line and column];;[path to file];;[the remark message]
// then print the absolute file path, line and column number.
void TextDiagnosticPrinter::printLocForRemarks(
    toolchain::raw_svector_ostream &diagMessageStream, toolchain::StringRef &diagMsg) {
  // split incoming string to get the absolute path and filename in the
  // case we are receiving optimization remarks from BackendRemarkConsumer
  diagMsg = diagMessageStream.str();
  toolchain::StringRef delimiter = ";;";

  size_t pos = 0;
  toolchain::SmallVector<toolchain::StringRef> tokens;
  while ((pos = diagMsg.find(delimiter)) != std::string::npos) {
    tokens.push_back(diagMsg.substr(0, pos));
    diagMsg = diagMsg.drop_front(pos + delimiter.size());
  }

  // tokens will always be of size 2 in the case of optimization
  // remark message received
  if (tokens.size() == 2) {
    // Extract absolute path
    toolchain::SmallString<128> absPath = toolchain::sys::path::relative_path(tokens[1]);
    toolchain::sys::path::remove_filename(absPath);
    // Add the last separator before the file name
    toolchain::sys::path::append(absPath, toolchain::sys::path::get_separator());
    toolchain::sys::path::make_preferred(absPath);

    // Used for changing only the bold attribute
    if (diagOpts.ShowColors)
      os.changeColor(toolchain::raw_ostream::SAVEDCOLOR, true);

    // Print path, file name, line and column
    os << absPath << tokens[0] << ": ";
  }
}

void TextDiagnosticPrinter::HandleDiagnostic(
    language::Core::DiagnosticsEngine::Level level, const language::Core::Diagnostic &info) {
  // Default implementation (Warnings/errors count).
  DiagnosticConsumer::HandleDiagnostic(level, info);

  // Render the diagnostic message into a temporary buffer eagerly. We'll use
  // this later as we print out the diagnostic to the terminal.
  toolchain::SmallString<100> outStr;
  info.FormatDiagnostic(outStr);

  toolchain::raw_svector_ostream diagMessageStream(outStr);
  printRemarkOption(diagMessageStream, level, info);

  if (!prefix.empty())
    os << prefix << ": ";

  // We only emit diagnostics in contexts that lack valid source locations.
  assert(!info.getLocation().isValid() &&
         "Diagnostics with valid source location are not supported");

  toolchain::StringRef diagMsg;
  printLocForRemarks(diagMessageStream, diagMsg);

  language::Compability::frontend::TextDiagnostic::printDiagnosticLevel(os, level,
                                                          diagOpts.ShowColors);
  language::Compability::frontend::TextDiagnostic::printDiagnosticMessage(
      os,
      /*IsSupplemental=*/level == language::Core::DiagnosticsEngine::Note, diagMsg,
      diagOpts.ShowColors);

  os.flush();
}
