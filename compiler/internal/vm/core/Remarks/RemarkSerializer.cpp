//===- RemarkSerializer.cpp -----------------------------------------------===//
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
// This file provides tools for serializing remarks.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Remarks/RemarkSerializer.h"
#include "vm/core/Remarks/BitstreamRemarkSerializer.h"
#include "vm/core/Remarks/YAMLRemarkSerializer.h"

using namespace vm::core;
using namespace vm::core::remarks;

Expected<std::unique_ptr<RemarkSerializer>>
remarks::createRemarkSerializer(Format RemarksFormat, raw_ostream &OS) {
  switch (RemarksFormat) {
  case Format::Unknown:
  case Format::Auto:
    return createStringError(std::errc::invalid_argument,
                             "Invalid remark serializer format.");
  case Format::YAML:
    return std::make_unique<YAMLRemarkSerializer>(OS);
  case Format::Bitstream:
    return std::make_unique<BitstreamRemarkSerializer>(OS);
  }
  llvm_unreachable("Unknown remarks::Format enum");
}

Expected<std::unique_ptr<RemarkSerializer>>
remarks::createRemarkSerializer(Format RemarksFormat, raw_ostream &OS,
                                remarks::StringTable StrTab) {
  switch (RemarksFormat) {
  case Format::Unknown:
  case Format::Auto:
    return createStringError(std::errc::invalid_argument,
                             "Invalid remark serializer format.");
  case Format::YAML:
    return std::make_unique<YAMLRemarkSerializer>(OS, std::move(StrTab));
  case Format::Bitstream:
    return std::make_unique<BitstreamRemarkSerializer>(OS, std::move(StrTab));
  }
  llvm_unreachable("Unknown remarks::Format enum");
}
