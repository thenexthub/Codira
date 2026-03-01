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

#include "LocateSymbolAtImpl.h"
#include "LocateDefinition4Import.h"

using namespace Codira;
using namespace Codira::AST;
using namespace Codira::Meta;

namespace ark {
void RedirectToMacroInvocation(const Decl &decl, LocatedSymbol &result, std::string &path)
{
    if (EndsWith(path, ".macrocall") && decl.curMacroCall) {
        Ptr<Node> curMacroCallNode = decl.curMacroCall;
        std::string sourceFile;
        if (RemoveFilePathExtension(path, ".macrocall", sourceFile)
            && FileUtil::FileExist(sourceFile)
        ) {
            URIForFile uri = {URI::URIFromAbsolutePath(sourceFile).ToString()};
            Range macroCallBeginRange = {curMacroCallNode->begin, curMacroCallNode->begin};
            result.Definition = {uri, TransformFromChar2IDE(macroCallBeginRange)};
        }
    }
}

bool GetDefinitionItems(const Decl &decl, LocatedSymbol &result)
{
    Trace::Log("GetDefinitionItems in.");
    // invalid fileID
    if (decl.identifier.Begin().fileID == 0) { return false; }
    Range range;
    // Handling the main constructor and constructors, including explicitly defined and implicitly generated ones
    // Implicit constructor -> jump to class name
    // Explicit constructor && in macro expansion file -> jump to class name
    // Explicit constructor && in source code -> jump to source code init() location
    if (decl.TestAnyAttr(Attribute::PRIMARY_CONSTRUCTOR, Attribute::CONSTRUCTOR)) {
        const std::string identifier = GetConstructorIdentifier(decl);
        range = GetConstructorRange(decl, identifier);
    } else {
        range = GetDeclRange(decl, static_cast<int>(CountUnicodeCharacters(decl.identifier)));
    }
    const unsigned int fileID = range.start.fileID;
    std::string path = CompilerCodiraProject::GetInstance()->GetFilePathByID(LocateSymbolAtImpl::curFilePath, fileID);
    // Modify range and path if decl is in the MacroCall file.
    auto index = ark::CompilerCodiraProject::GetInstance()->GetIndex();
    if (!index) {
        return false;
    }
    auto symFromIndex = index->GetAimSymbol(decl);
    if (!symFromIndex.IsInvalidSym() && !symFromIndex.location.fileUri.empty() && !symFromIndex.isCodeoSym &&
        EndsWith(symFromIndex.location.fileUri, ".macrocall") &&
        !decl.TestAttr(Attribute::IMPLICIT_ADD) && !symFromIndex.curMacroCall.fileUri.empty()) {
        path = symFromIndex.curMacroCall.fileUri;
        range.start = symFromIndex.curMacroCall.begin;
        range.end = symFromIndex.curMacroCall.begin;
    }
    URIForFile uri = {URI::URIFromAbsolutePath(path).ToString()};
    result.Name = decl.identifier;
    ArkAST *arkAst = CompilerCodiraProject::GetInstance()->GetArkAST(path);
    // jump to lib
    const std::string standardDeclAbsolutePath = GetStandardDeclAbsolutePath(&decl, path);
    if (standardDeclAbsolutePath != "") {
        uri.file = URI::URIFromAbsolutePath(standardDeclAbsolutePath).ToString();
        result.Definition = {uri, TransformFromChar2IDE(range)};
        return true;
    }
    if (!FileUtil::FileExist(path)) {
        if (MessageHeaderEndOfLine::GetIsDeveco() && !symFromIndex.IsInvalidSym() &&
            !symFromIndex.declaration.IsZeroLoc()) {
            uri.file = URI::URIFromAbsolutePath(symFromIndex.declaration.fileUri).ToString();
            range.start = symFromIndex.declaration.begin;
            range.end = symFromIndex.declaration.end;
            result.Definition = {uri, TransformFromChar2IDE(range)};
            return true;
        }
        return false;
    }
    if (arkAst) {
        UpdateRange(arkAst->tokens, range, decl);
    }
    result.Definition = {uri, TransformFromChar2IDE(range)};
    // if is in macrocall file, redirect to macro call pos. ex: @Entry
    RedirectToMacroInvocation(decl, result, path);
    return true;
}

std::string LocateSymbolAtImpl::curFilePath = "";

bool LocateSymbolAtImpl::LocateSymbolAt(const ArkAST &ast, LocatedSymbol &result, Position pos)
{
    Logger &logger = Logger::Instance();
    logger.LogMessage(MessageType::MSG_LOG, "LocatedSymbolImpl::LocateSymbolAt in.");
    // update pos fileID
    pos.fileID = ast.fileID;
    // adjust position from IDE to AST
    pos = PosFromIDE2Char(pos);
    PositionIDEToUTF8(ast.tokens, pos, *ast.file);
    // get curFilePath
    curFilePath = ast.file ? ast.file->filePath : "";
    LowFileName(curFilePath);
    // check current token is the kind required in function CheckTokenKind(TokenKind)
    std::vector<Symbol *> syms;
    std::vector<Ptr<Codira::AST::Decl> > decls;
    Ptr<Decl> decl = ast.GetDeclByPosition(pos, syms, decls, {true, false});
    if (!decl) {
        LocateDefinition4Import::getImportDecl(syms, ast, pos, decl);
        if (decl) {
            return GetDefinitionItems(*decl, result);
        }
        return false;
    }
    if (!syms[0] || IsMarkPos(syms[0]->node, pos) || IsResourcePos(ast, syms[0]->node, pos)) {
        return false;
    }
    // Cross language
    if (decl->astKind == ASTKind::FUNC_DECL && decl->TestAttr(Attribute::FOREIGN)) {
        CrossDefinition(result.CrossMessage, dynamic_cast<FuncDecl*>(decl.get()));
    }
    if (decl->astKind == ASTKind::GENERIC_PARAM_DECL) {
        auto temp = ast.FindRealGenericParamDeclForExtend(decl->identifier, syms);
        if (temp != nullptr) {
            decl = temp;
        }
    }
    bool ret = GetDefinitionItems(*decl, result);
    return ret;
}

void LocateSymbolAtImpl::CrossDefinition(std::vector<message> &CrossMessage, Ptr<Codira::AST::FuncDecl> funcDecl)
{
    CrossDefinitionCodira2C::Codira2CGetFuncMessage(CrossMessage, funcDecl);
}
} // namespace ark
