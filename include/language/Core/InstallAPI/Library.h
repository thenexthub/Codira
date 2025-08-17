//===- InstallAPI/Library.h -------------------------------------*- C++ -*-===//
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
/// Defines the content of a library, such as public and private
/// header files, and whether it is a framework.
///
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_INSTALLAPI_LIBRARY_H
#define LANGUAGE_CORE_INSTALLAPI_LIBRARY_H

#include "language/Core/InstallAPI/HeaderFile.h"
#include "language/Core/InstallAPI/MachO.h"

namespace language::Core::installapi {

class Library {
public:
  Library(StringRef Directory) : BaseDirectory(Directory) {}

  /// Capture the name of the framework by the install name.
  ///
  /// \param InstallName The install name of the library encoded in a dynamic
  /// library.
  static StringRef getFrameworkNameFromInstallName(StringRef InstallName);

  /// Get name of library by the discovered file path.
  StringRef getName() const;

  /// Get discovered path of library.
  StringRef getPath() const { return BaseDirectory; }

  /// Add a header file that belongs to the library.
  ///
  /// \param FullPath Path to header file.
  /// \param Type Access level of header.
  /// \param IncludePath The way the header should be included.
  void addHeaderFile(StringRef FullPath, HeaderType Type,
                     StringRef IncludePath = StringRef()) {
    Headers.emplace_back(FullPath, Type, IncludePath);
  }

  /// Determine if library is empty.
  bool empty() {
    return SubFrameworks.empty() && Headers.empty() &&
           FrameworkVersions.empty();
  }

private:
  std::string BaseDirectory;
  HeaderSeq Headers;
  std::vector<Library> SubFrameworks;
  std::vector<Library> FrameworkVersions;
  bool IsUnwrappedDylib{false};

  friend class DirectoryScanner;
};

} // namespace language::Core::installapi

#endif // LANGUAGE_CORE_INSTALLAPI_LIBRARY_H
