//===--- TypeCheckRegex.cpp - Regex type checking utilities ---------------===//
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

#include "language/AST/ASTContext.h"
#include "language/AST/Decl.h"
#include "language/AST/DiagnosticsSema.h"
#include "language/AST/Expr.h"
#include "language/AST/Type.h"
#include "language/AST/TypeCheckRequests.h"
#include "language/AST/Types.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/Bridging/ASTGen.h"

using namespace language;

typedef uint16_t CaptureStructureSerializationVersion;

static unsigned
getCaptureStructureSerializationAllocationSize(unsigned regexLength) {
  return sizeof(CaptureStructureSerializationVersion) + regexLength + 1;
}

enum class RegexCaptureStructureCode: uint8_t {
  End           = 0,
  Atom          = 1,
  NamedAtom     = 2,
  FormArray     = 3,
  FormOptional  = 4,
  BeginTuple    = 5,
  EndTuple      = 6,
  CaseCount
};

/// Decodes regex capture types from the given serialization and appends the
/// decoded capture types to @p result. Returns true if the serialization is
/// malformed.
static bool decodeRegexCaptureTypes(ASTContext &ctx,
                                    ArrayRef<uint8_t> serialization,
                                    Type atomType,
                                    SmallVectorImpl<TupleTypeElt> &result) {
  // Encoding rules:
  // encode(〚`T`〛) ==> <version>, 〚`T`〛, .end
  // 〚`T` (atom)〛 ==> .atom
  // 〚`name: T` (atom)〛 ==> .atom, `name`, '\0'
  // 〚`[T]`〛 ==> 〚`T`〛, .formArray
  // 〚`T?`〛 ==> 〚`T`〛, .formOptional
  // 〚`(T0, T1, ...)` (top level)〛 ==> 〚`T0`〛, 〚`T1`〛, ...
  // 〚`(T0, T1, ...)`〛 ==> .beginTuple, 〚`T0`〛, 〚`T1`〛, ..., .endTuple
  //
  // For details, see apple/language-experimental-string-processing.
  using Version = CaptureStructureSerializationVersion;
  static const Version implVersion = 1;
  unsigned size = serialization.size();
  // A serialization should store a version and `.end` at the very least.
  unsigned minSize = sizeof(Version) + sizeof(RegexCaptureStructureCode);
  if (size < minSize)
    return false;
  // Read version.
  Version version = *reinterpret_cast<const Version *>(serialization.data());
  if (version != implVersion)
    return true;
  // Read contents.
  SmallVector<SmallVector<TupleTypeElt, 4>, 4> scopes(1);
  unsigned offset = sizeof(Version);
  auto consumeCode = [&]() -> std::optional<RegexCaptureStructureCode> {
    auto rawValue = serialization[offset];
    if (rawValue >= (uint8_t)RegexCaptureStructureCode::CaseCount)
      return std::nullopt;
    offset += sizeof(RegexCaptureStructureCode);
    return (RegexCaptureStructureCode)rawValue;
  };
  do {
    auto code = consumeCode();
    if (!code)
      return false;
    switch (*code) {
    case RegexCaptureStructureCode::End:
      offset = size;
      break;
    case RegexCaptureStructureCode::Atom:
      scopes.back().push_back(atomType);
      break;
    case RegexCaptureStructureCode::NamedAtom: {
      auto *namePtr = reinterpret_cast<const char *>(
          serialization.slice(offset).data());
      auto length = strnlen(namePtr, size - offset);
      if (length >= size - offset)
        return true; // Unterminated string.
      StringRef name(namePtr, length);
      scopes.back().push_back(
          TupleTypeElt(atomType, ctx.getIdentifier(name)));
      offset += length + /*NUL*/ 1;
      break;
    }
    case RegexCaptureStructureCode::FormArray: {
      auto &element = scopes.back().back();
      element = TupleTypeElt(ArraySliceType::get(element.getType()),
                             element.getName());
      break;
    }
    case RegexCaptureStructureCode::FormOptional: {
      auto &element = scopes.back().back();
      element = TupleTypeElt(OptionalType::get(element.getType()),
                             element.getName());
      break;
    }
    case RegexCaptureStructureCode::BeginTuple:
      scopes.push_back({});
      break;
    case RegexCaptureStructureCode::EndTuple: {
      auto children = scopes.pop_back_val();
      assert(children.size() > 1);
      auto type = TupleType::get(children, ctx);
      scopes.back().push_back(Type(type));
      break;
    }
    case RegexCaptureStructureCode::CaseCount:
      toolchain_unreachable("Handled earlier");
    }
  } while (offset < size);
  if (scopes.size() != 1)
    return true; // Unterminated tuple.
  auto &elements = scopes.back();
  result.append(elements.begin(), elements.end());
  return false;
}

static Type computeRegexLiteralType(const RegexLiteralExpr *regex,
                                    ArrayRef<uint8_t> serializedCaptures) {
  auto &ctx = regex->getASTContext();
  auto *regexDecl = ctx.getRegexDecl();
  if (!regexDecl) {
    ctx.Diags.diagnose(regex->getLoc(), diag::string_processing_lib_missing,
                       ctx.Id_Regex.str());
    return Type();
  }

  SmallVector<TupleTypeElt, 4> matchElements;
  if (decodeRegexCaptureTypes(ctx, serializedCaptures,
                              /*atomType*/ ctx.getSubstringType(),
                              matchElements)) {
    ctx.Diags.diagnose(regex->getLoc(),
                       diag::regex_capture_types_failed_to_decode);
    return Type();
  }
  assert(!matchElements.empty() && "Should have decoded at least an atom");
  if (matchElements.size() == 1)
    return BoundGenericStructType::get(regexDecl, Type(),
                                       matchElements.front().getType());
  // Form a tuple.
  auto matchType = TupleType::get(matchElements, ctx);
  return BoundGenericStructType::get(regexDecl, Type(), {matchType});
}

RegexLiteralPatternInfo
RegexLiteralPatternInfoRequest::evaluate(Evaluator &eval,
                                         const RegexLiteralExpr *regex) const {
#if LANGUAGE_BUILD_REGEX_PARSER_IN_COMPILER
  auto &ctx = regex->getASTContext();
  auto regexText = regex->getParsedRegexText();

  // Let the Codira library parse the contents, returning an error, or null if
  // successful.
  size_t version = 0;
  auto capturesSize =
      getCaptureStructureSerializationAllocationSize(regexText.size());
  std::vector<uint8_t> capturesBuf(capturesSize);

  BridgedRegexLiteralPatternFeatures bridgedFeatures;
  LANGUAGE_DEFER {
    language_ASTGen_freeBridgedRegexLiteralPatternFeatures(bridgedFeatures);
  };

  bool hadError = language_ASTGen_parseRegexLiteral(
      regexText,
      /*versionOut=*/&version,
      /*captureStructureOut=*/capturesBuf.data(),
      /*captureStructureSize=*/capturesBuf.size(),
      /*patternFeaturesOut=*/&bridgedFeatures,
      /*diagBaseLoc=*/regex->getLoc(), &ctx.Diags);
  if (hadError)
    return {regexText, Type(), /*version*/ 0, /*features*/ {}};

  SmallVector<RegexLiteralPatternFeature> features;
  for (auto &bridgedFeature : bridgedFeatures.unbridged())
    features.push_back(bridgedFeature.unbridged());

  assert(version >= 1);
  auto regexTy = computeRegexLiteralType(regex, capturesBuf);

  // FIXME: We need to plumb through the 'regexToEmit' result to the caller.
  // For now, it is the same as the input.
  return {/*regexToEmit*/ regexText, regexTy, version,
          ctx.AllocateCopy(features)};
#else
  toolchain_unreachable("Shouldn't have parsed a RegexLiteralExpr");
#endif
}

StringRef RegexLiteralFeatureDescriptionRequest::evaluate(
    Evaluator &evaluator, RegexLiteralPatternFeatureKind kind,
    ASTContext *ctx) const {
#if LANGUAGE_BUILD_REGEX_PARSER_IN_COMPILER
  // The resulting string is allocated in the ASTContext, we can return the
  // StringRef directly.
  BridgedStringRef str;
  language_ASTGen_getDescriptionForRegexPatternFeature(kind, *ctx, &str);
  return str.unbridged();
#else
  toolchain_unreachable("Shouldn't have parsed a RegexLiteralExpr");
#endif
}

AvailabilityRange RegexLiteralFeatureAvailabilityRequest::evaluate(
    Evaluator &evaluator, RegexLiteralPatternFeatureKind kind,
    ASTContext *ctx) const {
#if LANGUAGE_BUILD_REGEX_PARSER_IN_COMPILER
  BridgedCodiraVersion version;
  language_ASTGen_getCodiraVersionForRegexPatternFeature(kind, &version);
  return ctx->getCodiraAvailability(version.getMajor(), version.getMinor());
#else
  toolchain_unreachable("Shouldn't have parsed a RegexLiteralExpr");
#endif
}
