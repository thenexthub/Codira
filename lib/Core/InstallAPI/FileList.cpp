//===- FileList.cpp ---------------------------------------------*- C++ -*-===//
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

#include "language/Core/InstallAPI/FileList.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/JSON.h"
#include "toolchain/TextAPI/TextAPIError.h"
#include <optional>

// clang-format off
/*
InstallAPI JSON Input Format specification.

{
  "headers" : [                              # Required: Key must exist.
    {                                        # Optional: May contain 0 or more header inputs.
      "path" : "/usr/include/mach-o/dlfn.h", # Required: Path should point to destination
                                             #           location where applicable.
      "type" : "public",                     # Required: Maps to HeaderType for header.
      "language": "c++"                      # Optional: Language mode for header.
    }
  ],
  "version" : "3"                            # Required: Version 3 supports language mode
                                                         & project header input.
}
*/
// clang-format on

using namespace toolchain;
using namespace toolchain::json;
using namespace toolchain::MachO;
using namespace language::Core::installapi;

namespace {
class Implementation {
private:
  Expected<StringRef> parseString(const Object *Obj, StringRef Key,
                                  StringRef Error);
  Expected<StringRef> parsePath(const Object *Obj);
  Expected<HeaderType> parseType(const Object *Obj);
  std::optional<language::Core::Language> parseLanguage(const Object *Obj);
  Error parseHeaders(Array &Headers);

public:
  std::unique_ptr<MemoryBuffer> InputBuffer;
  language::Core::FileManager *FM;
  unsigned Version;
  HeaderSeq HeaderList;

  Error parse(StringRef Input);
};

Expected<StringRef>
Implementation::parseString(const Object *Obj, StringRef Key, StringRef Error) {
  auto Str = Obj->getString(Key);
  if (!Str)
    return make_error<StringError>(Error, inconvertibleErrorCode());
  return *Str;
}

Expected<HeaderType> Implementation::parseType(const Object *Obj) {
  auto TypeStr =
      parseString(Obj, "type", "required field 'type' not specified");
  if (!TypeStr)
    return TypeStr.takeError();

  if (*TypeStr == "public")
    return HeaderType::Public;
  else if (*TypeStr == "private")
    return HeaderType::Private;
  else if (*TypeStr == "project" && Version >= 2)
    return HeaderType::Project;

  return make_error<TextAPIError>(TextAPIErrorCode::InvalidInputFormat,
                                  "unsupported header type");
}

Expected<StringRef> Implementation::parsePath(const Object *Obj) {
  auto Path = parseString(Obj, "path", "required field 'path' not specified");
  if (!Path)
    return Path.takeError();

  return *Path;
}

std::optional<language::Core::Language>
Implementation::parseLanguage(const Object *Obj) {
  auto Language = Obj->getString("language");
  if (!Language)
    return std::nullopt;

  return StringSwitch<language::Core::Language>(*Language)
      .Case("c", language::Core::Language::C)
      .Case("c++", language::Core::Language::CXX)
      .Case("objective-c", language::Core::Language::ObjC)
      .Case("objective-c++", language::Core::Language::ObjCXX)
      .Default(language::Core::Language::Unknown);
}

Error Implementation::parseHeaders(Array &Headers) {
  for (const auto &H : Headers) {
    auto *Obj = H.getAsObject();
    if (!Obj)
      return make_error<StringError>("expect a JSON object",
                                     inconvertibleErrorCode());
    auto Type = parseType(Obj);
    if (!Type)
      return Type.takeError();
    auto Path = parsePath(Obj);
    if (!Path)
      return Path.takeError();
    auto Language = parseLanguage(Obj);

    StringRef PathStr = *Path;
    if (*Type == HeaderType::Project) {
      HeaderList.emplace_back(
          HeaderFile{PathStr, *Type, /*IncludeName=*/"", Language});
      continue;
    }

    if (FM)
      if (!FM->getOptionalFileRef(PathStr))
        return createFileError(
            PathStr, make_error_code(std::errc::no_such_file_or_directory));

    auto IncludeName = createIncludeHeaderName(PathStr);
    HeaderList.emplace_back(PathStr, *Type,
                            IncludeName.has_value() ? IncludeName.value() : "",
                            Language);
  }

  return Error::success();
}

Error Implementation::parse(StringRef Input) {
  auto Val = json::parse(Input);
  if (!Val)
    return Val.takeError();

  auto *Root = Val->getAsObject();
  if (!Root)
    return make_error<StringError>("not a JSON object",
                                   inconvertibleErrorCode());

  auto VersionStr = Root->getString("version");
  if (!VersionStr)
    return make_error<TextAPIError>(TextAPIErrorCode::InvalidInputFormat,
                                    "required field 'version' not specified");
  if (VersionStr->getAsInteger(10, Version))
    return make_error<TextAPIError>(TextAPIErrorCode::InvalidInputFormat,
                                    "invalid version number");

  if (Version < 1 || Version > 3)
    return make_error<TextAPIError>(TextAPIErrorCode::InvalidInputFormat,
                                    "unsupported version");

  // Not specifying any header files should be atypical, but valid.
  auto Headers = Root->getArray("headers");
  if (!Headers)
    return Error::success();

  Error Err = parseHeaders(*Headers);
  if (Err)
    return Err;

  return Error::success();
}
} // namespace

toolchain::Error
FileListReader::loadHeaders(std::unique_ptr<MemoryBuffer> InputBuffer,
                            HeaderSeq &Destination, language::Core::FileManager *FM) {
  Implementation Impl;
  Impl.InputBuffer = std::move(InputBuffer);
  Impl.FM = FM;

  if (toolchain::Error Err = Impl.parse(Impl.InputBuffer->getBuffer()))
    return Err;

  Destination.reserve(Destination.size() + Impl.HeaderList.size());
  toolchain::move(Impl.HeaderList, std::back_inserter(Destination));

  return Error::success();
}
