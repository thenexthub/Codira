//===- ArchiveYAML.cpp - ELF YAMLIO implementation -------------------- ----===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// This file defines classes for handling the YAML representation of archives.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ObjectYAML/ArchiveYAML.h"

namespace vm::core {

namespace yaml {

void MappingTraits<ArchYAML::Archive>::mapping(IO &IO, ArchYAML::Archive &A) {
  assert(!IO.getContext() && "The IO context is initialized already");
  IO.setContext(&A);
  IO.mapTag("!Arch", true);
  IO.mapOptional("Magic", A.Magic, "!<arch>\n");
  IO.mapOptional("Members", A.Members);
  IO.mapOptional("Content", A.Content);
  IO.setContext(nullptr);
}

std::string MappingTraits<ArchYAML::Archive>::validate(IO &,
                                                       ArchYAML::Archive &A) {
  if (A.Members && A.Content)
    return "\"Content\" and \"Members\" cannot be used together";
  return "";
}

void MappingTraits<ArchYAML::Archive::Child>::mapping(
    IO &IO, ArchYAML::Archive::Child &E) {
  assert(IO.getContext() && "The IO context is not initialized");
  for (auto &P : E.Fields)
    IO.mapOptional(P.first.data(), P.second.Value, P.second.DefaultValue);
  IO.mapOptional("Content", E.Content);
  IO.mapOptional("PaddingByte", E.PaddingByte);
}

std::string
MappingTraits<ArchYAML::Archive::Child>::validate(IO &,
                                                  ArchYAML::Archive::Child &C) {
  for (auto &P : C.Fields)
    if (P.second.Value.size() > P.second.MaxLength)
      return ("the maximum length of \"" + P.first + "\" field is " +
              Twine(P.second.MaxLength))
          .str();
  return "";
}

} // end namespace yaml

} // end namespace vm::core
