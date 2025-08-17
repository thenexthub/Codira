//===---------- IssueHash.cpp - Generate identification hashes --*- C++ -*-===//
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

#include "language/Core/Analysis/IssueHash.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Lex/Lexer.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/Twine.h"
#include "toolchain/Support/LineIterator.h"
#include "toolchain/Support/MD5.h"

#include <optional>
#include <sstream>
#include <string>

using namespace language::Core;

// Get a string representation of the parts of the signature that can be
// overloaded on.
static std::string GetSignature(const FunctionDecl *Target) {
  if (!Target)
    return "";
  std::string Signature;

  // When a flow sensitive bug happens in templated code we should not generate
  // distinct hash value for every instantiation. Use the signature from the
  // primary template.
  if (const FunctionDecl *InstantiatedFrom =
          Target->getTemplateInstantiationPattern())
    Target = InstantiatedFrom;

  if (!isa<CXXConstructorDecl>(Target) && !isa<CXXDestructorDecl>(Target) &&
      !isa<CXXConversionDecl>(Target))
    Signature.append(Target->getReturnType().getAsString()).append(" ");
  Signature.append(Target->getQualifiedNameAsString()).append("(");

  for (int i = 0, paramsCount = Target->getNumParams(); i < paramsCount; ++i) {
    if (i)
      Signature.append(", ");
    Signature.append(Target->getParamDecl(i)->getType().getAsString());
  }

  if (Target->isVariadic())
    Signature.append(", ...");
  Signature.append(")");

  const auto *TargetT =
      toolchain::dyn_cast_or_null<FunctionType>(Target->getType().getTypePtr());

  if (!TargetT || !isa<CXXMethodDecl>(Target))
    return Signature;

  if (TargetT->isConst())
    Signature.append(" const");
  if (TargetT->isVolatile())
    Signature.append(" volatile");
  if (TargetT->isRestrict())
    Signature.append(" restrict");

  if (const auto *TargetPT =
          dyn_cast_or_null<FunctionProtoType>(Target->getType().getTypePtr())) {
    switch (TargetPT->getRefQualifier()) {
    case RQ_LValue:
      Signature.append(" &");
      break;
    case RQ_RValue:
      Signature.append(" &&");
      break;
    default:
      break;
    }
  }

  return Signature;
}

static std::string GetEnclosingDeclContextSignature(const Decl *D) {
  if (!D)
    return "";

  if (const auto *ND = dyn_cast<NamedDecl>(D)) {
    std::string DeclName;

    switch (ND->getKind()) {
    case Decl::Namespace:
    case Decl::Record:
    case Decl::CXXRecord:
    case Decl::Enum:
      DeclName = ND->getQualifiedNameAsString();
      break;
    case Decl::CXXConstructor:
    case Decl::CXXDestructor:
    case Decl::CXXConversion:
    case Decl::CXXMethod:
    case Decl::Function:
      DeclName = GetSignature(dyn_cast_or_null<FunctionDecl>(ND));
      break;
    case Decl::ObjCMethod:
      // ObjC Methods can not be overloaded, qualified name uniquely identifies
      // the method.
      DeclName = ND->getQualifiedNameAsString();
      break;
    default:
      break;
    }

    return DeclName;
  }

  return "";
}

static StringRef GetNthLineOfFile(std::optional<toolchain::MemoryBufferRef> Buffer,
                                  int Line) {
  if (!Buffer)
    return "";

  toolchain::line_iterator LI(*Buffer, false);
  for (; !LI.is_at_eof() && LI.line_number() != Line; ++LI)
    ;

  return *LI;
}

static std::string NormalizeLine(const SourceManager &SM, const FullSourceLoc &L,
                                 const LangOptions &LangOpts) {
  static StringRef Whitespaces = " \t\n";

  StringRef Str = GetNthLineOfFile(SM.getBufferOrNone(L.getFileID(), L),
                                   L.getExpansionLineNumber());
  StringRef::size_type col = Str.find_first_not_of(Whitespaces);
  if (col == StringRef::npos)
    col = 1; // The line only contains whitespace.
  else
    col++;
  SourceLocation StartOfLine =
      SM.translateLineCol(SM.getFileID(L), L.getExpansionLineNumber(), col);
  std::optional<toolchain::MemoryBufferRef> Buffer =
      SM.getBufferOrNone(SM.getFileID(StartOfLine), StartOfLine);
  if (!Buffer)
    return {};

  const char *BufferPos = SM.getCharacterData(StartOfLine);

  Token Token;
  Lexer Lexer(SM.getLocForStartOfFile(SM.getFileID(StartOfLine)), LangOpts,
              Buffer->getBufferStart(), BufferPos, Buffer->getBufferEnd());

  size_t NextStart = 0;
  std::ostringstream LineBuff;
  while (!Lexer.LexFromRawLexer(Token) && NextStart < 2) {
    if (Token.isAtStartOfLine() && NextStart++ > 0)
      continue;
    LineBuff << std::string(SM.getCharacterData(Token.getLocation()),
                            Token.getLength());
  }

  return LineBuff.str();
}

static toolchain::SmallString<32> GetMD5HashOfContent(StringRef Content) {
  toolchain::MD5 Hash;
  toolchain::MD5::MD5Result MD5Res;
  SmallString<32> Res;

  Hash.update(Content);
  Hash.final(MD5Res);
  toolchain::MD5::stringifyResult(MD5Res, Res);

  return Res;
}

std::string language::Core::getIssueString(const FullSourceLoc &IssueLoc,
                                  StringRef CheckerName,
                                  StringRef WarningMessage,
                                  const Decl *IssueDecl,
                                  const LangOptions &LangOpts) {
  static StringRef Delimiter = "$";

  return (toolchain::Twine(CheckerName) + Delimiter +
          GetEnclosingDeclContextSignature(IssueDecl) + Delimiter +
          Twine(IssueLoc.getExpansionColumnNumber()) + Delimiter +
          NormalizeLine(IssueLoc.getManager(), IssueLoc, LangOpts) +
          Delimiter + WarningMessage)
      .str();
}

SmallString<32> language::Core::getIssueHash(const FullSourceLoc &IssueLoc,
                                    StringRef CheckerName,
                                    StringRef WarningMessage,
                                    const Decl *IssueDecl,
                                    const LangOptions &LangOpts) {

  return GetMD5HashOfContent(getIssueString(
      IssueLoc, CheckerName, WarningMessage, IssueDecl, LangOpts));
}
