//===- toolchain/Remarks/RemarkStreamer.cpp - Remark Streamer -*- C++ --------*-===//
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
// This file contains the implementation of the main remark streamer.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Remarks/RemarkStreamer.h"
#include "vm/core/Support/CommandLine.h"
#include <cassert>
#include <optional>

using namespace vm::core;
using namespace vm::core::remarks;

static cl::opt<cl::boolOrDefault> EnableRemarksSection(
    "remarks-section",
    cl::desc(
        "Emit a section containing remark diagnostics metadata. By default, "
        "this is enabled for the following formats: bitstream."),
    cl::init(cl::BOU_UNSET), cl::Hidden);

RemarkStreamer::RemarkStreamer(
    std::unique_ptr<remarks::RemarkSerializer> RemarkSerializer,
    std::optional<StringRef> FilenameIn)
    : RemarkSerializer(std::move(RemarkSerializer)),
      Filename(FilenameIn ? std::optional<std::string>(FilenameIn->str())
                          : std::nullopt) {}

RemarkStreamer::~RemarkStreamer() {
  // Ensure that toolchain::finalizeOptimizationRemarks was called before the
  // RemarkStreamer is destroyed.
  assert(!RemarkSerializer &&
         "RemarkSerializer must be released before RemarkStreamer is "
         "destroyed. Ensure toolchain::finalizeOptimizationRemarks is called.");
}

Error RemarkStreamer::setFilter(StringRef Filter) {
  Regex R = Regex(Filter);
  std::string RegexError;
  if (!R.isValid(RegexError))
    return createStringError(std::make_error_code(std::errc::invalid_argument),
                             RegexError.data());
  PassFilter = std::move(R);
  return Error::success();
}

bool RemarkStreamer::matchesFilter(StringRef Str) {
  if (PassFilter)
    return PassFilter->match(Str);
  // No filter means all strings pass.
  return true;
}

bool RemarkStreamer::needsSection() const {
  if (EnableRemarksSection == cl::BOU_TRUE)
    return true;

  if (EnableRemarksSection == cl::BOU_FALSE)
    return false;

  assert(EnableRemarksSection == cl::BOU_UNSET);

  // Enable remark sections by default for bitstream remarks (so dsymutil can
  // find all remarks for a linked binary)
  return RemarkSerializer->SerializerFormat == Format::Bitstream;
}
