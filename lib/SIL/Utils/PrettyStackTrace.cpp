//===--- PrettyStackTrace.cpp - Defines SIL crash prettifiers -------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#include "language/Basic/QuotedString.h"
#include "language/AST/PrettyStackTrace.h"
#include "language/AST/Decl.h"
#include "language/AST/Expr.h"
#include "language/AST/Pattern.h"
#include "language/AST/Stmt.h"
#include "language/SIL/PrettyStackTrace.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILModule.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;

toolchain::cl::opt<bool>
SILPrintOnError("sil-print-on-error", toolchain::cl::init(false),
                toolchain::cl::desc("Printing SIL function bodies in crash diagnostics."));

toolchain::cl::opt<bool> SILPrintModuleOnError(
    "sil-print-module-on-error", toolchain::cl::init(false),
    toolchain::cl::desc("Printing SIL module in crash diagnostics."));

static void printLocationDescription(toolchain::raw_ostream &out,
                                         SILLocation::FilenameAndLocation loc,
                                         ASTContext &Context) {
  out << "<<debugloc at " << QuotedString(loc.filename)
      << ":" << loc.line << ":" << loc.column << ">>";
}

void language::printSILLocationDescription(toolchain::raw_ostream &out,
                                        SILLocation loc,
                                        ASTContext &Context) {
  if (loc.isASTNode()) {
    if (auto decl = loc.getAsASTNode<Decl>()) {
      printDeclDescription(out, decl);
    } else if (auto expr = loc.getAsASTNode<Expr>()) {
      printExprDescription(out, expr, Context);
    } else if (auto stmt = loc.getAsASTNode<Stmt>()) {
      printStmtDescription(out, stmt, Context);
    } else if (auto pattern = loc.castToASTNode<Pattern>()) {
      printPatternDescription(out, pattern, Context);
    } else {
      out << "<<unknown AST node>>";
    }
    return;
  }
  if (loc.isFilenameAndLocation()) {
    printLocationDescription(out, *loc.getFilenameAndLocation(), Context);
    return;
  }
  if (loc.isSILFile()) {
    printSourceLocDescription(out, loc.getSourceLoc(), Context);
    return;
  }
  out << "<<invalid location>>";
  return;
}

void PrettyStackTraceSILLocation::print(toolchain::raw_ostream &out) const {
  out << "While " << Action << " at ";
  printSILLocationDescription(out, Loc, Context);
}

void PrettyStackTraceSILFunction::print(toolchain::raw_ostream &out) const {
  out << "While " << action << " SIL function ";
  if (!fn) {
    out << " <<null>>";
    return;
  }

  printFunctionInfo(out);
}

void PrettyStackTraceSILFunction::printFunctionInfo(toolchain::raw_ostream &out) const {  
  out << "\"";
  fn->printName(out);
  out << "\".\n";

  if (!fn->getLocation().isNull()) {
    out << " for ";
    printSILLocationDescription(out, fn->getLocation(),
                                fn->getModule().getASTContext());
  }
  if (SILPrintOnError)
    fn->print(out);
  if (SILPrintModuleOnError)
    fn->getModule().print(out, fn->getModule().getCodiraModule());
}

void PrettyStackTraceSILNode::print(toolchain::raw_ostream &out) const {
  out << "While " << Action << " SIL node ";
  if (Node)
    out << *Node;
}

void PrettyStackTraceSILDeclRef::print(toolchain::raw_ostream &out) const {
  out << "While " << action << " SIL decl '";
  declRef.print(out);
  out << "'\n";
}
