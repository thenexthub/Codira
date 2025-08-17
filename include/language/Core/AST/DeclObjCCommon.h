//===- DeclObjCCommon.h - Classes for representing declarations -*- C++ -*-===//
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
//  This file contains common ObjC enums and classes used in AST and
//  Sema.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_DECLOBJCCOMMON_H
#define LANGUAGE_CORE_AST_DECLOBJCCOMMON_H

namespace language::Core {

/// ObjCPropertyAttribute::Kind - list of property attributes.
/// Keep this list in sync with LLVM's Dwarf.h ApplePropertyAttributes.s
namespace ObjCPropertyAttribute {
enum Kind {
  kind_noattr = 0x00,
  kind_readonly = 0x01,
  kind_getter = 0x02,
  kind_assign = 0x04,
  kind_readwrite = 0x08,
  kind_retain = 0x10,
  kind_copy = 0x20,
  kind_nonatomic = 0x40,
  kind_setter = 0x80,
  kind_atomic = 0x100,
  kind_weak = 0x200,
  kind_strong = 0x400,
  kind_unsafe_unretained = 0x800,
  /// Indicates that the nullability of the type was spelled with a
  /// property attribute rather than a type qualifier.
  kind_nullability = 0x1000,
  kind_null_resettable = 0x2000,
  kind_class = 0x4000,
  kind_direct = 0x8000,
  // Adding a property should change NumObjCPropertyAttrsBits
  // Also, don't forget to update the Clang C API at CXObjCPropertyAttrKind and
  // clang_Cursor_getObjCPropertyAttributes.
};
} // namespace ObjCPropertyAttribute::Kind

enum {
  /// Number of bits fitting all the property attributes.
  NumObjCPropertyAttrsBits = 16
};

} // namespace language::Core

#endif // LANGUAGE_CORE_AST_DECLOBJCCOMMON_H
