//===--- DeclarationFragmentPrinter.cpp - Declaration Fragment Printer ----===//
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

#include "language/AST/USRGeneration.h"
#include "language/Basic/Assertions.h"
#include "language/Parse/Token.h"
#include "DeclarationFragmentPrinter.h"
#include "SymbolGraphASTWalker.h"

using namespace language;
using namespace symbolgraphgen;

void DeclarationFragmentPrinter::openFragment(FragmentKind Kind) {
  assert(Kind != FragmentKind::None);
  if (this->Kind != Kind) {
    closeFragment();
    this->Kind = Kind,
    Spelling.clear();
    USR.clear();
  }
}

StringRef
DeclarationFragmentPrinter::getKindSpelling(FragmentKind Kind) const {
  switch (Kind) {
    case FragmentKind::Keyword:
      return "keyword";
    case FragmentKind::Attribute:
      return "attribute";
    case FragmentKind::NumberLiteral:
      return "number";
    case FragmentKind::StringLiteral:
      return "string";
    case FragmentKind::Identifier:
      return "identifier";
    case FragmentKind::TypeIdentifier:
      return "typeIdentifier";
    case FragmentKind::GenericParameter:
      return "genericParameter";
    case FragmentKind::InternalParam:
      return "internalParam";
    case FragmentKind::ExternalParam:
      return "externalParam";
    case FragmentKind::Text:
      return "text";
    case FragmentKind::None:
      toolchain_unreachable("Fragment kind of 'None' has no spelling");
  }
  toolchain_unreachable("invalid fragment kind");
}

void DeclarationFragmentPrinter::closeFragment() {
  if (Kind == FragmentKind::None) {
    return;
  }

  ++NumFragments;

  if (!Spelling.empty()) {
    OS.object([&](){
      OS.attribute("kind", getKindSpelling(Kind));
      OS.attribute("spelling", Spelling.str());
      if (!USR.empty()) {
        OS.attribute("preciseIdentifier", USR.str());
      }
    });
  }

  Spelling.clear();
  USR.clear();
  Kind = FragmentKind::None;
}

void DeclarationFragmentPrinter::printDeclLoc(const Decl *D) {
  switch (D->getKind()) {
    case DeclKind::Constructor:
    case DeclKind::Destructor:
    case DeclKind::Subscript:
      openFragment(FragmentKind::Keyword);
      break;
    default:
      openFragment(FragmentKind::Identifier);
      break;
  }
}

void
DeclarationFragmentPrinter::printNamePre(PrintNameContext Context) {
  switch (Context) {
  case PrintNameContext::Keyword:
  case PrintNameContext::IntroducerKeyword:
    openFragment(FragmentKind::Keyword);
    break;
  case PrintNameContext::GenericParameter:
    openFragment(FragmentKind::GenericParameter);
    break;
  case PrintNameContext::Attribute:
    openFragment(FragmentKind::Attribute);
    break;
  case PrintNameContext::ClassDynamicSelf:
  case PrintNameContext::FunctionParameterExternal:
    openFragment(FragmentKind::ExternalParam);
    break;
  case PrintNameContext::FunctionParameterLocal:
    openFragment(FragmentKind::InternalParam);
    break;
  case PrintNameContext::TupleElement:
  case PrintNameContext::TypeMember:
  case PrintNameContext::Normal:
    break;
  }
}

void DeclarationFragmentPrinter::printStructurePre(PrintStructureKind Kind,
                                                   const Decl *D) {
  switch (Kind) {
    case PrintStructureKind::NumberLiteral:
      openFragment(FragmentKind::NumberLiteral);
      break;
    case PrintStructureKind::StringLiteral:
      openFragment(FragmentKind::StringLiteral);
      break;
    default:
      break;
  }
}

void DeclarationFragmentPrinter::printTypeRef(Type T, const TypeDecl *RefTo,
    Identifier Name,
    PrintNameContext NameContext) {
  if (Kind != FragmentKind::Attribute) {
    openFragment(FragmentKind::TypeIdentifier);
  } else {
    // create a separate fragment so that only the attribute name is linkable
    closeFragment();
    openFragment(FragmentKind::Attribute);
  }
  printText(Name.str());
  USR.clear();

  auto ShouldLink = Name.str() != "Self";
  if (T) {
    if (const auto *TD = T->getAnyNominal()) {
      if (SG->isImplicitlyPrivate(TD)) {
        ShouldLink = false;
      }
    }

    if (T->isTypeParameter() && T->getKind() != TypeKind::DependentMember) {
      ShouldLink = false;
    }
  }

  if (ShouldLink) {
    toolchain::raw_svector_ostream OS(USR);
    ide::printDeclUSR(RefTo, OS);
    if (ReferencedDecls)
      ReferencedDecls->insert(RefTo);
  }
  closeFragment();
}

void DeclarationFragmentPrinter::printText(StringRef Text) {
  if (Kind == FragmentKind::None) {
    openFragment(FragmentKind::Text);
  }
  Spelling.append(Text);
}

void DeclarationFragmentPrinter::printAbridgedType(const GenericTypeDecl *TD,
                                                   bool PrintKeyword) {
  if (PrintKeyword) {
    openFragment(DeclarationFragmentPrinter::FragmentKind::Keyword);
    switch (TD->getKind()) {
      case DeclKind::Struct:
        printText(getTokenText(tok::kw_struct));
        break;
      case DeclKind::Enum:
        printText(getTokenText(tok::kw_enum));
        break;
      case DeclKind::Protocol:
        printText(getTokenText(tok::kw_protocol));
        break;
      case DeclKind::Class: {
        auto *refDecl = cast<ClassDecl>(TD);
        if (refDecl->isExplicitDistributedActor()) {
          printText("distributed actor");
        } else if (refDecl->isExplicitActor()) {
          printText("actor");
        } else {
          printText(getTokenText(tok::kw_class));
        }
      break;
      }
      case DeclKind::TypeAlias:
        printText(getTokenText(tok::kw_typealias));
        break;
      case DeclKind::OpaqueType:
        toolchain_unreachable("OpaqueType should not be in symbol graphs!");
      default:
        toolchain_unreachable("GenericTypeDecl kind not handled in DeclarationFragmentPrinter!");
    }

    openFragment(DeclarationFragmentPrinter::FragmentKind::Text);
    printText(" ");
  }

  openFragment(DeclarationFragmentPrinter::FragmentKind::Identifier);
  printText(TD->getNameStr());
}
