//===--- Context.cpp - Demangler Context ----------------------------------===//
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
//
//  This file implements the demangler Context.
//
//===----------------------------------------------------------------------===//

#include "language/Demangling/Demangler.h"
#include "language/Demangling/ManglingMacros.h"
#include "language/Demangling/ManglingUtils.h"
#include "language/Demangling/NamespaceMacros.h"

namespace language {
namespace Demangle {
LANGUAGE_BEGIN_INLINE_NAMESPACE

//////////////////////////////////
// Context member functions     //
//////////////////////////////////

Context::Context() : D(new Demangler) {
}

Context::~Context() {
  delete D;
}

void Context::clear() {
  D->clear();
}

NodePointer Context::demangleSymbolAsNode(toolchain::StringRef MangledName) {
#if LANGUAGE_SUPPORT_OLD_MANGLING
  if (isMangledName(MangledName)) {
    return D->demangleSymbol(MangledName);
  }
  return demangleOldSymbolAsNode(MangledName, *D);
#else
  return D->demangleSymbol(MangledName);
#endif
}

NodePointer Context::demangleTypeAsNode(toolchain::StringRef MangledName) {
  return D->demangleType(MangledName);
}

#if LANGUAGE_STDLIB_HAS_TYPE_PRINTING

std::string Context::demangleSymbolAsString(toolchain::StringRef MangledName,
                                            const DemangleOptions &Options) {
  NodePointer root = demangleSymbolAsNode(MangledName);
  if (!root) return MangledName.str();

  std::string demangling = nodeToString(root, Options);
  if (demangling.empty())
    return MangledName.str();
  return demangling;
}

void Context::demangleSymbolAsString(toolchain::StringRef MangledName,
                                     NodePrinter &Printer) {
  NodePointer root = demangleSymbolAsNode(MangledName);
  nodeToString(root, Printer);
}

std::string Context::demangleTypeAsString(toolchain::StringRef MangledName,
                                          const DemangleOptions &Options) {
  NodePointer root = demangleTypeAsNode(MangledName);
  if (!root) return MangledName.str();
  
  std::string demangling = nodeToString(root, Options);
  if (demangling.empty())
    return MangledName.str();
  return demangling;
}

#endif

// Removes a '.<n>' suffix from \p Name. <n> is either a number or a combination of
// '.<other-text>.<n>'.
// Such symbols are produced in IRGen or in LLVM optimizations.
static toolchain::StringRef stripSuffix(toolchain::StringRef Name) {
  // A suffix always ends with a digit. Do this quick check to avoid scanning through the whole
  // symbol name if the symbol has no suffix (= the common case).
  if (language::Mangle::isDigit(Name.back())) {
    size_t dotPos = Name.find('.');
    if (dotPos != StringRef::npos) {
      Name = Name.substr(0, dotPos);
    }
  }
  return Name;
}

// Removes a 'TQ<index>' or 'TY<index>' from \p Name.
static toolchain::StringRef stripAsyncContinuation(toolchain::StringRef Name) {
  if (!Name.ends_with("_"))
    return Name;

  StringRef Stripped = Name.drop_back();
  while (!Stripped.empty() && language::Mangle::isDigit(Stripped.back()))
    Stripped = Stripped.drop_back();

  if (Stripped.ends_with("TQ") || Stripped.ends_with("TY"))
    return Stripped.drop_back(2);

  return Name;
}

bool Context::isThunkSymbol(toolchain::StringRef MangledName) {
  if (isMangledName(MangledName)) {
    MangledName = stripAsyncContinuation(stripSuffix(MangledName));
    // First do a quick check
    if (MangledName.ends_with("TA") ||  // partial application forwarder
        MangledName.ends_with("Ta") ||  // ObjC partial application forwarder
        MangledName.ends_with("To") ||  // language-as-ObjC thunk
        MangledName.ends_with("TO") ||  // ObjC-as-language thunk
        MangledName.ends_with("TR") ||  // reabstraction thunk helper function
        MangledName.ends_with("Tr") ||  // reabstraction thunk
        MangledName.ends_with("TW") ||  // protocol witness thunk
        MangledName.ends_with("fC")) {  // allocating constructor

      // To avoid false positives, we need to fully demangle the symbol.
      NodePointer Nd = D->demangleSymbol(MangledName);
      if (!Nd || Nd->getKind() != Node::Kind::Global ||
          Nd->getNumChildren() == 0)
        return false;

      switch (Nd->getFirstChild()->getKind()) {
        case Node::Kind::ObjCAttribute:
        case Node::Kind::NonObjCAttribute:
        case Node::Kind::PartialApplyObjCForwarder:
        case Node::Kind::PartialApplyForwarder:
        case Node::Kind::ReabstractionThunkHelper:
        case Node::Kind::ReabstractionThunk:
        case Node::Kind::ProtocolWitness:
        case Node::Kind::Allocator:
          return true;
        default:
          break;
      }
    }
    return false;
  }

  if (MangledName.starts_with("_T")) {
    // Old mangling.
    StringRef Remaining = MangledName.substr(2);
    if (Remaining.starts_with("To") ||   // language-as-ObjC thunk
        Remaining.starts_with("TO") ||   // ObjC-as-language thunk
        Remaining.starts_with("PA_") ||  // partial application forwarder
        Remaining.starts_with("PAo_")) { // ObjC partial application forwarder
      return true;
    }
  }
  return false;
}

std::string Context::getThunkTarget(toolchain::StringRef MangledName) {
  if (!isThunkSymbol(MangledName))
    return std::string();

  if (isMangledName(MangledName)) {
    // If the symbol has a suffix we cannot derive the target.
    if (stripSuffix(MangledName) != MangledName)
      return std::string();

    // Ignore any async continuation suffix
    MangledName = stripAsyncContinuation(MangledName);

    // The targets of those thunks not derivable from the mangling.
    if (MangledName.ends_with("TR") ||
        MangledName.ends_with("Tr") ||
        MangledName.ends_with("TW") )
      return std::string();

    if (MangledName.ends_with("fC")) {
      std::string target = MangledName.str();
      target[target.size() - 1] = 'c';
      return target;
    }

    return MangledName.substr(0, MangledName.size() - 2).str();
  }
  // Old mangling.
  assert(MangledName.starts_with("_T"));
  StringRef Remaining = MangledName.substr(2);
  if (Remaining.starts_with("PA_"))
    return Remaining.substr(3).str();
  if (Remaining.starts_with("PAo_"))
    return Remaining.substr(4).str();
  assert(Remaining.starts_with("To") || Remaining.starts_with("TO"));
  return std::string("_T") + Remaining.substr(2).str();
}

bool Context::hasCodiraCallingConvention(toolchain::StringRef MangledName) {
  Node *Global = demangleSymbolAsNode(MangledName);
  if (!Global || Global->getKind() != Node::Kind::Global ||
      Global->getNumChildren() == 0)
    return false;

  Node *TopLevel = Global->getFirstChild();
  switch (TopLevel->getKind()) {
    // Functions, which don't have the language calling conventions:
    case Node::Kind::TypeMetadataAccessFunction:
    case Node::Kind::ValueWitness:
    case Node::Kind::ProtocolWitnessTableAccessor:
    case Node::Kind::GenericProtocolWitnessTableInstantiationFunction:
    case Node::Kind::LazyProtocolWitnessTableAccessor:
    case Node::Kind::AssociatedTypeMetadataAccessor:
    case Node::Kind::AssociatedTypeWitnessTableAccessor:
    case Node::Kind::BaseWitnessTableAccessor:
    case Node::Kind::ObjCAttribute:
      return false;
    default:
      break;
  }
  return true;
}

std::string Context::getModuleName(toolchain::StringRef mangledName) {
  NodePointer node = demangleSymbolAsNode(mangledName);
  while (node) {
    switch (node->getKind()) {
    case Demangle::Node::Kind::Module:
      return node->getText().str();
    case Demangle::Node::Kind::TypeMangling:
    case Demangle::Node::Kind::Type:
      node = node->getFirstChild();
      break;
    case Demangle::Node::Kind::Global: {
      NodePointer newNode = nullptr;
      for (NodePointer child : *node) {
        if (!isFunctionAttr(child->getKind())) {
          newNode = child;
          break;
        }
      }
      node = newNode;
      break;
    }
    default:
      if (isSpecialized(node)) {
        auto unspec = getUnspecialized(node, *D);
        if (!unspec.isSuccess())
          node = nullptr;
        else
          node = unspec.result();
        break;
      }
      if (isContext(node->getKind())) {
        node = node->getFirstChild();
        break;
      }
      return std::string();
    }
  }
  return std::string();
}


//////////////////////////////////
// Public utility functions     //
//////////////////////////////////

#if LANGUAGE_STDLIB_HAS_TYPE_PRINTING

std::string demangleSymbolAsString(const char *MangledName,
                                   size_t MangledNameLength,
                                   const DemangleOptions &Options) {
  Context Ctx;
  return Ctx.demangleSymbolAsString(StringRef(MangledName, MangledNameLength),
                                    Options);
}

void demangleSymbolAsString(StringRef MangledName, NodePrinter &Printer) {
  Context Ctx;
  return Ctx.demangleSymbolAsString(MangledName, Printer);
}

std::string demangleTypeAsString(const char *MangledName,
                                 size_t MangledNameLength,
                                 const DemangleOptions &Options) {
  Context Ctx;
  return Ctx.demangleTypeAsString(StringRef(MangledName, MangledNameLength),
                                  Options);
}

#endif

LANGUAGE_END_INLINE_NAMESPACE
} // namespace Demangle
} // namespace language
