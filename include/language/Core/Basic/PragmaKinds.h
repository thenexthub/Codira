//===--- PragmaKinds.h - #pragma comment() kinds  ---------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_BASIC_PRAGMAKINDS_H
#define LANGUAGE_CORE_BASIC_PRAGMAKINDS_H

namespace language::Core {

enum PragmaMSCommentKind {
  PCK_Unknown,
  PCK_Linker,   // #pragma comment(linker, ...)
  PCK_Lib,      // #pragma comment(lib, ...)
  PCK_Compiler, // #pragma comment(compiler, ...)
  PCK_ExeStr,   // #pragma comment(exestr, ...)
  PCK_User      // #pragma comment(user, ...)
};

enum PragmaMSStructKind {
  PMSST_OFF, // #pragms ms_struct off
  PMSST_ON   // #pragms ms_struct on
};

enum PragmaFloatControlKind {
  PFC_Unknown,
  PFC_Precise,   // #pragma float_control(precise, [,on])
  PFC_NoPrecise, // #pragma float_control(precise, off)
  PFC_Except,    // #pragma float_control(except [,on])
  PFC_NoExcept,  // #pragma float_control(except, off)
  PFC_Push,      // #pragma float_control(push)
  PFC_Pop        // #pragma float_control(pop)
};

enum PragmaFPKind {
  PFK_Contract,    // #pragma clang fp contract
  PFK_Reassociate, // #pragma clang fp reassociate
  PFK_Reciprocal,  // #pragma clang fp reciprocal
  PFK_Exceptions,  // #pragma clang fp exceptions
  PFK_EvalMethod   // #pragma clang fp eval_method
};
}

#endif
