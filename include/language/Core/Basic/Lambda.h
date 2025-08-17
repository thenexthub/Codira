//===--- Lambda.h - Types for C++ Lambdas -----------------------*- C++ -*-===//
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
/// Defines several types used to describe C++ lambda expressions
/// that are shared between the parser and AST.
///
//===----------------------------------------------------------------------===//


#ifndef LANGUAGE_CORE_BASIC_LAMBDA_H
#define LANGUAGE_CORE_BASIC_LAMBDA_H

namespace language::Core {

/// The default, if any, capture method for a lambda expression.
enum LambdaCaptureDefault {
  LCD_None,
  LCD_ByCopy,
  LCD_ByRef
};

/// The different capture forms in a lambda introducer
///
/// C++11 allows capture of \c this, or of local variables by copy or
/// by reference.  C++1y also allows "init-capture", where the initializer
/// is an expression.
enum LambdaCaptureKind {
  LCK_This,   ///< Capturing the \c *this object by reference
  LCK_StarThis, ///< Capturing the \c *this object by copy
  LCK_ByCopy, ///< Capturing by copy (a.k.a., by value)
  LCK_ByRef,  ///< Capturing by reference
  LCK_VLAType ///< Capturing variable-length array type
};

} // end namespace language::Core

#endif // LANGUAGE_CORE_BASIC_LAMBDA_H
