//===--- StmtViz.cpp - Graphviz visualization for Stmt ASTs -----*- C++ -*-===//
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
//  This file implements Stmt::viewAST, which generates a Graphviz DOT file
//  that depicts the AST and then calls Graphviz/dot+gv on it.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/StmtGraphTraits.h"
#include "language/Core/AST/Decl.h"
#include "toolchain/Support/GraphWriter.h"

using namespace language::Core;

void Stmt::viewAST() const {
#ifndef NDEBUG
  toolchain::ViewGraph(this,"AST");
#else
  toolchain::errs() << "Stmt::viewAST is only available in debug builds on "
               << "systems with Graphviz or gv!\n";
#endif
}

namespace toolchain {
template<>
struct DOTGraphTraits<const Stmt*> : public DefaultDOTGraphTraits {
  DOTGraphTraits (bool isSimple=false) : DefaultDOTGraphTraits(isSimple) {}

  static std::string getNodeLabel(const Stmt* Node, const Stmt* Graph) {

#ifndef NDEBUG
    std::string OutStr;
    toolchain::raw_string_ostream Out(OutStr);

    if (Node)
      Out << Node->getStmtClassName();
    else
      Out << "<NULL>";

    if (OutStr[0] == '\n') OutStr.erase(OutStr.begin());

    // Process string output to make it nicer...
    for (unsigned i = 0; i != OutStr.length(); ++i)
      if (OutStr[i] == '\n') {                            // Left justify
        OutStr[i] = '\\';
        OutStr.insert(OutStr.begin()+i+1, 'l');
      }

    return OutStr;
#else
    return "";
#endif
  }
};
} // end namespace toolchain
