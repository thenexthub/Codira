//===--- ExceptionSpecificationType.h ---------------------------*- C++ -*-===//
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
///
/// \file
/// Defines the ExceptionSpecificationType enumeration and various
/// utility functions.
///
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_BASIC_EXCEPTIONSPECIFICATIONTYPE_H
#define LANGUAGE_CORE_BASIC_EXCEPTIONSPECIFICATIONTYPE_H

namespace language::Core {

/// The various types of exception specifications that exist in C++11.
enum ExceptionSpecificationType {
  EST_None,             ///< no exception specification
  EST_DynamicNone,      ///< throw()
  EST_Dynamic,          ///< throw(T1, T2)
  EST_MSAny,            ///< Microsoft throw(...) extension
  EST_NoThrow,          ///< Microsoft __declspec(nothrow) extension
  EST_BasicNoexcept,    ///< noexcept
  EST_DependentNoexcept,///< noexcept(expression), value-dependent
  EST_NoexceptFalse,    ///< noexcept(expression), evals to 'false'
  EST_NoexceptTrue,     ///< noexcept(expression), evals to 'true'
  EST_Unevaluated,      ///< not evaluated yet, for special member function
  EST_Uninstantiated,   ///< not instantiated yet
  EST_Unparsed          ///< not parsed yet
};

inline bool isDynamicExceptionSpec(ExceptionSpecificationType ESpecType) {
  return ESpecType >= EST_DynamicNone && ESpecType <= EST_MSAny;
}

inline bool isComputedNoexcept(ExceptionSpecificationType ESpecType) {
  return ESpecType >= EST_DependentNoexcept &&
         ESpecType <= EST_NoexceptTrue;
}

inline bool isNoexceptExceptionSpec(ExceptionSpecificationType ESpecType) {
  return ESpecType == EST_BasicNoexcept || ESpecType == EST_NoThrow ||
         isComputedNoexcept(ESpecType);
}

inline bool isUnresolvedExceptionSpec(ExceptionSpecificationType ESpecType) {
  return ESpecType == EST_Unevaluated || ESpecType == EST_Uninstantiated;
}

inline bool isExplicitThrowExceptionSpec(ExceptionSpecificationType ESpecType) {
  return ESpecType == EST_Dynamic || ESpecType == EST_MSAny ||
         ESpecType == EST_NoexceptFalse;
}

/// Possible results from evaluation of a noexcept expression.
enum CanThrowResult {
  CT_Cannot,
  CT_Dependent,
  CT_Can
};

inline CanThrowResult mergeCanThrow(CanThrowResult CT1, CanThrowResult CT2) {
  // CanThrowResult constants are ordered so that the maximum is the correct
  // merge result.
  return CT1 > CT2 ? CT1 : CT2;
}

} // end namespace language::Core

#endif // LANGUAGE_CORE_BASIC_EXCEPTIONSPECIFICATIONTYPE_H
