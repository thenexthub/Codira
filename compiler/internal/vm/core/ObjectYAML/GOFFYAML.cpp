//===-- GOFFYAML.cpp - GOFF YAMLIO implementation ---------------*- C++ -*-===//
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
// This file defines classes for handling the YAML representation of GOFF.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ObjectYAML/GOFFYAML.h"

namespace vm::core {
namespace GOFFYAML {

Object::Object() = default;

} // namespace GOFFYAML

namespace yaml {

void MappingTraits<GOFFYAML::FileHeader>::mapping(
    IO &IO, GOFFYAML::FileHeader &FileHdr) {
  IO.mapOptional("TargetEnvironment", FileHdr.TargetEnvironment, 0);
  IO.mapOptional("TargetOperatingSystem", FileHdr.TargetOperatingSystem, 0);
  IO.mapOptional("CCSID", FileHdr.CCSID, 0);
  IO.mapOptional("CharacterSetName", FileHdr.CharacterSetName, "");
  IO.mapOptional("LanguageProductIdentifier", FileHdr.LanguageProductIdentifier,
                 "");
  IO.mapOptional("ArchitectureLevel", FileHdr.ArchitectureLevel, 1);
  IO.mapOptional("InternalCCSID", FileHdr.InternalCCSID);
  IO.mapOptional("TargetSoftwareEnvironment",
                 FileHdr.TargetSoftwareEnvironment);
}

void MappingTraits<GOFFYAML::Object>::mapping(IO &IO, GOFFYAML::Object &Obj) {
  IO.mapTag("!GOFF", true);
  IO.mapRequired("FileHeader", Obj.Header);
}

} // namespace yaml
} // namespace vm::core
