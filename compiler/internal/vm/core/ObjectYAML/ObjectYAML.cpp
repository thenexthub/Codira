//===- ObjectYAML.cpp - YAML utilities for object files -------------------===//
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
// This file defines a wrapper class for handling tagged YAML input
//
//===----------------------------------------------------------------------===//

#include "vm/core/ObjectYAML/ObjectYAML.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/Support/YAMLParser.h"
#include "vm/core/Support/YAMLTraits.h"
#include <string>

using namespace vm::core;
using namespace yaml;

void MappingTraits<YamlObjectFile>::mapping(IO &IO,
                                            YamlObjectFile &ObjectFile) {
  if (IO.outputting()) {
    if (ObjectFile.Elf)
      MappingTraits<ELFYAML::Object>::mapping(IO, *ObjectFile.Elf);
    if (ObjectFile.Coff)
      MappingTraits<COFFYAML::Object>::mapping(IO, *ObjectFile.Coff);
    if (ObjectFile.Goff)
      MappingTraits<GOFFYAML::Object>::mapping(IO, *ObjectFile.Goff);
    if (ObjectFile.MachO)
      MappingTraits<MachOYAML::Object>::mapping(IO, *ObjectFile.MachO);
    if (ObjectFile.FatMachO)
      MappingTraits<MachOYAML::UniversalBinary>::mapping(IO,
                                                         *ObjectFile.FatMachO);
  } else {
    Input &In = (Input &)IO;
    if (IO.mapTag("!Arch")) {
      ObjectFile.Arch.reset(new ArchYAML::Archive());
      MappingTraits<ArchYAML::Archive>::mapping(IO, *ObjectFile.Arch);
      std::string Err =
          MappingTraits<ArchYAML::Archive>::validate(IO, *ObjectFile.Arch);
      if (!Err.empty())
        IO.setError(Err);
    } else if (IO.mapTag("!ELF")) {
      ObjectFile.Elf.reset(new ELFYAML::Object());
      MappingTraits<ELFYAML::Object>::mapping(IO, *ObjectFile.Elf);
    } else if (IO.mapTag("!COFF")) {
      ObjectFile.Coff.reset(new COFFYAML::Object());
      MappingTraits<COFFYAML::Object>::mapping(IO, *ObjectFile.Coff);
    } else if (IO.mapTag("!GOFF")) {
      ObjectFile.Goff.reset(new GOFFYAML::Object());
      MappingTraits<GOFFYAML::Object>::mapping(IO, *ObjectFile.Goff);
    } else if (IO.mapTag("!mach-o")) {
      ObjectFile.MachO.reset(new MachOYAML::Object());
      MappingTraits<MachOYAML::Object>::mapping(IO, *ObjectFile.MachO);
    } else if (IO.mapTag("!fat-mach-o")) {
      ObjectFile.FatMachO.reset(new MachOYAML::UniversalBinary());
      MappingTraits<MachOYAML::UniversalBinary>::mapping(IO,
                                                         *ObjectFile.FatMachO);
    } else if (IO.mapTag("!minidump")) {
      ObjectFile.Minidump.reset(new MinidumpYAML::Object());
      MappingTraits<MinidumpYAML::Object>::mapping(IO, *ObjectFile.Minidump);
    } else if (IO.mapTag("!Offload")) {
      ObjectFile.Offload.reset(new OffloadYAML::Binary());
      MappingTraits<OffloadYAML::Binary>::mapping(IO, *ObjectFile.Offload);
    } else if (IO.mapTag("!WASM")) {
      ObjectFile.Wasm.reset(new WasmYAML::Object());
      MappingTraits<WasmYAML::Object>::mapping(IO, *ObjectFile.Wasm);
    } else if (IO.mapTag("!XCOFF")) {
      ObjectFile.Xcoff.reset(new XCOFFYAML::Object());
      MappingTraits<XCOFFYAML::Object>::mapping(IO, *ObjectFile.Xcoff);
    } else if (IO.mapTag("!dxcontainer")) {
      ObjectFile.DXContainer.reset(new DXContainerYAML::Object());
      MappingTraits<DXContainerYAML::Object>::mapping(IO,
                                                      *ObjectFile.DXContainer);
    } else if (const Node *N = In.getCurrentNode()) {
      if (N->getRawTag().empty())
        IO.setError("YAML Object File missing document type tag!");
      else
        IO.setError("YAML Object File unsupported document type tag '" +
                    N->getRawTag() + "'!");
    }
  }
}
