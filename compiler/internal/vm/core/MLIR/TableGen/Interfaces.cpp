//===- Interfaces.cpp - Interface classes ---------------------------------===//
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

#include "mlir/TableGen/Interfaces.h"
#include "vm/core/ADT/FunctionExtras.h"
#include "vm/core/ADT/StringSet.h"
#include "vm/core/TableGen/Error.h"
#include "vm/core/TableGen/Record.h"

using namespace mlir;
using namespace mlir::tblgen;
using toolchain::DagInit;
using toolchain::DefInit;
using toolchain::Init;
using toolchain::ListInit;
using toolchain::Record;
using toolchain::StringInit;

//===----------------------------------------------------------------------===//
// InterfaceMethod
//===----------------------------------------------------------------------===//

InterfaceMethod::InterfaceMethod(const Record *def, std::string uniqueName)
    : def(def), uniqueName(uniqueName) {
  const DagInit *args = def->getValueAsDag("arguments");
  for (unsigned i = 0, e = args->getNumArgs(); i != e; ++i) {
    arguments.push_back({cast<StringInit>(args->getArg(i))->getValue(),
                         args->getArgNameStr(i)});
  }
}

StringRef InterfaceMethod::getReturnType() const {
  return def->getValueAsString("returnType");
}

// Return the name of this method.
StringRef InterfaceMethod::getName() const {
  return def->getValueAsString("name");
}

// Return the name of this method.
StringRef InterfaceMethod::getUniqueName() const { return uniqueName; }

// Return if this method is static.
bool InterfaceMethod::isStatic() const {
  return def->isSubClassOf("StaticInterfaceMethod");
}

// Return the body for this method if it has one.
std::optional<StringRef> InterfaceMethod::getBody() const {
  // Trim leading and trailing spaces from the default implementation.
  auto value = def->getValueAsString("body").trim();
  return value.empty() ? std::optional<StringRef>() : value;
}

// Return the default implementation for this method if it has one.
std::optional<StringRef> InterfaceMethod::getDefaultImplementation() const {
  // Trim leading and trailing spaces from the default implementation.
  auto value = def->getValueAsString("defaultBody").trim();
  return value.empty() ? std::optional<StringRef>() : value;
}

// Return the description of this method if it has one.
std::optional<StringRef> InterfaceMethod::getDescription() const {
  auto value = def->getValueAsString("description");
  return value.empty() ? std::optional<StringRef>() : value;
}

ArrayRef<InterfaceMethod::Argument> InterfaceMethod::getArguments() const {
  return arguments;
}

bool InterfaceMethod::arg_empty() const { return arguments.empty(); }

//===----------------------------------------------------------------------===//
// Interface
//===----------------------------------------------------------------------===//

Interface::Interface(const Record *def) : def(def) {
  assert(def->isSubClassOf("Interface") &&
         "must be subclass of TableGen 'Interface' class");

  // Initialize the interface methods.
  auto *listInit = dyn_cast<ListInit>(def->getValueInit("methods"));
  // In case of overloaded methods, we need to find a unique name for each for
  // the internal function pointer in the "vtable" we generate. This is an
  // internal name, we could use a randomly generated name as long as there are
  // no collisions.
  StringSet<> uniqueNames;
  for (const Init *init : listInit->getElements()) {
    std::string name =
        cast<DefInit>(init)->getDef()->getValueAsString("name").str();
    while (!uniqueNames.insert(name).second) {
      name = name + "_" + std::to_string(uniqueNames.size());
    }
    methods.emplace_back(cast<DefInit>(init)->getDef(), name);
  }

  // Initialize the interface base classes.
  auto *basesInit = dyn_cast<ListInit>(def->getValueInit("baseInterfaces"));
  // Chained inheritance will produce duplicates in the base interface set.
  StringSet<> basesAdded;
  toolchain::unique_function<void(Interface)> addBaseInterfaceFn =
      [&](const Interface &baseInterface) {
        // Inherit any base interfaces.
        for (const auto &baseBaseInterface : baseInterface.getBaseInterfaces())
          addBaseInterfaceFn(baseBaseInterface);

        // Add the base interface.
        if (basesAdded.contains(baseInterface.getName()))
          return;
        baseInterfaces.push_back(std::make_unique<Interface>(baseInterface));
        basesAdded.insert(baseInterface.getName());
      };
  for (const Init *init : basesInit->getElements())
    addBaseInterfaceFn(Interface(cast<DefInit>(init)->getDef()));
}

// Return the name of this interface.
StringRef Interface::getName() const {
  return def->getValueAsString("cppInterfaceName");
}

// Returns this interface's name prefixed with namespaces.
std::string Interface::getFullyQualifiedName() const {
  StringRef cppNamespace = getCppNamespace();
  StringRef name = getName();
  if (cppNamespace.empty())
    return name.str();
  return (cppNamespace + "::" + name).str();
}

// Return the C++ namespace of this interface.
StringRef Interface::getCppNamespace() const {
  return def->getValueAsString("cppNamespace");
}

// Return the methods of this interface.
ArrayRef<InterfaceMethod> Interface::getMethods() const { return methods; }

// Return the description of this method if it has one.
std::optional<StringRef> Interface::getDescription() const {
  auto value = def->getValueAsString("description");
  return value.empty() ? std::optional<StringRef>() : value;
}

// Return the interfaces extra class declaration code.
std::optional<StringRef> Interface::getExtraClassDeclaration() const {
  auto value = def->getValueAsString("extraClassDeclaration");
  return value.empty() ? std::optional<StringRef>() : value;
}

// Return the traits extra class declaration code.
std::optional<StringRef> Interface::getExtraTraitClassDeclaration() const {
  auto value = def->getValueAsString("extraTraitClassDeclaration");
  return value.empty() ? std::optional<StringRef>() : value;
}

// Return the shared extra class declaration code.
std::optional<StringRef> Interface::getExtraSharedClassDeclaration() const {
  auto value = def->getValueAsString("extraSharedClassDeclaration");
  return value.empty() ? std::optional<StringRef>() : value;
}

std::optional<StringRef> Interface::getExtraClassOf() const {
  auto value = def->getValueAsString("extraClassOf");
  return value.empty() ? std::optional<StringRef>() : value;
}

// Return the body for this method if it has one.
std::optional<StringRef> Interface::getVerify() const {
  // Only OpInterface supports the verify method.
  if (!isa<OpInterface>(this))
    return std::nullopt;
  auto value = def->getValueAsString("verify");
  return value.empty() ? std::optional<StringRef>() : value;
}

bool Interface::verifyWithRegions() const {
  return def->getValueAsBit("verifyWithRegions");
}

//===----------------------------------------------------------------------===//
// AttrInterface
//===----------------------------------------------------------------------===//

bool AttrInterface::classof(const Interface *interface) {
  return interface->getDef().isSubClassOf("AttrInterface");
}

//===----------------------------------------------------------------------===//
// OpInterface
//===----------------------------------------------------------------------===//

bool OpInterface::classof(const Interface *interface) {
  return interface->getDef().isSubClassOf("OpInterface");
}

//===----------------------------------------------------------------------===//
// TypeInterface
//===----------------------------------------------------------------------===//

bool TypeInterface::classof(const Interface *interface) {
  return interface->getDef().isSubClassOf("TypeInterface");
}

//===----------------------------------------------------------------------===//
// DialectInterface
//===----------------------------------------------------------------------===//

bool DialectInterface::classof(const Interface *interface) {
  return interface->getDef().isSubClassOf("DialectInterface");
}
