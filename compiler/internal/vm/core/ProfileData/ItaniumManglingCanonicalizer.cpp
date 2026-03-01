//===----------------- ItaniumManglingCanonicalizer.cpp -------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "vm/core/ProfileData/ItaniumManglingCanonicalizer.h"
#include "vm/core/ADT/DenseMap.h"
#include "vm/core/ADT/FoldingSet.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Demangle/ItaniumDemangle.h"
#include "vm/core/Support/Allocator.h"

using namespace vm::core;
using toolchain::itanium_demangle::ForwardTemplateReference;
using toolchain::itanium_demangle::Node;
using toolchain::itanium_demangle::NodeKind;

namespace {
struct FoldingSetNodeIDBuilder {
  toolchain::FoldingSetNodeID &ID;
  void operator()(const Node *P) { ID.AddPointer(P); }
  void operator()(std::string_view Str) {
    if (Str.empty())
      ID.AddString({});
    else
      ID.AddString(toolchain::StringRef(&*Str.begin(), Str.size()));
  }
  template <typename T>
  std::enable_if_t<std::is_integral_v<T> || std::is_enum_v<T>> operator()(T V) {
    ID.AddInteger((unsigned long long)V);
  }
  void operator()(itanium_demangle::NodeArray A) {
    ID.AddInteger(A.size());
    for (const Node *N : A)
      (*this)(N);
  }
};

template<typename ...T>
void profileCtor(toolchain::FoldingSetNodeID &ID, Node::Kind K, T ...V) {
  FoldingSetNodeIDBuilder Builder = {ID};
  Builder(K);
  int VisitInOrder[] = {
    (Builder(V), 0) ...,
    0 // Avoid empty array if there are no arguments.
  };
  (void)VisitInOrder;
}

// FIXME: Convert this to a generic lambda when possible.
template<typename NodeT> struct ProfileSpecificNode {
  FoldingSetNodeID &ID;
  template<typename ...T> void operator()(T ...V) {
    profileCtor(ID, NodeKind<NodeT>::Kind, V...);
  }
};

struct ProfileNode {
  FoldingSetNodeID &ID;
  template<typename NodeT> void operator()(const NodeT *N) {
    N->match(ProfileSpecificNode<NodeT>{ID});
  }
};

template<> void ProfileNode::operator()(const ForwardTemplateReference *N) {
  llvm_unreachable("should never canonicalize a ForwardTemplateReference");
}

void profileNode(toolchain::FoldingSetNodeID &ID, const Node *N) {
  N->visit(ProfileNode{ID});
}

class FoldingNodeAllocator {
  class alignas(alignof(Node *)) NodeHeader : public toolchain::FoldingSetNode {
  public:
    // 'Node' in this context names the injected-class-name of the base class.
    itanium_demangle::Node *getNode() {
      return reinterpret_cast<itanium_demangle::Node *>(this + 1);
    }
    void Profile(toolchain::FoldingSetNodeID &ID) { profileNode(ID, getNode()); }
  };

  BumpPtrAllocator RawAlloc;
  toolchain::FoldingSet<NodeHeader> Nodes;

public:
  void reset() {}

  template <typename T, typename... Args>
  std::pair<Node *, bool> getOrCreateNode(bool CreateNewNodes, Args &&... As) {
    // FIXME: Don't canonicalize forward template references for now, because
    // they contain state (the resolved template node) that's not known at their
    // point of creation.
    if (std::is_same<T, ForwardTemplateReference>::value) {
      // Note that we don't use if-constexpr here and so we must still write
      // this code in a generic form.
      return {new (RawAlloc.Allocate(sizeof(T), alignof(T)))
                  T(std::forward<Args>(As)...),
              true};
    }

    toolchain::FoldingSetNodeID ID;
    profileCtor(ID, NodeKind<T>::Kind, As...);

    void *InsertPos;
    if (NodeHeader *Existing = Nodes.FindNodeOrInsertPos(ID, InsertPos))
      return {static_cast<T*>(Existing->getNode()), false};

    if (!CreateNewNodes)
      return {nullptr, true};

    static_assert(alignof(T) <= alignof(NodeHeader),
                  "underaligned node header for specific node kind");
    void *Storage =
        RawAlloc.Allocate(sizeof(NodeHeader) + sizeof(T), alignof(NodeHeader));
    NodeHeader *New = new (Storage) NodeHeader;
    T *Result = new (New->getNode()) T(std::forward<Args>(As)...);
    Nodes.InsertNode(New, InsertPos);
    return {Result, true};
  }

  template<typename T, typename... Args>
  Node *makeNode(Args &&...As) {
    return getOrCreateNode<T>(true, std::forward<Args>(As)...).first;
  }

  void *allocateNodeArray(size_t sz) {
    return RawAlloc.Allocate(sizeof(Node *) * sz, alignof(Node *));
  }
};

class CanonicalizerAllocator : public FoldingNodeAllocator {
  Node *MostRecentlyCreated = nullptr;
  Node *TrackedNode = nullptr;
  bool TrackedNodeIsUsed = false;
  bool CreateNewNodes = true;
  toolchain::SmallDenseMap<Node*, Node*, 32> Remappings;

  template<typename T, typename ...Args> Node *makeNodeSimple(Args &&...As) {
    std::pair<Node *, bool> Result =
        getOrCreateNode<T>(CreateNewNodes, std::forward<Args>(As)...);
    if (Result.second) {
      // Node is new. Make a note of that.
      MostRecentlyCreated = Result.first;
    } else if (Result.first) {
      // Node is pre-existing; check if it's in our remapping table.
      if (auto *N = Remappings.lookup(Result.first)) {
        Result.first = N;
        assert(!Remappings.contains(Result.first) &&
               "should never need multiple remap steps");
      }
      if (Result.first == TrackedNode)
        TrackedNodeIsUsed = true;
    }
    return Result.first;
  }

  /// Helper to allow makeNode to be partially-specialized on T.
  template<typename T> struct MakeNodeImpl {
    CanonicalizerAllocator &Self;
    template<typename ...Args> Node *make(Args &&...As) {
      return Self.makeNodeSimple<T>(std::forward<Args>(As)...);
    }
  };

public:
  template<typename T, typename ...Args> Node *makeNode(Args &&...As) {
    return MakeNodeImpl<T>{*this}.make(std::forward<Args>(As)...);
  }

  void reset() { MostRecentlyCreated = nullptr; }

  void setCreateNewNodes(bool CNN) { CreateNewNodes = CNN; }

  void addRemapping(Node *A, Node *B) {
    // Note, we don't need to check whether B is also remapped, because if it
    // was we would have already remapped it when building it.
    Remappings.insert(std::make_pair(A, B));
  }

  bool isMostRecentlyCreated(Node *N) const { return MostRecentlyCreated == N; }

  void trackUsesOf(Node *N) {
    TrackedNode = N;
    TrackedNodeIsUsed = false;
  }
  bool trackedNodeIsUsed() const { return TrackedNodeIsUsed; }
};

// FIXME: Also expand built-in substitutions?

using CanonicalizingDemangler =
    itanium_demangle::ManglingParser<CanonicalizerAllocator>;
} // namespace

struct ItaniumManglingCanonicalizer::Impl {
  CanonicalizingDemangler Demangler = {nullptr, nullptr};
};

ItaniumManglingCanonicalizer::ItaniumManglingCanonicalizer() : P(new Impl) {}
ItaniumManglingCanonicalizer::~ItaniumManglingCanonicalizer() { delete P; }

ItaniumManglingCanonicalizer::EquivalenceError
ItaniumManglingCanonicalizer::addEquivalence(FragmentKind Kind, StringRef First,
                                             StringRef Second) {
  auto &Alloc = P->Demangler.ASTAllocator;
  Alloc.setCreateNewNodes(true);

  auto Parse = [&](StringRef Str) {
    P->Demangler.reset(Str.begin(), Str.end());
    Node *N = nullptr;
    switch (Kind) {
      // A <name>, with minor extensions to allow arbitrary namespace and
      // template names that can't easily be written as <name>s.
    case FragmentKind::Name:
      // Very special case: allow "St" as a shorthand for "3std". It's not
      // valid as a <name> mangling, but is nonetheless the most natural
      // way to name the 'std' namespace.
      if (Str.size() == 2 && P->Demangler.consumeIf("St"))
        N = P->Demangler.make<itanium_demangle::NameType>("std");
      // We permit substitutions to name templates without their template
      // arguments. This mostly just falls out, as almost all template names
      // are valid as <name>s, but we also want to parse <substitution>s as
      // <name>s, even though they're not.
      else if (Str.starts_with("S"))
        // Parse the substitution and optional following template arguments.
        N = P->Demangler.parseType();
      else
        N = P->Demangler.parseName();
      break;

      // A <type>.
    case FragmentKind::Type:
      N = P->Demangler.parseType();
      break;

      // An <encoding>.
    case FragmentKind::Encoding:
      N = P->Demangler.parseEncoding();
      break;
    }

    // If we have trailing junk, the mangling is invalid.
    if (P->Demangler.numLeft() != 0)
      N = nullptr;

    // If any node was created after N, then we cannot safely remap it because
    // it might already be in use by another node.
    return std::make_pair(N, Alloc.isMostRecentlyCreated(N));
  };

  Node *FirstNode, *SecondNode;
  bool FirstIsNew, SecondIsNew;

  std::tie(FirstNode, FirstIsNew) = Parse(First);
  if (!FirstNode)
    return EquivalenceError::InvalidFirstMangling;

  Alloc.trackUsesOf(FirstNode);
  std::tie(SecondNode, SecondIsNew) = Parse(Second);
  if (!SecondNode)
    return EquivalenceError::InvalidSecondMangling;

  // If they're already equivalent, there's nothing to do.
  if (FirstNode == SecondNode)
    return EquivalenceError::Success;

  if (FirstIsNew && !Alloc.trackedNodeIsUsed())
    Alloc.addRemapping(FirstNode, SecondNode);
  else if (SecondIsNew)
    Alloc.addRemapping(SecondNode, FirstNode);
  else
    return EquivalenceError::ManglingAlreadyUsed;

  return EquivalenceError::Success;
}

static ItaniumManglingCanonicalizer::Key
parseMaybeMangledName(CanonicalizingDemangler &Demangler, StringRef Mangling,
                      bool CreateNewNodes) {
  Demangler.ASTAllocator.setCreateNewNodes(CreateNewNodes);
  Demangler.reset(Mangling.begin(), Mangling.end());
  // Attempt demangling only for names that look like C++ mangled names.
  // Otherwise, treat them as extern "C" names. We permit the latter to
  // be remapped by (eg)
  //   encoding 6memcpy 7memmove
  // consistent with how they are encoded as local-names inside a C++ mangling.
  Node *N;
  if (Mangling.starts_with("_Z") || Mangling.starts_with("__Z") ||
      Mangling.starts_with("___Z") || Mangling.starts_with("____Z"))
    N = Demangler.parse();
  else
    N = Demangler.make<itanium_demangle::NameType>(
        std::string_view(Mangling.data(), Mangling.size()));
  return reinterpret_cast<ItaniumManglingCanonicalizer::Key>(N);
}

ItaniumManglingCanonicalizer::Key
ItaniumManglingCanonicalizer::canonicalize(StringRef Mangling) {
  return parseMaybeMangledName(P->Demangler, Mangling, true);
}

ItaniumManglingCanonicalizer::Key
ItaniumManglingCanonicalizer::lookup(StringRef Mangling) {
  return parseMaybeMangledName(P->Demangler, Mangling, false);
}
