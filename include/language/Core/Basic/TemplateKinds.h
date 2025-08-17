//===--- TemplateKinds.h - Enum values for C++ Template Kinds ---*- C++ -*-===//
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
/// Defines the language::Core::TemplateNameKind enum.
///
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_BASIC_TEMPLATEKINDS_H
#define LANGUAGE_CORE_BASIC_TEMPLATEKINDS_H

namespace language::Core {

/// Specifies the kind of template name that an identifier refers to.
/// Be careful when changing this: this enumeration is used in diagnostics.
enum TemplateNameKind {
  /// The name does not refer to a template.
  TNK_Non_template = 0,
  /// The name refers to a function template or a set of overloaded
  /// functions that includes at least one function template, or (in C++20)
  /// refers to a set of non-template functions but is followed by a '<'.
  TNK_Function_template,
  /// The name refers to a template whose specialization produces a
  /// type. The template itself could be a class template, template
  /// template parameter, or template alias.
  TNK_Type_template,
  /// The name refers to a variable template whose specialization produces a
  /// variable.
  TNK_Var_template,
  /// The name refers to a dependent template name:
  /// \code
  /// template<typename MetaFun, typename T1, typename T2> struct apply2 {
  ///   typedef typename MetaFun::template apply<T1, T2>::type type;
  /// };
  /// \endcode
  ///
  /// Here, "apply" is a dependent template name within the typename
  /// specifier in the typedef. "apply" is a nested template, and
  /// whether the template name is assumed to refer to a type template or a
  /// function template depends on the context in which the template
  /// name occurs.
  TNK_Dependent_template_name,
  /// Lookup for the name failed, but we're assuming it was a template name
  /// anyway. In C++20, this is mandatory in order to parse ADL-only function
  /// template specialization calls.
  TNK_Undeclared_template,
  /// The name refers to a concept.
  TNK_Concept_template,
};

}
#endif
