//===- NestedNameSpecifier.cpp - C++ nested name specifiers ---------------===//
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
//  This file defines the NestedNameSpecifier class, which represents
//  a C++ nested-name-specifier.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/NestedNameSpecifier.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/DependenceFlags.h"
#include "language/Core/AST/PrettyPrinter.h"
#include "language/Core/AST/TemplateName.h"
#include "language/Core/AST/Type.h"
#include "language/Core/AST/TypeLoc.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/LangOptions.h"
#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>

using namespace language::Core;

const NamespaceAndPrefixStorage *
NestedNameSpecifier::MakeNamespaceAndPrefixStorage(
    const ASTContext &Ctx, const NamespaceBaseDecl *Namespace,
    NestedNameSpecifier Prefix) {
  toolchain::FoldingSetNodeID ID;
  NamespaceAndPrefixStorage::Profile(ID, Namespace, Prefix);

  void *InsertPos = nullptr;
  NamespaceAndPrefixStorage *S =
      Ctx.NamespaceAndPrefixStorages.FindNodeOrInsertPos(ID, InsertPos);
  if (!S) {
    S = new (Ctx, alignof(NamespaceAndPrefixStorage))
        NamespaceAndPrefixStorage(Namespace, Prefix);
    Ctx.NamespaceAndPrefixStorages.InsertNode(S, InsertPos);
  }
  return S;
}

bool NestedNameSpecifier::isFullyQualified() const {
  switch (getKind()) {
  case NestedNameSpecifier::Kind::Global:
    return true;
  case NestedNameSpecifier::Kind::Null:
  case NestedNameSpecifier::Kind::MicrosoftSuper:
    return false;
  case NestedNameSpecifier::Kind::Namespace:
    return getAsNamespaceAndPrefix().Prefix.isFullyQualified();
  case NestedNameSpecifier::Kind::Type:
    return getAsType()->getPrefix().isFullyQualified();
  }
  toolchain_unreachable("Invalid NNS Kind!");
}

NestedNameSpecifierDependence NestedNameSpecifier::getDependence() const {
  switch (getKind()) {
  case Kind::Null:
  case Kind::Global:
  case Kind::Namespace:
    return NestedNameSpecifierDependence::None;
  case Kind::MicrosoftSuper: {
    CXXRecordDecl *RD = getAsMicrosoftSuper();
    return RD->isDependentContext()
               ? NestedNameSpecifierDependence::DependentInstantiation |
                     NestedNameSpecifierDependence::Dependent
               : NestedNameSpecifierDependence::None;
  }
  case Kind::Type:
    return toNestedNameSpecifierDependence(getAsType()->getDependence());
  }
  toolchain_unreachable("Invalid NNS Kind!");
}

/// Print this nested name specifier to the given output
/// stream.
void NestedNameSpecifier::print(raw_ostream &OS, const PrintingPolicy &Policy,
                                bool ResolveTemplateArguments,
                                bool PrintFinalScopeResOp) const {
  switch (getKind()) {
  case Kind::Namespace: {
    auto [Namespace, Prefix] = getAsNamespaceAndPrefix();
    Prefix.print(OS, Policy);
    if (const auto *NS = dyn_cast<NamespaceDecl>(Namespace)) {
      assert(!NS->isAnonymousNamespace());
      OS << NS->getName();
    } else {
      OS << cast<NamespaceAliasDecl>(Namespace)->getName();
    }
    break;
  }
  case Kind::Global:
    OS << "::";
    return;
  case Kind::MicrosoftSuper:
    OS << "__super";
    break;
  case Kind::Type: {
    PrintingPolicy InnerPolicy(Policy);
    InnerPolicy.SuppressTagKeyword = true;
    QualType(getAsType(), 0).print(OS, InnerPolicy);
    break;
  }
  case Kind::Null:
    return;
  }
  if (PrintFinalScopeResOp)
    OS << "::";
}

LLVM_DUMP_METHOD void NestedNameSpecifier::dump(toolchain::raw_ostream *OS,
                                                const LangOptions *LO) const {
  print(OS ? *OS : toolchain::errs(), LO ? *LO : LangOptions());
}

LLVM_DUMP_METHOD void NestedNameSpecifier::dump(const LangOptions &LO) const {
  dump(/*OS=*/nullptr, &LO);
}
LLVM_DUMP_METHOD void NestedNameSpecifier::dump(toolchain::raw_ostream &OS) const {
  dump(&OS);
}
LLVM_DUMP_METHOD void NestedNameSpecifier::dump(toolchain::raw_ostream &OS,
                                                const LangOptions &LO) const {
  dump(&OS, &LO);
}

SourceLocation NestedNameSpecifierLoc::getBeginLoc() const {
  if (!Qualifier)
    return SourceLocation();

  NestedNameSpecifierLoc First = *this;
  while (NestedNameSpecifierLoc Prefix = First.getAsNamespaceAndPrefix().Prefix)
    First = Prefix;
  return First.getLocalSourceRange().getBegin();
}

static void Append(char *Start, char *End, char *&Buffer, unsigned &BufferSize,
                   unsigned &BufferCapacity) {
  if (Start == End)
    return;

  if (BufferSize + (End - Start) > BufferCapacity) {
    // Reallocate the buffer.
    unsigned NewCapacity = std::max(
        (unsigned)(BufferCapacity ? BufferCapacity * 2 : sizeof(void *) * 2),
        (unsigned)(BufferSize + (End - Start)));
    if (!BufferCapacity) {
      char *NewBuffer = static_cast<char *>(toolchain::safe_malloc(NewCapacity));
      if (Buffer)
        memcpy(NewBuffer, Buffer, BufferSize);
      Buffer = NewBuffer;
    } else {
      Buffer = static_cast<char *>(toolchain::safe_realloc(Buffer, NewCapacity));
    }
    BufferCapacity = NewCapacity;
  }
  assert(Buffer && Start && End && End > Start && "Illegal memory buffer copy");
  memcpy(Buffer + BufferSize, Start, End - Start);
  BufferSize += End - Start;
}

/// Save a source location to the given buffer.
static void SaveSourceLocation(SourceLocation Loc, char *&Buffer,
                               unsigned &BufferSize, unsigned &BufferCapacity) {
  SourceLocation::UIntTy Raw = Loc.getRawEncoding();
  Append(reinterpret_cast<char *>(&Raw),
         reinterpret_cast<char *>(&Raw) + sizeof(Raw), Buffer, BufferSize,
         BufferCapacity);
}

/// Save a pointer to the given buffer.
static void SavePointer(void *Ptr, char *&Buffer, unsigned &BufferSize,
                        unsigned &BufferCapacity) {
  Append(reinterpret_cast<char *>(&Ptr),
         reinterpret_cast<char *>(&Ptr) + sizeof(void *),
         Buffer, BufferSize, BufferCapacity);
}

NestedNameSpecifierLocBuilder::
NestedNameSpecifierLocBuilder(const NestedNameSpecifierLocBuilder &Other)
    : Representation(Other.Representation) {
  if (!Other.Buffer)
    return;

  if (Other.BufferCapacity == 0) {
    // Shallow copy is okay.
    Buffer = Other.Buffer;
    BufferSize = Other.BufferSize;
    return;
  }

  // Deep copy
  Append(Other.Buffer, Other.Buffer + Other.BufferSize, Buffer, BufferSize,
         BufferCapacity);
}

NestedNameSpecifierLocBuilder &
NestedNameSpecifierLocBuilder::
operator=(const NestedNameSpecifierLocBuilder &Other) {
  Representation = Other.Representation;

  if (Buffer && Other.Buffer && BufferCapacity >= Other.BufferSize) {
    // Re-use our storage.
    BufferSize = Other.BufferSize;
    memcpy(Buffer, Other.Buffer, BufferSize);
    return *this;
  }

  // Free our storage, if we have any.
  if (BufferCapacity) {
    free(Buffer);
    BufferCapacity = 0;
  }

  if (!Other.Buffer) {
    // Empty.
    Buffer = nullptr;
    BufferSize = 0;
    return *this;
  }

  if (Other.BufferCapacity == 0) {
    // Shallow copy is okay.
    Buffer = Other.Buffer;
    BufferSize = Other.BufferSize;
    return *this;
  }

  // Deep copy.
  BufferSize = 0;
  Append(Other.Buffer, Other.Buffer + Other.BufferSize, Buffer, BufferSize,
         BufferCapacity);
  return *this;
}

void NestedNameSpecifierLocBuilder::Make(ASTContext &Context, TypeLoc TL,
                                         SourceLocation ColonColonLoc) {
  assert(!Representation);
  Representation = NestedNameSpecifier(TL.getTypePtr());

  // Push source-location info into the buffer.
  SavePointer(TL.getOpaqueData(), Buffer, BufferSize, BufferCapacity);
  SaveSourceLocation(ColonColonLoc, Buffer, BufferSize, BufferCapacity);
}

void NestedNameSpecifierLocBuilder::Extend(ASTContext &Context,
                                           const NamespaceBaseDecl *Namespace,
                                           SourceLocation NamespaceLoc,
                                           SourceLocation ColonColonLoc) {
  Representation = NestedNameSpecifier(Context, Namespace, Representation);

  // Push source-location info into the buffer.
  SaveSourceLocation(NamespaceLoc, Buffer, BufferSize, BufferCapacity);
  SaveSourceLocation(ColonColonLoc, Buffer, BufferSize, BufferCapacity);
}

void NestedNameSpecifierLocBuilder::MakeGlobal(ASTContext &Context,
                                               SourceLocation ColonColonLoc) {
  assert(!Representation && "Already have a nested-name-specifier!?");
  Representation = NestedNameSpecifier::getGlobal();

  // Push source-location info into the buffer.
  SaveSourceLocation(ColonColonLoc, Buffer, BufferSize, BufferCapacity);
}

void NestedNameSpecifierLocBuilder::MakeMicrosoftSuper(
    ASTContext &Context, CXXRecordDecl *RD, SourceLocation SuperLoc,
    SourceLocation ColonColonLoc) {
  Representation = NestedNameSpecifier(RD);

  // Push source-location info into the buffer.
  SaveSourceLocation(SuperLoc, Buffer, BufferSize, BufferCapacity);
  SaveSourceLocation(ColonColonLoc, Buffer, BufferSize, BufferCapacity);
}

void NestedNameSpecifierLocBuilder::PushTrivial(ASTContext &Context,
                                                NestedNameSpecifier Qualifier,
                                                SourceRange R) {
  // Construct bogus (but well-formed) source information for the
  // nested-name-specifier.
  switch (Qualifier.getKind()) {
  case NestedNameSpecifier::Kind::Null:
    return;
  case NestedNameSpecifier::Kind::Namespace: {
    auto [_1, Prefix] = Qualifier.getAsNamespaceAndPrefix();
    PushTrivial(Context, Prefix, R.getBegin());
    SaveSourceLocation(R.getBegin(), Buffer, BufferSize, BufferCapacity);
    break;
  }
  case NestedNameSpecifier::Kind::Type: {
    TypeSourceInfo *TSInfo = Context.getTrivialTypeSourceInfo(
        QualType(Qualifier.getAsType(), 0), R.getBegin());
    SavePointer(TSInfo->getTypeLoc().getOpaqueData(), Buffer, BufferSize,
                BufferCapacity);
    break;
  }
  case NestedNameSpecifier::Kind::Global:
  case NestedNameSpecifier::Kind::MicrosoftSuper:
    break;
  }
  SaveSourceLocation(R.getEnd(), Buffer, BufferSize, BufferCapacity);
}

void NestedNameSpecifierLocBuilder::Adopt(NestedNameSpecifierLoc Other) {
  if (BufferCapacity)
    free(Buffer);

  if (!Other) {
    Representation = std::nullopt;
    BufferSize = 0;
    return;
  }

  // Rather than copying the data (which is wasteful), "adopt" the
  // pointer (which points into the ASTContext) but set the capacity to zero to
  // indicate that we don't own it.
  Representation = Other.getNestedNameSpecifier();
  Buffer = static_cast<char *>(Other.getOpaqueData());
  BufferSize = Other.getDataLength();
  BufferCapacity = 0;
}

NestedNameSpecifierLoc
NestedNameSpecifierLocBuilder::getWithLocInContext(ASTContext &Context) const {
  if (!Representation)
    return NestedNameSpecifierLoc();

  // If we adopted our data pointer from elsewhere in the AST context, there's
  // no need to copy the memory.
  if (BufferCapacity == 0)
    return NestedNameSpecifierLoc(Representation, Buffer);

  // FIXME: After copying the source-location information, should we free
  // our (temporary) buffer and adopt the ASTContext-allocated memory?
  // Doing so would optimize repeated calls to getWithLocInContext().
  void *Mem = Context.Allocate(BufferSize, alignof(void *));
  memcpy(Mem, Buffer, BufferSize);
  return NestedNameSpecifierLoc(Representation, Mem);
}
