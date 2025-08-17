//=======- PtrTypesSemantics.cpp ---------------------------------*- C++ -*-==//
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

#ifndef LANGUAGE_CORE_ANALYZER_WEBKIT_PTRTYPESEMANTICS_H
#define LANGUAGE_CORE_ANALYZER_WEBKIT_PTRTYPESEMANTICS_H

#include "toolchain/ADT/APInt.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/DenseSet.h"
#include "toolchain/ADT/PointerUnion.h"
#include <optional>

namespace language::Core {
class CXXBaseSpecifier;
class CXXMethodDecl;
class CXXRecordDecl;
class Decl;
class FunctionDecl;
class QualType;
class RecordType;
class Stmt;
class TranslationUnitDecl;
class Type;
class TypedefDecl;

// Ref-countability of a type is implicitly defined by Ref<T> and RefPtr<T>
// implementation. It can be modeled as: type T having public methods ref() and
// deref()

// In WebKit there are two ref-counted templated smart pointers: RefPtr<T> and
// Ref<T>.

/// \returns CXXRecordDecl of the base if the type has ref as a public method,
/// nullptr if not, std::nullopt if inconclusive.
std::optional<const language::Core::CXXRecordDecl *>
hasPublicMethodInBase(const CXXBaseSpecifier *Base,
                      toolchain::StringRef NameToMatch);

/// \returns true if \p Class is ref-countable, false if not, std::nullopt if
/// inconclusive.
std::optional<bool> isRefCountable(const language::Core::CXXRecordDecl *Class);

/// \returns true if \p Class is checked-pointer compatible, false if not,
/// std::nullopt if inconclusive.
std::optional<bool> isCheckedPtrCapable(const language::Core::CXXRecordDecl *Class);

/// \returns true if \p Class is ref-counted, false if not.
bool isRefCounted(const language::Core::CXXRecordDecl *Class);

/// \returns true if \p Class is a CheckedPtr / CheckedRef, false if not.
bool isCheckedPtr(const language::Core::CXXRecordDecl *Class);

/// \returns true if \p Class is a RetainPtr, false if not.
bool isRetainPtr(const language::Core::CXXRecordDecl *Class);

/// \returns true if \p Class is a smart pointer (RefPtr, WeakPtr, etc...),
/// false if not.
bool isSmartPtr(const language::Core::CXXRecordDecl *Class);

/// \returns true if \p Class is ref-countable AND not ref-counted, false if
/// not, std::nullopt if inconclusive.
std::optional<bool> isUncounted(const language::Core::QualType T);

/// \returns true if \p Class is CheckedPtr capable AND not checked, false if
/// not, std::nullopt if inconclusive.
std::optional<bool> isUnchecked(const language::Core::QualType T);

/// An inter-procedural analysis facility that detects CF types with the
/// underlying pointer type.
class RetainTypeChecker {
  toolchain::DenseSet<const RecordType *> CFPointees;
  toolchain::DenseSet<const Type *> RecordlessTypes;
  bool IsARCEnabled{false};
  bool DefaultSynthProperties{true};

public:
  void visitTranslationUnitDecl(const TranslationUnitDecl *);
  void visitTypedef(const TypedefDecl *);
  bool isUnretained(const QualType, bool ignoreARC = false);
  bool isARCEnabled() const { return IsARCEnabled; }
  bool defaultSynthProperties() const { return DefaultSynthProperties; }
};

/// \returns true if \p Class is NS or CF objects AND not retained, false if
/// not, std::nullopt if inconclusive.
std::optional<bool> isUnretained(const language::Core::QualType T, bool IsARCEnabled);

/// \returns true if \p Class is ref-countable AND not ref-counted, false if
/// not, std::nullopt if inconclusive.
std::optional<bool> isUncounted(const language::Core::CXXRecordDecl* Class);

/// \returns true if \p Class is CheckedPtr capable AND not checked, false if
/// not, std::nullopt if inconclusive.
std::optional<bool> isUnchecked(const language::Core::CXXRecordDecl *Class);

/// \returns true if \p T is either a raw pointer or reference to an uncounted
/// class, false if not, std::nullopt if inconclusive.
std::optional<bool> isUncountedPtr(const language::Core::QualType T);

/// \returns true if \p T is either a raw pointer or reference to an unchecked
/// class, false if not, std::nullopt if inconclusive.
std::optional<bool> isUncheckedPtr(const language::Core::QualType T);

/// \returns true if \p T is either a raw pointer or reference to an uncounted
/// or unchecked class, false if not, std::nullopt if inconclusive.
std::optional<bool> isUnsafePtr(const QualType T, bool IsArcEnabled);

/// \returns true if \p T is a RefPtr, Ref, CheckedPtr, CheckedRef, or its
/// variant, false if not.
bool isRefOrCheckedPtrType(const language::Core::QualType T);

/// \returns true if \p T is a RetainPtr, false if not.
bool isRetainPtrType(const language::Core::QualType T);

/// \returns true if \p T is a RefPtr, Ref, CheckedPtr, CheckedRef, or
/// unique_ptr, false if not.
bool isOwnerPtrType(const language::Core::QualType T);

/// \returns true if \p F creates ref-countable object from uncounted parameter,
/// false if not.
bool isCtorOfRefCounted(const language::Core::FunctionDecl *F);

/// \returns true if \p F creates checked ptr object from uncounted parameter,
/// false if not.
bool isCtorOfCheckedPtr(const language::Core::FunctionDecl *F);

/// \returns true if \p F creates ref-countable or checked ptr object from
/// uncounted parameter, false if not.
bool isCtorOfSafePtr(const language::Core::FunctionDecl *F);

/// \returns true if \p Name is RefPtr, Ref, or its variant, false if not.
bool isRefType(const std::string &Name);

/// \returns true if \p Name is CheckedRef or CheckedPtr, false if not.
bool isCheckedPtr(const std::string &Name);

/// \returns true if \p Name is RetainPtr or its variant, false if not.
bool isRetainPtr(const std::string &Name);

/// \returns true if \p Name is a smart pointer type name, false if not.
bool isSmartPtrClass(const std::string &Name);

/// \returns true if \p M is getter of a ref-counted class, false if not.
std::optional<bool> isGetterOfSafePtr(const language::Core::CXXMethodDecl *Method);

/// \returns true if \p F is a conversion between ref-countable or ref-counted
/// pointer types.
bool isPtrConversion(const FunctionDecl *F);

/// \returns true if \p F is a builtin function which is considered trivial.
bool isTrivialBuiltinFunction(const FunctionDecl *F);

/// \returns true if \p F is a static singleton function.
bool isSingleton(const FunctionDecl *F);

/// An inter-procedural analysis facility that detects functions with "trivial"
/// behavior with respect to reference counting, such as simple field getters.
class TrivialFunctionAnalysis {
public:
  /// \returns true if \p D is a "trivial" function.
  bool isTrivial(const Decl *D) const { return isTrivialImpl(D, TheCache); }
  bool isTrivial(const Stmt *S) const { return isTrivialImpl(S, TheCache); }

private:
  friend class TrivialFunctionAnalysisVisitor;

  using CacheTy =
      toolchain::DenseMap<toolchain::PointerUnion<const Decl *, const Stmt *>, bool>;
  mutable CacheTy TheCache{};

  static bool isTrivialImpl(const Decl *D, CacheTy &Cache);
  static bool isTrivialImpl(const Stmt *S, CacheTy &Cache);
};

} // namespace language::Core

#endif
