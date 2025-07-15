//===--- SerializedDiagnosticConsumer.h - Serialize Diagnostics -*- C++ -*-===//
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
//  This file defines the SerializedDiagnosticConsumer class, which
//  serializes diagnostics to Clang's serialized diagnostic bitcode format.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SERIALIZEDDIAGNOSTICCONSUMER_H
#define LANGUAGE_SERIALIZEDDIAGNOSTICCONSUMER_H

#include <memory>

namespace toolchain {
  class StringRef;
}

namespace language {

  class DiagnosticConsumer;

  namespace serialized_diagnostics {
    /// Create a DiagnosticConsumer that serializes diagnostics to a file, using
    /// Clang's serialized diagnostics format.
    ///
    /// \param outputPath the file path to write the diagnostics to.
    ///
    /// \returns A new diagnostic consumer that serializes diagnostics.
    std::unique_ptr<DiagnosticConsumer>
    createConsumer(toolchain::StringRef outputPath, bool emitMacroExpansionFiles);
  }
}

#endif
