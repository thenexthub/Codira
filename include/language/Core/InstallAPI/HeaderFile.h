//===- InstallAPI/HeaderFile.h ----------------------------------*- C++ -*-===//
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
/// Representations of a library's headers for InstallAPI.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_INSTALLAPI_HEADERFILE_H
#define LANGUAGE_CORE_INSTALLAPI_HEADERFILE_H

#include "language/Core/Basic/FileManager.h"
#include "language/Core/Basic/LangStandard.h"
#include "language/Core/InstallAPI/MachO.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/Regex.h"
#include <optional>
#include <string>

namespace language::Core::installapi {
enum class HeaderType {
  /// Represents declarations accessible to all clients.
  Public,
  /// Represents declarations accessible to a disclosed set of clients.
  Private,
  /// Represents declarations only accessible as implementation details to the
  /// input library.
  Project,
  /// Unset or unknown type.
  Unknown,
};

inline StringRef getName(const HeaderType T) {
  switch (T) {
  case HeaderType::Public:
    return "Public";
  case HeaderType::Private:
    return "Private";
  case HeaderType::Project:
    return "Project";
  case HeaderType::Unknown:
    return "Unknown";
  }
  toolchain_unreachable("unexpected header type");
}

class HeaderFile {
  /// Full input path to header.
  std::string FullPath;
  /// Access level of header.
  HeaderType Type;
  /// Expected way header will be included by clients.
  std::string IncludeName;
  /// Supported language mode for header.
  std::optional<language::Core::Language> Language;
  /// Exclude header file from processing.
  bool Excluded{false};
  /// Add header file to processing.
  bool Extra{false};
  /// Specify that header file is the umbrella header for library.
  bool Umbrella{false};

public:
  HeaderFile() = delete;
  HeaderFile(StringRef FullPath, HeaderType Type,
             StringRef IncludeName = StringRef(),
             std::optional<language::Core::Language> Language = std::nullopt)
      : FullPath(FullPath), Type(Type), IncludeName(IncludeName),
        Language(Language) {}

  static toolchain::Regex getFrameworkIncludeRule();

  HeaderType getType() const { return Type; }
  StringRef getIncludeName() const { return IncludeName; }
  StringRef getPath() const { return FullPath; }

  void setExtra(bool V = true) { Extra = V; }
  void setExcluded(bool V = true) { Excluded = V; }
  void setUmbrellaHeader(bool V = true) { Umbrella = V; }
  bool isExtra() const { return Extra; }
  bool isExcluded() const { return Excluded; }
  bool isUmbrellaHeader() const { return Umbrella; }

  bool useIncludeName() const {
    return Type != HeaderType::Project && !IncludeName.empty();
  }

  bool operator==(const HeaderFile &Other) const {
    return std::tie(Type, FullPath, IncludeName, Language, Excluded, Extra,
                    Umbrella) == std::tie(Other.Type, Other.FullPath,
                                          Other.IncludeName, Other.Language,
                                          Other.Excluded, Other.Extra,
                                          Other.Umbrella);
  }

  bool operator<(const HeaderFile &Other) const {
    /// For parsing of headers based on ordering,
    /// group by type, then whether its an umbrella.
    /// Capture 'extra' headers last.
    /// This optimizes the chance of a sucessful parse for
    /// headers that violate IWYU.
    if (isExtra() && Other.isExtra())
      return std::tie(Type, Umbrella) < std::tie(Other.Type, Other.Umbrella);

    return std::tie(Type, Umbrella, Extra, FullPath) <
           std::tie(Other.Type, Other.Umbrella, Other.Extra, Other.FullPath);
  }
};

/// Glob that represents a pattern of header files to retreive.
class HeaderGlob {
private:
  std::string GlobString;
  toolchain::Regex Rule;
  HeaderType Type;
  bool FoundMatch{false};

public:
  HeaderGlob(StringRef GlobString, toolchain::Regex &&, HeaderType Type);

  /// Create a header glob from string for the header access level.
  static toolchain::Expected<std::unique_ptr<HeaderGlob>>
  create(StringRef GlobString, HeaderType Type);

  /// Query if provided header matches glob.
  bool match(const HeaderFile &Header);

  /// Query if a header was matched in the glob, used primarily for error
  /// reporting.
  bool didMatch() { return FoundMatch; }

  /// Provide back input glob string.
  StringRef str() { return GlobString; }
};

/// Assemble expected way header will be included by clients.
/// As in what maps inside the brackets of `#include <IncludeName.h>`
/// For example,
/// "/System/Library/Frameworks/Foo.framework/Headers/Foo.h" returns
/// "Foo/Foo.h"
///
/// \param FullPath Path to the header file which includes the library
/// structure.
std::optional<std::string> createIncludeHeaderName(const StringRef FullPath);
using HeaderSeq = std::vector<HeaderFile>;

/// Determine if Path is a header file.
/// It does not touch the file system.
///
/// \param  Path File path to file.
bool isHeaderFile(StringRef Path);

/// Given input directory, collect all header files.
///
/// \param FM FileManager for finding input files.
/// \param Directory Path to directory file.
toolchain::Expected<PathSeq> enumerateFiles(language::Core::FileManager &FM,
                                       StringRef Directory);

} // namespace language::Core::installapi

#endif // LANGUAGE_CORE_INSTALLAPI_HEADERFILE_H
