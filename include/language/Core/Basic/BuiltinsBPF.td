//===--- BuiltinsBPF.td - BPF Builtin function database ---------*- C++ -*-===//
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

include "clang/Basic/BuiltinsBase.td"

// Get record field information
def PreserveFieldInfo : TargetBuiltin {
  let Spellings = ["__builtin_preserve_field_info"];
  let Attributes = [CustomTypeChecking];
  let Prototype = "unsigned int(...)";
}

// Get BTF type id
def BtfTypeID : TargetBuiltin {
  let Spellings = ["__builtin_btf_type_id"];
  let Attributes = [CustomTypeChecking];
  let Prototype = "long unsigned int(...)";
}

// Get type information
def PreserveTypeInfo : TargetBuiltin {
  let Spellings = ["__builtin_preserve_type_info"];
  let Attributes = [CustomTypeChecking];
  let Prototype = "long unsigned int(...)";
}

// Preserve enum value
def PreserveEnumValue : TargetBuiltin {
  let Spellings = ["__builtin_preserve_enum_value"];
  let Attributes = [CustomTypeChecking];
  let Prototype = "long int(...)";
}
