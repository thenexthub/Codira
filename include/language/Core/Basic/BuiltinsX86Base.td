//===--- BuiltinsX86Base.td - X86 Builtin function classes ------*- C++ -*-===//
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
// This file defines the X86-specific builtin function classes.
//
//===----------------------------------------------------------------------===//

include "clang/Basic/BuiltinsBase.td"

def X86Prefix : NamePrefix<"__builtin_ia32_">;

class X86Builtin<string prototype> : TargetBuiltin {
  let Spellings = [NAME];
  let Prototype = prototype;
  let EnableOpenCLLong = 1;
  let RequiredNamePrefix = X86Prefix; // Adds a prefix to the name.
}

class X86NoPrefixBuiltin<string prototype> : TargetBuiltin {
  let Spellings = [NAME];
  let Prototype = prototype;
}

class X86LibBuiltin<string prototype> : TargetLibBuiltin {
  let Spellings = [NAME];
  let Prototype = prototype;
}
