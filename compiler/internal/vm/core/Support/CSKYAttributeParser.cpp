//===-- CSKYAttributeParser.cpp - CSKY Attribute Parser -----------------===//
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

#include "vm/core/Support/CSKYAttributeParser.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/Support/Errc.h"

using namespace vm::core;

const CSKYAttributeParser::DisplayHandler
    CSKYAttributeParser::displayRoutines[] = {
        {
            CSKYAttrs::CSKY_ARCH_NAME,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_CPU_NAME,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_ISA_FLAGS,
            &ELFCompactAttrParser::integerAttribute,
        },
        {
            CSKYAttrs::CSKY_ISA_EXT_FLAGS,
            &ELFCompactAttrParser::integerAttribute,
        },
        {
            CSKYAttrs::CSKY_DSP_VERSION,
            &CSKYAttributeParser::dspVersion,
        },
        {
            CSKYAttrs::CSKY_VDSP_VERSION,
            &CSKYAttributeParser::vdspVersion,
        },
        {
            CSKYAttrs::CSKY_FPU_VERSION,
            &CSKYAttributeParser::fpuVersion,
        },
        {
            CSKYAttrs::CSKY_FPU_ABI,
            &CSKYAttributeParser::fpuABI,
        },
        {
            CSKYAttrs::CSKY_FPU_ROUNDING,
            &CSKYAttributeParser::fpuRounding,
        },
        {
            CSKYAttrs::CSKY_FPU_DENORMAL,
            &CSKYAttributeParser::fpuDenormal,
        },
        {
            CSKYAttrs::CSKY_FPU_EXCEPTION,
            &CSKYAttributeParser::fpuException,
        },
        {
            CSKYAttrs::CSKY_FPU_NUMBER_MODULE,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_FPU_HARDFP,
            &CSKYAttributeParser::fpuHardFP,
        }};

Error CSKYAttributeParser::handler(uint64_t tag, bool &handled) {
  handled = false;
  for (const auto &AH : displayRoutines) {
    if (uint64_t(AH.attribute) == tag) {
      if (Error e = (this->*AH.routine)(tag))
        return e;
      handled = true;
      break;
    }
  }

  return Error::success();
}

Error CSKYAttributeParser::dspVersion(unsigned tag) {
  static const char *const strings[] = {"Error", "DSP Extension", "DSP 2.0"};
  return parseStringAttribute("Tag_CSKY_DSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::vdspVersion(unsigned tag) {
  static const char *const strings[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  return parseStringAttribute("Tag_CSKY_VDSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuVersion(unsigned tag) {
  static const char *const strings[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  return parseStringAttribute("Tag_CSKY_FPU_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuABI(unsigned tag) {
  static const char *const strings[] = {"Error", "Soft", "SoftFP", "Hard"};
  return parseStringAttribute("Tag_CSKY_FPU_ABI", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuRounding(unsigned tag) {
  static const char *const strings[] = {"None", "Needed"};
  return parseStringAttribute("Tag_CSKY_FPU_ROUNDING", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuDenormal(unsigned tag) {
  static const char *const strings[] = {"None", "Needed"};
  return parseStringAttribute("Tag_CSKY_FPU_DENORMAL", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuException(unsigned tag) {
  static const char *const strings[] = {"None", "Needed"};
  return parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuHardFP(unsigned tag) {
  uint64_t value = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  if (value & 0x1) {
    description += LS;
    description += "Half";
  }
  if ((value >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  if ((value >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  if (description.empty()) {
    printAttribute(tag, value, "");
    return createStringError(errc::invalid_argument,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(value));
  }

  printAttribute(tag, value, description);
  return Error::success();
}
