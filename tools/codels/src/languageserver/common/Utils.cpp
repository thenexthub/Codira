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

#include "Utils.h"
#include "Codira/Basic/Version.h"
#include "Inherit/InheritDeclUtil.h"
#include "../CompilerCodiraProject.h"

using namespace Codira::AST;
using namespace Codira::FileUtil;

namespace ark {
const int NUMBER_FOR_LINE_COMMENT = 2; // length of "//"
const int NUMBER_FOR_DOC_COMMENT = 3;  // length of "/**"
const std::string PKG_NAME_OHOS_LABELS = "ohos.labels";
const std::string HIDE_ANNO_NAME = "Hide";
const std::unordered_map<ASTKind, SymbolKind> AST_KIND_TO_SYMBOL_KIND = {
    {ASTKind::INTERFACE_DECL, SymbolKind::INTERFACE_DECL},
    {ASTKind::CLASS_DECL, SymbolKind::CLASS},
    {ASTKind::STRUCT_DECL, SymbolKind::STRUCT},
    {ASTKind::EXTEND_DECL, SymbolKind::OBJECT},
    {ASTKind::TYPE_ALIAS_DECL, SymbolKind::OBJECT},
    {ASTKind::ENUM_DECL, SymbolKind::ENUM},
    {ASTKind::VAR_DECL, SymbolKind::VARIABLE},
    {ASTKind::FUNC_DECL, SymbolKind::FUNCTION},
    {ASTKind::PRIMARY_CTOR_DECL, SymbolKind::FUNCTION},
    {ASTKind::MACRO_DECL, SymbolKind::FUNCTION},
    {ASTKind::MAIN_DECL, SymbolKind::FUNCTION},
    {ASTKind::PROP_DECL, SymbolKind::PROPERTY}};

TypeCompatibility CheckTypeCompatibility(const Ty *lvalue, const Ty *rvalue)
{
    if (!lvalue || !rvalue) {
        return TypeCompatibility::INCOMPATIBLE;
    }
    // for TupleType, its members need type compatibility check.
    auto ltuple = dynamic_cast<const TupleTy *>(lvalue);
    auto rtuple = dynamic_cast<const TupleTy *>(rvalue);
    if (ltuple && rtuple) {
        if (ltuple->typeArgs.size() != rtuple->typeArgs.size()) {
            return TypeCompatibility::INCOMPATIBLE;
        }
        auto ret = TypeCompatibility::IDENTICAL;
        for (size_t i = 0; i < ltuple->typeArgs.size(); i++) {
            auto res = CheckTypeCompatibility(ltuple->typeArgs[i], rtuple->typeArgs[i]);
            if (res == TypeCompatibility::INCOMPATIBLE) {
                return res;
            }
        }
        return ret;
    }
    if (lvalue->String() == rvalue->String() || lvalue->kind == TypeKind::TYPE_GENERICS ||
        rvalue->kind == TypeKind::TYPE_GENERICS) {
        return TypeCompatibility::IDENTICAL;
    }
    return TypeCompatibility::INCOMPATIBLE;
}

bool IsHiddenDecl(const Ptr<Node> node) {
    if (!Options::GetInstance().IsOptionSet("test") && !MessageHeaderEndOfLine::GetIsDeveco()) {
        return false;
    }
    auto decl = DynamicCast<Decl *>(node.get());
    if (!decl) {
        return false;
    }
    for (auto& anno: decl->annotations) {
        if (anno->identifier != HIDE_ANNO_NAME) {
            continue;
        }
        auto target = anno->baseExpr ? anno->baseExpr->GetTarget() : nullptr;
        if (target) {
            return target->GetFullPackageName() == PKG_NAME_OHOS_LABELS && target->outerDecl &&
                target->outerDecl->identifier == HIDE_ANNO_NAME; 
        }
        // 6.0 annotation is not export target, to compatible with 6.0, treat as hidden
        return true;
    }
    return false;
}

bool IsFuncParameterTypesIdentical(const FuncTy &t1, const FuncTy &t2)
{
    bool result{false};
    if (t1.paramTys.size() == t2.paramTys.size()) {
        result = true;
        for (size_t i = 0; i < t2.paramTys.size(); i++) {
            result = result && (CheckTypeCompatibility(t1.paramTys[i], t2.paramTys[i]) == TypeCompatibility::IDENTICAL);
        }
    }

    return result;
}

bool IsMatchingCompletion(const std::string &prefix, const std::string &completionName, bool caseSensitive)
{
    if (prefix.empty()) {
        return true;
    }

    // If case-insensitive, convert the character string in advance.
    if (!caseSensitive) {
        std::string lowerPrefix;
        std::string lowerCompletion;

        // Pre-allocate space to improve performance
        lowerPrefix.reserve(prefix.length());
        lowerCompletion.reserve(completionName.length());

        // Convert to lowercase
        std::transform(prefix.begin(), prefix.end(), std::back_inserter(lowerPrefix),
            [](unsigned char c) { return std::tolower(c); });
        std::transform(completionName.begin(), completionName.end(), std::back_inserter(lowerCompletion),
            [](unsigned char c) { return std::tolower(c); });

        // Use the converted character string for matching.
        size_t pos = 0;
        for (const auto &ch : lowerPrefix) {
            pos = lowerCompletion.find(ch, pos);
            if (pos == std::string::npos) {
                return false;
            }
            ++pos;
        }
    } else {
        // Original case-sensitive logic
        size_t pos = 0;
        for (const auto &ch : prefix) {
            pos = completionName.find(ch, pos);
            if (pos == std::string::npos) {
                return false;
            }
            ++pos;
        }
    }

    return true;
}

std::string GetSortText(double score)
{
    // ensure score between 0 and 1
    score = std::max(0.0, std::min(1.0, score));

    // revert score to decimal
    score = 1.0 - score;

    // convert reverted score to 6-digit integer(keep 6 decimal places of precision)
    int scaledScore = static_cast<int>(score * 1e6 + CONSTANTS::ROUND_NUM);

    // formatted as a 6-digit string with leading zeros
    std::ostringstream oss;
    oss << std::setw(CONSTANTS::SORT_TEXT_SIZE) << std::setfill('0') << scaledScore;

    return oss.str();
}

std::string GetFilterText(const std::string &name, const std::string &prefix)
{
    std::string filterText = name;
    if (Options::GetInstance().IsOptionSet("test") || MessageHeaderEndOfLine::GetIsDeveco()) {
        return filterText;
    }
    return prefix + "_" + name;
}

Range GetNamedFuncArgRange(const Codira::AST::Node &node)
{
    Range range;
    auto *symbol = node.symbol;
    if (symbol) {
        range = {node.begin,
                 {node.begin.fileID, node.begin.line,
                  node.begin.column + static_cast<int>(CountUnicodeCharacters(symbol->name))}};
    }
    return range;
}

Range GetDeclRange(const Codira::AST::Decl &decl, int length)
{
    Range range;
    if (decl.astKind == ASTKind::EXTEND_DECL) {
        auto extendDecl = dynamic_cast<const ExtendDecl *>(&decl);
        if (extendDecl && extendDecl->extendedType) {
            return {extendDecl->extendedType->GetBegin(), extendDecl->extendedType->GetEnd()};
        }
    }
    if (decl.astKind == ASTKind::GENERIC_PARAM_DECL) {
        return {decl.GetBegin(), decl.GetEnd()};
    }
    range.start = decl.GetIdentifierPos();
    range.end = decl.GetIdentifierPos();
    range.end.column += length;
    return range;
}

Range GetIdentifierRange(Ptr<const Codira::AST::Node> node)
{
    if (!node || !node->symbol) {
        return {};
    }
    auto *type = dynamic_cast<const Type *>(node.get());
    if (type && !type->typeParameterName.empty() && type->symbol) {
        return Range{type->typePos,
                     {type->GetBegin().fileID, type->GetBegin().line,
                      type->typePos.column + static_cast<int>(CountUnicodeCharacters(type->symbol->name))}};
    }
    // check is rawIdentifier
    if ((node->GetEnd().column - 1 - node->GetBegin().column - 1) == static_cast<int>(node->symbol->name.size())) {
        return Range{node->GetBegin() + 1, node->GetEnd() - 1};
    }
    return Range{node->GetBegin(),
                 {node->GetBegin().fileID, node->GetBegin().line,
                  node->GetBegin().column + static_cast<int>(CountUnicodeCharacters(node->symbol->name))}};
}

Range GetRefTypeRange(Ptr<const Codira::AST::Node> node)
{
    if (!node) {
        return {};
    }
    auto *type = dynamic_cast<const Type *>(node.get());
    if (type && !type->typeParameterName.empty() && type->symbol) {
        return Range{type->typePos,
                     {type->GetBegin().fileID, type->GetBegin().line,
                      type->typePos.column + static_cast<int>(CountUnicodeCharacters(type->symbol->name))}};
    }
    return {};
}

CommentKind GetCommentKind(const std::string &comment)
{
    if (comment.size() < NUMBER_FOR_LINE_COMMENT) {
        return CommentKind::NO_COMMENT;
    }
    if (comment.substr(0, NUMBER_FOR_LINE_COMMENT) == "//") {
        return CommentKind::LINE_COMMENT;
    }
    std::string prefixComment;
    if (comment.size() > NUMBER_FOR_DOC_COMMENT) {
        prefixComment = comment.substr(0, NUMBER_FOR_DOC_COMMENT);
    }
    std::string suffixComment = comment.substr(comment.size() - NUMBER_FOR_LINE_COMMENT);
    if (suffixComment == "*/") {
        if (prefixComment == "/**" && comment.size() > NUMBER_FOR_DOC_COMMENT + 1) {
            return CommentKind::DOC_COMMENT;
        } else if (prefixComment.substr(0, NUMBER_FOR_LINE_COMMENT) == "/*") {
            return CommentKind::BLOCK_COMMENT;
        }
    }
    return CommentKind::NO_COMMENT;
}

std::string PrintTypeArgs(std::vector<Ptr<Ty>> tyArgs, const std::pair<bool, int> isVarray)
{
    if (tyArgs.empty()) {
        return "";
    }
    std::string res{"<"};
    for (size_t i = 0; i < tyArgs.size() - 1; i++) {
        if (tyArgs.at(i) == nullptr) {
            continue;
        } else if (tyArgs.at(i)->name.empty()) {
            (void)res.append(tyArgs.at(i)->String() + ", ");
        } else {
            (void)res.append(GetString(*tyArgs.at(i)) + ", ");
        }
    }
    size_t lastIndex = tyArgs.size() - 1;
    if (tyArgs.at(lastIndex) == nullptr) {
        (void)res.append(">");
        return res;
    }
    if (tyArgs.at(lastIndex)->name.empty()) {
        (void)res.append(tyArgs.at(lastIndex)->String());
    } else {
        (void)res.append(GetString(*tyArgs.at(lastIndex)));
    }
    if (isVarray.first) {
        (void)res.append(", $");
        (void)res.append(std::to_string(isVarray.second));
    }
    (void)res.append(">");
    return res;
}

std::string GetString(const Ty &ty)
{
    // check VArray
    auto vArrayTy = dynamic_cast<const Codira::AST::VArrayTy *>(&ty);
    std::pair<bool, int> isVArray = {false, 0};
    if (vArrayTy) {
        isVArray = {true, vArrayTy->size};
    }
    return ty.name.empty() ? ty.String() : ty.name + PrintTypeArgs(ty.typeArgs, isVArray);
}

std::string ReplaceTuple(const std::string &type)
{
    std::string result;
    std::string searchText = "Tuple<";
    size_t searchLen = 6;
    for (size_t i = 0; i < type.length();) {
        if (i + searchLen <= type.length() && type.substr(i, searchLen) == searchText) {
            size_t start = i;
            i += searchLen;
            int count = 1;
            size_t innerStart = i;
            MatchBracket(type, i, count);
            if (count == 0) {
                std::string inner = type.substr(innerStart, i - 1 - innerStart);
                std::string replacedInner = ReplaceTuple(inner);
                result += "(" + replacedInner + ")";
            } else {
                result += type.substr(start, i - start);
            }
        } else {
            result += type[i];
            i++;
        }
    }
    return result;
}

void MatchBracket(const std::string &type, size_t &index, int &count)
{
    while (index < type.length() && count > 0) {
        if (type[index] == '<') {
            count++;
        } else if (type[index] == '>') {
            count--;
        }
        index++;
    }
}

std::string GetVarDeclType(Ptr<VarDecl> decl, SourceManager *sourceManager)
{
    std::string type;
    if ((!decl->ty || GetString(*decl->ty) == "UnknownType") && decl->type) {
        type = sourceManager->GetContentBetween(decl->type->begin, decl->type->end);
        std::string realType = ReplaceTuple(type);
        type = realType.empty() ? type : realType;
        return type;
    }
    if (!decl->ty) {
        return type;
    }
    if (decl->ty->kind == TypeKind::TYPE_FUNC) {
        ItemResolverUtil::GetDetailByTy(decl->ty, type, true);
        std::string realType = ReplaceTuple(type);
        type = realType.empty() ? type : realType;
        return type;
    }
    type = GetString(*decl->ty);
    std::string realType = ReplaceTuple(type);
    type = realType.empty() ? type : realType;
    return type;
}

bool IsZeroPosition(Ptr<const Node> node) { return node && node->end.line == 0 && node->end.column == 0; }

bool ValidExtendIncludeGenericParam(Ptr<const Decl> decl)
{
    return decl && (decl->astKind == ASTKind::CLASS_DECL || decl->astKind == ASTKind::STRUCT_DECL);
}

void SetRangForInterpolatedString(const Codira::Token &curToken, Ptr<const Codira::AST::Node> node, Range &range)
{
    if (!node || curToken.kind != Codira::TokenKind::STRING_LITERAL) {
        return;
    }

    range.start = {node->begin.fileID, node->begin.line, node->begin.column};
    range.end = {node->end.fileID, node->end.line, node->end.column};
}

void SetRangForInterpolatedStrInRename(const Codira::Token &curToken, Ptr<const Codira::AST::Node> node,
    Range &range, Codira::Position pos)
{
    if (!node || (curToken.kind != Codira::TokenKind::STRING_LITERAL
                     && curToken.kind != Codira::TokenKind::MULTILINE_STRING)) {
        return;
    }
    std::string nodeStr = node->ToString();
    if (pos < curToken.Begin() || pos > curToken.End() || nodeStr.empty()) {
        return;
    }
    Position curPos = node->GetBegin();
    int index = -1;
    for (int i = 0; i <= static_cast<int>(nodeStr.length()); ++i) {
        if (i < static_cast<int>(nodeStr.length()) && nodeStr[i] == '\n') {
            curPos.line++;
            curPos.column = 0;
        }
        if (curPos == pos) {
            index = i;
            break;
        }
        curPos.column++;
    }
    if (index == -1) {
        return;
    }
    int start = index;
    int end = index;
    while (start > 0 && (std::iswalnum(nodeStr[start - 1]) || nodeStr[start - 1] == L'_')) {
        start--;
    }
    while (end < static_cast<int>(nodeStr.length()) && (std::iswalnum(nodeStr[end]) || nodeStr[end] == L'_')) {
        end++;
    }
    std::string identifier = nodeStr.substr(start, end - start);
    if (IsValidIdentifier(identifier) && (pos.column - (index - start) > 0)) {
        range.start = {pos.fileID, pos.line, pos.column - (index - start)};
        range.end = {pos.fileID, pos.line, pos.column + (end - index)};
    }
}

bool IsFuncSignatureIdentical(const Codira::AST::FuncDecl &funcDecl1, const Codira::AST::FuncDecl &funcDecl2)
{
    if (funcDecl1.identifier != funcDecl2.identifier) {
        return false;
    }
    auto funcTy1 = dynamic_cast<FuncTy *>(funcDecl1.ty.get());
    auto funcTy2 = dynamic_cast<FuncTy *>(funcDecl2.ty.get());
    if (!funcTy1 || !funcTy2 || !IsFuncParameterTypesIdentical(*funcTy1, *funcTy2) ||
        !(CheckTypeCompatibility(funcTy1->retTy, funcTy2->retTy) == TypeCompatibility::IDENTICAL)) {
        return false;
    }
    return true;
}

std::vector<Symbol *> SearchContext(const Codira::ASTContext *astContext, const std::string &query)
{
    std::vector<Symbol *> temp;
    if (!astContext || !astContext->searcher) {
        return temp;
    }
    return astContext->searcher->Search(*astContext, query);
}

std::set<Ptr<Codira::AST::Decl>> GetInheritDecls(Ptr<const Codira::AST::Decl> decl)
{
    std::set<Ptr<Decl>> decls;
    if (decl && (decl->astKind == ASTKind::FUNC_DECL || decl->astKind == ASTKind::PROP_DECL)) {
        InheritDeclUtil inherit(decl);
        inherit.HandleFuncDecl(true);
        return inherit.GetRelatedFuncDecls();
    }
    return decls;
}

bool IsFromSrcOrNoSrc(Ptr<const Node> node)
{
    if (!node || !node->curFile || !node->curFile->curPackage) {
        return false;
    }
    return CompilerCodiraProject::GetInstance()->PkgIsFromSrcOrNoSrc(node);
}

bool IsFromCIMap(const std::string &fullPkgName)
{
    return CompilerCodiraProject::GetInstance()->PkgIsFromCIMap(fullPkgName);
}

bool IsFromCIMapNotInSrc(const std::string &fullPkgName)
{
    return CompilerCodiraProject::GetInstance()->PkgIsFromCIMapNotInSrc(fullPkgName);
}

Range GetRangeFromNode(Ptr<const Node> p, const std::vector<Codira::Token> &tokens)
{
    Range range;
    if (!p) {
        return range;
    }
    if (dynamic_cast<const MemberAccess *>(p.get())) {
        range = GetUserRange<MemberAccess>(*p);
    } else if (dynamic_cast<const QualifiedType *>(p.get())) {
        range = GetUserRange<QualifiedType>(*p);
    } else if (p->astKind == ASTKind::MACRO_EXPAND_DECL) {
        range = GetMacroRange<MacroExpandDecl>(*p);
    } else if (p->astKind == ASTKind::MACRO_EXPAND_EXPR) {
        range = GetMacroRange<MacroExpandExpr>(*p);
    } else if (p->astKind == ASTKind::FUNC_ARG) {
        range = GetNamedFuncArgRange(*p);
    } else if (p->ty && !p->ty->typeArgs.empty()) {
        range = GetIdentifierRange(p);
    } else if (p->astKind == ASTKind::REF_TYPE) {
        range = GetRefTypeRange(p);
    } else {
        range = {p->GetBegin(), p->GetEnd()};
    }
    if (range.end.column == 0 && range.end.line == 0) {
        range = {p->GetBegin(), p->GetEnd()};
    }
    UpdateRange(tokens, range, *p);
    return range;
}

SymbolKind GetSymbolKind(const ASTKind astKind)
{
    if (AST_KIND_TO_SYMBOL_KIND.count(astKind)) {
        return AST_KIND_TO_SYMBOL_KIND.at(astKind);
    }
    return SymbolKind::NULL_KIND;
}

bool InValidDecl(Ptr<const Decl> decl)
{
    return (decl &&
            (decl->astKind == ASTKind::PRIMARY_CTOR_DECL ||
             (decl->TestAttr(Codira::AST::Attribute::COMPILER_ADD) &&
              !decl->TestAttr(Codira::AST::Attribute::IS_CLONED_SOURCE_CODE) &&
              !decl->TestAttr(Codira::AST::Attribute::PRIMARY_CONSTRUCTOR) && decl->astKind != ASTKind::EXTEND_DECL)));
}

bool IsRelativePathByImported(const std::string &path)
{
    // like moduleName/packageName/file.code
    std::string modulePath = GetDirPath(GetDirPath(path));
    return modulePath.find_last_of(DIR_SEPARATOR) == std::string::npos;
}

bool IsFullPackageName(const std::string &pkgName)
{
    // FullPackageName doesn't contain DIR_SEPARATOR
    auto name = SplitFullPackage(pkgName);
    return name.first.find_last_of(DIR_SEPARATOR) == std::string::npos;
}

std::string GetPkgNameFromNode(Ptr<const Codira::AST::Node> node)
{
    if (!node) {
        return "";
    }
    auto fileNode = dynamic_cast<const File *>(node.get());
    if (!fileNode && !node->curFile) {
        return "";
    }
    std::string path = !fileNode ? node->curFile->filePath : fileNode->filePath;
    if (!IsRelativePathByImported(path)) {
        return CompilerCodiraProject::GetInstance()->GetFullPkgName(path);
    }
    return GetRealPkgNameFromPath(PathWindowsToLinux(GetDirPath(path)));
}

void SetHeadByFilePath(const std::string &filePath)
{
    auto instance = CompilerCodiraProject::GetInstance();
    if (!instance) {
        return;
    }
    auto fullPkgName = instance->GetFullPkgName(filePath);
    if (instance->PkgHasSemaCache(fullPkgName)) {
        instance->SetHead(fullPkgName);
    }
}

std::unordered_set<Ptr<Node>> GetOnePkgUsers(const Codira::AST::Decl &decl,
                                             const std::string &curPkg,
                                             const std::string &curFilePath,
                                             bool isRename,
                                             const std::string &editPkg)
{
    std::unordered_set<Ptr<Node>> users;
    if (curPkg.empty()) {
        return users;
    }
    auto instance = CompilerCodiraProject::GetInstance();
    if (!instance->PkgHasSemaCache(curPkg)) {
        if (IsFromCIMap(curPkg)) {
            instance->IncrementTempPkgCompile(curPkg);
        } else {
            instance->IncrementTempPkgCompileNotInSrc(curPkg);
        }
    }
    auto package = instance->GetSourcePackagesByPkg(curPkg);
    if (!package) {
        return users;
    }
    auto temp = FindDeclUsage(decl, *package, isRename);
    users.insert(temp.begin(), temp.end());

    // setHead for editPkg and declPkg
    SetHeadByFilePath(curFilePath);
    SetHeadByFilePath(editPkg);
    return users;
}

void AddTopDecl(const std::string &curPkg,
                std::map<std::string, bool> &superDecl,
                std::vector<Ptr<Decl>> &users,
                const OwnedPtr<Decl> &memDecl,
                Ptr<InheritableDecl> inheritableDecl)
{
    for (auto &in : inheritableDecl->inheritedTypes) {
        if (!in->ty || in->ty->name.empty()) {
            continue;
        }
        if (superDecl.find(in->ty->name) != superDecl.end()) {
            users.push_back(memDecl.get());
            return;
        }
    }
    auto extendDecls = CompilerCodiraProject::GetInstance()->GetExtendDecls(inheritableDecl, curPkg);
    for (auto extendDecl : extendDecls) {
        for (auto &in : extendDecl->inheritedTypes) {
            if (!in->ty || in->ty->name.empty()) {
                continue;
            }
            if (superDecl.find(in->ty->name) != superDecl.end()) {
                users.push_back(memDecl.get());
                return;
            }
        }
    }
}

std::vector<Ptr<Decl>> GetAimSubDecl(const std::string &curPkg,
                                     std::map<std::string, bool> superDecl,
                                     const std::string &editPkg)
{
    std::vector<Ptr<Decl>> users{};
    if (curPkg.empty() || !IsFromCIMap(curPkg) && !IsFromCIMapNotInSrc(curPkg)) {
        return users;
    }
    auto instance = CompilerCodiraProject::GetInstance();
    if (!instance->PkgHasSemaCache(curPkg)) {
        if (IsFromCIMap(curPkg)) {
            instance->IncrementTempPkgCompile(curPkg);
        } else {
            instance->IncrementTempPkgCompileNotInSrc(curPkg);
        }
        SetHeadByFilePath(editPkg);
    }

    auto package = instance->GetSourcePackagesByPkg(curPkg);
    if (!package) {
        return users;
    }
    // found Inherited subclass
    for (auto &topDecl : package->files) {
        for (auto &memDecl : topDecl->decls) {
            auto inheritableDecl = dynamic_cast<InheritableDecl *>(memDecl.get().get());
            if (!inheritableDecl) {
                continue;
            }
            AddTopDecl(curPkg, superDecl, users, memDecl, inheritableDecl);
        }
    }
    return users;
}

int GetDotIndexPos(const std::vector<Codira::Token> &tokens, const int end, const int start)
{
    if (end <= start || end < 1 || static_cast<unsigned long>(end) > tokens.size() || start < 0 ||
        static_cast<unsigned long>(start) > tokens.size()) {
        return -1;
    }
    // deal from std import math.{item}
    for (auto i = end - 1; i > start; i--) {
        if (tokens[i] == "{" && i > 0 && tokens[i - 1] == ".") {
            return i - 1;
        }
        if (tokens[i] == "}") {
            break;
        }
    }
    // deal from std import math.[item],math.[item]
    for (auto i = end; i >= start; i--) {
        if (tokens[i] == ".") {
            return i;
        }
        if (tokens[i] == ",") {
            break;
        }
    }
    return -1;
}

void ConvertCarriageToSpace(std::string &str)
{
    if (str.empty()) {
        return;
    }
    for (auto &ch : str) {
        if (ch == '\n') {
            ch = ' ';
        }
    }
}

void GetConditionCompile(const nlohmann::json &initializationOptions,
                         std::unordered_map<std::string, std::string> &conditions)
{
    if (initializationOptions.contains(CONSTANTS::CONDITION_COMPILE_OPTION)) {
        auto conditionCompiles = initializationOptions[CONSTANTS::CONDITION_COMPILE_OPTION];
        auto conditionItems = conditionCompiles.items();
        for (auto &item : conditionItems) {
            auto &key = item.key();
            conditions[key] = conditionCompiles.value(key, "");
        }
    }
}

void GetModuleConditionCompile(
    const nlohmann::json &initializationOptions,
    const std::unordered_map<std::string, std::string> &globalConditions,
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> &conditions)
{
    if (!initializationOptions.contains(CONSTANTS::MODULE_CONDITION_COMPILE_OPTION)) {
        return;
    }
    auto customModuleConditions = initializationOptions[CONSTANTS::MODULE_CONDITION_COMPILE_OPTION];
    const auto moduleConditionItems = customModuleConditions.items();
    for (auto &item : moduleConditionItems) {
        auto &moduleName = item.key();
        std::unordered_map<std::string, std::string> moduleConditions = globalConditions;
        auto customConditionItems = customModuleConditions[moduleName].items();
        for (auto &conditionItem : customConditionItems) {
            auto &key = conditionItem.key();
            moduleConditions[key] = customModuleConditions[moduleName].value(key, "");
        }
        conditions[moduleName] = moduleConditions;
    }
}

void GetSingleConditionCompile(
    const nlohmann::json &initializationOptions,
    const std::unordered_map<std::string, std::string> &globalConditions,
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& modulesConditions,
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> &conditions)
{
    if (!initializationOptions.contains(CONSTANTS::SINGLE_CONDITION_COMPILE_OPTION)) {
        return;
    }
    auto customSingleConditionCompiles = initializationOptions[CONSTANTS::SINGLE_CONDITION_COMPILE_OPTION];
    auto packageNameItems = customSingleConditionCompiles.items();
    for (auto &item : packageNameItems) {
        auto &packageName = item.key();
        auto moduleName = packageName;
        const size_t firstDotPos = packageName.find_last_of('.');
        if (firstDotPos > 0) {
            moduleName = packageName.substr(0, firstDotPos);
        }
        std::unordered_map<std::string, std::string> packageConditions = globalConditions;
        if (modulesConditions.find(moduleName) != modulesConditions.end()) {
            std::unordered_map<std::string, std::string> curModulesConditions = modulesConditions[moduleName];
            for (const auto& moduleConditionsItem : curModulesConditions) {
                auto &key = moduleConditionsItem.first;
                packageConditions[key] = moduleConditionsItem.second;
            }
        }
        auto conditionItems = customSingleConditionCompiles[packageName].items();
        for (auto &conditionItem : conditionItems) {
            auto &key = conditionItem.key();
            packageConditions[key] = customSingleConditionCompiles[packageName].value(key, "");
        }
        conditions[packageName] = packageConditions;
    }
}

void GetConditionCompilePaths(const nlohmann::json &initializationOptions, std::vector<std::string> &conditionPaths)
{
    if (initializationOptions.contains(CONSTANTS::CONDITION_COMPILE_PATHS)) {
        auto conditionCompilePaths = initializationOptions[CONSTANTS::CONDITION_COMPILE_PATHS];
        for (int i = 0; i < static_cast<int>(conditionCompilePaths.size()); ++i) {
            conditionPaths.push_back(conditionCompilePaths[i].get<std::string>());
        }
    }
}

void DealMemberParam(const std::string &curFullPkgName,
                     std::vector<Symbol *> &syms,
                     std::vector<Ptr<Codira::AST::Decl>> &decls,
                     Ptr<const Codira::AST::Decl> oldDecl)
{
    auto *ast = CompilerCodiraProject::GetInstance()->GetArkAST(curFullPkgName);
    if (ast == nullptr) {
        return;
    }
    Ptr<const VarDecl> varDecl = dynamic_cast<const VarDecl *>(oldDecl.get());
    if (varDecl && varDecl->isMemberParam) {
        Position newPos = {ast->fileID, varDecl->identifier.Begin().line, varDecl->identifier.Begin().column};
        if (ast->fileID == varDecl->GetIdentifierPos().fileID) {
            (void)ast->GetDeclByPosition(newPos, syms, decls, {true, false});
        }
    }
}

void LowFileName(std::basic_string<char> &filePath)
{
    if (FileUtil::GetFileExtension(filePath) != "code") {
        return;
    }
#ifdef _WIN32
    auto len = filePath.length();
    for (int i = len - 1; i >= 0; i--) {
        if (filePath[i] == '/' || filePath[i] == '\\') {
            break;
        }
        if (isupper(filePath[i])) {
            filePath[i] = tolower(filePath[i]);
        }
    }
#endif
}

std::pair<std::string, std::string> SplitFullPackage(const std::string &fullPackageName)
{
    std::string moduleName;
    std::string packageName;
    auto found = fullPackageName.find_first_of(CONSTANTS::DOT);
    if (found != std::string::npos) {
        moduleName = fullPackageName.substr(0, found);
        auto temp = fullPackageName.substr(found);
        if (!temp.empty()) {
            packageName = temp.substr(1);
        }
        return std::make_pair(moduleName, packageName);
    }
    return std::make_pair(fullPackageName, "");
}

std::string PathWindowsToLinux(const std::string &path)
{
    std::string str = std::string(path);
    for (auto &s : str) {
        if (s == '\\') {
            s = '/';
        }
    }
    return str;
}

std::optional<std::string> GetRelativePath(const std::string &basePath, const std::string &path)
{
    std::string normalizedBasePath;
    std::string normalizedPath;
    normalizedBasePath = GetAbsPath(Normalize(basePath)) | IdenticalFunc;
    normalizedPath = GetAbsPath(Normalize(path)) | IdenticalFunc;
    normalizedBasePath = normalizedBasePath.empty() ? "" : JoinPath(normalizedBasePath, "");
    normalizedPath = normalizedPath.empty() ? "" : JoinPath(normalizedPath, "");
    if (normalizedBasePath == normalizedPath) {
        return "";
    }
    auto found = normalizedPath.find(normalizedBasePath);
    if (found != std::string::npos) {
        return normalizedPath.substr(found + normalizedBasePath.size());
    }
    return {};
}

bool IsMarkPos(Ptr<const Codira::AST::Node> node, Codira::Position pos)
{
    auto *type = dynamic_cast<const Type *>(node.get());
    if (type && !type->typeParameterName.empty()) {
        if (pos.column < type->typePos.column) {
            return true;
        }
    }
    return false;
}

bool IsResourcePos(const ArkAST &ast, Ptr<const Codira::AST::Node> node, Codira::Position pos)
{
    if (node && node->astKind == ASTKind::MACRO_EXPAND_EXPR && node->symbol &&
        (node->symbol->name == "r" || node->symbol->name == "rawfile")) {
        int start = ast.GetCurTokenByStartColumn(node->begin, 0,
            static_cast<int>(ast.tokens.size()) - 1);
        if (start == -1) {
            return false;
        }
        int end = ast.GetCurTokenByStartColumn(node->end, 0, static_cast<int>(ast.tokens.size()) - 1);
        if (end == -1) {
            return false;
        }
        std::vector<Codira::Token> resourceTokens =
            std::vector<Codira::Token>(ast.tokens.begin() + start, ast.tokens.begin() + end);
        int curIdx = ast.GetCurTokenByStartColumn(pos, 0, static_cast<int>(ast.tokens.size()) - 1) - start;
        if (curIdx < 0 || curIdx >= static_cast<int>(resourceTokens.size())) {
            return false;
        }
        auto it = std::remove_if(resourceTokens.begin(), resourceTokens.end(),
            [](const Token& token) {
                return token.kind == TokenKind::NL;
            });
        resourceTokens.erase(it, resourceTokens.end());
        int argStart = 2;
        int argEnd = static_cast<int>(resourceTokens.size() - 1);
        if (curIdx > argStart && curIdx < argEnd) {
            return true;
        }
    }
    return false;
}

std::string LSPJoinPath(const std::string &base, const std::string &append)
{
#ifdef _WIN32
    return PathWindowsToLinux(JoinPath(base, append));
#else
    return JoinPath(base, append);
#endif
}

std::string Digest(const std::string &pkgPath)
{
    if (!FileUtil::FileExist(pkgPath)) {
        return "";
    }
    // scan package source and digest
    std::string reason;
    std::string contents;
    // pkgPath is a regular file
    if (!IsDir(pkgPath) && HasExtension(pkgPath, "code")) {
        contents += Codira::CODIRA_VERSION;
        contents = FileUtil::ReadFileContent(pkgPath, reason).value_or("");
        return std::to_string(std::hash<std::string>{}(contents));
    }

    if (IsDir(pkgPath)) {
        auto files = GetAllFilesUnderCurrentPath(pkgPath, "code", true);
        if (files.empty()) {
            return "";
        }
        contents += Codira::CODIRA_VERSION + pkgPath;
        for (const auto &file : files) {
            auto filePath = pkgPath + FILE_SEPARATOR + file;
            contents += filePath + FileUtil::ReadFileContent(filePath, reason).value_or("");
        }
    }
    return std::to_string(std::hash<std::string>{}(contents));
}

std::string DigestForCodeo(const std::string &codeoPath)
{
    if (!FileUtil::FileExist(codeoPath)) {
        return "";
    }
    std::string reason;
    std::string contents = FileUtil::ReadFileContent(codeoPath, reason).value_or("");
    contents += Codira::CODIRA_VERSION + codeoPath;
    return std::to_string(std::hash<std::string>{}(contents));
}

lsp::SymbolID GetSymbolId(const Decl &decl)
{
    auto identifier = decl.exportId;
    if (decl.astKind == ASTKind::FUNC_PARAM) {
        CODEC_NULLPTR_CHECK(decl.outerDecl);
        if (!decl.outerDecl) {
            return lsp::INVALID_SYMBOL_ID;
        }
        CODEC_ASSERT(!decl.outerDecl->exportId.empty());
        identifier = decl.outerDecl->exportId + "$" + decl.identifier;
    }
    if (identifier.empty()) {
        return lsp::INVALID_SYMBOL_ID;
    }
    size_t ret = 0;
    ret = hash_combine<std::string>(ret, identifier);
    return ret;
}

uint32_t GetFileIdForDB(const std::string &fileName)
{
    size_t ret = 0;
    ret = hash_combine<std::string>(ret, fileName) & 0x7FFFFFFF;
    return ret;
}

std::vector<std::string> GetFuncParamsTypeName(Ptr<const Codira::AST::FuncDecl> decl)
{
    std::vector<std::string> paramsLists;
    for (auto &paramList : decl->funcBody->paramLists) {
        for (auto &param : paramList->params) {
            if (param == nullptr || param->ty == nullptr) {
                (void)paramsLists.emplace_back("");
                continue;
            }
            (void)paramsLists.emplace_back(GetString(*param->ty));
        }
    }
    return paramsLists;
}

Range GetConstructorRange(const Decl &decl, const std::string identifier)
{
    Range range;
    // we can't pass by ownership of funcBody as it's a unique_ptr
    auto funcDecl = dynamic_cast<const FuncDecl *>(&decl);
    if (funcDecl == nullptr || funcDecl->funcBody == nullptr) {
        return range;
    }
    if (funcDecl->funcBody->parentClassLike != nullptr) {
        range = {funcDecl->funcBody->parentClassLike->GetIdentifierPos(),
                 funcDecl->funcBody->parentClassLike->GetIdentifierPos()};
    } else if (funcDecl->funcBody->parentStruct != nullptr) {
        range = {funcDecl->funcBody->parentStruct->GetIdentifierPos(),
                 funcDecl->funcBody->parentStruct->GetIdentifierPos()};
    } else if (funcDecl->funcBody->parentEnum != nullptr) {
        range = {funcDecl->funcBody->parentEnum->GetIdentifierPos(),
                 funcDecl->funcBody->parentEnum->GetIdentifierPos()};
    }
    int realLength = static_cast<int>(CountUnicodeCharacters(identifier));
    range.end.column += realLength;
    if (decl.TestAttr(Codira::AST::Attribute::PRIMARY_CONSTRUCTOR) ||
        !decl.TestAttr(Codira::AST::Attribute::COMPILER_ADD)) {
        range = GetDeclRange(decl, realLength);
    }
    return range;
}

std::string GetConstructorIdentifier(const Decl &decl, bool getTargetName)
{
    std::string identifier = "";
    auto funcDecl = dynamic_cast<const FuncDecl *>(&decl);
    if (funcDecl == nullptr || funcDecl->funcBody == nullptr) {
        return identifier;
    }
    if (getTargetName && funcDecl->TestAttr(Attribute::CONSTRUCTOR) && !funcDecl->TestAttr(Attribute::COMPILER_ADD)) {
        return funcDecl->identifier;
    }
    if (funcDecl->funcBody->parentClassLike != nullptr) { // should be in the class
        identifier = funcDecl->funcBody->parentClassLike->identifier;
    } else if (funcDecl->funcBody->parentStruct != nullptr) { // should be in the struct
        identifier = funcDecl->funcBody->parentStruct->identifier;
    } else if (funcDecl->funcBody->parentEnum != nullptr) { // should be in the enum
        identifier = funcDecl->funcBody->parentEnum->identifier;
    }
    return identifier;
}

std::string GetVarDeclType(Ptr<const VarDecl> decl)
{
    std::string detail;
    if (decl == nullptr || decl->ty == nullptr) {
        return detail;
    }
    if (decl->ty->kind != TypeKind::TYPE_FUNC) {
        detail = GetString(*decl->ty);
    }
    auto funcTy = dynamic_cast<FuncTy *>(decl->ty.get());
    if (funcTy == nullptr) {
        return detail;
    }
    detail += "(";
    for (auto &param : funcTy->paramTys) {
        if (detail != "(") {
            detail += ", ";
        }
        if (param != nullptr) {
            detail += GetString(*param);
        }
    }
    detail += ")";
    if (funcTy->retTy != nullptr) {
        detail += ": ";
        detail += funcTy->retTy->String();
    }
    return detail;
}

std::string GetStandardDeclAbsolutePath(Ptr<const Decl> decl, std::string &path)
{
    std::string absolutePath = "";
    if (decl->astKind == ASTKind::BUILTIN_DECL) {
        return "";
    }

    if (!FileUtil::IsAbsolutePath(path)) {
        auto codePath = CompilerCodiraProject::GetInstance()->GetStdLibPath();
        auto base = FileUtil::GetFileBase(path);
        auto extension = FileUtil::GetFileExtension(path);
        for (auto &i : base) {
            if (i == '.') {
                i = '/';
            }
        }
        path = base + '.' + extension;
        absolutePath = codePath + FILE_SEPARATOR + path;
        if (FileUtil::FileExist(absolutePath)) {
            return absolutePath;
        }
    }
    return "";
}

bool IsModifierBeforeDecl(Ptr<const Decl> decl, const Position &pos)
{
    if (decl && decl->identifier.IsRaw()) {
        return (pos.fileID == decl->GetBegin().fileID && pos >= (decl->GetBegin() - 1) &&
                pos < (decl->GetIdentifierPos() - 1));
    }
    return !decl ||
           (pos.fileID == decl->GetBegin().fileID && pos >= decl->GetBegin() && pos < decl->GetIdentifierPos());
}

Range GetProperRange(const Ptr<Node>& node, const std::vector<Codira::Token> &tokens, bool converted)
{
    Range range;
    if (node->astKind == ASTKind::FUNC_ARG) {
        if (auto funcArg = dynamic_cast<FuncArg *>(node.get())) {
            range.start = funcArg->name.Begin();
            range.end = funcArg->name.Begin() + CountUnicodeCharacters(funcArg->name);
            UpdateRange(tokens, range, *node);
            return TransformFromChar2IDE(range);
        }
    }
    range.start = node->GetBegin();
    range.end = node->GetEnd();
    UpdateRange(tokens, range, *node);
    return TransformFromChar2IDE(range);
}

#ifdef _WIN32
void GetRealFileName(std::string &fileName, std::string &filePath)
{
    auto dirPath = FileUtil::GetDirPath(filePath);
    auto files = FileUtil::GetAllFilesUnderCurrentPath(dirPath, "code", false);
    for (const auto& file : files) {
        auto lowerName = file;
        LowFileName(lowerName);
        if (lowerName != fileName) {
            continue;
        }
        fileName = file;
        break;
    }
}
#endif

std::string Trim(const std::string &str)
{
    std::string result = str;
    (void)result.erase(result.begin(),
                       std::find_if(result.begin(), result.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    (void)result.erase(
        std::find_if(result.rbegin(), result.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(),
        result.end());

    return result;
}

std::string LTrim(std::string& str)
{
    std::string result = str;
    (void)result.erase(str.begin(), std::find_if(str.begin(), str.end(),
                                        [](const auto& it) { return !std::isspace(it); }));

    return result;
}

std::string RTrim(std::string& str)
{
    std::string result = str;
    str.erase(
        std::find_if(str.rbegin(), str.rend(),
            [](const auto& it) { return !std::isspace(it); }).base(), str.end());

    return result;
}

std::string GetRealPkgNameFromPath(const std::string &str)
{
    auto realPkgName = str;
#ifdef _WIN32
    auto pkgName = Codira::StringConvertor::GBKToUTF8(str);
    if (pkgName) {
        realPkgName = pkgName.value();
    }
#endif
    return realPkgName;
}

bool CheckIsRawIdentifier(Ptr<Node> node)
{
    if (auto decl = DynamicCast<Decl *>(node); decl && decl->identifier.IsRaw()) {
        return true;
    } else if (auto vOrEPattern = DynamicCast<VarOrEnumPattern *>(node);
        vOrEPattern && vOrEPattern->identifier.IsRaw()) {
        return true;
    } else if (auto arg = DynamicCast<FuncArg *>(node); arg && arg->name.IsRaw()) {
        return true;
    } else if (auto member = DynamicCast<MemberAccess *>(node); member && member->field.IsRaw()) {
        return true;
    }
    return false;
}

bool InImportSpec(const File &file, Position pos)
{
    if (pos == INVALID_POSITION) {
        return false;
    }
    if (file.imports.empty()) {
        return true;
    } else {
        auto line = file.imports.back()->begin.line;
        if (pos.line > line) {
            return false;
        }
    }
    return true;
}

bool IsInCodelibDir(const std::string &path)
{
    auto codeLibPath = CompilerCodiraProject::GetInstance()->GetStdLibPath();
    if (!codeLibPath.empty() && path.find(codeLibPath) != std::string::npos) {
        return true;
    }
    return false;
}

void CategorizeFiles(
    const std::vector<std::string> &files, std::vector<std::string> &nativeFiles)
{
    for (const auto &file : files) {
        std::string baseName = file.substr(0, file.find(".code"));
        nativeFiles.push_back(file);
    }
}

bool EndsWith(const std::string &str, const std::string &suffix)
{
    if (str.length() >= suffix.length()) {
        return (str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0);
    } else {
        return false;
    }
}

bool RemoveFilePathExtension(const std::string &path, const std::string &extension, std::string &res)
{
    res = path;
    if (res.size() >= extension.size()
        && res.compare(res.size() - extension.size(), extension.size(), extension) == 0) {
        res = res.substr(0, res.size() - extension.size());
        return true;
    }
    return false;
}

bool IsUnderPath(const std::string &basePath, const std::string &targetPath)
{
    auto normalizePath = [](const std::string &path) -> std::string {
        std::string normalizedPath = path;
        if (!normalizedPath.empty() && (normalizedPath.back() == '/' || normalizedPath.back() == '\\')) {
            normalizedPath.pop_back();
        }
        return normalizedPath;
    };

    std::string normBasePath = normalizePath(basePath);
    std::string normTargetPath = normalizePath(targetPath);

    std::replace(normBasePath.begin(), normBasePath.end(), '\\', '/');
    std::replace(normTargetPath.begin(), normTargetPath.end(), '\\', '/');

    if (normBasePath.back() != '/') {
        normBasePath += "/";
    }

    if (normTargetPath.find(normBasePath) == 0) {
        if (normTargetPath.size() == normBasePath.size() || normTargetPath[normBasePath.size() - 1] == '/') {
            return true;
        }
    }

    return false;
}

std::string GetSubStrBetweenSingleQuote(const std::string& str)
{
    size_t start = str.find('\'');
    if (start == std::string::npos) {
        return "";
    }

    size_t end = str.find('\'', start + 1);
    if (end == std::string::npos) {
        return "";
    }

    return str.substr(start + 1, end - start - 1);
}

void GetCurFileImportedSymbolIDs(Codira::ImportManager *importManager, Ptr<const File> file,
    std::unordered_set<ark::lsp::SymbolID> &symbolIDs)
{
    if (!file || !importManager) {
        return;
    }
    for (auto const &declSet : importManager->GetImportedDecls(*file)) {
        for (const auto &decl : declSet.second) {
            if (!decl) {
                continue;
            }
            symbolIDs.insert(GetDeclSymbolID(*decl));
        }
    }
}

ark::lsp::SymbolID GetDeclSymbolID(const Decl& decl)
{
    auto identifier = decl.exportId;
    if (decl.astKind == ASTKind::FUNC_PARAM) {
        if (!decl.outerDecl) {
            return ark::lsp::INVALID_SYMBOL_ID;
        }
        identifier = decl.outerDecl->exportId + "$" + decl.identifier;
    }
    if (identifier.empty()) {
        return ark::lsp::INVALID_SYMBOL_ID;
    }
    size_t ret = 0;
    ret = hash_combine<std::string>(ret, identifier);
    return ret;
}

bool IsValidIdentifier(const std::string& identifier)
{
    if (identifier.empty()) {
        return false;
    }

    wchar_t firstChar = identifier[0];
    if (!(std::iswalpha(firstChar) || firstChar == L'_')) {
        return false;
    }

    for (size_t i = 1; i < identifier.length(); ++i) {
        wchar_t ch = identifier[i];
        if (!(std::iswalnum(ch) || ch == L'_')) {
            return false;
        }
    }

    return true;
}

bool DeleteCharForPosition(std::string& text, int row, int column)
{
    if (row < 1 || column < 1) {
        return false;
    }
    int r = 1, c = 1;
    for (size_t i = 0; i < text.length(); i++) {
        if (r == row && c == column) {
            text.erase(i, 1);
            return true;
        }
        if (text[i] == '\n' || text[i] == '\r') {
            r++;
            c = 1;
            if (text[i] == '\r' && i + 1 < text.length() && text[i + 1] == '\n') {
                i++;
            }
        } else {
            c++;
        }
    }
    return false;
}

uint64_t GenTaskId(const std::string &packageName)
{
    return std::hash<std::string>{}(packageName);
}

char GetSeparator()
{
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

bool IsFirstSubDir(const std::string& dir, const std::string& subDir)
{
    if (subDir.substr(0, subDir.find_last_of(GetSeparator())) == dir) {
        return true;
    }
    return false;
}

int GetCurTokenInTargetTokens(const Position &pos, const std::vector<Token> &tokens, int start, int end)
{
    if (start > end) {
        return -1;
    }

    int midIndex = (start + end) / 2;
    auto midToken = tokens[static_cast<unsigned int>(midIndex)];
    Position midTokenBegin = midToken.Begin();
    // case: a  [pos] b, we can skip the space to find a's index
    Position midTokenEndNextPos = midToken.End();

    if (pos <= midTokenBegin) {
        return GetCurTokenInTargetTokens(pos, tokens, start, midIndex - 1);
    }
    if (pos <= midTokenEndNextPos) {
        return midIndex;
    }
    return GetCurTokenInTargetTokens(pos, tokens, midIndex + 1, end);
}

std::string remove_quotes(std::string str)
{
    auto newEnd = std::remove_if(str.begin(), str.end(), [](char ch) {
        return ch == '\"' || ch == '\'';
    });
    str.erase(newEnd, str.end());
    return str;
}

IDArray GetArrayFromID(uint64_t hash)
{
    uint64_t bit = 8;
    IDArray result(bit, 0);
    for (unsigned I = 0; I < result.size(); ++I) {
        result[I] = uint8_t(hash);
        hash >>= bit;
    }
    return result;
}

std::optional<std::string> GetSysCap(const Expr& e)
{
    Ptr<const LitConstExpr> lce = nullptr;
    if (e.astKind == ASTKind::CALL_EXPR) {
        auto ce = StaticCast<CallExpr>(&e);
        if (ce->args.size() == 1 && ce->args[0]->expr) {
            lce = DynamicCast<LitConstExpr>(ce->args[0]->expr.get());
        }
    } else if (e.astKind == ASTKind::LIT_CONST_EXPR) {
        lce = StaticCast<LitConstExpr>(&e);
    }
    if (!lce || lce->kind != LitConstKind::STRING) {
        return {};
    }
    return lce->stringValue;
}

std::string GetSysCapFromDecl(const Decl &decl)
{
    std::string syscapName{};
    for (auto& annotation : decl.annotations) {
        if (annotation->identifier != "APILevel") {
            continue;
        }
        for (auto& arg : annotation->args) {
            if (arg->name != "syscap") {
                continue;
            }
            if (!arg->expr) {
                return syscapName;
            }
            auto syscapName = ark::GetSysCap(*arg->expr.get());
            return syscapName.value_or("");
        }
        break;
    }
    return syscapName;
}

TokenKind FindPreFirstValidTokenKind(const ark::ArkAST &input, int index)
{
    if (index >= input.tokens.size()) {
        return TokenKind::INIT;
    }
    while (--index > 0) {
        auto kind = input.tokens[index].kind;
        if (kind != TokenKind::NL && kind != TokenKind::COMMENT) {
            return kind;
        }
    }
    return TokenKind::INIT;
}

Position FindLastImportPos(const File &file)
{
    int lastImportLine = 0;
    for(const auto &import : file.imports) {
        if (!import) 
        {
            continue;
        }
        lastImportLine = std::max(import->content.rightCurlPos.line, std::max(import->importPos.line, lastImportLine));
    }
    Position pkgPos = file.package->packagePos;
    if (lastImportLine == 0 && pkgPos.line > 0) {
        lastImportLine = pkgPos.line;
    }
    return {pkgPos.fileID, lastImportLine + 1, 1};
}

std::vector<std::string> Split(const std::string &str, const std::string &pattern)
{
    std::vector<std::string> result;
    if (str.empty())
        return result;
    std::string strs = str + pattern;
    size_t pos = strs.find(pattern);

    while (pos != std::string::npos) {
        std::string temp = strs.substr(0, pos);
        result.push_back(temp);
        strs = strs.substr(pos + 1, strs.size());
        pos = strs.find(pattern);
    }
    return result;
}

std::vector<std::string> GetAllFilePathUnderCurrentPath(const std::string& path, const std::string& extension,
    bool shouldSkipTestFiles, bool shouldSkipRegularFiles)
{
    std::vector<std::string> allFiles;
    auto files = FileUtil::GetAllFilesUnderCurrentPath(path, extension,
        shouldSkipTestFiles, shouldSkipRegularFiles);
    std::for_each(files.begin(), files.end(), [&allFiles, &path](const std::string &fileName) {
        allFiles.emplace_back(NormalizePath(FileUtil::JoinPath(path, fileName)));
    });
    auto dirs = FileUtil::GetAllDirsUnderCurrentPath(path);
    std::for_each(dirs.begin(), dirs.end(), [&](const std::string &dir) {
        auto files = FileUtil::GetAllFilesUnderCurrentPath(dir, extension,
            shouldSkipTestFiles, shouldSkipRegularFiles);
        std::for_each(files.begin(), files.end(), [&allFiles, &dir](const std::string &fileName) {
            allFiles.emplace_back(NormalizePath(FileUtil::JoinPath(dir, fileName)));
        });
    });
    return allFiles;
}
} // namespace ark
