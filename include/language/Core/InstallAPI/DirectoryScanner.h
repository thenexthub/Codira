//===- InstallAPI/DirectoryScanner.h ----------------------------*- C++ -*-===//
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
/// The DirectoryScanner for collecting library files on the file system.
///
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_INSTALLAPI_DIRECTORYSCANNER_H
#define LANGUAGE_CORE_INSTALLAPI_DIRECTORYSCANNER_H

#include "language/Core/Basic/FileManager.h"
#include "language/Core/InstallAPI/Library.h"

namespace language::Core::installapi {

enum ScanMode {
  /// Scanning Framework directory.
  ScanFrameworks,
  /// Scanning Dylib directory.
  ScanDylibs,
};

class DirectoryScanner {
public:
  DirectoryScanner(FileManager &FM, ScanMode Mode = ScanMode::ScanFrameworks)
      : FM(FM), Mode(Mode) {}

  /// Scan for all input files throughout directory.
  ///
  /// \param Directory Path of input directory.
  toolchain::Error scan(StringRef Directory);

  /// Take over ownership of stored libraries.
  std::vector<Library> takeLibraries() { return std::move(Libraries); };

  /// Get all the header files in libraries.
  ///
  /// \param Libraries Reference of collection of libraries.
  static HeaderSeq getHeaders(ArrayRef<Library> Libraries);

private:
  /// Collect files for dylibs in usr/(local)/lib within directory.
  toolchain::Error scanForUnwrappedLibraries(StringRef Directory);

  /// Collect files for any frameworks within directory.
  toolchain::Error scanForFrameworks(StringRef Directory);

  /// Get a library from the libraries collection.
  Library &getOrCreateLibrary(StringRef Path, std::vector<Library> &Libs) const;

  /// Collect multiple frameworks from directory.
  toolchain::Error scanMultipleFrameworks(StringRef Directory,
                                     std::vector<Library> &Libs) const;
  /// Collect files from nested frameworks.
  toolchain::Error scanSubFrameworksDirectory(StringRef Directory,
                                         std::vector<Library> &Libs) const;

  /// Collect files from framework path.
  toolchain::Error scanFrameworkDirectory(StringRef Path, Library &Framework) const;

  /// Collect header files from path.
  toolchain::Error scanHeaders(StringRef Path, Library &Lib, HeaderType Type,
                          StringRef BasePath,
                          StringRef ParentPath = StringRef()) const;

  /// Collect files from Version directories inside Framework directories.
  toolchain::Error scanFrameworkVersionsDirectory(StringRef Path,
                                             Library &Lib) const;
  FileManager &FM;
  ScanMode Mode;
  StringRef RootPath;
  std::vector<Library> Libraries;
};

} // namespace language::Core::installapi

#endif // LANGUAGE_CORE_INSTALLAPI_DIRECTORYSCANNER_H
