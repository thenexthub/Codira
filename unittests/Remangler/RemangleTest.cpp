//===----------------------------------------------------------------------===//
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

#include "language/Demangling/Demangle.h"
#include "language/Demangling/Demangler.h"
#include "language/Strings.h"
#include "toolchain/ADT/StringRef.h"

#include "gtest/gtest.h"

/// Helper class to conveniently construct demangle tree hierarchies.
class NodeBuilder {
  using NodePointer = language::Demangle::NodePointer;
  using Kind = language::Demangle::Node::Kind;
  
  language::Demangle::Demangler &m_dem;

public:
  NodeBuilder(language::Demangle::Demangler &dem) : m_dem(dem) {
#ifndef NDEBUG
    m_dem.disableAssertionsForUnitTest = true;
#endif
  }
  NodePointer Node(Kind kind, StringRef text) {
    return m_dem.createNode(kind, text);
  }
  NodePointer NodeWithIndex(Kind kind, language::Demangle::Node::IndexType index) {
    return m_dem.createNode(kind, index);
  }
  NodePointer Node(Kind kind, NodePointer child0 = nullptr,
                   NodePointer child1 = nullptr,
                   NodePointer child2 = nullptr,
                   NodePointer child3 = nullptr) {
    NodePointer node = m_dem.createNode(kind);

    if (child0)
      node->addChild(child0, m_dem);
    if (child1)
      node->addChild(child1, m_dem);
    if (child2)
      node->addChild(child2, m_dem);
    if (child3)
      node->addChild(child3, m_dem);
    return node;
  }
  NodePointer IntType() {
    return Node(Node::Kind::Type,
                Node(Node::Kind::Structure,
                     Node(Node::Kind::Module, language::STDLIB_NAME),
                     Node(Node::Kind::Identifier, "Int")));
  }
  NodePointer GlobalTypeMangling(NodePointer type) {
    assert(type && type->getKind() == Node::Kind::Type);
    return Node(Node::Kind::Global, Node(Node::Kind::TypeMangling, type));
  }
  NodePointer GlobalType(NodePointer type) {
    assert(type && type->getKind() != Node::Kind::Type &&
           type->getKind() != Node::Kind::TypeMangling &&
           type->getKind() != Node::Kind::Global);
    return GlobalTypeMangling(Node(Node::Kind::Type, type));
  }

  ManglingErrorOr<StringRef> remangle(NodePointer node) {
    return mangleNode(
        node,
        [](SymbolicReferenceKind, const void *) -> NodePointer {
          return nullptr;
        },
        m_dem);
  }
  std::string remangleResult(NodePointer node) {
    return remangle(node).result().str();
  }
  bool remangleSuccess(NodePointer node) {
    return remangle(node).isSuccess();
  }
};

TEST(TestCodiraRemangler, DependentGenericConformanceRequirement) {
  using namespace language::Demangle;
  using Kind = language::Demangle::Node::Kind;
  Demangler dem;
  NodeBuilder b(dem);
  {
    // Well-formed.
    NodePointer n = b.GlobalType(b.Node(
        Kind::DependentGenericType,
        b.Node(Kind::DependentGenericType,
               b.Node(Kind::DependentGenericSignature,
                      b.NodeWithIndex(Kind::DependentGenericParamCount, 1),
                      b.Node(Kind::DependentGenericConformanceRequirement,
                             b.Node(Kind::Type,
                                    b.Node(Kind::DependentGenericParamType,
                                           b.NodeWithIndex(Kind::Index, 0),
                                           b.NodeWithIndex(Kind::Index, 0))),
                             b.Node(Kind::Type,
                                    b.Node(Kind::Protocol,
                                           b.Node(Kind::Module, "M"),
                                           b.Node(Kind::Identifier, "B"))))),
               b.IntType())));
    ASSERT_EQ(b.remangleResult(n), "$sSi1M1BRzluuD");
  }
  {
    // Malformed.
    NodePointer n = b.GlobalType(b.Node(
        Kind::DependentGenericType,
        b.Node(Kind::DependentGenericType,
               b.Node(Kind::DependentGenericSignature,
                      b.NodeWithIndex(Kind::DependentGenericParamCount, 1),
                      b.Node(Kind::DependentGenericConformanceRequirement,
                             b.Node(Kind::Type,
                                    b.Node(Kind::DependentGenericParamType,
                                           b.NodeWithIndex(Kind::Index, 0),
                                           b.NodeWithIndex(Kind::Index, 0))))),
               b.IntType())));
    ASSERT_FALSE(b.remangleSuccess(n));
  }
}
