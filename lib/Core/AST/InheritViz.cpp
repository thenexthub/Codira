//===- InheritViz.cpp - Graphviz visualization for inheritance --*- C++ -*-===//
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
//  This file implements CXXRecordDecl::viewInheritance, which
//  generates a GraphViz DOT file that depicts the class inheritance
//  diagram and then calls Graphviz/dot+gv on it.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/TypeOrdering.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/GraphWriter.h"
#include "toolchain/Support/raw_ostream.h"
#include <map>
#include <set>
using namespace language::Core;

namespace {
/// InheritanceHierarchyWriter - Helper class that writes out a
/// GraphViz file that diagrams the inheritance hierarchy starting at
/// a given C++ class type. Note that we do not use LLVM's
/// GraphWriter, because the interface does not permit us to properly
/// differentiate between uses of types as virtual bases
/// vs. non-virtual bases.
class InheritanceHierarchyWriter {
  ASTContext& Context;
  raw_ostream &Out;
  std::map<QualType, int, QualTypeOrdering> DirectBaseCount;
  std::set<QualType, QualTypeOrdering> KnownVirtualBases;

public:
  InheritanceHierarchyWriter(ASTContext& Context, raw_ostream& Out)
    : Context(Context), Out(Out) { }

  void WriteGraph(QualType Type) {
    Out << "digraph \"" << toolchain::DOT::EscapeString(Type.getAsString())
        << "\" {\n";
    WriteNode(Type, false);
    Out << "}\n";
  }

protected:
  /// WriteNode - Write out the description of node in the inheritance
  /// diagram, which may be a base class or it may be the root node.
  void WriteNode(QualType Type, bool FromVirtual);

  /// WriteNodeReference - Write out a reference to the given node,
  /// using a unique identifier for each direct base and for the
  /// (only) virtual base.
  raw_ostream& WriteNodeReference(QualType Type, bool FromVirtual);
};
} // namespace

void InheritanceHierarchyWriter::WriteNode(QualType Type, bool FromVirtual) {
  QualType CanonType = Context.getCanonicalType(Type);

  if (FromVirtual) {
    if (!KnownVirtualBases.insert(CanonType).second)
      return;

    // We haven't seen this virtual base before, so display it and
    // its bases.
  }

  // Declare the node itself.
  Out << "  ";
  WriteNodeReference(Type, FromVirtual);

  // Give the node a label based on the name of the class.
  std::string TypeName = Type.getAsString();
  Out << " [ shape=\"box\", label=\"" << toolchain::DOT::EscapeString(TypeName);

  // If the name of the class was a typedef or something different
  // from the "real" class name, show the real class name in
  // parentheses so we don't confuse ourselves.
  if (TypeName != CanonType.getAsString()) {
    Out << "\\n(" << CanonType.getAsString() << ")";
  }

  // Finished describing the node.
  Out << " \"];\n";

  // Display the base classes.
  const auto *Decl = static_cast<const CXXRecordDecl *>(
      Type->castAs<RecordType>()->getOriginalDecl());
  for (const auto &Base : Decl->bases()) {
    QualType CanonBaseType = Context.getCanonicalType(Base.getType());

    // If this is not virtual inheritance, bump the direct base
    // count for the type.
    if (!Base.isVirtual())
      ++DirectBaseCount[CanonBaseType];

    // Write out the node (if we need to).
    WriteNode(Base.getType(), Base.isVirtual());

    // Write out the edge.
    Out << "  ";
    WriteNodeReference(Type, FromVirtual);
    Out << " -> ";
    WriteNodeReference(Base.getType(), Base.isVirtual());

    // Write out edge attributes to show the kind of inheritance.
    if (Base.isVirtual()) {
      Out << " [ style=\"dashed\" ]";
    }
    Out << ";";
  }
}

/// WriteNodeReference - Write out a reference to the given node,
/// using a unique identifier for each direct base and for the
/// (only) virtual base.
raw_ostream&
InheritanceHierarchyWriter::WriteNodeReference(QualType Type,
                                               bool FromVirtual) {
  QualType CanonType = Context.getCanonicalType(Type);

  Out << "Class_" << CanonType.getAsOpaquePtr();
  if (!FromVirtual)
    Out << "_" << DirectBaseCount[CanonType];
  return Out;
}

/// viewInheritance - Display the inheritance hierarchy of this C++
/// class using GraphViz.
void CXXRecordDecl::viewInheritance(ASTContext& Context) const {
  QualType Self = Context.getCanonicalTagType(this);

  int FD;
  SmallString<128> Filename;
  if (std::error_code EC = toolchain::sys::fs::createTemporaryFile(
          Self.getAsString(), "dot", FD, Filename)) {
    toolchain::errs() << "Error: " << EC.message() << "\n";
    return;
  }

  toolchain::errs() << "Writing '" << Filename << "'... ";

  toolchain::raw_fd_ostream O(FD, true);

  InheritanceHierarchyWriter Writer(Context, O);
  Writer.WriteGraph(Self);
  toolchain::errs() << " done. \n";

  O.close();

  // Display the graph
  DisplayGraph(Filename);
}
