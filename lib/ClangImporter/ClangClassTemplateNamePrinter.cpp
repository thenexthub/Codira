//===--- ClangClassTemplateNamePrinter.cpp --------------------------------===//
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

#include "ClangClassTemplateNamePrinter.h"
#include "ImporterImpl.h"
#include "language/ClangImporter/ClangImporter.h"
#include "clang/AST/TemplateArgumentVisitor.h"
#include "clang/AST/TypeVisitor.h"

using namespace language;
using namespace language::importer;

struct TemplateInstantiationNamePrinter
    : clang::TypeVisitor<TemplateInstantiationNamePrinter, std::string> {
  ASTContext &languageCtx;
  NameImporter *nameImporter;
  ImportNameVersion version;

  ClangImporter::Implementation *importerImpl;

  TemplateInstantiationNamePrinter(ASTContext &languageCtx,
                                   NameImporter *nameImporter,
                                   ImportNameVersion version,
                                   ClangImporter::Implementation *importerImpl)
      : languageCtx(languageCtx), nameImporter(nameImporter), version(version),
        importerImpl(importerImpl) {}

  std::string VisitType(const clang::Type *type) {
    // Print "_" as a fallback if we couldn't emit a more meaningful type name.
    return "_";
  }

  std::string VisitBuiltinType(const clang::BuiltinType *type) {
    switch (type->getKind()) {
    case clang::BuiltinType::Void:
      return "Void";
    case clang::BuiltinType::NullPtr:
      return "__cxxNullPtrT";

#define MAP_BUILTIN_TYPE(CLANG_BUILTIN_KIND, LANGUAGE_TYPE_NAME)                  \
    case clang::BuiltinType::CLANG_BUILTIN_KIND:                               \
      return #LANGUAGE_TYPE_NAME;
#include "language/ClangImporter/BuiltinMappedTypes.def"
    default:
      break;
    }

    return VisitType(type);
  }

  std::string VisitTagType(const clang::TagType *type) {
    auto tagDecl = type->getAsTagDecl();
    if (auto namedArg = dyn_cast_or_null<clang::NamedDecl>(tagDecl)) {
      if (auto typeDefDecl = tagDecl->getTypedefNameForAnonDecl())
        namedArg = typeDefDecl;
      toolchain::SmallString<128> storage;
      toolchain::raw_svector_ostream buffer(storage);

      // Print the fully-qualified type name.
      std::vector<DeclName> qualifiedNameComponents;
      auto unqualifiedName = nameImporter->importName(namedArg, version);
      qualifiedNameComponents.push_back(unqualifiedName.getDeclName());
      const clang::DeclContext *parentCtx =
          unqualifiedName.getEffectiveContext().getAsDeclContext();
      while (parentCtx) {
        if (auto namedParentDecl = dyn_cast<clang::NamedDecl>(parentCtx)) {
          // If this component of the fully-qualified name is a decl that is
          // imported into Codira, remember its name.
          auto componentName =
              nameImporter->importName(namedParentDecl, version);
          qualifiedNameComponents.push_back(componentName.getDeclName());
          parentCtx = componentName.getEffectiveContext().getAsDeclContext();
        } else {
          // If this component is not imported into Codira, skip it.
          parentCtx = parentCtx->getParent();
        }
      }

      toolchain::interleave(
          toolchain::reverse(qualifiedNameComponents),
          [&](const DeclName &each) { each.print(buffer); },
          [&]() { buffer << "."; });
      return buffer.str().str();
    }
    return "_";
  }

  std::string VisitPointerType(const clang::PointerType *type) {
    std::string pointeeResult = Visit(type->getPointeeType().getTypePtr());

    enum class TagTypeDecorator { None, UnsafePointer, UnsafeMutablePointer };

    // If this is a pointer to foreign reference type, we should not wrap
    // it in Unsafe(Mutable)?Pointer, since it will be imported as a class
    // in Codira.
    bool isReferenceType = false;
    if (auto tagDecl = type->getPointeeType()->getAsTagDecl()) {
      if (auto *rd = dyn_cast<clang::RecordDecl>(tagDecl))
        isReferenceType = recordHasReferenceSemantics(rd, importerImpl);
    }

    TagTypeDecorator decorator;
    if (!isReferenceType)
      decorator = type->getPointeeType().isConstQualified()
                      ? TagTypeDecorator::UnsafePointer
                      : TagTypeDecorator::UnsafeMutablePointer;
    else
      decorator = TagTypeDecorator::None;

    toolchain::SmallString<128> storage;
    toolchain::raw_svector_ostream buffer(storage);
    if (decorator != TagTypeDecorator::None)
      buffer << (decorator == TagTypeDecorator::UnsafePointer
                     ? "UnsafePointer"
                     : "UnsafeMutablePointer")
             << '<';
    buffer << pointeeResult;
    if (decorator != TagTypeDecorator::None)
      buffer << '>';

    return buffer.str().str();
  }

  std::string VisitFunctionProtoType(const clang::FunctionProtoType *type) {
    toolchain::SmallString<128> storage;
    toolchain::raw_svector_ostream buffer(storage);

    buffer << "((";
    toolchain::interleaveComma(type->getParamTypes(), buffer,
                          [&](const clang::QualType &paramType) {
                            buffer << Visit(paramType.getTypePtr());
                          });
    buffer << ") -> ";
    buffer << Visit(type->getReturnType().getTypePtr());
    buffer << ")";

    return buffer.str().str();
  }

  std::string VisitVectorType(const clang::VectorType *type) {
    return (Twine("SIMD") + std::to_string(type->getNumElements()) + "<" +
            Visit(type->getElementType().getTypePtr()) + ">")
        .str();
  }

  std::string VisitArrayType(const clang::ArrayType *type) {
    return (Twine("[") + Visit(type->getElementType().getTypePtr()) + "]")
        .str();
  }

  std::string VisitConstantArrayType(const clang::ConstantArrayType *type) {
    return (Twine("Vector<") + Visit(type->getElementType().getTypePtr()) +
            ", " + std::to_string(type->getSExtSize()) + ">")
        .str();
  }
};

struct TemplateArgumentPrinter
    : clang::ConstTemplateArgumentVisitor<TemplateArgumentPrinter, void,
                                          toolchain::raw_svector_ostream &> {
  TemplateInstantiationNamePrinter typePrinter;

  TemplateArgumentPrinter(ASTContext &languageCtx, NameImporter *nameImporter,
                          ImportNameVersion version,
                          ClangImporter::Implementation *importerImpl)
      : typePrinter(languageCtx, nameImporter, version, importerImpl) {}

  void VisitTemplateArgument(const clang::TemplateArgument &arg,
                             toolchain::raw_svector_ostream &buffer) {
    // Print "_" as a fallback if we couldn't emit a more meaningful type name.
    buffer << "_";
  }

  void VisitTypeTemplateArgument(const clang::TemplateArgument &arg,
                                 toolchain::raw_svector_ostream &buffer) {
    auto ty = arg.getAsType();

    if (ty.isConstQualified())
      buffer << "__cxxConst<";
    if (ty.isVolatileQualified())
      buffer << "__cxxVolatile<";

    buffer << typePrinter.Visit(ty.getTypePtr());

    if (ty.isVolatileQualified())
      buffer << ">";
    if (ty.isConstQualified())
      buffer << ">";
  }

  void VisitIntegralTemplateArgument(const clang::TemplateArgument &arg,
                                     toolchain::raw_svector_ostream &buffer) {
    buffer << "_";
    if (arg.getIntegralType()->isBuiltinType()) {
      buffer << typePrinter.Visit(arg.getIntegralType().getTypePtr()) << "_";
    }
    auto value = arg.getAsIntegral();
    if (value.isNegative()) {
      value.negate();
      buffer << "Neg_";
    }
    value.print(buffer, arg.getIntegralType()->isSignedIntegerType());
  }

  void VisitPackTemplateArgument(const clang::TemplateArgument &arg,
                                 toolchain::raw_svector_ostream &buffer) {
    VisitTemplateArgumentArray(arg.getPackAsArray(), buffer);
  }

  void VisitTemplateArgumentArray(ArrayRef<clang::TemplateArgument> args,
                                  toolchain::raw_svector_ostream &buffer) {
    bool needsComma = false;
    for (auto &arg : args) {
      // Do not try to print empty packs.
      if (arg.getKind() == clang::TemplateArgument::ArgKind::Pack &&
          arg.getPackAsArray().empty())
        continue;

      if (needsComma)
        buffer << ", ";
      Visit(arg, buffer);
      needsComma = true;
    }
  }
};

std::string language::importer::printClassTemplateSpecializationName(
    const clang::ClassTemplateSpecializationDecl *decl, ASTContext &languageCtx,
    NameImporter *nameImporter, ImportNameVersion version) {
  TemplateArgumentPrinter templateArgPrinter(languageCtx, nameImporter, version,
                                             nameImporter->getImporterImpl());

  toolchain::SmallString<128> storage;
  toolchain::raw_svector_ostream buffer(storage);
  decl->printName(buffer);
  buffer << "<";
  templateArgPrinter.VisitTemplateArgumentArray(
      decl->getTemplateArgs().asArray(), buffer);
  buffer << ">";
  return buffer.str().str();
}
