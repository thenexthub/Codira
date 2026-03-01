//===-- LVSourceLanguage.cpp ----------------------------------------------===//
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
// This file implements LVSourceLanguage.
//
//===----------------------------------------------------------------------===//

#include "vm/core/DebugInfo/LogicalView/Core/LVSourceLanguage.h"
#include "vm/core/DebugInfo/CodeView/EnumTables.h"
#include "vm/core/Support/ScopedPrinter.h"

using namespace vm::core;
using namespace vm::core::logicalview;

StringRef LVSourceLanguage::getName() const {
  if (!isValid())
    return {};
  switch (getTag()) {
  case LVSourceLanguage::TagDwarf:
    return toolchain::dwarf::LanguageString(getLang());
  case LVSourceLanguage::TagCodeView: {
    static auto LangNames = toolchain::codeview::getSourceLanguageNames();
    return LangNames[getLang()].Name;
  }
  default:
    llvm_unreachable("Unsupported language");
  }
}
