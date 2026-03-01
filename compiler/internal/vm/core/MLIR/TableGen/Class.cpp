//===- Class.cpp - Helper classes for Op C++ code emission --------------===//
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

#include "mlir/TableGen/Class.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/Support/Debug.h"

using namespace mlir;
using namespace mlir::tblgen;

/// Returns space to be emitted after the given C++ `type`. return "" if the
/// ends with '&' or '*', or is empty, else returns " ".
static StringRef getSpaceAfterType(StringRef type) {
  return (type.empty() || type.ends_with("&") || type.ends_with("*")) ? ""
                                                                      : " ";
}

//===----------------------------------------------------------------------===//
// MethodParameter definitions
//===----------------------------------------------------------------------===//

void MethodParameter::writeDeclTo(raw_indented_ostream &os) const {
  if (optional)
    os << "/*optional*/";
  os << type << getSpaceAfterType(type) << name;
  if (hasDefaultValue())
    os << " = " << defaultValue;
}

void MethodParameter::writeDefTo(raw_indented_ostream &os) const {
  if (optional)
    os << "/*optional*/";
  os << type << getSpaceAfterType(type) << name;
}

//===----------------------------------------------------------------------===//
// MethodParameters definitions
//===----------------------------------------------------------------------===//

void MethodParameters::writeDeclTo(raw_indented_ostream &os) const {
  toolchain::interleaveComma(parameters, os,
                        [&os](auto &param) { param.writeDeclTo(os); });
}
void MethodParameters::writeDefTo(raw_indented_ostream &os) const {
  toolchain::interleaveComma(parameters, os,
                        [&os](auto &param) { param.writeDefTo(os); });
}

bool MethodParameters::subsumes(const MethodParameters &other) const {
  // These parameters do not subsume the others if there are fewer parameters
  // or their types do not match.
  if (parameters.size() < other.parameters.size())
    return false;
  if (!std::equal(
          other.parameters.begin(), other.parameters.end(), parameters.begin(),
          [](auto &lhs, auto &rhs) { return lhs.getType() == rhs.getType(); }))
    return false;

  // If all the common parameters have the same type, we can elide the other
  // method if this method has the same number of parameters as other or if the
  // first paramater after the common parameters has a default value (and, as
  // required by C++, subsequent parameters will have default values too).
  return parameters.size() == other.parameters.size() ||
         parameters[other.parameters.size()].hasDefaultValue();
}

//===----------------------------------------------------------------------===//
// MethodSignature definitions
//===----------------------------------------------------------------------===//

bool MethodSignature::makesRedundant(const MethodSignature &other) const {
  return methodName == other.methodName &&
         parameters.subsumes(other.parameters);
}

void MethodSignature::writeDeclTo(raw_indented_ostream &os) const {
  os << returnType << getSpaceAfterType(returnType) << methodName << "(";
  parameters.writeDeclTo(os);
  os << ")";
}

void MethodSignature::writeDefTo(raw_indented_ostream &os,
                                 StringRef namePrefix) const {
  os << returnType << getSpaceAfterType(returnType) << namePrefix
     << (namePrefix.empty() ? "" : "::") << methodName << "(";
  parameters.writeDefTo(os);
  os << ")";
}

void MethodSignature::writeTemplateParamsTo(
    mlir::raw_indented_ostream &os) const {
  if (templateParams.empty())
    return;

  os << "template <";
  toolchain::interleaveComma(templateParams, os,
                        [&](StringRef param) { os << "typename " << param; });
  os << ">\n";
}

//===----------------------------------------------------------------------===//
// MethodBody definitions
//===----------------------------------------------------------------------===//

MethodBody::MethodBody(bool declOnly)
    : declOnly(declOnly), stringOs(body), os(stringOs) {}

void MethodBody::writeTo(raw_indented_ostream &os) const {
  auto bodyRef = StringRef(body).ltrim('\n');
  os << bodyRef;
  if (bodyRef.empty())
    return;
  if (bodyRef.back() != '\n')
    os << "\n";
}

//===----------------------------------------------------------------------===//
// Method definitions
//===----------------------------------------------------------------------===//

void Method::writeDeclTo(raw_indented_ostream &os) const {
  methodSignature.writeTemplateParamsTo(os);
  if (deprecationMessage) {
    os << "[[deprecated(\"";
    os.write_escaped(*deprecationMessage);
    os << "\")]]\n";
  }
  if (isStatic())
    os << "static ";
  if (properties & ConstexprValue)
    os << "constexpr ";
  methodSignature.writeDeclTo(os);
  if (isConst())
    os << " const";
  if (!isInline()) {
    os << ";\n";
    return;
  }
  os << " {\n";
  methodBody.writeTo(os);
  os << "}\n\n";
}

void Method::writeDefTo(raw_indented_ostream &os, StringRef namePrefix) const {
  // The method has no definition to write if it is declaration only or inline.
  if (properties & Declaration || isInline())
    return;

  methodSignature.writeDefTo(os, namePrefix);
  if (isConst())
    os << " const";
  os << " {\n";
  methodBody.writeTo(os);
  os << "}\n\n";
}

bool Method::methodPropertiesAreCompatible(Properties properties) {
  const bool isStatic = (properties & Method::Static);
  const bool isConstructor = (properties & Method::Constructor);
  // const bool isPrivate = (properties & Method::Private);
  const bool isDeclaration = (properties & Method::Declaration);
  const bool isInline = (properties & Method::Inline);
  const bool isConstexprValue = (properties & Method::ConstexprValue);
  const bool isConst = (properties & Method::Const);

  // Note: assert to immediately fail and thus simplify debugging.
  if (isStatic && isConstructor) {
    assert(false && "constructor cannot be static");
    return false;
  }
  if (isConstructor && isConst) { // albeit constexpr is fine
    assert(false && "constructor cannot be const");
    return false;
  }
  if (isDeclaration && isInline) {
    assert(false &&
           "declaration implies no definition and thus cannot be inline");
    return false;
  }
  if (isDeclaration && isConstexprValue) {
    assert(false &&
           "declaration implies no definition and thus cannot be constexpr");
    return false;
  }

  return true;
}

//===----------------------------------------------------------------------===//
// Constructor definitions
//===----------------------------------------------------------------------===//

void Constructor::writeDeclTo(raw_indented_ostream &os) const {
  methodSignature.writeTemplateParamsTo(os);
  if (properties & ConstexprValue)
    os << "constexpr ";
  methodSignature.writeDeclTo(os);
  if (!isInline()) {
    os << ";\n\n";
    return;
  }
  os << ' ';
  if (!initializers.empty())
    os << ": ";
  toolchain::interleaveComma(initializers, os,
                        [&](auto &initializer) { initializer.writeTo(os); });
  if (!initializers.empty())
    os << ' ';
  os << "{";
  methodBody.writeTo(os);
  os << "}\n\n";
}

void Constructor::writeDefTo(raw_indented_ostream &os,
                             StringRef namePrefix) const {
  // The method has no definition to write if it is declaration only or inline.
  if (properties & Declaration || isInline())
    return;

  methodSignature.writeDefTo(os, namePrefix);
  os << ' ';
  if (!initializers.empty())
    os << ": ";
  toolchain::interleaveComma(initializers, os,
                        [&](auto &initializer) { initializer.writeTo(os); });
  if (!initializers.empty())
    os << ' ';
  os << "{";
  methodBody.writeTo(os);
  os << "}\n\n";
}

void Constructor::MemberInitializer::writeTo(raw_indented_ostream &os) const {
  os << name << '(' << value << ')';
}

//===----------------------------------------------------------------------===//
// Visibility definitions
//===----------------------------------------------------------------------===//

namespace mlir {
namespace tblgen {
raw_ostream &operator<<(raw_ostream &os, Visibility visibility) {
  switch (visibility) {
  case Visibility::Public:
    return os << "public";
  case Visibility::Protected:
    return os << "protected";
  case Visibility::Private:
    return os << "private";
  }
  return os;
}
} // namespace tblgen
} // namespace mlir

//===----------------------------------------------------------------------===//
// ParentClass definitions
//===----------------------------------------------------------------------===//

void ParentClass::writeTo(raw_indented_ostream &os) const {
  os << visibility << ' ' << name;
  if (!templateParams.empty()) {
    auto scope = os.scope("<", ">", /*indent=*/false);
    toolchain::interleaveComma(templateParams, os,
                          [&](auto &param) { os << param; });
  }
}

//===----------------------------------------------------------------------===//
// UsingDeclaration definitions
//===----------------------------------------------------------------------===//

void UsingDeclaration::writeDeclTo(raw_indented_ostream &os) const {
  if (!templateParams.empty()) {
    os << "template <";
    toolchain::interleaveComma(templateParams, os, [&](StringRef paramName) {
      os << "typename " << paramName;
    });
    os << ">\n";
  }
  os << "using " << name;
  if (!value.empty())
    os << " = " << value;
  os << ";\n";
}

//===----------------------------------------------------------------------===//
// Field definitions
//===----------------------------------------------------------------------===//

void Field::writeDeclTo(raw_indented_ostream &os) const {
  os << type << ' ' << name << ";\n";
}

//===----------------------------------------------------------------------===//
// VisibilityDeclaration definitions
//===----------------------------------------------------------------------===//

void VisibilityDeclaration::writeDeclTo(raw_indented_ostream &os) const {
  os.unindent();
  os << visibility << ":\n";
  os.indent();
}

//===----------------------------------------------------------------------===//
// ExtraClassDeclaration definitions
//===----------------------------------------------------------------------===//

void ExtraClassDeclaration::writeDeclTo(raw_indented_ostream &os) const {
  os.printReindented(extraClassDeclaration);
}

void ExtraClassDeclaration::writeDefTo(raw_indented_ostream &os,
                                       StringRef namePrefix) const {
  os.printReindented(extraClassDefinition);
}

//===----------------------------------------------------------------------===//
// Class definitions
//===----------------------------------------------------------------------===//

ParentClass &Class::addParent(ParentClass parent) {
  parents.push_back(std::move(parent));
  return parents.back();
}

void Class::writeDeclTo(raw_indented_ostream &os) const {
  if (!templateParams.empty()) {
    os << "template <";
    toolchain::interleaveComma(templateParams, os,
                          [&](StringRef param) { os << "typename " << param; });
    os << ">\n";
  }

  // Declare the class.
  os << (isStruct ? "struct" : "class") << ' ' << className << ' ';

  // Declare the parent classes, if any.
  if (!parents.empty()) {
    os << ": ";
    toolchain::interleaveComma(parents, os,
                          [&](auto &parent) { parent.writeTo(os); });
    os << ' ';
  }
  auto classScope = os.scope("{\n", "};\n", /*indent=*/true);

  // Print all the class declarations.
  for (auto &decl : declarations)
    decl->writeDeclTo(os);
}

void Class::writeDefTo(raw_indented_ostream &os) const {
  // Print all the definitions.
  for (auto &decl : declarations)
    decl->writeDefTo(os, className);
}

void Class::finalize() {
  // Sort the methods by public and private. Remove them from the pending list
  // of methods.
  SmallVector<std::unique_ptr<Method>> publicMethods, privateMethods;
  for (auto &method : methods) {
    if (method->isPrivate())
      privateMethods.push_back(std::move(method));
    else
      publicMethods.push_back(std::move(method));
  }
  methods.clear();

  // If the last visibility declaration wasn't `public`, add one that is. Then,
  // declare the public methods.
  if (!publicMethods.empty() && getLastVisibilityDecl() != Visibility::Public)
    declare<VisibilityDeclaration>(Visibility::Public);
  for (auto &method : publicMethods)
    declarations.push_back(std::move(method));

  // If the last visibility declaration wasn't `private`, add one that is. Then,
  // declare the private methods.
  if (!privateMethods.empty() && getLastVisibilityDecl() != Visibility::Private)
    declare<VisibilityDeclaration>(Visibility::Private);
  for (auto &method : privateMethods)
    declarations.push_back(std::move(method));

  // All fields added to the pending list are private and declared at the bottom
  // of the class. If the last visibility declaration wasn't `private`, add one
  // that is, then declare the fields.
  if (!fields.empty() && getLastVisibilityDecl() != Visibility::Private)
    declare<VisibilityDeclaration>(Visibility::Private);
  for (auto &field : fields)
    declare<Field>(std::move(field));
  fields.clear();
}

Visibility Class::getLastVisibilityDecl() const {
  auto reverseDecls = toolchain::reverse(declarations);
  auto it = toolchain::find_if(reverseDecls, toolchain::IsaPred<VisibilityDeclaration>);
  return it == reverseDecls.end()
             ? (isStruct ? Visibility::Public : Visibility::Private)
             : cast<VisibilityDeclaration>(**it).getVisibility();
}

Method *insertAndPruneMethods(std::vector<std::unique_ptr<Method>> &methods,
                              std::unique_ptr<Method> newMethod) {
  if (toolchain::any_of(methods, [&](auto &method) {
        return method->makesRedundant(*newMethod);
      }))
    return nullptr;

  toolchain::erase_if(methods, [&](auto &method) {
    return newMethod->makesRedundant(*method);
  });
  methods.push_back(std::move(newMethod));
  return methods.back().get();
}

Method *Class::addMethodAndPrune(Method &&newMethod) {
  return insertAndPruneMethods(methods,
                               std::make_unique<Method>(std::move(newMethod)));
}

Constructor *Class::addConstructorAndPrune(Constructor &&newCtor) {
  return dyn_cast_or_null<Constructor>(insertAndPruneMethods(
      methods, std::make_unique<Constructor>(std::move(newCtor))));
}
