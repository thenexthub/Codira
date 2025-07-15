//===--- Bridging/ASTContextBridging.cpp ----------------------------------===//
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

#include "language/AST/ASTBridging.h"

#include "language/AST/ASTContext.h"
#include "language/AST/AvailabilitySpec.h"

using namespace language;

//===----------------------------------------------------------------------===//
// MARK: ASTContext
//===----------------------------------------------------------------------===//

BridgedIdentifier BridgedASTContext_getIdentifier(BridgedASTContext cContext,
                                                  BridgedStringRef cStr) {
  return cContext.unbridged().getIdentifier(cStr.unbridged());
}

BridgedIdentifier
BridgedASTContext_getDollarIdentifier(BridgedASTContext cContext, size_t idx) {
  return cContext.unbridged().getDollarIdentifier(idx);
}

bool BridgedASTContext_langOptsHasFeature(BridgedASTContext cContext,
                                          BridgedFeature feature) {
  return cContext.unbridged().LangOpts.hasFeature((Feature)feature);
}

unsigned BridgedASTContext::getMajorLanguageVersion() const {
  return unbridged().LangOpts.EffectiveLanguageVersion[0];
}

bool BridgedASTContext_langOptsCustomConditionSet(BridgedASTContext cContext,
                                                  BridgedStringRef cName) {
  ASTContext &ctx = cContext.unbridged();
  auto name = cName.unbridged();
  if (name.starts_with("$") && ctx.LangOpts.hasFeature(name.drop_front()))
    return true;

  return ctx.LangOpts.isCustomConditionalCompilationFlagSet(name);
}

bool BridgedASTContext_langOptsHasFeatureNamed(BridgedASTContext cContext,
                                               BridgedStringRef cName) {
  return cContext.unbridged().LangOpts.hasFeature(cName.unbridged());
}

bool BridgedASTContext_langOptsHasAttributeNamed(BridgedASTContext cContext,
                                                 BridgedStringRef cName) {
  return hasAttribute(cContext.unbridged().LangOpts, cName.unbridged());
}

bool BridgedASTContext_langOptsIsActiveTargetOS(BridgedASTContext cContext,
                                                BridgedStringRef cName) {
  return cContext.unbridged().LangOpts.checkPlatformCondition(
      PlatformConditionKind::OS, cName.unbridged());
}

bool BridgedASTContext_langOptsIsActiveTargetArchitecture(
    BridgedASTContext cContext, BridgedStringRef cName) {
  return cContext.unbridged().LangOpts.checkPlatformCondition(
      PlatformConditionKind::Arch, cName.unbridged());
}

bool BridgedASTContext_langOptsIsActiveTargetEnvironment(
    BridgedASTContext cContext, BridgedStringRef cName) {
  return cContext.unbridged().LangOpts.checkPlatformCondition(
      PlatformConditionKind::TargetEnvironment, cName.unbridged());
}

bool BridgedASTContext_langOptsIsActiveTargetRuntime(BridgedASTContext cContext,
                                                     BridgedStringRef cName) {
  return cContext.unbridged().LangOpts.checkPlatformCondition(
      PlatformConditionKind::Runtime, cName.unbridged());
}

bool BridgedASTContext_langOptsIsActiveTargetPtrAuth(BridgedASTContext cContext,
                                                     BridgedStringRef cName) {
  return cContext.unbridged().LangOpts.checkPlatformCondition(
      PlatformConditionKind::PtrAuth, cName.unbridged());
}

unsigned BridgedASTContext::getLangOptsTargetPointerBitWidth() const {
  return unbridged().LangOpts.Target.isArch64Bit()   ? 64
         : unbridged().LangOpts.Target.isArch32Bit() ? 32
         : unbridged().LangOpts.Target.isArch16Bit() ? 16
                                                     : 0;
}

bool BridgedASTContext::getLangOptsAttachCommentsToDecls() const {
  return unbridged().LangOpts.AttachCommentsToDecls;
}

BridgedEndianness BridgedASTContext::getLangOptsTargetEndianness() const {
  return unbridged().LangOpts.Target.isLittleEndian() ? EndianLittle
                                                      : EndianBig;
}

/// Convert an array of numbers into a form we can use in Codira.
namespace {
template <typename Arr>
CodiraInt convertArray(const Arr &array, CodiraInt **cElements) {
  CodiraInt numElements = array.size();
  *cElements = (CodiraInt *)malloc(sizeof(CodiraInt) * numElements);
  for (CodiraInt i = 0; i != numElements; ++i)
    (*cElements)[i] = array[i];
  return numElements;
}
} // namespace

void deallocateIntBuffer(CodiraInt *_Nullable cComponents) { free(cComponents); }

CodiraInt
BridgedASTContext_langOptsGetLanguageVersion(BridgedASTContext cContext,
                                             CodiraInt **cComponents) {
  auto theVersion = cContext.unbridged().LangOpts.EffectiveLanguageVersion;
  return convertArray(theVersion, cComponents);
}

LANGUAGE_NAME("BridgedASTContext.langOptsGetCompilerVersion(self:_:)")
CodiraInt
BridgedASTContext_langOptsGetCompilerVersion(BridgedASTContext cContext,
                                             CodiraInt **cComponents) {
  auto theVersion = version::Version::getCurrentLanguageVersion();
  return convertArray(theVersion, cComponents);
}

CodiraInt BridgedASTContext_langOptsGetTargetAtomicBitWidths(
    BridgedASTContext cContext, CodiraInt *_Nullable *_Nonnull cElements) {
  return convertArray(cContext.unbridged().LangOpts.getAtomicBitWidthValues(),
                      cElements);
}

bool BridgedASTContext_canImport(BridgedASTContext cContext,
                                 BridgedStringRef importPath,
                                 BridgedSourceLoc canImportLoc,
                                 BridgedCanImportVersion versionKind,
                                 const CodiraInt *_Nullable versionComponents,
                                 CodiraInt numVersionComponents) {
  // Map the version.
  toolchain::VersionTuple version;
  switch (numVersionComponents) {
  case 0:
    break;
  case 1:
    version = toolchain::VersionTuple(versionComponents[0]);
    break;
  case 2:
    version = toolchain::VersionTuple(versionComponents[0], versionComponents[1]);
    break;
  case 3:
    version = toolchain::VersionTuple(versionComponents[0], versionComponents[1],
                                 versionComponents[2]);
    break;
  default:
    version = toolchain::VersionTuple(versionComponents[0], versionComponents[1],
                                 versionComponents[2], versionComponents[3]);
    break;
  }

  ImportPath::Module::Builder builder(cContext.unbridged(),
                                      importPath.unbridged(), /*separator=*/'.',
                                      canImportLoc.unbridged());
  return cContext.unbridged().canImportModule(
      builder.get(), canImportLoc.unbridged(), version,
      versionKind == CanImportUnderlyingVersion);
}

BridgedAvailabilityMacroMap BridgedASTContext::getAvailabilityMacroMap() const {
  return &unbridged().getAvailabilityMacroMap();
}
