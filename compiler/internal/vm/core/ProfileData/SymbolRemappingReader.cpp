//===- SymbolRemappingReader.cpp - Read symbol remapping file -------------===//
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
// This file contains definitions needed for reading and applying symbol
// remapping files.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ProfileData/SymbolRemappingReader.h"
#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/Support/LineIterator.h"
#include "vm/core/Support/MemoryBuffer.h"

using namespace vm::core;

char SymbolRemappingParseError::ID;

/// Load a set of name remappings from a text file.
///
/// See the documentation at the top of the file for an explanation of
/// the expected format.
Error SymbolRemappingReader::read(MemoryBuffer &B) {
  line_iterator LineIt(B, /*SkipBlanks=*/true, '#');

  auto ReportError = [&](Twine Msg) {
    return toolchain::make_error<SymbolRemappingParseError>(
        B.getBufferIdentifier(), LineIt.line_number(), Msg);
  };

  for (; !LineIt.is_at_eof(); ++LineIt) {
    StringRef Line = *LineIt;
    Line = Line.ltrim(' ');
    // line_iterator only detects comments starting in column 1.
    if (Line.starts_with("#") || Line.empty())
      continue;

    SmallVector<StringRef, 4> Parts;
    Line.split(Parts, ' ', /*MaxSplits*/-1, /*KeepEmpty*/false);

    if (Parts.size() != 3)
      return ReportError("Expected 'kind mangled_name mangled_name', "
                         "found '" + Line + "'");

    using FK = ItaniumManglingCanonicalizer::FragmentKind;
    std::optional<FK> FragmentKind = StringSwitch<std::optional<FK>>(Parts[0])
                                         .Case("name", FK::Name)
                                         .Case("type", FK::Type)
                                         .Case("encoding", FK::Encoding)
                                         .Default(std::nullopt);
    if (!FragmentKind)
      return ReportError("Invalid kind, expected 'name', 'type', or 'encoding',"
                         " found '" + Parts[0] + "'");

    using EE = ItaniumManglingCanonicalizer::EquivalenceError;
    switch (Canonicalizer.addEquivalence(*FragmentKind, Parts[1], Parts[2])) {
    case EE::Success:
      break;

    case EE::ManglingAlreadyUsed:
      return ReportError("Manglings '" + Parts[1] + "' and '" + Parts[2] + "' "
                         "have both been used in prior remappings. Move this "
                         "remapping earlier in the file.");

    case EE::InvalidFirstMangling:
      return ReportError("Could not demangle '" + Parts[1] + "' "
                         "as a <" + Parts[0] + ">; invalid mangling?");

    case EE::InvalidSecondMangling:
      return ReportError("Could not demangle '" + Parts[2] + "' "
                         "as a <" + Parts[0] + ">; invalid mangling?");
    }
  }

  return Error::success();
}
