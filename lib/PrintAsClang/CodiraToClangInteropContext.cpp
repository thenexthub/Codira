//===--- CodiraToClangInteropContext.cpp - Interop context -------*- C++ -*-===//
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

#include "CodiraToClangInteropContext.h"
#include "language/AST/Decl.h"
#include "language/IRGen/IRABIDetailsProvider.h"

using namespace language;

CodiraToClangInteropContext::CodiraToClangInteropContext(
    ModuleDecl &mod, const IRGenOptions &irGenOpts)
    : mod(mod), irGenOpts(irGenOpts) {}

CodiraToClangInteropContext::~CodiraToClangInteropContext() {}

IRABIDetailsProvider &CodiraToClangInteropContext::getIrABIDetails() {
  if (!irABIDetails)
    irABIDetails = std::make_unique<IRABIDetailsProvider>(mod, irGenOpts);
  return *irABIDetails;
}

void CodiraToClangInteropContext::runIfStubForDeclNotEmitted(
    StringRef stubName, toolchain::function_ref<void(void)> function) {
  auto result = emittedStubs.insert(stubName);
  if (result.second)
    function();
}

void CodiraToClangInteropContext::recordExtensions(
    const NominalTypeDecl *typeDecl, const ExtensionDecl *ext) {
  auto it = extensions.insert(
      std::make_pair(typeDecl, std::vector<const ExtensionDecl *>()));
  it.first->second.push_back(ext);
}

toolchain::ArrayRef<const ExtensionDecl *>
CodiraToClangInteropContext::getExtensionsForNominalType(
    const NominalTypeDecl *typeDecl) const {
  auto exts = extensions.find(typeDecl);
  if (exts != extensions.end())
    return exts->getSecond();
  return {};
}
