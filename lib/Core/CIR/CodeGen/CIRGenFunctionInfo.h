//==-- CIRGenFunctionInfo.h - Representation of fn argument/return types ---==//
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
// Defines CIRGenFunctionInfo and associated types used in representing the
// CIR source types and ABI-coerced types for function arguments and
// return values.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_CIR_CIRGENFUNCTIONINFO_H
#define LANGUAGE_CORE_CIR_CIRGENFUNCTIONINFO_H

#include "language/Core/AST/CanonicalType.h"
#include "language/Core/CIR/MissingFeatures.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/Support/TrailingObjects.h"

namespace language::Core::CIRGen {

/// A class for recording the number of arguments that a function signature
/// requires.
class RequiredArgs {
  /// The number of required arguments, or ~0 if the signature does not permit
  /// optional arguments.
  unsigned numRequired;

public:
  enum All_t { All };

  RequiredArgs(All_t _) : numRequired(~0U) {}
  explicit RequiredArgs(unsigned n) : numRequired(n) { assert(n != ~0U); }

  unsigned getOpaqueData() const { return numRequired; }

  bool allowsOptionalArgs() const { return numRequired != ~0U; }

  /// Compute the arguments required by the given formal prototype, given that
  /// there may be some additional, non-formal arguments in play.
  ///
  /// If FD is not null, this will consider pass_object_size params in FD.
  static RequiredArgs
  getFromProtoWithExtraSlots(const language::Core::FunctionProtoType *prototype,
                             unsigned additional) {
    if (!prototype->isVariadic())
      return All;

    if (prototype->hasExtParameterInfos())
      toolchain_unreachable("NYI");

    return RequiredArgs(prototype->getNumParams() + additional);
  }

  static RequiredArgs
  getFromProtoWithExtraSlots(language::Core::CanQual<language::Core::FunctionProtoType> prototype,
                             unsigned additional) {
    return getFromProtoWithExtraSlots(prototype.getTypePtr(), additional);
  }

  unsigned getNumRequiredArgs() const {
    assert(allowsOptionalArgs());
    return numRequired;
  }
};

// The TrailingObjects for this class contain the function return type in the
// first CanQualType slot, followed by the argument types.
class CIRGenFunctionInfo final
    : public toolchain::FoldingSetNode,
      private toolchain::TrailingObjects<CIRGenFunctionInfo, CanQualType> {
  RequiredArgs required;

  unsigned numArgs;

  CanQualType *getArgTypes() { return getTrailingObjects(); }
  const CanQualType *getArgTypes() const { return getTrailingObjects(); }

  CIRGenFunctionInfo() : required(RequiredArgs::All) {}

public:
  static CIRGenFunctionInfo *create(CanQualType resultType,
                                    toolchain::ArrayRef<CanQualType> argTypes,
                                    RequiredArgs required);

  void operator delete(void *p) { ::operator delete(p); }

  // Friending class TrailingObjects is apparantly not good enough for MSVC, so
  // these have to be public.
  friend class TrailingObjects;

  using const_arg_iterator = const CanQualType *;
  using arg_iterator = CanQualType *;

  // This function has to be CamelCase because toolchain::FoldingSet requires so.
  // NOLINTNEXTLINE(readability-identifier-naming)
  static void Profile(toolchain::FoldingSetNodeID &id, RequiredArgs required,
                      CanQualType resultType,
                      toolchain::ArrayRef<CanQualType> argTypes) {
    id.AddBoolean(required.getOpaqueData());
    resultType.Profile(id);
    for (const CanQualType &arg : argTypes)
      arg.Profile(id);
  }

  // NOLINTNEXTLINE(readability-identifier-naming)
  void Profile(toolchain::FoldingSetNodeID &id) {
    // If the Profile functions get out of sync, we can end up with incorrect
    // function signatures, so we call the static Profile function here rather
    // than duplicating the logic.
    Profile(id, required, getReturnType(), arguments());
  }

  toolchain::ArrayRef<CanQualType> arguments() const {
    return toolchain::ArrayRef<CanQualType>(argTypesBegin(), numArgs);
  }

  toolchain::ArrayRef<CanQualType> requiredArguments() const {
    return toolchain::ArrayRef<CanQualType>(argTypesBegin(), getNumRequiredArgs());
  }

  CanQualType getReturnType() const { return getArgTypes()[0]; }

  const_arg_iterator argTypesBegin() const { return getArgTypes() + 1; }
  const_arg_iterator argTypesEnd() const { return getArgTypes() + 1 + numArgs; }
  arg_iterator argTypesBegin() { return getArgTypes() + 1; }
  arg_iterator argTypesEnd() { return getArgTypes() + 1 + numArgs; }

  unsigned argTypeSize() const { return numArgs; }

  toolchain::MutableArrayRef<CanQualType> argTypes() {
    return toolchain::MutableArrayRef<CanQualType>(argTypesBegin(), numArgs);
  }
  toolchain::ArrayRef<CanQualType> argTypes() const {
    return toolchain::ArrayRef<CanQualType>(argTypesBegin(), numArgs);
  }

  bool isVariadic() const { return required.allowsOptionalArgs(); }
  RequiredArgs getRequiredArgs() const { return required; }
  unsigned getNumRequiredArgs() const {
    return isVariadic() ? getRequiredArgs().getNumRequiredArgs()
                        : argTypeSize();
  }
};

} // namespace language::Core::CIRGen

#endif
