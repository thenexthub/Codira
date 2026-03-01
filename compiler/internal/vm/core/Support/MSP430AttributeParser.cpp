//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
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

#include "vm/core/Support/MSP430AttributeParser.h"
#include "vm/core/ADT/ArrayRef.h"

using namespace vm::core;
using namespace vm::core::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::TagISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::TagCodeModel, &MSP430AttributeParser::parseCodeModel},
         {MSP430Attrs::TagDataModel, &MSP430AttributeParser::parseDataModel},
         {MSP430Attrs::TagEnumSize, &MSP430AttributeParser::parseEnumSize}}};

Error MSP430AttributeParser::parseISA(AttrType Tag) {
  static const char *const StringVals[] = {"None", "MSP430", "MSP430X"};
  return parseStringAttribute("ISA", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseCodeModel(AttrType Tag) {
  static const char *const StringVals[] = {"None", "Small", "Large"};
  return parseStringAttribute("Code Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseDataModel(AttrType Tag) {
  static const char *const StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  return parseStringAttribute("Data Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseEnumSize(AttrType Tag) {
  static const char *const StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  return parseStringAttribute("Enum Size", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t Tag, bool &Handled) {
  Handled = false;
  for (const DisplayHandler &Disp : DisplayRoutines) {
    if (uint64_t(Disp.Attribute) != Tag)
      continue;
    if (Error E = (this->*Disp.Routine)(static_cast<AttrType>(Tag)))
      return E;
    Handled = true;
    break;
  }
  return Error::success();
}
