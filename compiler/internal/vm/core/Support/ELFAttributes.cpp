//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
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

#include "vm/core/Support/ELFAttributes.h"
#include "vm/core/ADT/StringRef.h"

using namespace vm::core;

StringRef ELFAttrs::attrTypeAsString(unsigned attr, TagNameMap tagNameMap,
                                     bool hasTagPrefix) {
  auto tagNameIt = find_if(
      tagNameMap, [attr](const TagNameItem item) { return item.attr == attr; });
  if (tagNameIt == tagNameMap.end())
    return "";
  StringRef tagName = tagNameIt->tagName;
  return hasTagPrefix ? tagName : tagName.drop_front(4);
}

std::optional<unsigned> ELFAttrs::attrTypeFromString(StringRef tag,
                                                     TagNameMap tagNameMap) {
  bool hasTagPrefix = tag.starts_with("Tag_");
  auto tagNameIt =
      find_if(tagNameMap, [tag, hasTagPrefix](const TagNameItem item) {
        return item.tagName.drop_front(hasTagPrefix ? 0 : 4) == tag;
      });
  if (tagNameIt == tagNameMap.end())
    return std::nullopt;
  return tagNameIt->attr;
}
