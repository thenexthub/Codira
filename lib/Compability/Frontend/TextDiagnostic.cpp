//===--- TextDiagnostic.cpp - Text Diagnostic Pretty-Printing -------------===//
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Frontend/TextDiagnostic.h"
#include "language/Core/Basic/DiagnosticOptions.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language::Compability::frontend;

// TODO: Similar enums are defined in clang/lib/Frontend/TextDiagnostic.cpp.
// It would be best to share them
static const enum toolchain::raw_ostream::Colors noteColor =
    toolchain::raw_ostream::BLACK;
static const enum toolchain::raw_ostream::Colors remarkColor =
    toolchain::raw_ostream::BLUE;
static const enum toolchain::raw_ostream::Colors warningColor =
    toolchain::raw_ostream::MAGENTA;
static const enum toolchain::raw_ostream::Colors errorColor = toolchain::raw_ostream::RED;
static const enum toolchain::raw_ostream::Colors fatalColor = toolchain::raw_ostream::RED;
// Used for changing only the bold attribute.
static const enum toolchain::raw_ostream::Colors savedColor =
    toolchain::raw_ostream::SAVEDCOLOR;

TextDiagnostic::TextDiagnostic() {}

TextDiagnostic::~TextDiagnostic() {}

/*static*/ void
TextDiagnostic::printDiagnosticLevel(toolchain::raw_ostream &os,
                                     language::Core::DiagnosticsEngine::Level level,
                                     bool showColors) {
  if (showColors) {
    // Print diagnostic category in bold and color
    switch (level) {
    case language::Core::DiagnosticsEngine::Ignored:
      toolchain_unreachable("Invalid diagnostic type");
    case language::Core::DiagnosticsEngine::Note:
      os.changeColor(noteColor, true);
      break;
    case language::Core::DiagnosticsEngine::Remark:
      os.changeColor(remarkColor, true);
      break;
    case language::Core::DiagnosticsEngine::Warning:
      os.changeColor(warningColor, true);
      break;
    case language::Core::DiagnosticsEngine::Error:
      os.changeColor(errorColor, true);
      break;
    case language::Core::DiagnosticsEngine::Fatal:
      os.changeColor(fatalColor, true);
      break;
    }
  }

  switch (level) {
  case language::Core::DiagnosticsEngine::Ignored:
    toolchain_unreachable("Invalid diagnostic type");
  case language::Core::DiagnosticsEngine::Note:
    os << "note";
    break;
  case language::Core::DiagnosticsEngine::Remark:
    os << "remark";
    break;
  case language::Core::DiagnosticsEngine::Warning:
    os << "warning";
    break;
  case language::Core::DiagnosticsEngine::Error:
    os << "error";
    break;
  case language::Core::DiagnosticsEngine::Fatal:
    os << "fatal error";
    break;
  }

  os << ": ";

  if (showColors)
    os.resetColor();
}

/*static*/
void TextDiagnostic::printDiagnosticMessage(toolchain::raw_ostream &os,
                                            bool isSupplemental,
                                            toolchain::StringRef message,
                                            bool showColors) {
  if (showColors && !isSupplemental) {
    // Print primary diagnostic messages in bold and without color.
    os.changeColor(savedColor, true);
  }

  os << message;

  if (showColors)
    os.resetColor();
  os << '\n';
}
