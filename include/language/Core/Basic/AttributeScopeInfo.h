//==- AttributeScopeInfo.h - Base info about an Attribute Scope --*- C++ -*-==//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
// This file defines the AttributeScopeInfo type, which represents information
// about the scope of an attribute.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_ATTRIBUTESCOPEINFO_H
#define LANGUAGE_CORE_BASIC_ATTRIBUTESCOPEINFO_H

#include "language/Core/Basic/SourceLocation.h"

namespace language::Core {

class IdentifierInfo;

class AttributeScopeInfo {
public:
  AttributeScopeInfo() = default;

  AttributeScopeInfo(const IdentifierInfo *Name, SourceLocation NameLoc)
      : Name(Name), NameLoc(NameLoc) {}

  AttributeScopeInfo(const IdentifierInfo *Name, SourceLocation NameLoc,
                     SourceLocation CommonScopeLoc)
      : Name(Name), NameLoc(NameLoc), CommonScopeLoc(CommonScopeLoc) {}

  const IdentifierInfo *getName() const { return Name; }
  SourceLocation getNameLoc() const { return NameLoc; }

  bool isValid() const { return Name != nullptr; }
  bool isExplicit() const { return CommonScopeLoc.isInvalid(); }

private:
  const IdentifierInfo *Name = nullptr;
  SourceLocation NameLoc;
  SourceLocation CommonScopeLoc;
};

} // namespace language::Core

#endif // LANGUAGE_CORE_BASIC_ATTRIBUTESCOPEINFO_H
