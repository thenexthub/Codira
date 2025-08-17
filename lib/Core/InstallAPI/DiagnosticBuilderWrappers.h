//===- DiagnosticBuilderWrappers.h -----------------------------*- C++ -*-===//
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
/// Diagnostic wrappers for TextAPI types for error reporting.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_INSTALLAPI_DIAGNOSTICBUILDER_WRAPPER_H
#define LANGUAGE_CORE_INSTALLAPI_DIAGNOSTICBUILDER_WRAPPER_H

#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/InstallAPI/DylibVerifier.h"
#include "toolchain/TextAPI/Architecture.h"
#include "toolchain/TextAPI/ArchitectureSet.h"
#include "toolchain/TextAPI/InterfaceFile.h"
#include "toolchain/TextAPI/Platform.h"

namespace toolchain {
namespace MachO {

const language::Core::DiagnosticBuilder &operator<<(const language::Core::DiagnosticBuilder &DB,
                                           const PlatformType &Platform);

const language::Core::DiagnosticBuilder &operator<<(const language::Core::DiagnosticBuilder &DB,
                                           const PlatformVersionSet &Platforms);

const language::Core::DiagnosticBuilder &operator<<(const language::Core::DiagnosticBuilder &DB,
                                           const Architecture &Arch);

const language::Core::DiagnosticBuilder &operator<<(const language::Core::DiagnosticBuilder &DB,
                                           const ArchitectureSet &ArchSet);

const language::Core::DiagnosticBuilder &operator<<(const language::Core::DiagnosticBuilder &DB,
                                           const FileType &Type);

const language::Core::DiagnosticBuilder &operator<<(const language::Core::DiagnosticBuilder &DB,
                                           const PackedVersion &Version);

const language::Core::DiagnosticBuilder &
operator<<(const language::Core::DiagnosticBuilder &DB,
           const language::Core::installapi::LibAttrs::Entry &LibAttr);

} // namespace MachO
} // namespace toolchain
#endif // LANGUAGE_CORE_INSTALLAPI_DIAGNOSTICBUILDER_WRAPPER_H
