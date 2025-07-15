//===--- CodiraMangling.cpp ------------------------------------------------===//
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

#include "CodiraLangSupport.h"

#include "language/Demangling/Demangle.h"
#include "language/Demangling/Demangler.h"

using namespace SourceKit;
using namespace language;
using namespace ide;

void CodiraLangSupport::demangleNames(
    ArrayRef<const char *> MangledNames, bool Simplified,
    std::function<void(const RequestResult<ArrayRef<std::string>> &)>
        Receiver) {

  language::Demangle::DemangleOptions DemangleOptions;
  if (Simplified) {
    DemangleOptions =
        language::Demangle::DemangleOptions::SimplifiedUIDemangleOptions();
  }

  auto getDemangledName = [&](StringRef MangledName) -> std::string {
    if (!language::Demangle::isCodiraSymbol(MangledName))
      return std::string(); // Not a mangled name

    std::string Result =
        language::Demangle::demangleSymbolAsString(MangledName, DemangleOptions);

    if (Result == MangledName)
      return std::string(); // Not a mangled name

    return Result;
  };

  SmallVector<std::string, 4> results;
  for (auto &MangledName : MangledNames) {
    results.push_back(getDemangledName(MangledName));
  }

  Receiver(RequestResult<ArrayRef<std::string>>::fromResult(results));
}

static ManglingErrorOr<std::string> mangleSimpleClass(StringRef moduleName,
                                                      StringRef className) {
  using namespace language::Demangle;
  Demangler Dem;
  auto moduleNode = Dem.createNode(Node::Kind::Module, moduleName);
  auto IdNode = Dem.createNode(Node::Kind::Identifier, className);
  auto classNode = Dem.createNode(Node::Kind::Class);
  auto typeNode = Dem.createNode(Node::Kind::Type);
  auto typeManglingNode = Dem.createNode(Node::Kind::TypeMangling);
  auto globalNode = Dem.createNode(Node::Kind::Global);

  classNode->addChild(moduleNode, Dem);
  classNode->addChild(IdNode, Dem);
  typeNode->addChild(classNode, Dem);
  typeManglingNode->addChild(typeNode, Dem);
  globalNode->addChild(typeManglingNode, Dem);
  return mangleNode(globalNode, Mangle::ManglingFlavor::Default);
}

void CodiraLangSupport::mangleSimpleClassNames(
    ArrayRef<std::pair<StringRef, StringRef>> ModuleClassPairs,
    std::function<void(const RequestResult<ArrayRef<std::string>> &)>
        Receiver) {

  SmallVector<std::string, 4> results;
  for (auto &pair : ModuleClassPairs) {
    auto mangling = mangleSimpleClass(pair.first, pair.second);
    if (!mangling.isSuccess()) {
      std::string message = "name mangling failed for ";
      message += pair.first;
      message += ".";
      message += pair.second;
      Receiver(RequestResult<ArrayRef<std::string>>::fromError(message));
      return;
    }
    results.push_back(mangling.result());
  }
  Receiver(RequestResult<ArrayRef<std::string>>::fromResult(results));
}
