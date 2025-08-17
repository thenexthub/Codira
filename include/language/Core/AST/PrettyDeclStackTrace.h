//===- PrettyDeclStackTrace.h - Stack trace for decl processing -*- C++ -*-===//
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
// This file defines an toolchain::PrettyStackTraceEntry object for showing
// that a particular declaration was being processed when a crash
// occurred.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_PRETTYDECLSTACKTRACE_H
#define LANGUAGE_CORE_AST_PRETTYDECLSTACKTRACE_H

#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/Support/PrettyStackTrace.h"

namespace language::Core {

class ASTContext;
class Decl;

/// PrettyDeclStackTraceEntry - If a crash occurs in the parser while
/// parsing something related to a declaration, include that
/// declaration in the stack trace.
class PrettyDeclStackTraceEntry : public toolchain::PrettyStackTraceEntry {
  ASTContext &Context;
  Decl *TheDecl;
  SourceLocation Loc;
  const char *Message;

public:
  PrettyDeclStackTraceEntry(ASTContext &Ctx, Decl *D, SourceLocation Loc,
                            const char *Msg)
    : Context(Ctx), TheDecl(D), Loc(Loc), Message(Msg) {}

  void print(raw_ostream &OS) const override;
};

}

#endif
