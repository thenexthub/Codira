//===-- APINotesTypes.cpp - API Notes Data Types ----------------*- C++ -*-===//
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

#include "language/Core/APINotes/Types.h"
#include "toolchain/Support/raw_ostream.h"

namespace language::Core {
namespace api_notes {
LLVM_DUMP_METHOD void CommonEntityInfo::dump(toolchain::raw_ostream &OS) const {
  if (Unavailable)
    OS << "[Unavailable] (" << UnavailableMsg << ")" << ' ';
  if (UnavailableInSwift)
    OS << "[UnavailableInSwift] ";
  if (SwiftPrivateSpecified)
    OS << (SwiftPrivate ? "[SwiftPrivate] " : "");
  if (!SwiftName.empty())
    OS << "Swift Name: " << SwiftName << ' ';
  OS << '\n';
}

LLVM_DUMP_METHOD void CommonTypeInfo::dump(toolchain::raw_ostream &OS) const {
  static_cast<const CommonEntityInfo &>(*this).dump(OS);
  if (SwiftBridge)
    OS << "Swift Briged Type: " << *SwiftBridge << ' ';
  if (NSErrorDomain)
    OS << "NSError Domain: " << *NSErrorDomain << ' ';
  OS << '\n';
}

LLVM_DUMP_METHOD void ContextInfo::dump(toolchain::raw_ostream &OS) {
  static_cast<CommonTypeInfo &>(*this).dump(OS);
  if (HasDefaultNullability)
    OS << "DefaultNullability: " << DefaultNullability << ' ';
  if (HasDesignatedInits)
    OS << "[HasDesignatedInits] ";
  if (SwiftImportAsNonGenericSpecified)
    OS << (SwiftImportAsNonGeneric ? "[SwiftImportAsNonGeneric] " : "");
  if (SwiftObjCMembersSpecified)
    OS << (SwiftObjCMembers ? "[SwiftObjCMembers] " : "");
  OS << '\n';
}

LLVM_DUMP_METHOD void VariableInfo::dump(toolchain::raw_ostream &OS) const {
  static_cast<const CommonEntityInfo &>(*this).dump(OS);
  if (NullabilityAudited)
    OS << "Audited Nullability: " << Nullable << ' ';
  if (!Type.empty())
    OS << "C Type: " << Type << ' ';
  OS << '\n';
}

LLVM_DUMP_METHOD void ObjCPropertyInfo::dump(toolchain::raw_ostream &OS) const {
  static_cast<const VariableInfo &>(*this).dump(OS);
  if (SwiftImportAsAccessorsSpecified)
    OS << (SwiftImportAsAccessors ? "[SwiftImportAsAccessors] " : "");
  OS << '\n';
}

LLVM_DUMP_METHOD void ParamInfo::dump(toolchain::raw_ostream &OS) const {
  static_cast<const VariableInfo &>(*this).dump(OS);
  if (NoEscapeSpecified)
    OS << (NoEscape ? "[NoEscape] " : "");
  if (LifetimeboundSpecified)
    OS << (Lifetimebound ? "[Lifetimebound] " : "");
  OS << "RawRetainCountConvention: " << RawRetainCountConvention << ' ';
  OS << '\n';
}

LLVM_DUMP_METHOD void FunctionInfo::dump(toolchain::raw_ostream &OS) const {
  static_cast<const CommonEntityInfo &>(*this).dump(OS);
  OS << (NullabilityAudited ? "[NullabilityAudited] " : "")
     << "RawRetainCountConvention: " << RawRetainCountConvention << ' ';
  if (!ResultType.empty())
    OS << "Result Type: " << ResultType << ' ';
  if (!SwiftReturnOwnership.empty())
    OS << "SwiftReturnOwnership: " << SwiftReturnOwnership << ' ';
  if (!Params.empty())
    OS << '\n';
  for (auto &PI : Params)
    PI.dump(OS);
}

LLVM_DUMP_METHOD void ObjCMethodInfo::dump(toolchain::raw_ostream &OS) {
  static_cast<FunctionInfo &>(*this).dump(OS);
  if (Self)
    Self->dump(OS);
  OS << (DesignatedInit ? "[DesignatedInit] " : "")
     << (RequiredInit ? "[RequiredInit] " : "") << '\n';
}

LLVM_DUMP_METHOD void CXXMethodInfo::dump(toolchain::raw_ostream &OS) {
  static_cast<FunctionInfo &>(*this).dump(OS);
  if (This)
    This->dump(OS);
}

LLVM_DUMP_METHOD void TagInfo::dump(toolchain::raw_ostream &OS) {
  static_cast<CommonTypeInfo &>(*this).dump(OS);
  if (HasFlagEnum)
    OS << (IsFlagEnum ? "[FlagEnum] " : "");
  if (EnumExtensibility)
    OS << "Enum Extensibility: " << static_cast<long>(*EnumExtensibility)
       << ' ';
  if (SwiftCopyableSpecified)
    OS << (SwiftCopyable ? "[SwiftCopyable] " : "[~SwiftCopyable]");
  if (SwiftEscapableSpecified)
    OS << (SwiftEscapable ? "[SwiftEscapable] " : "[~SwiftEscapable]");
  OS << '\n';
}

LLVM_DUMP_METHOD void TypedefInfo::dump(toolchain::raw_ostream &OS) const {
  static_cast<const CommonTypeInfo &>(*this).dump(OS);
  if (SwiftWrapper)
    OS << "Swift Type: " << static_cast<long>(*SwiftWrapper) << ' ';
  OS << '\n';
}
} // namespace api_notes
} // namespace language::Core
