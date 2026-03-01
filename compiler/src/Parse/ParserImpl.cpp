/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

/**
 * @file
 *
 * This file implements API of class Parser by encapsulating API of ParserImpl.
 */

#include "ParserImpl.h"
#include "Codira/AST/Match.h"

namespace Codira {
OwnedPtr<AST::File> Parser::ParseTopLevel()
{
    return impl->ParseTopLevel();
}
OwnedPtr<AST::Decl> Parser::ParseDecl(ScopeKind scopeKind)
{
    return impl->ParseDecl(scopeKind, {}, {});
}

OwnedPtr<AST::Expr> Parser::ParseExpr()
{
    return impl->ParseExpr();
}
OwnedPtr<AST::Expr> Parser::ParseExprLibast()
{
    return impl->ParseExpr(ExprKind::UNKNOWN_EXPR);
}
OwnedPtr<AST::Type> Parser::ParseType()
{
    return impl->ParseType();
}
OwnedPtr<AST::Pattern> Parser::ParsePattern()
{
    return impl->ParsePattern();
}
std::vector<OwnedPtr<AST::Node>> Parser::ParseNodes(std::variant<ScopeKind, ExprKind> scope,
    AST::Node& currentMacroCall, const std::set<AST::Modifier>& modifiers,
    std::vector<OwnedPtr<AST::Annotation>> annos)
{
    return impl->ParseNodes(scope, currentMacroCall, modifiers, std::move(annos));
}
void Parser::ParseAnnotationArguments(AST::Annotation& anno) const
{
    return impl->ParseAnnotationArguments(anno);
}

OwnedPtr<AST::Annotation> Parser::ParseCustomAnnotation() const
{
    return impl->ParseCustomAnnotation();
}

DiagnosticEngine& Parser::GetDiagnosticEngine() const
{
    return impl->diag;
}
std::size_t Parser::GetProcessedTokens() const
{
    return impl->GetProcessedTokens();
}
std::string Parser::GetPrimaryDeclIdentRawValue() const
{
    return impl->GetPrimaryDeclIdentRawValue();
}
Parser& Parser::SetPrimaryDecl(const std::string& decl)
{
    impl->SetPrimaryDecl(decl);
    return *this;
}
size_t Parser::GetLineNum() const
{
    return impl->GetLineNum();
}
Parser& Parser::SetModuleName(const std::string& name)
{
    impl->moduleName = name;
    return *this;
}
Parser& Parser::SetForImport(bool isForImport)
{
    impl->forImport = isForImport;
    return *this;
}
Parser& Parser::SetCurFile(Ptr<AST::File> curFile)
{
    impl->currentFile = curFile;
    return *this;
}

Parser& Parser::EnableCustomAnno()
{
    impl->enableCustomAnno = true;
    return *this;
}

Parser& Parser::SetEHEnabled(bool enabled)
{
    impl->enableEH = enabled;
    impl->lexer->SetEHEnabled(enabled);
    return *this;
}

bool Parser::IsEHEnabled() const
{
    return impl->enableEH;
}

TokenVecMap Parser::GetCommentsMap() const
{
    return impl->commentsMap;
}

void Parser::SetCompileOptions(const GlobalOptions& opts)
{
    impl->backend = opts.backend;
    impl->scanDepPkg = opts.scanDepPkg;
    impl->calculateLineNum = opts.enableTimer || opts.enableMemoryCollect;
    impl->enableInteropCODEMapping = opts.enableInteropCODEMapping;
    // set compile options for codemp implementation
    impl->mpImpl->SetCompileOptions(opts);
    // Effect handlers break backwards compatibility by introducing new
    // keywords, so we disable them from the parser unless the user
    // explicitly asks to compile with effect handler support
    SetEHEnabled(opts.enableEH);
}

bool Parser::Skip(TokenKind kind)
{
    return impl->Skip(kind);
}

const Token& Parser::Peek()
{
    return impl->Peek();
}
void Parser::Next()
{
    return impl->Next();
}
bool Parser::Seeing(TokenKind kind)
{
    return impl->Seeing(kind);
}
bool Parser::Seeing(TokenKind rangeLeft, TokenKind rangeRight)
{
    return impl->Seeing(rangeLeft, rangeRight);
}
bool Parser::SeeingAny(const std::vector<TokenKind>& kinds)
{
    return impl->SeeingAny(kinds);
}
bool Parser::Seeing(const std::vector<TokenKind>& kinds, bool skipNewline)
{
    return impl->Seeing(kinds, skipNewline);
}

bool Parser::SeeingCombinator(const std::vector<TokenKind>& kinds)
{
    return impl->SeeingCombinator(kinds);
}

bool Parser::SeeingTokenAndCombinator(TokenKind kind, const std::vector<TokenKind>& cmb)
{
    return impl->SeeingTokenAndCombinator(kind, cmb);
}

void Parser::SkipCombinator(const std::vector<TokenKind>& kinds)
{
    return impl->SkipCombinator(kinds);
}

const Token& Parser::LookAhead() const
{
    return impl->lookahead;
}
const Token& Parser::LastToken() const
{
    return impl->lastToken;
}

Ptr<Node> Parser::CurMacroCall() const
{
    return impl->curMacroCall;
}

Parser::~Parser()
{
    delete impl;
}

// implementation of ParserImpl
ParserImpl::ParserImpl(unsigned int fileID, const std::string& input, DiagnosticEngine& diag, SourceManager& sm,
    bool attachComment, bool parsingDeclFiles)
    : diag(diag), sourceManager(sm),
      lexer{std::make_unique<Lexer>(fileID, input, diag, sm, attachComment)},
      enableAttachComment(attachComment), parseDeclFile{parsingDeclFiles}, mpImpl{new MPParserImpl(*this)},
      ffiParser{new FFIParserImpl(*this)}
{
    // do not call EnterXXXMod here because that would then call enter that mod in lexer, but lexer would initialize
    // itself
    ctx.push_back(ParserContext::NORMAL);
}

ParserImpl::ParserImpl(const std::string& input, DiagnosticEngine& diag, SourceManager& sm, const Position& pos,
    bool attachComment, bool parsingDeclFiles)
    : diag(diag), sourceManager(sm),
      lexer{std::make_unique<Lexer>(input, diag, sm, pos, attachComment)},
      enableAttachComment{attachComment}, parseDeclFile{parsingDeclFiles}, mpImpl{new MPParserImpl(*this)},
      ffiParser{new FFIParserImpl(*this)}
{
    ctx.push_back(ParserContext::NORMAL);
}

ParserImpl::ParserImpl(const std::vector<Token>& inputTokens, DiagnosticEngine& diag, SourceManager& sm,
    bool attachComment, bool parsingDeclFiles)
    : diag(diag), sourceManager(sm),
      lexer{std::make_unique<Lexer>(inputTokens, diag, sm, attachComment)},
      enableAttachComment{attachComment}, parseDeclFile{parsingDeclFiles}, mpImpl{new MPParserImpl(*this)},
      ffiParser{new FFIParserImpl(*this)}
{
    ctx.push_back(ParserContext::NORMAL);
}

ParserImpl::~ParserImpl()
{
    delete mpImpl;
    mpImpl = nullptr;

    delete ffiParser;
    ffiParser = nullptr;
}

/**
 * Checks whether a member decl can be an abstract by context and already parsed info
 * @param decl member declaration
 */
bool ParserImpl::CanBeAbstract(const AST::Decl& decl, ScopeKind scopeKind) const
{
    auto pdecl = Ptr(&decl);
    switch (decl.astKind) {
        case ASTKind::FUNC_DECL: {
            auto fd = StaticAs<ASTKind::FUNC_DECL>(pdecl);
            if (fd->funcBody && fd->funcBody->body) {
                return false;
            }
            break;
        }
        case ASTKind::PROP_DECL: {
            auto pd = StaticAs<ASTKind::PROP_DECL>(pdecl);
            if (!pd->getters.empty() || !pd->setters.empty()) {
                return false;
            }
            break;
        }
        case ASTKind::PRIMARY_CTOR_DECL: {
            auto pcd = StaticAs<ASTKind::PRIMARY_CTOR_DECL>(pdecl);
            if (pcd->funcBody && pcd->funcBody->body) {
                return false;
            }
            break;
        }
        default: break;
    }

    if (scopeKind == ScopeKind::INTERFACE_BODY) {
        return true;
    }
    // modify to support common abstract
    if (scopeKind == ScopeKind::CLASS_BODY && !decl.TestAttr(Attribute::COMMON)) {
        return true;
    }
    return false;
}

void ParserImpl::CheckConstructorBody(AST::FuncDecl& ctor, ScopeKind scopeKind, bool inMacro)
{
    CODEC_ASSERT(ctor.TestAttr(Attribute::CONSTRUCTOR));
    if (ctor.funcBody && ctor.funcBody->retType) {
        ParseDiagnoseRefactor(
            DiagKindRefactor::parse_invalid_return_type, *ctor.funcBody->retType, "constructor");
        ctor.EnableAttr(Attribute::HAS_BROKEN);
    }
    /*
        If constructor parsed in macro or via libast does not have a body, then it's broken.
        If constructor is in scope of class/struct and it does not have a body, then it's acceptable*.
        * More concrete conditions are be checked during class/struct body parsing.
    */
    auto isInClassLike = scopeKind == ScopeKind::CLASS_BODY || scopeKind == ScopeKind::STRUCT_BODY;
    if ((!isInClassLike || inMacro) && (!ctor.funcBody || !ctor.funcBody->body) && !ctor.TestAttr(Attribute::COMMON)) {
        DiagMissingBody("constructor", "", ctor.end);
        ctor.EnableAttr(Attribute::HAS_BROKEN);
    }
}
}
