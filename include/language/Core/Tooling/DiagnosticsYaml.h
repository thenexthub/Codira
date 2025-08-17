//===-- DiagnosticsYaml.h -- Serialiazation for Diagnosticss ---*- C++ -*-===//
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
///
/// \file
/// This file defines the structure of a YAML document for serializing
/// diagnostics.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_DIAGNOSTICSYAML_H
#define LANGUAGE_CORE_TOOLING_DIAGNOSTICSYAML_H

#include "language/Core/Tooling/Core/Diagnostic.h"
#include "language/Core/Tooling/ReplacementsYaml.h"
#include "toolchain/Support/YAMLTraits.h"
#include <string>

LLVM_YAML_IS_SEQUENCE_VECTOR(language::Core::tooling::Diagnostic)
LLVM_YAML_IS_SEQUENCE_VECTOR(language::Core::tooling::DiagnosticMessage)
LLVM_YAML_IS_SEQUENCE_VECTOR(language::Core::tooling::FileByteRange)

namespace toolchain {
namespace yaml {

template <> struct MappingTraits<language::Core::tooling::FileByteRange> {
  static void mapping(IO &Io, language::Core::tooling::FileByteRange &R) {
    Io.mapRequired("FilePath", R.FilePath);
    Io.mapRequired("FileOffset", R.FileOffset);
    Io.mapRequired("Length", R.Length);
  }
};

template <> struct MappingTraits<language::Core::tooling::DiagnosticMessage> {
  static void mapping(IO &Io, language::Core::tooling::DiagnosticMessage &M) {
    Io.mapRequired("Message", M.Message);
    Io.mapOptional("FilePath", M.FilePath);
    Io.mapOptional("FileOffset", M.FileOffset);
    std::vector<language::Core::tooling::Replacement> Fixes;
    for (auto &Replacements : M.Fix) {
      toolchain::append_range(Fixes, Replacements.second);
    }
    Io.mapRequired("Replacements", Fixes);
    for (auto &Fix : Fixes) {
      toolchain::Error Err = M.Fix[Fix.getFilePath()].add(Fix);
      if (Err) {
        // FIXME: Implement better conflict handling.
        toolchain::errs() << "Fix conflicts with existing fix: "
                     << toolchain::toString(std::move(Err)) << "\n";
      }
    }
    Io.mapOptional("Ranges", M.Ranges);
  }
};

template <> struct MappingTraits<language::Core::tooling::Diagnostic> {
  /// Helper to (de)serialize a Diagnostic since we don't have direct
  /// access to its data members.
  class NormalizedDiagnostic {
  public:
    NormalizedDiagnostic(const IO &)
        : DiagLevel(language::Core::tooling::Diagnostic::Level::Warning) {}

    NormalizedDiagnostic(const IO &, const language::Core::tooling::Diagnostic &D)
        : DiagnosticName(D.DiagnosticName), Message(D.Message), Notes(D.Notes),
          DiagLevel(D.DiagLevel), BuildDirectory(D.BuildDirectory) {}

    language::Core::tooling::Diagnostic denormalize(const IO &) {
      return language::Core::tooling::Diagnostic(DiagnosticName, Message, Notes,
                                        DiagLevel, BuildDirectory);
    }

    std::string DiagnosticName;
    language::Core::tooling::DiagnosticMessage Message;
    SmallVector<language::Core::tooling::DiagnosticMessage, 1> Notes;
    language::Core::tooling::Diagnostic::Level DiagLevel;
    std::string BuildDirectory;
  };

  static void mapping(IO &Io, language::Core::tooling::Diagnostic &D) {
    MappingNormalization<NormalizedDiagnostic, language::Core::tooling::Diagnostic> Keys(
        Io, D);
    Io.mapRequired("DiagnosticName", Keys->DiagnosticName);
    Io.mapRequired("DiagnosticMessage", Keys->Message);
    Io.mapOptional("Notes", Keys->Notes);
    Io.mapOptional("Level", Keys->DiagLevel);
    Io.mapOptional("BuildDirectory", Keys->BuildDirectory);
  }
};

/// Specialized MappingTraits to describe how a
/// TranslationUnitDiagnostics is (de)serialized.
template <> struct MappingTraits<language::Core::tooling::TranslationUnitDiagnostics> {
  static void mapping(IO &Io, language::Core::tooling::TranslationUnitDiagnostics &Doc) {
    Io.mapRequired("MainSourceFile", Doc.MainSourceFile);
    Io.mapRequired("Diagnostics", Doc.Diagnostics);
  }
};

template <> struct ScalarEnumerationTraits<language::Core::tooling::Diagnostic::Level> {
  static void enumeration(IO &IO, language::Core::tooling::Diagnostic::Level &Value) {
    IO.enumCase(Value, "Warning", language::Core::tooling::Diagnostic::Warning);
    IO.enumCase(Value, "Error", language::Core::tooling::Diagnostic::Error);
    IO.enumCase(Value, "Remark", language::Core::tooling::Diagnostic::Remark);
  }
};

} // end namespace yaml
} // end namespace toolchain

#endif // LANGUAGE_CORE_TOOLING_DIAGNOSTICSYAML_H
