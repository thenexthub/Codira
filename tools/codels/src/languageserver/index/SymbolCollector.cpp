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

#include "SymbolCollector.h"
#include <cangjie/AST/PrintNode.h>
#include <queue>

#include "../CompilerCodiraProject.h"
#include "CodedIndex.h"
#include "MemIndex.h"

namespace {
using namespace Codira;
using namespace AST;
Ptr<Decl> GetRealDecl(Ptr<Decl> decl)
{
    if (auto fd = DynamicCast<FuncDecl *>(decl); fd && fd->propDecl) {
        return fd->propDecl;
    }
    return decl;
}

TypeSubst GenerateTypeMapping(Ptr<const Generic> generic, const std::vector<Ptr<Ty>> &typeArgs)
{
    TypeSubst mapping;
    if (!generic || generic->typeParameters.size() != typeArgs.size()) {
        return mapping;
    }
    for (size_t i = 0; i < typeArgs.size(); ++i) {
        if (Ty::IsTyCorrect(generic->typeParameters[i]->ty) && Ty::IsTyCorrect(typeArgs[i])) {
            if (auto genParam = DynamicCast<GenericsTy*>(generic->typeParameters[i]->ty)) {
                mapping[genParam] = typeArgs[i];
            }
        }
    }
    return mapping;
}

inline bool IsFitBaseMemberOverriddenCondition(const Decl &parent, const Decl &child)
{
    // Members must have same astKind, identifier, static status, generic status.
    return parent.astKind == child.astKind && parent.identifier == child.identifier &&
           !child.TestAnyAttr(Attribute::CONSTRUCTOR, Attribute::ENUM_CONSTRUCTOR) &&
           (parent.TestAttr(Attribute::STATIC) == child.TestAttr(Attribute::STATIC)) &&
           (parent.TestAttr(Attribute::GENERIC) == child.TestAttr(Attribute::GENERIC));
}

void GetAliasInImportMulti(std::unordered_map<std::string, std::string> aliasMap,
                           const std::vector<ImportContent>& contents)
{
    for (const auto &content : contents) {
        if (content.kind == ImportKind::IMPORT_ALIAS) {
            (void)aliasMap.insert_or_assign(content.identifier, content.aliasName);
        }
    }
}

inline bool NeedToObtainCodedDeclPos(bool isCodeoPkg)
{
    return ark::MessageHeaderEndOfLine::GetIsDeveco() && ark::lsp::CodedIndexer::GetInstance() != nullptr && isCodeoPkg &&
           !ark::lsp::CodedIndexer::GetInstance()->GetRunningState();
}

Ptr<const Node> GetCurMacro(const Decl& decl)
{
    Ptr<const Node> curCall = nullptr;
    if (decl.curMacroCall) {
        curCall = decl.curMacroCall;
    } else {
        Ptr<const Decl> tmpDecl = &decl;
        while (tmpDecl->outerDecl) {
            tmpDecl = tmpDecl->outerDecl;
            if (tmpDecl->curMacroCall) {
                curCall = tmpDecl->curMacroCall;
                break;
            }
        }
    }
    return curCall;
}

CommentGroups FindCommentsInMacroCall(const Ptr<const Decl> node, Ptr<const Node> curCall)
{
    if (!node || !curCall) {
        return {};
    }

    CommentGroups foundComments;
    std::string id = node->identifier;
    auto begin = node->GetBegin();
    auto end = node->GetEnd();

    if (curCall) {
        ConstWalker walker(curCall, [&foundComments, &id, &begin, &end](Ptr<const Node> node) -> VisitAction {
            Meta::match (*node)([&foundComments, &id, &begin, &end](const Decl& decl) {
                const auto [leadCommentGroup, innerCommentGroup, trailCommentGroup] = decl.comments;
                if (decl.identifier == id &&
                    begin == decl.GetBegin() &&
                    end == decl.GetEnd() &&
                    !decl.comments.IsEmpty()) {
                    foundComments = decl.comments;
                }
            });

            if (!foundComments.IsEmpty()) {
                return VisitAction::STOP_NOW;
            }
            return VisitAction::WALK_CHILDREN;
        });
        walker.Walk();
    }

    return foundComments;
}

ark::lsp::Modifier GetDeclModifier(const Decl &decl)
{
    if (decl.TestAttr(Attribute::PUBLIC)) {
        return ark::lsp::Modifier::PUBLIC;
    } else if (decl.TestAttr(Attribute::PROTECTED)) {
        return ark::lsp::Modifier::PROTECTED;
    } else if (decl.TestAttr(Attribute::INTERNAL)) {
        return ark::lsp::Modifier::INTERNAL;
    } else if (decl.TestAttr(Attribute::PRIVATE)) {
        return ark::lsp::Modifier::PRIVATE;
    }
    return ark::lsp::Modifier::UNDEFINED;
}

ark::lsp::Modifier GetPackageModifier(AccessLevel pkgAccess)
{
    switch (pkgAccess) {
        case AccessLevel::PUBLIC:
            return ark::lsp::Modifier::PUBLIC;
        case AccessLevel::PROTECTED:
            return ark::lsp::Modifier::PROTECTED;
        case AccessLevel::INTERNAL:
            return ark::lsp::Modifier::INTERNAL;
        case AccessLevel::PRIVATE:
            return ark::lsp::Modifier::PRIVATE;
        default:
            return ark::lsp::Modifier::PUBLIC;
    }
}

ark::lsp::CommentGroups GetDeclComments(const Decl &decl)
{
    if (!decl.comments.IsEmpty()) {
        return decl.comments;
    }
    CommentGroups foundComments;

    ConstWalker walker(&decl, [&foundComments](Ptr<const Node> node) -> VisitAction {
        Meta::match (*node)([&foundComments](const Decl& decl) {
            if (auto curCall = GetCurMacro(decl)) {
                CommentGroups comments = FindCommentsInMacroCall(&decl, curCall);
                if (!comments.IsEmpty()) {
                    foundComments = comments;
                }
            }
        });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();

    return foundComments;
}

bool IsExportExtendSuper(Ptr<ClassLikeDecl> decl)
{
    if (!decl || decl->TestAttr(Attribute::PRIVATE)) {
        return false;
    }
    if (decl->TestAnyAttr(Attribute::PUBLIC, Attribute::PROTECTED, Attribute::INTERNAL)) {
        return true;
    }
    // When the decl is `extend A<B>`, B may be decl without modifiers such as GenericParamDecl, BuiltinDecl.
    // In this case, they must be exported for extend's extendType checking.
    return true;
}

bool CheckExtendExported(const ExtendDecl* decl)
{
    auto extendedDecl = Ty::GetDeclPtrOfTy<InheritableDecl>(decl->ty);
    bool isInSamePkg = extendedDecl && extendedDecl->fullPackageName == decl->fullPackageName;
    auto isUpperBoundExport = [decl]() {
        bool isUpperboundAllExported = true;
        for (auto& tp : decl->generic->genericConstraints) {
            if (!tp) { return false; }
            for (auto& up : tp->upperBounds) {
                if (!up || up->GetTarget() == nullptr) {
                    continue;
                }
                isUpperboundAllExported = up->GetTarget()->IsExportedDecl() && isUpperboundAllExported;
            }
        }
        return isUpperboundAllExported;
    };
    // Direct Extensions Check:
    if (decl->inheritedTypes.empty()) {
        // Rule 1: In `package std.core`, direct extensions of types visible outside the package are exported.
        if (decl->fullPackageName == "std.core") {
            return true;
        }
        if (isInSamePkg) {
            // Rule 2: When direct extensions are defined in the same `package` as the extended type, whether the
            // extension is exported is determined by the lowest access level of the type used in the extended type and
            // the generic constraints (if any).
            if (!decl->generic) {
                return extendedDecl->IsExportedDecl();
            }
            return extendedDecl->IsExportedDecl() && isUpperBoundExport();
        }
        // Rule 3: When the direct extension is in a different `package` from the declaration of the type being
        // extended, the extension is never exported and can only be used in the current `package`.
        return false;
    }
    // Interface Extensions Check:
    if (isInSamePkg) {
        // Rule 1: When the interface extension and the extended type are in the same `package`, the extension is
        // exported together with the extended type and is not affected by the access level of the interface type.
        return extendedDecl->IsExportedDecl();
    }
    // Rule 2: When an interface extension is in a different `package` from the type being extended, whether the
    // interface extension is exported is determined by the smallest access level of the type used in the
    // interface type and the generic constraints (if any).
    bool isInterfaceAllExported = false;
    for (auto& inhertType : decl->inheritedTypes) {
        if (!inhertType || !inhertType->GetTarget()) {
            continue;
        }
        auto target = inhertType->GetTarget();
        if (auto decl = DynamicCast<ClassLikeDecl>(target)) {
            if (IsExportExtendSuper(decl)) {
                isInterfaceAllExported = true;
                break;
            }
        }
    }
    if (!decl->generic) {
        return isInterfaceAllExported;
    }
    return isInterfaceAllExported && isUpperBoundExport();
}

bool IsExportedExtendDecl(const ExtendDecl* decl)
{
    if (!decl) {
        return false;
    }
    // ExtendedType Check (Direct and Interface Extensions): If B in extend A<B> isn't exported, the extendDecl should
    // not be exported. For imported decl, extendedType may be nullptr and not ready.
    if (decl->extendedType != nullptr) {
        for (auto& it : decl->extendedType->GetTypeArgs()) {
            if (it && it->GetTarget() && !it->GetTarget()->IsExportedDecl()) {
                return false;
            }
        }
    }
    return CheckExtendExported(decl);
}
} // namespace

namespace ark::lsp {
const std::unordered_set<ASTKind> G_IGNORE_KINDS{ASTKind::MAIN_DECL, ASTKind::MACRO_DECL,
                                                 ASTKind::VAR_WITH_PATTERN_DECL,
                                                 ASTKind::PRIMARY_CTOR_DECL, ASTKind::INVALID_DECL};

enum class ElementIndex { NAME = 0, LOCATION = 1, ID = 2, DECLARATION = 3 };
void SymbolCollector::Preamble(const Package &package)
{
    for (const auto &file : package.files) {
        for (const auto &import : file->imports) {
            if (import->IsImportAlias()) {
                (void)aliasMap.insert_or_assign(import->content.identifier, import->content.aliasName.Val());
            }
            if (import->IsImportMulti()) {
                GetAliasInImportMulti(aliasMap, import->content.items);
            }
        }
    }
}

bool SymbolCollector::ShouldPassInCodedIndexing(Ptr<Node> node)
{
    return CodedIndexer::GetInstance() && CodedIndexer::GetInstance()->GetRunningState() && DynamicCast<FuncDecl *>(node);
}

void SymbolCollector::Build(const Package &package)
{
    Preamble(package);
    std::unordered_set<Ptr<InheritableDecl>> inheritableDecls;
    (void)scopes.emplace_back(&package, package.fullPackageName + ":");
    AccessLevel pkgAccess = package.accessible;
    for (auto &file : package.files) {
        auto filePath = file->curFile->filePath;
        auto collectPre = [this, &inheritableDecls, &filePath, &pkgAccess](auto node) {
            if (auto invocation = node->GetConstInvocation()) {
                CreateMacroRef(*node, *invocation);
            }
            if (!Ty::IsTyCorrect(node->ty)) {
                if (!ShouldPassInCodedIndexing(node)) {
                    return VisitAction::WALK_CHILDREN;
                }
            }
            if (node->astKind == ASTKind::PRIMARY_CTOR_DECL ||
                node->TestAnyAttr(Attribute::MACRO_INVOKE_FUNC, Attribute::IN_CORE)) {
                return VisitAction::SKIP_CHILDREN;
            }
            if (auto fd = DynamicCast<FuncDecl *>(node); fd && fd->propDecl) {
                return VisitAction::WALK_CHILDREN;
            } else if (auto id = DynamicCast<InheritableDecl *>(node)) {
                (void)inheritableDecls.emplace(id);
                if (IsHiddenDecl(id)) {
                    return VisitAction::SKIP_CHILDREN;
                }
            }
            if (Utils::In(node->astKind, G_IGNORE_KINDS)) {
                return VisitAction::WALK_CHILDREN;
            }
            if (IsHiddenDecl(node)) {
                return VisitAction::SKIP_CHILDREN;
            }
            CollectNode(node, filePath, pkgAccess);
            return VisitAction::WALK_CHILDREN;
        };

        auto collectPost = [this](auto node) {
            RestoreScope(*node);
            RestoreCrossScope(*node);
            return VisitAction::WALK_CHILDREN;
        };

        Walker(file.get(), collectPre, collectPost).Walk();

        // Some desugar node information is stored in trashBin.
        for (auto &it : file->trashBin) {
            Walker(it.get(), collectPre, collectPost).Walk();
        }

        for (auto &it : file->originalMacroCallNodes) {
            auto invocation = it->GetConstInvocation();
            if (!invocation) {
                continue;
            }
            CreateMacroRef(*it, *invocation);
            Walker(invocation->decl.get(), [this](auto node) {
                if (auto i = node->GetConstInvocation()) {
                    CreateMacroRef(*node, *i);
                    return VisitAction::WALK_CHILDREN;
                }
                return VisitAction::WALK_CHILDREN;
            }).Walk();
        }
        if (!CodedIndexer::GetInstance() || !CodedIndexer::GetInstance()->GetRunningState()) {
           CreateImportRef(*file); 
        }
    }
    scopes.pop_back();
    CollectRelations(inheritableDecls);
}

void SymbolCollector::CollectCrossScopes(Ptr<Node> node)
{
    if (auto assignExpr = DynamicCast<AssignExpr *>(node)) {
        if (!crossRegisterScopes.empty() &&
            crossRegisterScopes.back().second.first != CrossRegisterType::MODULE_REGISTER) {
                return;
            }
        UpdateCrossScope(*assignExpr, CrossRegisterType::MODULE_REGISTER, "");
    }
}

void SymbolCollector::CreateBaseOrExtendSymbol(const Decl &decl, const std::string &filePath, AccessLevel pkgAccess)
{
    auto inSupportBaseSym = IsGlobalOrMemberOrItsParam(decl) || IsLocalFuncOrLambda(decl);
    if (inSupportBaseSym) {
        CreateBaseSymbol(decl, filePath, pkgAccess);
    }  else if (IsExtendDecl(decl)) {
        CreateExtend(decl, filePath);
    }
}

void SymbolCollector::CreateBaseSymbol(const Decl &decl, const std::string &filePath, AccessLevel pkgAccess)
{
    if (!decl.curFile) {
        return;
    }
    CODEC_ASSERT(!scopes.empty());
    std::string curScope;
    for (auto [_, scope] : scopes) {
        curScope += scope;
    }
    curScope.pop_back(); // Pop back last delimiter char.

    auto &identifier =
        decl.TestAttr(Attribute::PRIMARY_CONSTRUCTOR) ? decl.identifierForLsp : decl.identifier;
    SymbolLocation loc;
    if (decl.TestAttr(Attribute::IMPLICIT_ADD, Attribute::CONSTRUCTOR)) {
        loc = {.begin = {0, 0}, .end = {0, 0}, .fileUri = decl.curFile->filePath};
    } else {
        auto identifierPos = decl.GetIdentifierPos();
        auto uri = decl.curFile->filePath;
        // decl in .macroCall file
        if (identifierPos.fileID != decl.begin.fileID) {
            uri = decl.curFile->macroCallFilePath;
        }
        loc = {identifierPos, identifierPos + CountUnicodeCharacters(identifier), uri};
    }
    SymbolLocation curMacroCall = {.begin = {0, 0}, .end = {0, 0}, .fileUri = ""};
    auto curMacroCallNode = decl.curMacroCall;
    if (curMacroCallNode && curMacroCallNode->curFile) {
        curMacroCall = {curMacroCallNode->begin, curMacroCallNode->end, curMacroCallNode->curFile->filePath};
    }
    SymbolLocation zeroLoc = {.begin = {0, 0}, .end = {0, 0}, .fileUri = ""};
    SymbolLocation declaration = {.begin = {0, 0}, .end = {0, 0}, .fileUri = ""};
    CommentGroups comments = GetDeclComments(decl);

    if (NeedToObtainCodedDeclPos(isCodeoPkg)) {
        declaration = CodedIndexer::GetInstance()->GetSymbolDeclaration(GetDeclSymbolID(decl), decl.fullPackageName);
        comments = CodedIndexer::GetInstance()->GetSymbolComments(GetDeclSymbolID(decl), decl.fullPackageName);
    }
    Symbol declSym;
    declSym.id = GetDeclSymbolID(decl);
    declSym.name = identifier;
    declSym.scope = curScope;
    declSym.location = loc;
    declSym.declaration = declaration;
    declSym.canonicalDeclaration = zeroLoc;
    declSym.kind = decl.astKind;
    declSym.signature = ItemResolverUtil::ResolveSignatureByNode(decl);
    declSym.returnType = GetTypeString(decl);
    declSym.idArray = GetArrayFromID(declSym.id);
    // config local lambda variables kind for callHierarchy
    bool isLocalLambda = declSym.kind == ASTKind::VAR_DECL && isLambda(decl);
    if (isLocalLambda) {
        declSym.kind = ASTKind::LAMBDA_EXPR;
    }
    declSym.modifier = GetDeclModifier(decl);
    declSym.isCodeoSym = isCodeoPkg;
    declSym.syscap = GetSysCapFromDecl(decl);
    declSym.comments = comments;
    SymbolCollector::CollectCompletionItem(decl, declSym);
    declSym.curMacroCall = curMacroCall;
    if (decl.HasAnno(AnnotationKind::DEPRECATED)) {
        declSym.isDeprecated = true;
    }
    if (!declSym.isCodeoSym && decl.curFile->curPackage) {
        declSym.curModule = SplitFullPackage(decl.curFile->curPackage->fullPackageName).first;
    }
    if (auto vd = DynamicCast<const VarDecl *>(&decl)) {
        declSym.isMemberParam = vd->isMemberParam;
    }
    declSym.pkgModifier = GetPackageModifier(pkgAccess);
    if (!loc.IsZeroLoc()) {
        UpdatePos(declSym.location, decl, filePath);
    }
    (void)pkgSymsMap.emplace_back(declSym);

    Ref refInfo{.location = loc, .kind = RefKind::DEFINITION, .container = GetContextID(), .isCodeoRef = isCodeoPkg};
    if (!loc.IsZeroLoc()) {
        UpdatePos(refInfo.location, decl, filePath);
    }
    (void)symbolRefMap[declSym.id].emplace_back(refInfo);
}

void SymbolCollector::CreateCrossSymbolByInterop(const Decl &decl)
{
    bool isInvalidCrossSymbol = !IsGlobalOrMemberOrItsParam(decl) || !decl.TestAttr(Attribute::PUBLIC);
    if (isInvalidCrossSymbol) {
        return;
    }
    auto &identifier =
        decl.TestAttr(Attribute::PRIMARY_CONSTRUCTOR) ? decl.identifierForLsp : decl.identifier;
    SymbolLocation loc;
    if (decl.TestAttr(Attribute::IMPLICIT_ADD, Attribute::CONSTRUCTOR)) {
        loc = {.begin = {0, 0}, .end = {0, 0}, .fileUri = decl.curFile->filePath};
    } else {
        auto identifierPos = decl.GetIdentifierPos();
        auto uri = decl.curFile->filePath;
        // decl in .macroCall file
        if (identifierPos.fileID != decl.begin.fileID) {
            uri = decl.curFile->macroCallFilePath;
        }
        loc = {identifierPos, identifierPos + CountUnicodeCharacters(identifier), uri};
    }
    SymbolID container{};
    std::string containerName;
    if (decl.outerDecl) {
        container = GetDeclSymbolID(*decl.outerDecl);
        containerName = decl.outerDecl->identifier;
    }
    CrossSymbol crossSym;
    crossSym.id = GetDeclSymbolID(decl);
    crossSym.name = identifier;
    crossSym.crossType = CrossType::ARK_TS_WITH_INTEROP;
    crossSym.location = loc;
    crossSym.container = container;
    crossSym.containerName = containerName;
    (void)crsSymsMap.emplace_back(crossSym);

    bool isContainMember = decl.astKind == ASTKind::CLASS_DECL || decl.astKind == ASTKind::CLASS_LIKE_DECL ||
        decl.astKind == ASTKind::INTERFACE_DECL || decl.astKind == ASTKind::STRUCT_DECL;
    if (!isContainMember) {
        return;
    }
    auto &members = decl.GetMemberDecls();
    if (members.empty()) {
        return;
    }
    for (auto &member: members) {
        if (!member->GetConstInvocation()) {
            CreateCrossSymbolByInterop(*member);
            continue;
        }
        // macro expand node, has invocation, need to get real decl
        const auto invocation = member->GetConstInvocation();
        bool isInvisible = false;
        auto realDecl = &invocation->decl;
        while (realDecl && realDecl->get()->astKind == ASTKind::MACRO_EXPAND_DECL
            && realDecl->get()->GetConstInvocation()) {
            // realDecl is macro expand node, check its name, invocation and attr
            if (realDecl->get()->symbol && realDecl->get()->symbol->name == "Interop") {
                for (const auto &attr : realDecl->get()->GetConstInvocation()->attrs) {
                    if (attr.Value() == "Invisible") {
                        isInvisible = true;
                        break;
                    }
                }
            }
            // get next decl for realDecl
            realDecl = &realDecl->get()->GetConstInvocation()->decl;
        }
        if (isInvisible || !realDecl) {
            continue;
        }
        CreateCrossSymbolByInterop(*realDecl->get());
    }
}

void SymbolCollector::CreateCrossSymbolByRegister(const NameReferenceExpr &ref, const SrcIdentifier &identifier)
{
    if (!ref.curFile) {
        return;
    }
    auto callExpr = DynamicCast<const CallExpr *>(ref.callOrPattern);
    if (!callExpr || callExpr->args.empty()) {
        return;
    }
    if (identifier == CROSS_ARK_TS_WITH_REGISTER_MODULE) {
        // registerModule
        DealRegisterModule(*callExpr);
        return;
    }
    size_t argNum = 2;
    if (callExpr->args.size() != argNum) {
        return;
    }
    const auto &registerIdentify = callExpr->args.at(0);
    const auto &registerTarget = callExpr->args.at(1);
    bool isInvalidRegister = !registerIdentify || !registerTarget || !registerTarget->ty || !registerTarget->expr;
    if (isInvalidRegister) {
        return;
    }
    if (identifier == CROSS_ARK_TS_WITH_REGISTER_CLASS) {
        // registerClass
        DealRegisterClass(*registerIdentify, *registerTarget);
        return;
    }
    // registerFunc
    DealRegisterFunc(*registerIdentify, *registerTarget);
}

void SymbolCollector::DealRegisterModule(const CallExpr &callExpr)
{
    const auto arg = callExpr.args.at(0).get();
    if (!arg) {
        return;
    }
    if (arg->expr->astKind == ASTKind::REF_EXPR) {
        const auto argRef = DynamicCast<const RefExpr *>(arg->expr.get());
        if (!argRef) {
            return;
        }
        const auto argTarget = argRef->aliasTarget ? argRef->aliasTarget : GetRealDecl(argRef->GetTarget());
        if (!argTarget || !argTarget->curFile) {
            return;
        }
        const auto targetSymbolId = GetDeclSymbolID(*argTarget);
        if (crossRegisterDecls.find(targetSymbolId) == crossRegisterDecls.end()) {
            return;
        }
        for (const auto &item : crossRegisterDecls[targetSymbolId]) {
            std::string name = remove_quotes(std::get<static_cast<size_t>(ElementIndex::NAME)>(item));
            if (name.empty()) {
                continue;
            }
            CrossSymbol sym;
            sym.name = name;
            sym.crossType = CrossType::ARK_TS_WITH_REGISTER;
            sym.location = std::get<static_cast<size_t>(ElementIndex::LOCATION)>(item);
            sym.id = std::get<static_cast<size_t>(ElementIndex::ID)>(item);
            sym.declaration = std::get<static_cast<size_t>(ElementIndex::DECLARATION)>(item);
            (void) crsSymsMap.emplace_back(sym);
        }
    }
    if (arg->expr->astKind == ASTKind::LAMBDA_EXPR) {
        auto lambdaExpr = DynamicCast<const LambdaExpr *>(arg->expr.get());
        bool isInvalidLambda = !lambdaExpr || !lambdaExpr->funcBody || !lambdaExpr->funcBody->body
            || lambdaExpr->funcBody->body->body.empty();
        if (isInvalidLambda) {
            return;
        }
        UpdateCrossScope(*lambdaExpr->funcBody->body, CrossRegisterType::MODULE_REGISTER, "");
    }
}

void SymbolCollector::DealRegisterClass(const FuncArg &registerIdentify, const FuncArg &registerTarget)
{
    if (registerTarget.expr->astKind == ASTKind::REF_EXPR) {
            const auto argRef = DynamicCast<const RefExpr *>(registerTarget.expr.get());
            if (!argRef) {
                return;
            }
            const auto argTarget = argRef->aliasTarget ? argRef->aliasTarget : GetRealDecl(argRef->GetTarget());
            if (!argTarget || !argTarget->curFile) {
                return;
            }
            const auto &identifier = argTarget->identifier;
            const std::string clazName = remove_quotes(registerIdentify.ToString());
            CrossSymbol crossSym;
            crossSym.id = GetDeclSymbolID(*argTarget);
            crossSym.name = clazName;
            crossSym.crossType = CrossType::ARK_TS_WITH_REGISTER;
            crossSym.location = {argTarget->GetIdentifierPos(),
                argTarget->GetIdentifierPos() + CountUnicodeCharacters(identifier), argTarget->curFile->filePath};
            crossSym.declaration = {registerIdentify.begin, registerIdentify.end, argTarget->curFile->filePath};
            (void) crsSymsMap.emplace_back(crossSym);
            const auto targetSymbolId = GetDeclSymbolID(*argTarget);
            if (crossRegisterDecls.find(targetSymbolId) == crossRegisterDecls.end()) {
                return;
            }
            for (const auto &item : crossRegisterDecls[targetSymbolId]) {
                std::string name = remove_quotes(std::get<static_cast<size_t>(ElementIndex::NAME)>(item));
                if (name.empty()) {
                    continue;
                }
                CrossSymbol sym;
                sym.name = name;
                sym.containerName = clazName;
                sym.crossType = CrossType::ARK_TS_WITH_REGISTER;
                sym.location = std::get<static_cast<size_t>(ElementIndex::LOCATION)>(item);
                sym.id = std::get<static_cast<size_t>(ElementIndex::ID)>(item);
                sym.declaration = std::get<static_cast<size_t>(ElementIndex::DECLARATION)>(item);
                (void) crsSymsMap.emplace_back(sym);
            }
        }
        if (registerTarget.expr->astKind == ASTKind::LAMBDA_EXPR) {
            auto lambdaExpr = DynamicCast<const LambdaExpr *>(registerTarget.expr.get());
            bool isInvalidLambda = !lambdaExpr || !lambdaExpr->funcBody || !lambdaExpr->funcBody->body
            || lambdaExpr->funcBody->body->body.empty();
            if (isInvalidLambda) {
                return;
            }
            CrossSymbol crossSym;
            crossSym.id = INVALID_SYMBOL_ID;
            crossSym.name = remove_quotes(registerIdentify.ToString());
            crossSym.crossType = CrossType::ARK_TS_WITH_REGISTER;
            crossSym.location = {registerTarget.begin, registerTarget.begin + 1, registerTarget.curFile->filePath};
            crossSym.declaration = {registerIdentify.begin, registerIdentify.end, registerTarget.curFile->filePath};
            (void) crsSymsMap.emplace_back(crossSym);
            UpdateCrossScope(*lambdaExpr->funcBody->body, CrossRegisterType::CLASS_REGISTER,
                registerIdentify.ToString());
        }
}

void SymbolCollector::DealRegisterFunc(const FuncArg &registerIdentify, const FuncArg &registerTarget)
{
    // public static func registerFunc(name: String, lambda: JSLambda): Unit
    if (registerTarget.ty->String() == JS_LAMBDA_TY) {
        if (registerTarget.expr->astKind == ASTKind::LAMBDA_EXPR) {
            CrossSymbol crossSym;
            crossSym.id = INVALID_SYMBOL_ID;
            crossSym.name = remove_quotes(registerIdentify.ToString());
            crossSym.crossType = CrossType::ARK_TS_WITH_REGISTER;
            crossSym.location = {registerTarget.begin, registerTarget.begin + 1, registerTarget.curFile->filePath};
            (void) crsSymsMap.emplace_back(crossSym);
            return;
        }
        if (registerTarget.expr->astKind == ASTKind::REF_EXPR) {
            const auto argRef = DynamicCast<const RefExpr *>(registerTarget.expr.get());
            if (!argRef) {
                return;
            }
            const auto argTarget = argRef->aliasTarget ? argRef->aliasTarget : GetRealDecl(argRef->GetTarget());
            if (!argTarget || !argTarget->curFile) {
                return;
            }
            const auto &identifier = argTarget->identifier;
            CrossSymbol crossSym;
            crossSym.id = GetDeclSymbolID(*argTarget);
            crossSym.name = remove_quotes(registerIdentify.ToString());
            crossSym.crossType = CrossType::ARK_TS_WITH_REGISTER;
            crossSym.declaration = {registerIdentify.begin, registerIdentify.end, argTarget->curFile->filePath};
            crossSym.location = {argTarget->GetIdentifierPos(),
                argTarget->GetIdentifierPos() + CountUnicodeCharacters(identifier), argTarget->curFile->filePath};
            (void) crsSymsMap.emplace_back(crossSym);
            return;
        }
    }
    if (registerTarget.ty->String() != FUNC_REGISTER_TY) {
        return;
    }
    // public static func registerFunc(name: String, register: FuncRegister): Unit
    if (registerTarget.expr->astKind == ASTKind::LAMBDA_EXPR) {
        auto lambdaExpr = DynamicCast<const LambdaExpr *>(registerTarget.expr.get());
        bool isInvalidLambda = !lambdaExpr || !lambdaExpr->funcBody || !lambdaExpr->funcBody->body;
        if (isInvalidLambda) {
            return;
        }
        ResloveBlock(*lambdaExpr->funcBody->body, registerIdentify.ToString());
    }
    if (registerTarget.expr->astKind == ASTKind::REF_EXPR) {
        const auto argRef = DynamicCast<const RefExpr *>(registerTarget.expr.get());
        if (!argRef) {
            return;
        }
        const auto argTarget = argRef->aliasTarget ? argRef->aliasTarget : GetRealDecl(argRef->GetTarget());
        if (!argTarget || !argTarget->curFile) {
            return;
        }
        const auto funcDecl = DynamicCast<const FuncDecl *>(argTarget);
        bool isInvalidLambda = !funcDecl || !funcDecl->funcBody || !funcDecl->funcBody->body;
        if (isInvalidLambda) {
            return;
        }
        ResloveBlock(*funcDecl->funcBody->body, registerIdentify.ToString());
    }
}

void SymbolCollector::ResloveBlock(const Block &block, const std::string &registerIdentify)
{
    if (block.body.empty()) {
        return;
    }
    auto expr = block.body.at(block.body.size() - 1).get();
    if (!expr || (expr->astKind != ASTKind::CALL_EXPR && expr->astKind != ASTKind::RETURN_EXPR)) {
        return;
    }
    if (expr->astKind == ASTKind::RETURN_EXPR) {
        const auto returnExpr = DynamicCast<const ReturnExpr *>(expr);
        if (!returnExpr) {
            return;
        }
        expr = returnExpr->expr;
    }
    const auto functionExpr = DynamicCast<const CallExpr *>(expr);
    if (!functionExpr || functionExpr->args.empty() || !functionExpr->args.at(0)) {
        return;
    }
    const auto &registerTarget = functionExpr->args.at(0);
    if (!registerTarget->curFile || !registerTarget->expr) {
        return;
    }
    if (registerTarget->expr->astKind == ASTKind::LAMBDA_EXPR) {
        CrossSymbol crossSym;
        crossSym.id = INVALID_SYMBOL_ID;
        crossSym.name = remove_quotes(registerIdentify);
        crossSym.crossType = CrossType::ARK_TS_WITH_REGISTER;
        crossSym.location = {registerTarget->begin, registerTarget->begin + 1, registerTarget->curFile->filePath};
        (void) crsSymsMap.emplace_back(crossSym);
        return;
    }
    if (registerTarget->expr->astKind == ASTKind::REF_EXPR) {
        const auto argRef = DynamicCast<const RefExpr *>(registerTarget->expr.get());
        if (!argRef) {
            return;
        }
        const auto argTarget = argRef->aliasTarget ? argRef->aliasTarget : GetRealDecl(argRef->GetTarget());
        if (!argTarget || !argTarget->curFile) {
            return;
        }
        CrossSymbol crossSym;
        crossSym.id = GetDeclSymbolID(*argTarget);
        crossSym.name = remove_quotes(registerIdentify);
        crossSym.crossType = CrossType::ARK_TS_WITH_REGISTER;
        crossSym.location = {argTarget->GetIdentifierPos(),
            argTarget->GetIdentifierPos() + CountUnicodeCharacters(argTarget->identifier),
            argTarget->curFile->filePath};
        (void) crsSymsMap.emplace_back(crossSym);
    }
}

void SymbolCollector::DealCrossSymbol(const NameReferenceExpr &ref, const Decl &target, const SrcIdentifier &identifier)
{
    if (CROSS_ARK_TS_WITH_REGISTER_NAMES.find(identifier) != CROSS_ARK_TS_WITH_REGISTER_NAMES.end()) {
        CreateCrossSymbolByRegister(ref, identifier);
        return;
    }
    if (identifier == FUNCTION_REGISTER_SYMBOL) {
        DealCrossFunctionSymbol(ref, identifier);
        return;
    }
    if (identifier == CLAZZ_REGISTER_SYMBOL) {
        DealCrossClassSymbol(ref, identifier);
        return;
    }
    if (identifier == ADD_METHOD) {
        DealAddCrossMethodSymbol(ref);
        return;
    }
    if (identifier == "[]" && target.outerDecl && target.outerDecl->identifier == JS_OBJECT_BASE_TY) {
        DealExportsRegisterSymbol(ref);
    }
}

void SymbolCollector::DealAddCrossMethodSymbol(const NameReferenceExpr &ref)
{
    const auto addMethodCallExpr = DynamicCast<const CallExpr *>(ref.callOrPattern);
    if (!addMethodCallExpr || addMethodCallExpr->args.empty() || !addMethodCallExpr->args.at(0)->expr) {
        return;
    }
    const auto &registerIdentify = DynamicCast<const CallExpr *>(addMethodCallExpr->args.at(0)->expr.get());
    if (!registerIdentify || registerIdentify->args.size() != 1) {
        return;
    }
    CrossRegisterType registerType = CrossRegisterType::GLOBAL_FUNC_REGISTER;
    if (!crossRegisterScopes.empty()) {
        const auto scope = crossRegisterScopes.back();
        registerType = scope.second.first;
    }
    if (registerType == CrossRegisterType::CLASS_REGISTER) {
        UpdateCrossScope(*addMethodCallExpr, CrossRegisterType::MEMBER_FUNC_REGISTER,
            registerIdentify->args.at(0)->ToString());
        return;
    }
    Ptr<const FuncDecl> decl;
    if (registerType != CrossRegisterType::CLASS_REGISTER && !scopes.empty()) {
        const auto scope = scopes.back();
        decl = DynamicCast<const FuncDecl *>(scope.first);
    }
    if (registerType != CrossRegisterType::CLASS_REGISTER && !decl) {
        return;
    }
    UpdateCrossScope(*addMethodCallExpr, CrossRegisterType::MEMBER_FUNC_REGISTER,
        registerIdentify->args.at(0)->ToString());
}

void SymbolCollector::DealCrossFunctionSymbol(const NameReferenceExpr &functionRef, const SrcIdentifier &identifier)
{
    CrossRegisterType registerOuterType = CrossRegisterType::GLOBAL_FUNC_REGISTER;
    CrossRegisterType registerFuncType = CrossRegisterType::GLOBAL_FUNC_REGISTER;
    size_t penultimateIndex = 2;
    if (crossRegisterScopes.size() > 1) {
        const auto scope = crossRegisterScopes.at(crossRegisterScopes.size() - penultimateIndex);
        registerOuterType = crossRegisterScopes.at(crossRegisterScopes.size() - penultimateIndex).second.first;
        registerFuncType = crossRegisterScopes.back().second.first;
    }

    // function in registerClass, and in registerClass lambda
    if (registerOuterType == CrossRegisterType::CLASS_REGISTER
        && registerFuncType == CrossRegisterType::MEMBER_FUNC_REGISTER) {
        // in class
        DealFunctionSymbolInRegisterClass(functionRef, identifier);
        return;
    }

    // function in registerModule, and in registerModule lambda
    if (registerOuterType == CrossRegisterType::MODULE_REGISTER &&
        registerFuncType == CrossRegisterType::MODULE_REGISTER) {
        DealFunctionSymbolInRegisterModule(functionRef, identifier);
        return;
    }

    // register function in func, this func used to ref_expr
    DealFunctionSymbolInFunc(functionRef, identifier);
}

void SymbolCollector::DealFunctionSymbolInRegisterClass(const NameReferenceExpr &functionRef,
    const SrcIdentifier &identifier)
{
    size_t penultimateIndex = 2;
    const auto clazScop = crossRegisterScopes.at(crossRegisterScopes.size() - penultimateIndex);
        const auto registerClazName = clazScop.second.second;
        const auto registerFuncName = crossRegisterScopes.back().second.second;
        const auto funcCallExpr = DynamicCast<const CallExpr *>(functionRef.callOrPattern);
        if (!funcCallExpr || funcCallExpr->args.empty() || !funcCallExpr->args.at(0)->expr) {
            return;
        }
        const auto &registerTarget = funcCallExpr->args.at(0)->expr.get();
        if (!registerTarget) {
            return;
        }
        if (registerTarget->astKind == ASTKind::REF_EXPR) {
            const auto argRef = DynamicCast<const RefExpr *>(registerTarget.get());
            if (!argRef) {
                return;
            }
            const auto argTarget = argRef->aliasTarget ? argRef->aliasTarget : GetRealDecl(argRef->GetTarget());
            if (!argTarget || !argTarget->curFile) {
                return;
            }
            CrossSymbol crossSym;
            crossSym.id = GetDeclSymbolID(*argTarget);
            crossSym.name = remove_quotes(registerFuncName);
            crossSym.containerName = remove_quotes(registerClazName);
            crossSym.crossType = CrossType::ARK_TS_WITH_REGISTER;
            crossSym.location = {argTarget->GetIdentifierPos(),
                argTarget->GetIdentifierPos() + CountUnicodeCharacters(identifier), argTarget->curFile->filePath};
            crossSym.declaration = {crossRegisterScopes.back().first->begin, crossRegisterScopes.back().first->end,
                argTarget->curFile->filePath};
            (void) crsSymsMap.emplace_back(crossSym);
        }
        if (registerTarget->astKind == ASTKind::LAMBDA_EXPR) {
            auto lambdaExpr = DynamicCast<const LambdaExpr *>(registerTarget.get());
            if (!lambdaExpr) {
                return;
            }
            CrossSymbol crossSym;
            crossSym.id = INVALID_SYMBOL_ID;
            crossSym.name = remove_quotes(registerFuncName);
            crossSym.containerName = remove_quotes(registerClazName);
            crossSym.crossType = CrossType::ARK_TS_WITH_REGISTER;
            crossSym.location = {registerTarget->begin, registerTarget->begin + 1, functionRef.curFile->filePath};
            (void) crsSymsMap.emplace_back(crossSym);
        }
}

void SymbolCollector::DealFunctionSymbolInRegisterModule(const NameReferenceExpr &functionRef,
    const SrcIdentifier &identifier)
{
    const auto registerFuncName = crossRegisterScopes.back().second.second;
    const auto funcCallExpr = DynamicCast<const CallExpr *>(functionRef.callOrPattern);
    if (!funcCallExpr || funcCallExpr->args.empty() || !funcCallExpr->args.at(0)->expr) {
        return;
    }
    const auto &registerTarget = funcCallExpr->args.at(0)->expr.get();
    if (!registerTarget) {
        return;
    }
    if (registerTarget->astKind == ASTKind::REF_EXPR) {
        const auto argRef = DynamicCast<const RefExpr *>(registerTarget.get());
        if (!argRef) {
            return;
        }
        const auto argTarget = argRef->aliasTarget ? argRef->aliasTarget : GetRealDecl(argRef->GetTarget());
        if (!argTarget || !argTarget->curFile) {
            return;
        }
        CrossSymbol crossSym;
        crossSym.id = GetDeclSymbolID(*argTarget);
        crossSym.name = remove_quotes(registerFuncName);
        crossSym.crossType = CrossType::ARK_TS_WITH_REGISTER;
        crossSym.location = {argTarget->GetIdentifierPos(),
            argTarget->GetIdentifierPos() + CountUnicodeCharacters(identifier), argTarget->curFile->filePath};
        crossSym.declaration = {crossRegisterScopes.back().first->begin, crossRegisterScopes.back().first->end,
            argTarget->curFile->filePath};
        (void) crsSymsMap.emplace_back(crossSym);
    }
    if (registerTarget->astKind == ASTKind::LAMBDA_EXPR) {
        auto lambdaExpr = DynamicCast<const LambdaExpr *>(registerTarget.get());
        if (!lambdaExpr) {
            return;
        }
        CrossSymbol crossSym;
        crossSym.id = INVALID_SYMBOL_ID;
        crossSym.name = remove_quotes(registerFuncName);
        crossSym.crossType = CrossType::ARK_TS_WITH_REGISTER;
        crossSym.location = {registerTarget->begin, registerTarget->begin + 1, functionRef.curFile->filePath};
        (void) crsSymsMap.emplace_back(crossSym);
    }
}

void SymbolCollector::DealFunctionSymbolInFunc(const NameReferenceExpr &functionRef,
    const SrcIdentifier &identifier)
{
    Ptr<const FuncDecl> decl;
    if (!scopes.empty()) {
        const auto scope = scopes.back();
        decl = DynamicCast<const FuncDecl *>(scope.first);
    }
    if (!decl || !decl->curFile || crossRegisterScopes.empty()) {
        return;
    }
    // member func register (function in registerClass, and in registerClass ref)
    // global func register (function in registerModule, and in registerModule ref)
    const auto registerFuncName = crossRegisterScopes.back().second.second;
    const auto funcCallExpr = DynamicCast<const CallExpr *>(functionRef.callOrPattern);
    if (!funcCallExpr || funcCallExpr->args.empty() || !funcCallExpr->args.at(0)->expr) {
        return;
    }
    const auto &registerTarget = funcCallExpr->args.at(0)->expr.get();
    if (!registerTarget) {
        return;
    }
    SymbolID declId = GetDeclSymbolID(*decl);
    if (registerTarget->astKind == ASTKind::REF_EXPR) {
        const auto argRef = DynamicCast<const RefExpr *>(registerTarget.get());
        if (!argRef) {
            return;
        }
        const auto argTarget = argRef->aliasTarget ? argRef->aliasTarget : GetRealDecl(argRef->GetTarget());
        if (!argTarget || !argTarget->curFile) {
            return;
        }
        SymbolLocation location = {argTarget->GetIdentifierPos(),
            argTarget->GetIdentifierPos() + CountUnicodeCharacters(identifier), argTarget->curFile->filePath};
        SymbolID registerFuncDeclId = GetDeclSymbolID(*argTarget);
        SymbolLocation declaration = {crossRegisterScopes.back().first->begin, crossRegisterScopes.back().first->end,
            argTarget->curFile->filePath};
        if (crossRegisterDecls.find(declId) != crossRegisterDecls.end()) {
            crossRegisterDecls[declId].emplace_back(
                std::make_tuple(remove_quotes(registerFuncName), location, registerFuncDeclId, declaration));
        } else {
            crossRegisterDecls[declId] = {
                std::make_tuple(remove_quotes(registerFuncName), location, registerFuncDeclId, declaration)};
        }
    }
    if (registerTarget->astKind == ASTKind::LAMBDA_EXPR) {
        auto lambdaExpr = DynamicCast<const LambdaExpr *>(registerTarget.get());
        if (!lambdaExpr) {
            return;
        }
        SymbolLocation location = {registerTarget->begin, registerTarget->begin + 1, functionRef.curFile->filePath};
        SymbolLocation declaration = {crossRegisterScopes.back().first->begin, crossRegisterScopes.back().first->end,
            functionRef.curFile->filePath};
        if (crossRegisterDecls.find(declId) != crossRegisterDecls.end()) {
            crossRegisterDecls[declId].emplace_back(std::make_tuple(
                remove_quotes(registerFuncName), location, 0, declaration));
        } else {
            crossRegisterDecls[declId] = {std::make_tuple(
                remove_quotes(registerFuncName), location, 0, declaration)};
        }
    }
}

void SymbolCollector::DealCrossClassSymbol(const NameReferenceExpr &clazzRef, const SrcIdentifier &identifier)
{
    std::string name;
    size_t penultimateIndex = 2;
    bool isInRegisterModule =
        !crossRegisterScopes.empty() && crossRegisterScopes.back().second.first == CrossRegisterType::MODULE_REGISTER;
    // clazz in registerModule, export["xxx"] = context.clazz(xxx).toJSValue()
    if (isInRegisterModule) {
        name = crossRegisterScopes.back().second.second;
        if (crossRegisterScopes.size() > 1 &&
            crossRegisterScopes.at(
                crossRegisterScopes.size() - penultimateIndex).second.first == CrossRegisterType::MODULE_REGISTER) {
            DealClassSymbolInRegisterModule(clazzRef, identifier);
            return;
        }
    }

    // registerClass clazz in func, this func used to ref_expr
    Ptr<const Decl> decl;
    if (!scopes.empty()) {
        for (size_t i = scopes.size() - 1; i >= 0; i--) {
            decl = DynamicCast<const Decl*>(scopes[i].first);
            if (decl && decl->astKind == ASTKind::VAR_DECL) {
                continue;
            }
            decl = DynamicCast<const FuncDecl *>(scopes[i].first);
            break;
        }
    }
    if (!decl || !decl->curFile || !decl->ty) {
        return;
    }
    DealClassSymbolInFunc(*decl, clazzRef, identifier);
}

void SymbolCollector::DealClassSymbolInRegisterModule(const NameReferenceExpr &clazzRef,
    const SrcIdentifier &identifier)
{
    const std::string name = crossRegisterScopes.back().second.second;
    // in registerModule lambda
    const auto funcCallExpr = DynamicCast<const CallExpr *>(clazzRef.callOrPattern);
    if (!funcCallExpr || funcCallExpr->args.empty() || !funcCallExpr->args.at(0)->expr) {
        return;
    }
    const auto &registerTarget = funcCallExpr->args.at(0)->expr.get();
    if (registerTarget->astKind == ASTKind::REF_EXPR) {
        const auto argRef = DynamicCast<const RefExpr *>(registerTarget.get());
        if (!argRef) {
            return;
        }
        const auto argTarget = argRef->aliasTarget ? argRef->aliasTarget : GetRealDecl(argRef->GetTarget());
        if (!argTarget || !argTarget->curFile) {
            return;
        }
        CrossSymbol crossSym;
        crossSym.id = GetDeclSymbolID(*argTarget);
        crossSym.name = remove_quotes(name);
        crossSym.crossType = CrossType::ARK_TS_WITH_REGISTER;
        crossSym.location = {argTarget->GetIdentifierPos(),
            argTarget->GetIdentifierPos() + CountUnicodeCharacters(identifier), argTarget->curFile->filePath};
        (void) crsSymsMap.emplace_back(crossSym);
    }
    if (registerTarget->astKind == ASTKind::LAMBDA_EXPR) {
        auto lambdaExpr = DynamicCast<const LambdaExpr *>(registerTarget.get());
        if (!lambdaExpr) {
            return;
        }
        CrossSymbol crossSym;
        crossSym.id = INVALID_SYMBOL_ID;
        crossSym.name = remove_quotes(name);
        crossSym.crossType = CrossType::ARK_TS_WITH_REGISTER;
        crossSym.location = {registerTarget->begin, registerTarget->begin + 1, clazzRef.curFile->filePath};
        (void) crsSymsMap.emplace_back(crossSym);
    }
}

void SymbolCollector::DealClassSymbolInFunc(const Decl &decl, const NameReferenceExpr &clazzRef,
    const SrcIdentifier &identifier)
{
    std::string name = "";
    const SymbolID containerId = GetContextID();
    const auto identifierPos = decl.GetIdentifierPos();
    const auto uri = decl.curFile->filePath;
    SymbolLocation location = {identifierPos, identifierPos + CountUnicodeCharacters(identifier), uri};
    if (!crossRegisterScopes.empty() && crossRegisterScopes.back().second.first == CrossRegisterType::MODULE_REGISTER) {
        // clazz register with, export["xxx"] = context.clazz(xxx).toJSValue()
        name = crossRegisterScopes.back().second.second;
        const auto funcCallExpr = DynamicCast<const CallExpr *>(clazzRef.callOrPattern);
        if (!funcCallExpr || funcCallExpr->args.empty() || !funcCallExpr->args.at(0)->expr) {
            return;
        }
        const auto &registerTarget = funcCallExpr->args.at(0)->expr.get();
        if (registerTarget->astKind == ASTKind::REF_EXPR) {
            const auto argRef = DynamicCast<const RefExpr *>(registerTarget.get());
            if (!argRef) {
                return;
            }
            const auto argTarget = argRef->aliasTarget ? argRef->aliasTarget : GetRealDecl(argRef->GetTarget());
            if (!argTarget || !argTarget->curFile) {
                return;
            }
            location = {argTarget->GetIdentifierPos(),
                argTarget->GetIdentifierPos() + CountUnicodeCharacters(identifier), argTarget->curFile->filePath};
        }
        if (registerTarget->astKind == ASTKind::LAMBDA_EXPR) {
            location = {registerTarget->begin, registerTarget->begin + 1, clazzRef.curFile->filePath};
        }
    }
    if (crossRegisterDecls.find(containerId) != crossRegisterDecls.end()) {
        crossRegisterDecls[containerId].emplace_back(std::make_tuple(name, location, 0, location));
    } else {
        crossRegisterDecls[containerId] = {std::make_tuple(name, location, 0, location)};
    }
}

void SymbolCollector::DealExportsRegisterSymbol(const NameReferenceExpr &ref)
{
    if (crossRegisterScopes.empty() || crossRegisterScopes.back().second.first != CrossRegisterType::MODULE_REGISTER) {
        return;
    }
    if (ref.astKind != ASTKind::MEMBER_ACCESS) {
        return;
    }

    auto callExpr = DynamicCast<const CallExpr *>(ref.callOrPattern);
    if (!callExpr || callExpr->args.empty()) {
        return;
    }
    const auto &registerIdentify = callExpr->args.at(0);
    if (!registerIdentify) {
        return;
    }
    auto crossScopeNode = crossRegisterScopes.back().first;
    crossRegisterScopes.pop_back();
    UpdateCrossScope(*crossScopeNode, CrossRegisterType::MODULE_REGISTER, registerIdentify->ToString());
}

struct ExtendInfo {
    SymbolID id;
    std::string name;
    ark::lsp::Modifier modifier;
    std::string interfaceName;
};

void SymbolCollector::CreateExtend(const Decl &decl, const std::string &filePath)
{
    auto extendDecl = DynamicCast<ExtendDecl *>(&decl);
    bool isInvalidExtend = !extendDecl || !IsExportedExtendDecl(extendDecl) || !extendDecl->extendedType ||
        extendDecl->inheritedTypes.empty();
    if (isInvalidExtend) {
        return;
    }
    auto target = extendDecl->extendedType->GetTarget();
    if (!target || (target->ty && target->ty->HasGeneric())) {
        return;
    }
    std::string targetName = ItemResolverUtil::ResolveSignatureByNode(*target);
    SymbolID symbolID = GetDeclSymbolID(*target);
    auto fullPackageName = extendDecl->fullPackageName;
    std::vector<ExtendInfo> extendVec;
    std::map<std::string, ExtendInfo> extendInfoMap;
    for (auto &member : extendDecl->members) {
        if (IsHiddenDecl(member) || !member->IsExportedDecl()) {
            continue;
        }
        std::string signature = ItemResolverUtil::ResolveSignatureByNode(*member);
        auto modifier = GetDeclModifier(*member);
        auto extendSymbolID = GetDeclSymbolID(*member);
        ExtendItem extendItem = {.id = extendSymbolID};
        extendVec.push_back({.id=extendSymbolID, .name=signature});
        extendInfoMap.insert_or_assign(signature, extendVec.back());
    }
    std::function<void(const InheritableDecl&, std::vector<Ptr<Decl>>&)> collectInheritMember =
        [&collectInheritMember, &extendInfoMap](const InheritableDecl& id,
                                                std::vector<Ptr<Decl>>& members) {
        for (auto& super : id.inheritedTypes) {
            auto targetDecl = DynamicCast<InheritableDecl>(super->GetTarget());
            if (targetDecl == nullptr) {
                continue;
            }
            collectInheritMember(*targetDecl, members);
        }
        if (id.astKind != ASTKind::INTERFACE_DECL) {
            return;
        }
        std::string interfaceName = ItemResolverUtil::ResolveSignatureByNode(id);
        auto modifier = GetDeclModifier(id);
        auto& inherMembers = id.GetMemberDecls();
        std::for_each(inherMembers.begin(), inherMembers.end(),
            [&members, &interfaceName, &modifier, &extendInfoMap](auto& m) {
            std::string name = ItemResolverUtil::ResolveSignatureByNode(*m);
            if (extendInfoMap.find(name) == extendInfoMap.end() && !m->TestAttr(Attribute::ABSTRACT)) {
                members.emplace_back(m);
            } else {
                extendInfoMap[name].interfaceName = interfaceName;
                extendInfoMap[name].modifier = modifier;
            }
        });
    };
    // get method with interface modifier
    for (const auto& types : extendDecl->inheritedTypes) {
        auto targetDecl = DynamicCast<InheritableDecl>(types->GetTarget());
        if (targetDecl == nullptr) {
            continue;
        }
        if (targetDecl->astKind != ASTKind::INTERFACE_DECL) {
            return;
        }
        auto interfaceModifier = GetDeclModifier(*targetDecl);
        auto extendSymbolID = GetDeclSymbolID(*targetDecl);
        std::vector<Ptr<Decl>> members;
        collectInheritMember(*targetDecl, members);
        std::string interfaceName = ItemResolverUtil::ResolveSignatureByNode(*targetDecl);
        for (const auto& member : members) {
            std::string signature = ItemResolverUtil::ResolveSignatureByNode(*member);
            auto extendSymbolID = GetDeclSymbolID(*member);
            ExtendInfo info = {.id = extendSymbolID, .name = signature,
                               .modifier = interfaceModifier, .interfaceName = interfaceName};
            extendInfoMap.insert_or_assign(signature, info);
        }
    }
    for (const auto& info : extendInfoMap) {
        ExtendItem extendItem = {.id = info.second.id,
                                 .modifier = info.second.modifier,
                                 .interfaceName = info.second.interfaceName};
        (void)symbolExtendMap[symbolID].emplace_back(extendItem);
    }
}

void SymbolCollector::CreateRef(const NameReferenceExpr &ref, const std::string &filePath)
{
    if (ref.TestAttr(Attribute::IMPLICIT_ADD)) {
        return;
    }
    auto refExpr = DynamicCast<const RefExpr *>(&ref);
    if (refExpr) {
        if (refExpr->isThis) {
            return;
        }
    }
    auto target = ref.aliasTarget ? ref.aliasTarget : GetRealDecl(ref.GetTarget());
    if (target == nullptr) {
        return;
    }
    if (!ref.curFile) {
        return;
    }
    auto ma = DynamicCast<const MemberAccess *>(&ref);
    if (ma && ma->field.Begin().IsZero()) {
        return;
    }
    auto identifier = target->TestAttr(Attribute::CONSTRUCTOR) ? target->outerDecl->identifier
                                                               : target->identifier;
    if ((!IsGlobalOrMemberOrItsParam(*target) && !IsLocalFuncOrLambda(*target))) {
        return;
    }
    if (aliasMap.find(identifier) != aliasMap.end()) {
        identifier = aliasMap[identifier];
    }
    auto offset = (identifier == "[]" || identifier == "()") ? 1 : CountUnicodeCharacters(identifier);
    auto begin = ma ? ma->GetFieldPos() : (refExpr ? refExpr->GetIdentifierPos() : ref.GetBegin());
    if (begin == INVALID_POSITION) {
        return;
    }
    auto end = begin + offset;
    auto uri = ref.curFile->filePath;
    if (begin.fileID != ref.begin.fileID) {
        uri = ref.curFile->macroCallFilePath;
    }
    if (uri.empty()) {
        return;
    }
    SymbolLocation loc{begin, end, uri};
    Ref refInfo{.location = loc,
        .kind = RefKind::REFERENCE,
        .container = GetContextID()};
    refInfo.isSuper = (refExpr && refExpr->isSuper);
    UpdatePos(refInfo.location, ref, filePath);
    (void)symbolRefMap[GetDeclSymbolID(*target)].emplace_back(refInfo);
    // collect cross symbol
    bool isInvalidSymbol = target->fullPackageName != CROSS_ARK_TS_WITH_REGISTER_PACKAGE|| ref.IsMacroCallNode()
    || EndsWith(uri, ".macrocall");
    if (isInvalidSymbol) {
        return;
    }
    DealCrossSymbol(ref, *target, identifier);
}

void SymbolCollector::CreateTypeRef(const Type &type, const std::string &filePath)
{
    auto target = type.GetTarget();
    if (target == nullptr || !target->TestAttr(Attribute::GLOBAL)) {
        return;
    }
    if (!type.curFile) {
        return;
    }
    auto qt = DynamicCast<const QualifiedType *>(&type);
    if (qt && qt->field.Begin().IsZero()) {
        return;
    }
    auto begin = qt ? qt->GetFieldPos() : type.GetBegin();
    if (!type.typePos.IsZero()) {
        begin = type.typePos;
    }
    auto end = qt ? begin + CountUnicodeCharacters(qt->field) : begin + CountUnicodeCharacters(target->identifier);
    SymbolLocation loc{begin, end, type.curFile->filePath};
    Ref refInfo{.location = loc, .kind = RefKind::REFERENCE, .container = GetContextID()};
    if (type.TestAttr(Attribute::COMPILER_ADD)) {
        return;
    }
    UpdatePos(refInfo.location, type, filePath);
    (void)symbolRefMap[GetDeclSymbolID(*target)].emplace_back(refInfo);
}

void SymbolCollector::CreateImportRef(const File &fileNode)
{
    if (!fileNode.curFile || !fileNode.curPackage) {
        return;
    }
    auto filePath = fileNode.curFile->filePath;
    auto GetPackagePrefixWithPaths = [](const std::vector<std::string> &prefixPaths)-> std::string {
        std::stringstream ss;
        for (const auto &prefix : prefixPaths) {
            ss << prefix << ".";
        }
        return ss.str().substr(0, ss.str().size() - 1);
    };
    auto ProcImport = [&fileNode, &GetPackagePrefixWithPaths, &filePath, this](
                            ImportSpec &importSpec, ImportContent &importContent, bool isAllImport) {
        const auto srcPkgName = fileNode.curPackage->fullPackageName;
        std::string packagePrefix = GetPackagePrefixWithPaths(importContent.prefixPaths);
        const auto targetPkg = this->importMgr.GetPackageDecl(packagePrefix);
        if (!targetPkg) {
            return;
        }
        const auto members = this->importMgr.GetPackageMembers(srcPkgName, targetPkg->fullPackageName);
        for (const auto &memberDecl : members) {
            if (const auto declPtr = dynamic_cast<const Decl *>(memberDecl.get());
                declPtr == nullptr || (!isAllImport && declPtr->identifier != importContent.identifier)) {
                    continue;
                }
            SymbolLocation loc{importSpec.begin, importSpec.end, filePath};
            Ref refInfo{.location = loc, .kind = RefKind::IMPORT};
            UpdatePos(refInfo.location, importSpec, filePath);
            (void)symbolRefMap[GetDeclSymbolID(*memberDecl)].emplace_back(refInfo);
        }
    };

    for (const auto &importSpec : fileNode.imports) {
        if (!importSpec || importSpec->IsImportMulti() || importSpec->end.IsZero()) {
            continue;
        }
        auto importContent = importSpec.get()->content;
        if (importContent.end.IsZero()) {
            continue;
        }
        ProcImport(*importSpec, importContent, importSpec->IsImportAll());
    }
}

void SymbolCollector::CreateMacroRef(const Node &node, const MacroInvocation &invocation)
{
    if (!invocation.target) {
        return;
    }
    if (invocation.decl && invocation.decl->astKind == ASTKind::MACRO_EXPAND_DECL &&
        invocation.decl->GetConstInvocation()) {
        CreateMacroRef(*invocation.decl, *invocation.decl->GetConstInvocation());
    }
    if (!node.curFile) {
        return;
    }
    auto begin = invocation.identifierPos;
    if (!invocation.fullNameDotPos.empty()) {
        begin = invocation.fullNameDotPos.back() + 1;
    }
    auto end = begin + CountUnicodeCharacters(invocation.identifier);
    SymbolLocation loc{begin, end, node.curFile->filePath};
    Ref refInfo{.location = loc, .kind = RefKind::REFERENCE, .container = GetContextID()};
    (void)symbolRefMap[GetDeclSymbolID(*invocation.target)].emplace_back(refInfo);

    // for cross language symbol
    if (node.symbol && node.symbol->name != CROSS_ARK_TS_WITH_INTEROP_NAME) {
        return;
    }
    for (const auto& attr : invocation.attrs) {
        if (attr.Value() == CROSS_ARK_TS_WITH_INTEROP_INVISIBLE_NAME) {
            return;
        }
    }
    if (invocation.decl) {
        CreateCrossSymbolByInterop(*invocation.decl);
    }
}

void SymbolCollector::CreateNamedArgRef(const CallExpr &ce)
{
    auto fd = ce.resolvedFunction;
    if (!fd || !fd->funcBody || fd->funcBody->paramLists.empty()) {
        return;
    }
    if (!ce.curFile) {
        return;
    }
    auto filePath = ce.curFile->filePath;
    std::unordered_map<std::string, SymbolID> namedParamSymbols;
    CODEC_NULLPTR_CHECK(fd->funcBody->paramLists[0]);
    for (auto &param : fd->funcBody->paramLists[0]->params) {
        CODEC_NULLPTR_CHECK(param);
        if (param->isNamedParam) {
            (void)namedParamSymbols.emplace(param->identifier, GetDeclSymbolID(*param));
        }
    }
    for (auto &arg : ce.args) {
        if (arg->name.Empty()) {
            continue;
        }
        auto found = namedParamSymbols.find(arg->name);
        if (found == namedParamSymbols.end()) {
            continue;
        }
        SymbolLocation loc{arg->name.Begin(), arg->name.Begin() + CountUnicodeCharacters(arg->name), filePath};
        Ref refInfo{.location = loc, .kind = RefKind::REFERENCE, .container = GetContextID()};
        (void)symbolRefMap[found->second].emplace_back(refInfo);
    }
}

void SymbolCollector::CollectRelations(
    const std::unordered_set<Ptr<InheritableDecl>> &inheritableDecls)
{
    for (auto &id : inheritableDecls) {
        for (auto &type : id->inheritedTypes) {
            auto decl = Ty::GetDeclPtrOfTy(type->ty);
            if (decl == nullptr || type->ty->IsObject()) { // Ignore core object.
                continue;
            }
            auto subject = GetDeclSymbolID(*decl);
            auto predicate = RelationKind::BASE_OF;
            auto object = GetDeclSymbolID(*id);
            if (auto ed = DynamicCast<const ExtendDecl *>(id.get())) {
                predicate = RelationKind::EXTEND;
                auto beingExtendDecl = Ty::GetDeclPtrOfTy(ed->extendedType->ty);
                if (beingExtendDecl == nullptr || ed->extendedType->ty->IsObject()) { // Ignore core object.
                    continue;
                }
                object = GetDeclSymbolID(*beingExtendDecl);
            }
            Relation relation{.subject = subject, .predicate = predicate, .object = object};
            (void)relations.emplace_back(relation);
        }

        for (auto &member : id->GetMemberDecls()) {
            if (!Ty::IsTyCorrect(member->ty)) {
                continue;
            }
            (void)relations.emplace_back(Relation{.subject = GetDeclSymbolID(*member),
                                                  .predicate = RelationKind::CONTAINED_BY,
                                                  .object = GetDeclSymbolID(*id)});
            bool ignore =
                member->astKind == ASTKind::VAR_DECL || member->TestAttr(Attribute::CONSTRUCTOR);
            if (ignore) {
                continue;
            }
            bool definedOverride = member->TestAttr(Attribute::REDEF, Attribute::STATIC) ||
                                   (member->TestAnyAttr(Attribute::PUBLIC, Attribute::PROTECTED) &&
                                    !member->TestAttr(Attribute::STATIC));
            if (definedOverride && id->astKind == ASTKind::CLASS_DECL) {
                if (auto parent =
                        FindOverriddenMemberFromSuperClass(*member, *StaticCast<ClassDecl *>(id))) {
                    Relation relation{.subject = GetDeclSymbolID(*parent),
                                      .predicate = RelationKind::RIDDEND_BY,
                                      .object = GetDeclSymbolID(*member)};
                    (void)relations.emplace_back(relation);
                    CollectNamedParam(parent, member.get());
                }
            }
            if (!member->TestAttr(Attribute::PUBLIC)) {
                continue;
            }
            auto parents = FindImplMemberFromInterface(*member, *id);
            for (auto parent : parents) {
                Relation relation{.subject = GetDeclSymbolID(*parent),
                                  .predicate = RelationKind::RIDDEND_BY,
                                  .object = GetDeclSymbolID(*member)};
                (void)relations.emplace_back(relation);
                CollectNamedParam(parent, member.get());
            }
        }
    }
}

Ptr<Decl> SymbolCollector::FindOverriddenMember(const Decl &member, const InheritableDecl &id,
                                               const TypeSubst &typeMapping,
                                               const std::function<bool(const Decl &)> &satisfy)
{
    Ptr<Decl> found = nullptr;
    for (auto &it : id.GetMemberDeclPtrs()) {
        CODEC_NULLPTR_CHECK(it);
        if (!Ty::IsTyCorrect(it->ty) || !satisfy(*it)) {
            continue;
        }
        auto memberTy = tyMgr.GetInstantiatedTy(it->ty, typeMapping);
        if (member.astKind == ASTKind::PROP_DECL) {
            if (memberTy == member.ty) {
                found = it;
            }
            break;
        } else if (member.astKind != ASTKind::FUNC_DECL) {
            continue;
        }
        if (member.TestAttr(Attribute::GENERIC)) {
            TypeSubst mapping = tyMgr.GenerateGenericMappingFromGeneric(
                *static_cast<FuncDecl *>(it.get()), *static_cast<const FuncDecl *>(&member));
            memberTy = tyMgr.GetInstantiatedTy(memberTy, mapping);
        }
        if (tyMgr.IsFuncParameterTypesIdentical(*StaticCast<FuncTy *>(memberTy),
                                                *StaticCast<FuncTy *>(member.ty))) {
            found = it;
            break;
        }
    }
    return found;
}

Ptr<Decl> SymbolCollector::FindOverriddenMemberFromSuperClass(const Decl &member,
                                                             const ClassDecl &cd)
{
    auto condition = [&member](const Decl &parent) {
        return IsFitBaseMemberOverriddenCondition(parent, member) &&
               ((parent.TestAnyAttr(Attribute::OPEN, Attribute::ABSTRACT) &&
                 parent.TestAnyAttr(Attribute::PUBLIC, Attribute::PROTECTED)) ||
                (parent.TestAttr(Attribute::STATIC) && !parent.TestAttr(Attribute::PRIVATE)));
    };
    TypeSubst typeMapping;
    Ptr<const ClassDecl> current = &cd;
    while (current != nullptr) {
        Ptr<ClassDecl> sd = nullptr;
        for (auto &it : current->inheritedTypes) {
            if (!Ty::IsTyCorrect(it->ty) || !it->ty->IsClass()) {
                continue;
            }
            sd = StaticCast<ClassDecl *>(Ty::GetDeclPtrOfTy(it->ty));
            CODEC_NULLPTR_CHECK(sd); // When ty is class and correct, sd must be non-null.
            typeMapping.merge(GenerateTypeMapping(sd->GetGeneric(), it->ty->typeArgs));
            if (auto parent = FindOverriddenMember(member, *sd, typeMapping, condition)) {
                return parent;
            }
            break;
        }
        current = sd;
    }
    return nullptr;
}

std::vector<Ptr<Decl>> SymbolCollector::FindImplMemberFromInterface(const Decl &member, const InheritableDecl &id)
{
    auto condition = [&member](const Decl &parent) {
        return IsFitBaseMemberOverriddenCondition(parent, member);
    };
    std::vector<Ptr<Decl>> ret;
    std::unordered_set<Ptr<Decl>> searched;
    std::queue<std::pair<Ptr<const InheritableDecl>, TypeSubst>> workList;
    workList.push(std::make_pair(&id, TypeSubst{}));
    while (!workList.empty()) {
        auto [curDecl, mapping] = workList.front();
        workList.pop();
        for (auto &it : curDecl->inheritedTypes) {
            if (!Ty::IsTyCorrect(it->ty)) {
                continue;
            }
            auto interfaceDecl = DynamicCast<InterfaceDecl *>(Ty::GetDeclPtrOfTy(it->ty));
            if (interfaceDecl == nullptr) {
                continue;
            }
            if (auto [_, success] = searched.emplace(interfaceDecl); !success) {
                continue;
            }
            auto currentMapping =
                GenerateTypeMapping(interfaceDecl->GetGeneric(), it->ty->typeArgs);
            currentMapping.insert(mapping.begin(), mapping.end());
            if (auto found =
                    FindOverriddenMember(member, *interfaceDecl, currentMapping, condition)) {
                (void)ret.emplace_back(found);
            } else {
                // If not found in current super interface, found from parent if exists.
                workList.push(std::make_pair(interfaceDecl, currentMapping));
            }
        }
    }
    return ret;
}

void SymbolCollector::CollectNamedParam(Ptr<const Decl> parent, Ptr<const Decl> member)
{
    std::unordered_map<std::string, std::pair<SymbolID, SymbolID>> record;
    auto pfd = DynamicCast<FuncDecl *>(parent);
    if (!pfd) {
        return;
    }
    for (const auto &pl : pfd->funcBody->paramLists) {
        for (const auto &fp : pl->params) {
            if (!fp->isNamedParam) {
                continue;
            }
            auto parentId = GetDeclSymbolID(*fp);
            (void)record.emplace(fp->identifier, std::make_pair(parentId, INVALID_SYMBOL_ID));
        }
    }

    auto mfd = DynamicCast<FuncDecl *>(member);
    if (!mfd) {
        return;
    }
    for (const auto &pl : mfd->funcBody->paramLists) {
        for (const auto &fp : pl->params) {
            if (!fp->isNamedParam) {
                continue;
            }
            auto memberId = GetDeclSymbolID(*fp);
            if (record.find(fp->identifier) != record.end()) {
                SymbolID tempId = record[fp->identifier].first;
                record[fp->identifier] = std::make_pair(tempId, memberId);
            }
        }
    }

    for (const auto &iter : record) {
        if (iter.second.first != INVALID_SYMBOL_ID && iter.second.second != INVALID_SYMBOL_ID) {
            Relation re{.subject = iter.second.first, .predicate = RelationKind::RIDDEND_BY,
                        .object = iter.second.second};
            (void)relations.emplace_back(re);
        }
    }
}

void SymbolCollector::UpdatePos(SymbolLocation &location, const Node &node,
                                const std::string &filePath)
{
    Range range = {location.begin, location.end};
    ark::ArkAST *arkAst = this->astMap[filePath].get();
    if (!arkAst) {
        return;
    }
    ark::UpdateRange(arkAst->tokens, range, node);
    location.begin = range.start;
    location.end = range.end;
}

void SymbolCollector::CollectCompletionItem(const Decl &decl, Symbol &declSym)
{
    std::string insertText = ItemResolverUtil::ResolveInsertByNode(decl);
    auto funcDecl = dynamic_cast<const FuncDecl*>(&decl);
    if (decl.TestAttr(Codira::AST::Attribute::MACRO_FUNC) && funcDecl && funcDecl->funcBody) {
        declSym.signature = decl.identifier;
        ItemResolverUtil::ResolveMacroParams(declSym.signature, funcDecl->funcBody->paramLists);
    }
    declSym.completionItems.push_back({declSym.signature, insertText});
    // add follow lambda completion
    std::string flSignature = ItemResolverUtil::ResolveFollowLambdaSignature(decl);
    if (!flSignature.empty()) {
        std::string flInsert = ItemResolverUtil::ResolveFollowLambdaInsert(decl);
        if (!flInsert.empty()) {
            declSym.completionItems.push_back({flSignature, flInsert});
        }
    }
    // add paramList completion for func_type's var_decl
    std::string varSignature;
    std::string varInsert;
    ItemResolverUtil::ResolveParamListFuncTypeVarDecl(decl, varSignature, varInsert);
    if (!varSignature.empty() && !varInsert.empty()) {
        declSym.completionItems.push_back({varSignature, varInsert});
    }
}

void SymbolCollector::CollectNode(Ptr<Node> node, const std::string& filePath, AccessLevel pkgAccess)
{
    if (auto decl = DynamicCast<Decl *>(node)) {
        CreateBaseOrExtendSymbol(*decl, filePath, pkgAccess);
        UpdateScope(*decl);
    } else if (auto ref = DynamicCast<NameReferenceExpr *>(node)) {
        CreateRef(*ref, filePath);
    } else if (auto ce = DynamicCast<CallExpr *>(node); ce && !ce->desugarExpr) {
        CreateNamedArgRef(*ce);
    } else if (auto type = DynamicCast<Type *>(node)) {
        CreateTypeRef(*type, filePath);
    }
    CollectCrossScopes(node);
}
} // namespace ark::lsp
