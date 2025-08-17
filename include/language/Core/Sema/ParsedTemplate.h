//===--- ParsedTemplate.h - Template Parsing Data Types ---------*- C++ -*-===//
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
//  This file provides data structures that store the parsed representation of
//  templates.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_PARSEDTEMPLATE_H
#define LANGUAGE_CORE_SEMA_PARSEDTEMPLATE_H

#include "language/Core/Basic/OperatorKinds.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Basic/TemplateKinds.h"
#include "language/Core/Sema/DeclSpec.h"
#include "language/Core/Sema/Ownership.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/SmallVector.h"
#include <cassert>
#include <cstdlib>
#include <new>

namespace language::Core {
  /// Represents the parsed form of a C++ template argument.
  class ParsedTemplateArgument {
  public:
    /// Describes the kind of template argument that was parsed.
    enum KindType {
      /// A template type parameter, stored as a type.
      Type,
      /// A non-type template parameter, stored as an expression.
      NonType,
      /// A template template argument, stored as a template name.
      Template
    };

    /// Build an empty template argument.
    ///
    /// This template argument is invalid.
    ParsedTemplateArgument() : Kind(Type), Arg(nullptr) { }

    /// Create a template type argument or non-type template argument.
    ///
    /// \param Arg the template type argument or non-type template argument.
    /// \param Loc the location of the type.
    ParsedTemplateArgument(KindType Kind, void *Arg, SourceLocation NameLoc)
        : Kind(Kind), Arg(Arg), NameLoc(NameLoc) {}

    /// Create a template template argument.
    ///
    /// \param SS the C++ scope specifier that precedes the template name, if
    /// any.
    ///
    /// \param Template the template to which this template template
    /// argument refers.
    ///
    /// \param TemplateLoc the location of the template name.
    ParsedTemplateArgument(SourceLocation TemplateKwLoc, const CXXScopeSpec &SS,
                           ParsedTemplateTy Template, SourceLocation NameLoc)
        : Kind(ParsedTemplateArgument::Template),
          Arg(Template.getAsOpaquePtr()), SS(SS), TemplateKwLoc(TemplateKwLoc),
          NameLoc(NameLoc) {}

    /// Determine whether the given template argument is invalid.
    bool isInvalid() const { return Arg == nullptr; }

    /// Determine what kind of template argument we have.
    KindType getKind() const { return Kind; }

    /// Retrieve the template type argument's type.
    ParsedType getAsType() const {
      assert(Kind == Type && "Not a template type argument");
      return ParsedType::getFromOpaquePtr(Arg);
    }

    /// Retrieve the non-type template argument's expression.
    Expr *getAsExpr() const {
      assert(Kind == NonType && "Not a non-type template argument");
      return static_cast<Expr*>(Arg);
    }

    /// Retrieve the template template argument's template name.
    ParsedTemplateTy getAsTemplate() const {
      assert(Kind == Template && "Not a template template argument");
      return ParsedTemplateTy::getFromOpaquePtr(Arg);
    }

    /// Retrieve the location of the template argument.
    SourceLocation getTemplateKwLoc() const { return TemplateKwLoc; }

    /// Retrieve the location of the template argument.
    SourceLocation getNameLoc() const { return NameLoc; }

    /// Retrieve the nested-name-specifier that precedes the template
    /// name in a template template argument.
    const CXXScopeSpec &getScopeSpec() const {
      assert(Kind == Template &&
             "Only template template arguments can have a scope specifier");
      return SS;
    }

    /// Retrieve the location of the ellipsis that makes a template
    /// template argument into a pack expansion.
    SourceLocation getEllipsisLoc() const {
      assert(Kind == Template &&
             "Only template template arguments can have an ellipsis");
      return EllipsisLoc;
    }

    /// Retrieve a pack expansion of the given template template
    /// argument.
    ///
    /// \param EllipsisLoc The location of the ellipsis.
    ParsedTemplateArgument getTemplatePackExpansion(
                                              SourceLocation EllipsisLoc) const;

  private:
    KindType Kind;

    /// The actual template argument representation, which may be
    /// an \c Sema::TypeTy* (for a type), an Expr* (for an
    /// expression), or an Sema::TemplateTy (for a template).
    void *Arg;

    /// The nested-name-specifier that can accompany a template template
    /// argument.
    CXXScopeSpec SS;

    /// the location of the template keyword.
    SourceLocation TemplateKwLoc;

    /// the location of the template name.
    SourceLocation NameLoc;

    /// The ellipsis location that can accompany a template template
    /// argument (turning it into a template template argument expansion).
    SourceLocation EllipsisLoc;
  };

  /// Information about a template-id annotation
  /// token.
  ///
  /// A template-id annotation token contains the template name,
  /// template arguments, and the source locations for important
  /// tokens. All of the information about template arguments is allocated
  /// directly after this structure.
  /// A template-id annotation token can also be generated by a type-constraint
  /// construct with no explicit template arguments, e.g. "template<C T>" would
  /// annotate C as a TemplateIdAnnotation with no template arguments (the angle
  /// locations would be invalid in this case).
  struct TemplateIdAnnotation final
      : private toolchain::TrailingObjects<TemplateIdAnnotation,
                                      ParsedTemplateArgument> {
    friend TrailingObjects;
    /// TemplateKWLoc - The location of the template keyword.
    /// For e.g. typename T::template Y<U>
    SourceLocation TemplateKWLoc;

    /// TemplateNameLoc - The location of the template name within the
    /// source.
    SourceLocation TemplateNameLoc;

    /// FIXME: Temporarily stores the name of a specialization
    const IdentifierInfo *Name;

    /// FIXME: Temporarily stores the overloaded operator kind.
    OverloadedOperatorKind Operator;

    /// The declaration of the template corresponding to the
    /// template-name.
    ParsedTemplateTy Template;

    /// The kind of template that Template refers to. If this is
    /// TNK_Non_template, an error was encountered and diagnosed
    /// when parsing or looking up the template name.
    TemplateNameKind Kind;

    /// The location of the '<' before the template argument
    /// list.
    SourceLocation LAngleLoc;

    /// The location of the '>' after the template argument
    /// list.
    SourceLocation RAngleLoc;

    /// NumArgs - The number of template arguments.
    unsigned NumArgs;

    /// Whether an error was encountered in the template arguments.
    /// If so, NumArgs and the trailing arguments are best-effort.
    bool ArgsInvalid;

    /// Retrieves a pointer to the template arguments
    ParsedTemplateArgument *getTemplateArgs() { return getTrailingObjects(); }

    /// Creates a new TemplateIdAnnotation with NumArgs arguments and
    /// appends it to List.
    static TemplateIdAnnotation *
    Create(SourceLocation TemplateKWLoc, SourceLocation TemplateNameLoc,
           const IdentifierInfo *Name, OverloadedOperatorKind OperatorKind,
           ParsedTemplateTy OpaqueTemplateName, TemplateNameKind TemplateKind,
           SourceLocation LAngleLoc, SourceLocation RAngleLoc,
           ArrayRef<ParsedTemplateArgument> TemplateArgs, bool ArgsInvalid,
           SmallVectorImpl<TemplateIdAnnotation *> &CleanupList) {
      TemplateIdAnnotation *TemplateId = new (toolchain::safe_malloc(
          totalSizeToAlloc<ParsedTemplateArgument>(TemplateArgs.size())))
          TemplateIdAnnotation(TemplateKWLoc, TemplateNameLoc, Name,
                               OperatorKind, OpaqueTemplateName, TemplateKind,
                               LAngleLoc, RAngleLoc, TemplateArgs, ArgsInvalid);
      CleanupList.push_back(TemplateId);
      return TemplateId;
    }

    void Destroy() {
      for (ParsedTemplateArgument &A :
           toolchain::make_range(getTemplateArgs(), getTemplateArgs() + NumArgs))
        A.~ParsedTemplateArgument();
      this->~TemplateIdAnnotation();
      free(this);
    }

    /// Determine whether this might be a type template.
    bool mightBeType() const {
      return Kind == TNK_Non_template ||
             Kind == TNK_Type_template ||
             Kind == TNK_Dependent_template_name ||
             Kind == TNK_Undeclared_template;
    }

    bool hasInvalidName() const { return Kind == TNK_Non_template; }
    bool hasInvalidArgs() const { return ArgsInvalid; }

    bool isInvalid() const { return hasInvalidName() || hasInvalidArgs(); }

  private:
    TemplateIdAnnotation(const TemplateIdAnnotation &) = delete;

    TemplateIdAnnotation(SourceLocation TemplateKWLoc,
                         SourceLocation TemplateNameLoc,
                         const IdentifierInfo *Name,
                         OverloadedOperatorKind OperatorKind,
                         ParsedTemplateTy OpaqueTemplateName,
                         TemplateNameKind TemplateKind,
                         SourceLocation LAngleLoc, SourceLocation RAngleLoc,
                         ArrayRef<ParsedTemplateArgument> TemplateArgs,
                         bool ArgsInvalid) noexcept
        : TemplateKWLoc(TemplateKWLoc), TemplateNameLoc(TemplateNameLoc),
          Name(Name), Operator(OperatorKind), Template(OpaqueTemplateName),
          Kind(TemplateKind), LAngleLoc(LAngleLoc), RAngleLoc(RAngleLoc),
          NumArgs(TemplateArgs.size()), ArgsInvalid(ArgsInvalid) {

      toolchain::uninitialized_copy(TemplateArgs, getTemplateArgs());
    }
    ~TemplateIdAnnotation() = default;
  };

  /// Retrieves the range of the given template parameter lists.
  SourceRange getTemplateParamsRange(TemplateParameterList const *const *Params,
                                     unsigned NumParams);
} // end namespace language::Core

#endif // LANGUAGE_CORE_SEMA_PARSEDTEMPLATE_H
