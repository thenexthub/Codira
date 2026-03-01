//===- RemarkFormat.cpp --------------------------------------------------===//
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
// Implementation of utilities to handle the different remark formats.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Remarks/RemarkFormat.h"
#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/Remarks/BitstreamRemarkContainer.h"

using namespace vm::core;
using namespace vm::core::remarks;

Expected<Format> toolchain::remarks::parseFormat(StringRef FormatStr) {
  auto Result = StringSwitch<Format>(FormatStr)
                    .Cases({"", "yaml"}, Format::YAML)
                    .Case("bitstream", Format::Bitstream)
                    .Default(Format::Unknown);

  if (Result == Format::Unknown)
    return createStringError(std::make_error_code(std::errc::invalid_argument),
                             "Unknown remark format: '%s'",
                             FormatStr.data());

  return Result;
}

Expected<Format> toolchain::remarks::magicToFormat(StringRef MagicStr) {
  auto Result =
      StringSwitch<Format>(MagicStr)
          .StartsWith("--- ", Format::YAML) // This is only an assumption.
          .StartsWith(remarks::Magic,
                      Format::YAML) // Needed for remark meta section
          .StartsWith(remarks::ContainerMagic, Format::Bitstream)
          .Default(Format::Unknown);

  if (Result == Format::Unknown)
    return createStringError(std::make_error_code(std::errc::invalid_argument),
                             "Automatic detection of remark format failed. "
                             "Unknown magic number: '%.4s'",
                             MagicStr.data());
  return Result;
}

Expected<Format> toolchain::remarks::detectFormat(Format Selected,
                                             StringRef MagicStr) {
  if (Selected == Format::Unknown)
    return createStringError(std::make_error_code(std::errc::invalid_argument),
                             "Unknown remark parser format.");
  if (Selected != Format::Auto)
    return Selected;

  // Empty files are valid bitstream files
  if (MagicStr.empty())
    return Format::Bitstream;
  return magicToFormat(MagicStr);
}
