//===--- SerializedDiagnosticPrinter.h - Diagnostics serializer -*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_FRONTEND_SERIALIZEDDIAGNOSTICPRINTER_H
#define LANGUAGE_CORE_FRONTEND_SERIALIZEDDIAGNOSTICPRINTER_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/Frontend/SerializedDiagnostics.h"
#include "toolchain/Bitstream/BitstreamWriter.h"

namespace toolchain {
class raw_ostream;
}

namespace language::Core {
class DiagnosticConsumer;
class DiagnosticOptions;

namespace serialized_diags {

/// Returns a DiagnosticConsumer that serializes diagnostics to
///  a bitcode file.
///
/// The created DiagnosticConsumer is designed for quick and lightweight
/// transfer of diagnostics to the enclosing build system (e.g., an IDE).
/// This allows wrapper tools for Clang to get diagnostics from Clang
/// (via libclang) without needing to parse Clang's command line output.
///
std::unique_ptr<DiagnosticConsumer> create(StringRef OutputFile,
                                           DiagnosticOptions &DiagOpts,
                                           bool MergeChildRecords = false);

} // end serialized_diags namespace
} // end clang namespace

#endif
