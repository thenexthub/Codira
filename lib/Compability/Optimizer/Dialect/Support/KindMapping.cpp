//===-- KindMapping.cpp ---------------------------------------------------===//
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/Dialect/Support/KindMapping.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "toolchain/IR/LLVMContext.h"
#include "toolchain/Support/CommandLine.h"

/// Allow the user to set the FIR intrinsic type kind value to LLVM type
/// mappings.  Note that these are not mappings from kind values to any
/// other MLIR dialect, only to LLVM IR. The default values follow the f18
/// front-end kind mappings.

using Bitsize = fir::KindMapping::Bitsize;
using KindTy = fir::KindMapping::KindTy;
using LLVMTypeID = fir::KindMapping::LLVMTypeID;
using MatchResult = fir::KindMapping::MatchResult;

static toolchain::cl::opt<std::string>
    clKindMapping("kind-mapping",
                  toolchain::cl::desc("kind mapping string to set kind precision"),
                  toolchain::cl::value_desc("kind-mapping-string"),
                  toolchain::cl::init(fir::KindMapping::getDefaultMap()));

static toolchain::cl::opt<std::string>
    clDefaultKinds("default-kinds",
                   toolchain::cl::desc("string to set default kind values"),
                   toolchain::cl::value_desc("default-kind-string"),
                   toolchain::cl::init(fir::KindMapping::getDefaultKinds()));

// Keywords for the floating point types.

static constexpr const char *kwHalf = "Half";
static constexpr const char *kwBFloat = "BFloat";
static constexpr const char *kwFloat = "Float";
static constexpr const char *kwDouble = "Double";
static constexpr const char *kwX86FP80 = "X86_FP80";
static constexpr const char *kwFP128 = "FP128";
static constexpr const char *kwPPCFP128 = "PPC_FP128";

/// Integral types default to the kind value being the size of the value in
/// bytes. The default is to scale from bytes to bits.
static Bitsize defaultScalingKind(KindTy kind) {
  const unsigned bitsInByte = 8;
  return kind * bitsInByte;
}

/// Floating-point types default to the kind value being the size of the value
/// in bytes. The default is to translate kinds of 2, 3, 4, 8, 10, and 16 to a
/// valid toolchain::Type::TypeID value. Otherwise, the default is FloatTyID.
static LLVMTypeID defaultRealKind(KindTy kind) {
  switch (kind) {
  case 2:
    return LLVMTypeID::HalfTyID;
  case 3:
    return LLVMTypeID::BFloatTyID;
  case 4:
    return LLVMTypeID::FloatTyID;
  case 8:
    return LLVMTypeID::DoubleTyID;
  case 10:
    return LLVMTypeID::X86_FP80TyID;
  case 16:
    return LLVMTypeID::FP128TyID;
  default:
    return LLVMTypeID::FloatTyID;
  }
}

// lookup the kind-value given the defaults, the mappings, and a KIND key
template <typename RT, char KEY>
static RT doLookup(std::function<RT(KindTy)> def,
                   const toolchain::DenseMap<std::pair<char, KindTy>, RT> &map,
                   KindTy kind) {
  std::pair<char, KindTy> key{KEY, kind};
  auto iter = map.find(key);
  if (iter != map.end())
    return iter->second;
  return def(kind);
}

// do a lookup for INTEGER, LOGICAL, or CHARACTER
template <char KEY, typename MAP>
static Bitsize getIntegerLikeBitsize(KindTy kind, const MAP &map) {
  return doLookup<Bitsize, KEY>(defaultScalingKind, map, kind);
}

// do a lookup for REAL or COMPLEX
template <char KEY, typename MAP>
static LLVMTypeID getFloatLikeTypeID(KindTy kind, const MAP &map) {
  return doLookup<LLVMTypeID, KEY>(defaultRealKind, map, kind);
}

template <char KEY, typename MAP>
static const toolchain::fltSemantics &getFloatSemanticsOfKind(KindTy kind,
                                                         const MAP &map) {
  switch (doLookup<LLVMTypeID, KEY>(defaultRealKind, map, kind)) {
  case LLVMTypeID::HalfTyID:
    return toolchain::APFloat::IEEEhalf();
  case LLVMTypeID::BFloatTyID:
    return toolchain::APFloat::BFloat();
  case LLVMTypeID::FloatTyID:
    return toolchain::APFloat::IEEEsingle();
  case LLVMTypeID::DoubleTyID:
    return toolchain::APFloat::IEEEdouble();
  case LLVMTypeID::X86_FP80TyID:
    return toolchain::APFloat::x87DoubleExtended();
  case LLVMTypeID::FP128TyID:
    return toolchain::APFloat::IEEEquad();
  case LLVMTypeID::PPC_FP128TyID:
    return toolchain::APFloat::PPCDoubleDouble();
  default:
    toolchain_unreachable("Invalid floating type");
  }
}

/// Parse an intrinsic type code. The codes are ('a', CHARACTER), ('c',
/// COMPLEX), ('i', INTEGER), ('l', LOGICAL), and ('r', REAL).
static MatchResult parseCode(char &code, const char *&ptr, const char *endPtr) {
  if (ptr >= endPtr)
    return mlir::failure();
  if (*ptr != 'a' && *ptr != 'c' && *ptr != 'i' && *ptr != 'l' && *ptr != 'r')
    return mlir::failure();
  code = *ptr++;
  return mlir::success();
}

/// Same as `parseCode` but adds the ('d', DOUBLE PRECISION) code.
static MatchResult parseDefCode(char &code, const char *&ptr,
                                const char *endPtr) {
  if (ptr >= endPtr)
    return mlir::failure();
  if (*ptr == 'd') {
    code = *ptr++;
    return mlir::success();
  }
  return parseCode(code, ptr, endPtr);
}

template <char ch>
static MatchResult parseSingleChar(const char *&ptr, const char *endPtr) {
  if (ptr >= endPtr || *ptr != ch)
    return mlir::failure();
  ++ptr;
  return mlir::success();
}

static MatchResult parseColon(const char *&ptr, const char *endPtr) {
  return parseSingleChar<':'>(ptr, endPtr);
}

static MatchResult parseComma(const char *&ptr, const char *endPtr) {
  return parseSingleChar<','>(ptr, endPtr);
}

/// Recognize and parse an unsigned integer.
static MatchResult parseInt(unsigned &result, const char *&ptr,
                            const char *endPtr) {
  const char *beg = ptr;
  while (ptr < endPtr && *ptr >= '0' && *ptr <= '9')
    ptr++;
  if (beg == ptr)
    return mlir::failure();
  toolchain::StringRef ref(beg, ptr - beg);
  int temp;
  if (ref.consumeInteger(10, temp))
    return mlir::failure();
  result = temp;
  return mlir::success();
}

static toolchain::LogicalResult matchString(const char *&ptr, const char *endPtr,
                                       toolchain::StringRef literal) {
  toolchain::StringRef s(ptr, endPtr - ptr);
  if (s.starts_with(literal)) {
    ptr += literal.size();
    return mlir::success();
  }
  return mlir::failure();
}

/// Recognize and parse the various floating-point keywords. These follow the
/// LLVM naming convention.
static MatchResult parseTypeID(LLVMTypeID &result, const char *&ptr,
                               const char *endPtr) {
  if (mlir::succeeded(matchString(ptr, endPtr, kwHalf))) {
    result = LLVMTypeID::HalfTyID;
    return mlir::success();
  }
  if (mlir::succeeded(matchString(ptr, endPtr, kwBFloat))) {
    result = LLVMTypeID::BFloatTyID;
    return mlir::success();
  }
  if (mlir::succeeded(matchString(ptr, endPtr, kwFloat))) {
    result = LLVMTypeID::FloatTyID;
    return mlir::success();
  }
  if (mlir::succeeded(matchString(ptr, endPtr, kwDouble))) {
    result = LLVMTypeID::DoubleTyID;
    return mlir::success();
  }
  if (mlir::succeeded(matchString(ptr, endPtr, kwX86FP80))) {
    result = LLVMTypeID::X86_FP80TyID;
    return mlir::success();
  }
  if (mlir::succeeded(matchString(ptr, endPtr, kwFP128))) {
    result = LLVMTypeID::FP128TyID;
    return mlir::success();
  }
  if (mlir::succeeded(matchString(ptr, endPtr, kwPPCFP128))) {
    result = LLVMTypeID::PPC_FP128TyID;
    return mlir::success();
  }
  return mlir::failure();
}

fir::KindMapping::KindMapping(mlir::MLIRContext *context, toolchain::StringRef map,
                              toolchain::ArrayRef<KindTy> defs)
    : context{context} {
  if (mlir::failed(setDefaultKinds(defs)))
    toolchain::report_fatal_error("bad default kinds");
  if (mlir::failed(parse(map)))
    toolchain::report_fatal_error("could not parse kind map");
}

fir::KindMapping::KindMapping(mlir::MLIRContext *context,
                              toolchain::ArrayRef<KindTy> defs)
    : KindMapping{context, clKindMapping, defs} {}

fir::KindMapping::KindMapping(mlir::MLIRContext *context)
    : KindMapping{context, clKindMapping, clDefaultKinds} {}

MatchResult fir::KindMapping::badMapString(const toolchain::Twine &ptr) {
  auto unknown = mlir::UnknownLoc::get(context);
  mlir::emitError(unknown, ptr);
  return mlir::failure();
}

MatchResult fir::KindMapping::parse(toolchain::StringRef kindMap) {
  if (kindMap.empty())
    return mlir::success();
  const char *srcPtr = kindMap.begin();
  const char *endPtr = kindMap.end();
  while (true) {
    char code = '\0';
    KindTy kind = 0;
    if (parseCode(code, srcPtr, endPtr) || parseInt(kind, srcPtr, endPtr))
      return badMapString(srcPtr);
    if (code == 'a' || code == 'i' || code == 'l') {
      Bitsize bits = 0;
      if (parseColon(srcPtr, endPtr) || parseInt(bits, srcPtr, endPtr))
        return badMapString(srcPtr);
      intMap[std::pair<char, KindTy>{code, kind}] = bits;
    } else if (code == 'r' || code == 'c') {
      LLVMTypeID id{};
      if (parseColon(srcPtr, endPtr) || parseTypeID(id, srcPtr, endPtr))
        return badMapString(srcPtr);
      floatMap[std::pair<char, KindTy>{code, kind}] = id;
    } else {
      return badMapString(srcPtr);
    }
    if (parseComma(srcPtr, endPtr))
      break;
  }
  if (srcPtr > endPtr)
    return badMapString(srcPtr);
  return mlir::success();
}

Bitsize fir::KindMapping::getCharacterBitsize(KindTy kind) const {
  return getIntegerLikeBitsize<'a'>(kind, intMap);
}

Bitsize fir::KindMapping::getIntegerBitsize(KindTy kind) const {
  return getIntegerLikeBitsize<'i'>(kind, intMap);
}

Bitsize fir::KindMapping::getLogicalBitsize(KindTy kind) const {
  return getIntegerLikeBitsize<'l'>(kind, intMap);
}

LLVMTypeID fir::KindMapping::getRealTypeID(KindTy kind) const {
  return getFloatLikeTypeID<'r'>(kind, floatMap);
}

LLVMTypeID fir::KindMapping::getComplexTypeID(KindTy kind) const {
  return getFloatLikeTypeID<'c'>(kind, floatMap);
}

Bitsize fir::KindMapping::getRealBitsize(KindTy kind) const {
  auto typeId = getFloatLikeTypeID<'r'>(kind, floatMap);
  toolchain::LLVMContext llCtxt; // FIXME
  return toolchain::Type::getPrimitiveType(llCtxt, typeId)->getPrimitiveSizeInBits();
}

const toolchain::fltSemantics &
fir::KindMapping::getFloatSemantics(KindTy kind) const {
  return getFloatSemanticsOfKind<'r'>(kind, floatMap);
}

std::string fir::KindMapping::mapToString() const {
  std::string result;
  bool addComma = false;
  for (auto [k, v] : intMap) {
    if (addComma)
      result.append(",");
    else
      addComma = true;
    result += k.first + std::to_string(k.second) + ":" + std::to_string(v);
  }
  for (auto [k, v] : floatMap) {
    if (addComma)
      result.append(",");
    else
      addComma = true;
    result.append(k.first + std::to_string(k.second) + ":");
    switch (v) {
    default:
      toolchain_unreachable("unhandled type-id");
    case LLVMTypeID::HalfTyID:
      result.append(kwHalf);
      break;
    case LLVMTypeID::BFloatTyID:
      result.append(kwBFloat);
      break;
    case LLVMTypeID::FloatTyID:
      result.append(kwFloat);
      break;
    case LLVMTypeID::DoubleTyID:
      result.append(kwDouble);
      break;
    case LLVMTypeID::X86_FP80TyID:
      result.append(kwX86FP80);
      break;
    case LLVMTypeID::FP128TyID:
      result.append(kwFP128);
      break;
    case LLVMTypeID::PPC_FP128TyID:
      result.append(kwPPCFP128);
      break;
    }
  }
  return result;
}

toolchain::LogicalResult
fir::KindMapping::setDefaultKinds(toolchain::ArrayRef<KindTy> defs) {
  if (defs.empty()) {
    // generic front-end defaults
    const KindTy genericKind = 4;
    defaultMap.insert({'a', 1});
    defaultMap.insert({'c', genericKind});
    defaultMap.insert({'d', 2 * genericKind});
    defaultMap.insert({'i', genericKind});
    defaultMap.insert({'l', genericKind});
    defaultMap.insert({'r', genericKind});
    return mlir::success();
  }
  if (defs.size() != 6)
    return mlir::failure();

  // defaults determined after command-line processing
  defaultMap.insert({'a', defs[0]});
  defaultMap.insert({'c', defs[1]});
  defaultMap.insert({'d', defs[2]});
  defaultMap.insert({'i', defs[3]});
  defaultMap.insert({'l', defs[4]});
  defaultMap.insert({'r', defs[5]});
  return mlir::success();
}

std::string fir::KindMapping::defaultsToString() const {
  return std::string("a") + std::to_string(defaultMap.find('a')->second) +
         std::string("c") + std::to_string(defaultMap.find('c')->second) +
         std::string("d") + std::to_string(defaultMap.find('d')->second) +
         std::string("i") + std::to_string(defaultMap.find('i')->second) +
         std::string("l") + std::to_string(defaultMap.find('l')->second) +
         std::string("r") + std::to_string(defaultMap.find('r')->second);
}

/// Convert a default intrinsic code into the proper position in the array. The
/// default kinds have a precise ordering.
static int codeToIndex(char code) {
  switch (code) {
  case 'a':
    return 0;
  case 'c':
    return 1;
  case 'd':
    return 2;
  case 'i':
    return 3;
  case 'l':
    return 4;
  case 'r':
    return 5;
  }
  toolchain_unreachable("invalid default kind intrinsic code");
}

std::vector<KindTy> fir::KindMapping::toDefaultKinds(toolchain::StringRef defs) {
  std::vector<KindTy> result(6);
  char code;
  KindTy kind;
  if (defs.empty())
    defs = clDefaultKinds;
  const char *srcPtr = defs.begin();
  const char *endPtr = defs.end();
  while (srcPtr < endPtr) {
    if (parseDefCode(code, srcPtr, endPtr) || parseInt(kind, srcPtr, endPtr))
      toolchain::report_fatal_error("invalid default kind code");
    result[codeToIndex(code)] = kind;
  }
  assert(srcPtr == endPtr);
  return result;
}

KindTy fir::KindMapping::defaultCharacterKind() const {
  auto iter = defaultMap.find('a');
  assert(iter != defaultMap.end());
  return iter->second;
}

KindTy fir::KindMapping::defaultComplexKind() const {
  auto iter = defaultMap.find('c');
  assert(iter != defaultMap.end());
  return iter->second;
}

KindTy fir::KindMapping::defaultDoubleKind() const {
  auto iter = defaultMap.find('d');
  assert(iter != defaultMap.end());
  return iter->second;
}

KindTy fir::KindMapping::defaultIntegerKind() const {
  auto iter = defaultMap.find('i');
  assert(iter != defaultMap.end());
  return iter->second;
}

KindTy fir::KindMapping::defaultLogicalKind() const {
  auto iter = defaultMap.find('l');
  assert(iter != defaultMap.end());
  return iter->second;
}

KindTy fir::KindMapping::defaultRealKind() const {
  auto iter = defaultMap.find('r');
  assert(iter != defaultMap.end());
  return iter->second;
}
