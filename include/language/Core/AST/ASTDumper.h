//===--- ASTDumper.h - Dumping implementation for ASTs --------------------===//
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

#ifndef LANGUAGE_CORE_AST_ASTDUMPER_H
#define LANGUAGE_CORE_AST_ASTDUMPER_H

#include "language/Core/AST/ASTNodeTraverser.h"
#include "language/Core/AST/TextNodeDumper.h"
#include "language/Core/Basic/SourceManager.h"

namespace language::Core {

class ASTDumper : public ASTNodeTraverser<ASTDumper, TextNodeDumper> {

  TextNodeDumper NodeDumper;

  raw_ostream &OS;

  const bool ShowColors;

public:
  ASTDumper(raw_ostream &OS, const ASTContext &Context, bool ShowColors)
      : NodeDumper(OS, Context, ShowColors), OS(OS), ShowColors(ShowColors) {}

  ASTDumper(raw_ostream &OS, bool ShowColors)
      : NodeDumper(OS, ShowColors), OS(OS), ShowColors(ShowColors) {}

  TextNodeDumper &doGetNodeDelegate() { return NodeDumper; }

  void dumpInvalidDeclContext(const DeclContext *DC);
  void dumpLookups(const DeclContext *DC, bool DumpDecls);

  template <typename SpecializationDecl>
  void dumpTemplateDeclSpecialization(const SpecializationDecl *D,
                                      bool DumpExplicitInst, bool DumpRefOnly);
  template <typename TemplateDecl>
  void dumpTemplateDecl(const TemplateDecl *D, bool DumpExplicitInst);

  void VisitFunctionTemplateDecl(const FunctionTemplateDecl *D);
  void VisitClassTemplateDecl(const ClassTemplateDecl *D);
  void VisitVarTemplateDecl(const VarTemplateDecl *D);
};

} // namespace language::Core

#endif
