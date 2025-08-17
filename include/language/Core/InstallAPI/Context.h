//===- InstallAPI/Context.h -------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_INSTALLAPI_CONTEXT_H
#define LANGUAGE_CORE_INSTALLAPI_CONTEXT_H

#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/FileManager.h"
#include "language/Core/InstallAPI/DylibVerifier.h"
#include "language/Core/InstallAPI/HeaderFile.h"
#include "language/Core/InstallAPI/MachO.h"
#include "toolchain/ADT/DenseMap.h"

namespace language::Core {
namespace installapi {
class FrontendRecordsSlice;

/// Struct used for generating validating InstallAPI.
/// The attributes captured represent all necessary information
/// to generate TextAPI output.
struct InstallAPIContext {

  /// Library attributes that are typically passed as linker inputs.
  BinaryAttrs BA;

  /// Install names of reexported libraries of a library.
  LibAttrs Reexports;

  /// All headers that represent a library.
  HeaderSeq InputHeaders;

  /// Active language mode to parse in.
  Language LangMode = Language::ObjC;

  /// Active header access type.
  HeaderType Type = HeaderType::Unknown;

  /// Active TargetSlice for symbol record collection.
  std::shared_ptr<FrontendRecordsSlice> Slice;

  /// FileManager for all I/O operations.
  FileManager *FM = nullptr;

  /// DiagnosticsEngine for all error reporting.
  DiagnosticsEngine *Diags = nullptr;

  /// Verifier when binary dylib is passed as input.
  std::unique_ptr<DylibVerifier> Verifier = nullptr;

  /// File Path of output location.
  toolchain::StringRef OutputLoc{};

  /// What encoding to write output as.
  FileType FT = FileType::TBD_V5;

  /// Populate entries of headers that should be included for TextAPI
  /// generation.
  void addKnownHeader(const HeaderFile &H);

  /// Record visited files during frontend actions to determine whether to
  /// include their declarations for TextAPI generation.
  ///
  /// \param FE Header that is being parsed.
  /// \param PP Preprocesser used for querying how header was imported.
  /// \return Access level of header if it should be included for TextAPI
  /// generation.
  std::optional<HeaderType> findAndRecordFile(const FileEntry *FE,
                                              const Preprocessor &PP);

private:
  using HeaderMap = toolchain::DenseMap<const FileEntry *, HeaderType>;

  // Collection of parsed header files and their access level. If set to
  // HeaderType::Unknown, they are not used for TextAPI generation.
  HeaderMap KnownFiles;

  // Collection of expected header includes and the access level for them.
  toolchain::DenseMap<StringRef, HeaderType> KnownIncludes;
};

/// Lookup the dylib or TextAPI file location for a system library or framework.
/// The search paths provided are searched in order.
/// @rpath based libraries are not supported.
///
/// \param InstallName The install name for the library.
/// \param FrameworkSearchPaths Search paths to look up frameworks with.
/// \param LibrarySearchPaths Search paths to look up dylibs with.
/// \param SearchPaths Fallback search paths if library was not found in earlier
/// paths.
/// \return The full path of the library.
std::string findLibrary(StringRef InstallName, FileManager &FM,
                        ArrayRef<std::string> FrameworkSearchPaths,
                        ArrayRef<std::string> LibrarySearchPaths,
                        ArrayRef<std::string> SearchPaths);
} // namespace installapi
} // namespace language::Core

#endif // LANGUAGE_CORE_INSTALLAPI_CONTEXT_H
