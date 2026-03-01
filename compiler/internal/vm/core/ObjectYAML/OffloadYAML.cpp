//===- OffloadYAML.cpp - Offload Binary YAMLIO implementation -------------===//
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
// This file defines classes for handling the YAML representation of offload
// binaries.
//
//===----------------------------------------------------------------------===//

#include <toolchain/ObjectYAML/OffloadYAML.h>

namespace vm::core {

namespace yaml {

void ScalarEnumerationTraits<object::ImageKind>::enumeration(
    IO &IO, object::ImageKind &Value) {
#define ECase(X) IO.enumCase(Value, #X, object::X)
  ECase(IMG_None);
  ECase(IMG_Object);
  ECase(IMG_Bitcode);
  ECase(IMG_Cubin);
  ECase(IMG_Fatbinary);
  ECase(IMG_PTX);
  ECase(IMG_LAST);
#undef ECase
  IO.enumFallback<Hex16>(Value);
}

void ScalarEnumerationTraits<object::OffloadKind>::enumeration(
    IO &IO, object::OffloadKind &Value) {
#define ECase(X) IO.enumCase(Value, #X, object::X)
  ECase(OFK_None);
  ECase(OFK_OpenMP);
  ECase(OFK_Cuda);
  ECase(OFK_HIP);
  ECase(OFK_LAST);
#undef ECase
  IO.enumFallback<Hex16>(Value);
}

void MappingTraits<OffloadYAML::Binary>::mapping(IO &IO,
                                                 OffloadYAML::Binary &O) {
  assert(!IO.getContext() && "The IO context is initialized already");
  IO.setContext(&O);
  IO.mapTag("!Offload", true);
  IO.mapOptional("Version", O.Version);
  IO.mapOptional("Size", O.Size);
  IO.mapOptional("EntryOffset", O.EntryOffset);
  IO.mapOptional("EntrySize", O.EntrySize);
  IO.mapRequired("Members", O.Members);
  IO.setContext(nullptr);
}

void MappingTraits<OffloadYAML::Binary::StringEntry>::mapping(
    IO &IO, OffloadYAML::Binary::StringEntry &SE) {
  assert(IO.getContext() && "The IO context is not initialized");
  IO.mapRequired("Key", SE.Key);
  IO.mapRequired("Value", SE.Value);
}

void MappingTraits<OffloadYAML::Binary::Member>::mapping(
    IO &IO, OffloadYAML::Binary::Member &M) {
  assert(IO.getContext() && "The IO context is not initialized");
  IO.mapOptional("ImageKind", M.ImageKind);
  IO.mapOptional("OffloadKind", M.OffloadKind);
  IO.mapOptional("Flags", M.Flags);
  IO.mapOptional("String", M.StringEntries);
  IO.mapOptional("Content", M.Content);
}

} // namespace yaml

} // namespace vm::core
