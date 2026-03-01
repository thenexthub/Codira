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

#include "TypeHierarchyImpl.h"
#include "../../CompilerCodiraProject.h"

using namespace std;
using namespace Codira;
using namespace Codira::AST;
using namespace Codira::Meta;
namespace {
    const std::unordered_map<ASTKind, ark::SymbolKind> AST2Symbol = {
        {ASTKind::CLASS_DECL,     ark::SymbolKind::CLASS},
        {ASTKind::INTERFACE_DECL, ark::SymbolKind::INTERFACE_DECL},
        {ASTKind::ENUM_DECL,      ark::SymbolKind::ENUM},
        {ASTKind::STRUCT_DECL,    ark::SymbolKind::STRUCT}
    };
    const vector<std::string> kernelLib = {"cangjieJS", "compress", "crypto", "encoding", "net", "numeric",
                                           "serialization", "std"};
}

namespace ark {
std::string TypeHierarchyImpl::curFilePath = "";
Position TypeHierarchyImpl::curPos;

bool IsInheritableDecl(const Decl &decl)
{
    return decl.astKind == ASTKind::CLASS_DECL || decl.astKind == ASTKind::INTERFACE_DECL ||
           decl.astKind == ASTKind::EXTEND_DECL || decl.astKind == ASTKind::ENUM_DECL ||
           decl.astKind == ASTKind::STRUCT_DECL;
}
// get TypeHierarchyItem by Decl
TypeHierarchyItem TypeHierarchyImpl::TypeHierarchyFrom(Ptr<const Decl> decl)
{
    TypeHierarchyItem result;
    if (decl != nullptr && IsInheritableDecl(*decl) && AST2Symbol.find(decl->astKind) != AST2Symbol.end()) {
        string path = decl->curFile->filePath;
        CompilerCodiraProject::GetInstance()->GetRealPath(path);
        result.uri.file = URI::URIFromAbsolutePath(path).ToString();
        result.kind = AST2Symbol.find(decl->astKind)->second;
        result.name = decl->identifier;
        result.range.start = decl->begin;
        result.range.end = decl->end;
        int length = static_cast<int>(CountUnicodeCharacters(decl->identifier));
        if (!decl->identifierForLsp.empty()) {
            length = static_cast<int>(CountUnicodeCharacters(decl->identifierForLsp));
        }
        Range range = GetDeclRange(*decl, length);
        // deal Chinese
        ArkAST *arkAst = CompilerCodiraProject::GetInstance()->GetArkAST(path);
        if (arkAst != nullptr) {
            PositionUTF8ToIDE(arkAst->tokens, result.range.start, *decl);
            PositionUTF8ToIDE(arkAst->tokens, result.range.end, *decl);
            PositionUTF8ToIDE(arkAst->tokens, range.start, *decl);
            PositionUTF8ToIDE(arkAst->tokens, range.end, *decl);
        }
        result.selectionRange = TransformFromChar2IDE(range);
        result.range = TransformFromChar2IDE(result.range);
        // deal kernel lib
        for (const auto &item : kernelLib) {
            if (decl->fullPackageName.rfind(item, 0) == 0) {
                result.uri.file = URI::URIFromAbsolutePath(curFilePath).ToString();
                result.selectionRange.start = curPos;
                result.selectionRange.end.line = curPos.line;
                result.selectionRange.end.column = curPos.column + 1;
                result.isKernel = true;
                break;
            }
        }
    }
    return result;
}

void DealTypeAliasDecl(Ptr<Decl> &decl)
{   /*
        //TypeAliasDecl
        class CP{
        }
        type NEWCP = CP // supoort this NEWCP Position typehierarchy
    */
    if (decl == nullptr) {
        return;
    }
    if (ark::Is<Codira::AST::TypeAliasDecl>(decl.get())) {
        auto typeAliasDecl = dynamic_cast<const TypeAliasDecl*>(decl.get());
        if (!typeAliasDecl || !typeAliasDecl->type) { return; }
        decl = ItemResolverUtil::GetDeclByTy(typeAliasDecl->type->ty);
    }
}

// prepareTypeHierarchy entrance
void TypeHierarchyImpl::FindTypeHierarchyImpl(const ark::ArkAST &ast, TypeHierarchyItem &result, Codira::Position pos)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "TypeHierarchyImpl::FindTypeHierarchyImpl in.");
    // update pos fileID
    pos.fileID = ast.fileID;
    curPos.column = pos.column;
    curPos.line = pos.line;
    curPos.fileID = pos.fileID;
    if (ast.file == nullptr) {
        return;
    }
    curFilePath = ast.file->filePath;
    // adjust position from IDE to AST
    pos = PosFromIDE2Char(pos);
    PositionIDEToUTF8(ast.tokens, pos, *ast.file);
    std::vector<Symbol *> syms;
    std::vector<Ptr<Codira::AST::Decl>> decls;
    Ptr<Decl> decl = ast.GetDeclByPosition(pos, syms, decls, {true, false});
    DealTypeAliasDecl(decl);
    if (decl == nullptr) {
        return;
    }
    /*
        primary constructor/constructor
        class CP{
            CP(){}
        }
        var cp = CP() // supoort this CP Position typehierarchy
    */
    if (decl->TestAttr(Codira::AST::Attribute::PRIMARY_CONSTRUCTOR) ||
        decl->TestAttr(Codira::AST::Attribute::CONSTRUCTOR)) {
        Ptr<const Decl> realDecl = decl->outerDecl;
        result = TypeHierarchyFrom(realDecl);
        result.isChildOrSuper = false;
        return;
    }
    if (!IsInheritableDecl(*decl)) {
        return;
    }
    result = TypeHierarchyFrom(decl);
    result.isChildOrSuper = false;
    result.symbolId = GetSymbolId(*decl);
}

// supertypes entrance
void TypeHierarchyImpl::FindSuperTypesImpl(std::vector<TypeHierarchyItem> &results,
                                           const TypeHierarchyItem &hierarchyItem)
{
    Trace::Log("TypeHierarchyImpl::FindSuperTypesImpl in.");
    if (hierarchyItem.isKernel && hierarchyItem.isChildOrSuper) {
        return;
    }
    if (hierarchyItem.symbolId == lsp::INVALID_SYMBOL_ID) {
        return;
    }

    auto index = CompilerCodiraProject::GetInstance()->GetIndex();
    if (!index) {
        return;
    }

    auto id = hierarchyItem.symbolId;
    std::unordered_set<lsp::SymbolID> superTypeIds;

    lsp::RelationsRequest inheritReq{id, lsp::RelationKind::BASE_OF};
    index->Relations(inheritReq, [&inheritReq, &superTypeIds](const lsp::Relation &spo) {
        // Subject    P    Object
        // ^               ^
        // SubType         SuperType
        if (spo.object == inheritReq.id && spo.predicate == inheritReq.predicate) {
            superTypeIds.emplace(spo.subject);
        }
    });

    lsp::RelationsRequest extendReq{id, lsp::RelationKind::EXTEND};
    index->Relations(extendReq, [&extendReq, &superTypeIds](const lsp::Relation &spo) {
        // Subject    P    Object
        // ^               ^
        // SubType         SuperType
        if (spo.object == extendReq.id && spo.predicate == extendReq.predicate) {
            superTypeIds.emplace(spo.subject);
        }
    });

    lsp::LookupRequest lookupReq{superTypeIds};
    index->Lookup(lookupReq, [&results](const lsp::Symbol &sym) {
        TypeHierarchyItem item;
        item.uri.file = URI::URIFromAbsolutePath(sym.location.fileUri).ToString();
        item.kind = AST2Symbol.find(sym.kind)->second;
        item.name = sym.name;
        auto range = TransformFromChar2IDE({sym.location.begin, sym.location.end});
        item.selectionRange = item.range = range;
        item.isKernel = false;
        item.symbolId = sym.id;
        results.emplace_back(item);
    });
}

// subtypes entrance
void TypeHierarchyImpl::FindSubTypesImpl(std::vector<TypeHierarchyItem> &results,
                                         const TypeHierarchyItem &hierarchyItem)
{
    Trace::Log("TypeHierarchyImpl::FindSubTypesImpl in.");
    if (hierarchyItem.isKernel && hierarchyItem.isChildOrSuper) {
        return;
    }
    if (hierarchyItem.symbolId == lsp::INVALID_SYMBOL_ID) {
        return;
    }

    auto index = CompilerCodiraProject::GetInstance()->GetIndex();
    if (!index) {
        return;
    }

    auto id = hierarchyItem.symbolId;
    std::unordered_set<lsp::SymbolID> subTypeIds;

    lsp::RelationsRequest relationReq{id, lsp::RelationKind::BASE_OF};
    index->Relations(relationReq, [&relationReq, &subTypeIds](const lsp::Relation &spo) {
        // Subject    BASE OF    Object
        // ^                     ^
        // SuperType               SubType
        std::cerr << "relationReq:" << relationReq.id << ", " << static_cast<uint32_t>(relationReq.predicate) << "\n";
        if (spo.subject == relationReq.id && spo.predicate == relationReq.predicate) {
            subTypeIds.emplace(spo.object);
        }
    });

    lsp::RelationsRequest extendReq{id, lsp::RelationKind::EXTEND};
    index->Relations(extendReq, [&extendReq, &subTypeIds](const lsp::Relation &spo) {
        // Subject    EXTEND    Object
        // ^                    ^
        // SuperType              SubType
        if (spo.subject == extendReq.id && spo.predicate == extendReq.predicate) {
            subTypeIds.emplace(spo.object);
        }
    });

    lsp::LookupRequest lookupReq{subTypeIds};
    index->Lookup(lookupReq, [&results](const lsp::Symbol &sym) {
        TypeHierarchyItem item;
        item.uri.file = URI::URIFromAbsolutePath(sym.location.fileUri).ToString();
        auto it = AST2Symbol.find(sym.kind);
        if (it != AST2Symbol.end()) {
            item.kind = AST2Symbol.find(sym.kind)->second;
        }
        item.name = sym.name;
        auto range = TransformFromChar2IDE({sym.location.begin, sym.location.end});
        item.selectionRange = item.range = range;
        item.isKernel = false;
        item.symbolId = sym.id;
        results.emplace_back(item);
    });
}
} // namespace ark
