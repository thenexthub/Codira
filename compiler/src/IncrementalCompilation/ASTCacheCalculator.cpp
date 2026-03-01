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

#include "Codira/IncrementalCompilation/ASTCacheCalculator.h"

#include "Codira/AST/Utils.h"
#include "Codira/IncrementalCompilation/IncrementalScopeAnalysis.h"
#include "Codira/IncrementalCompilation/Utils.h"
#include "Codira/Mangle/ASTMangler.h"
#include "Codira/Utils/SafePointer.h"

using namespace Codira::AST;

namespace Codira::IncrementalCompilation {
class ASTCacheCalculatorImpl {
public:
    explicit ASTCacheCalculatorImpl(
        ASTCacheCalculator& calculator, const AST::Package& p1, std::pair<bool, bool> srcInfo1)
        : p{p1},
          mangler{p1.fullPackageName},
          srcInfo{srcInfo1},
          calc{calculator},
          mangled2Decl{calc.mangled2Decl},
          ret{calc.ret},
          duplicatedMangleNames{calc.duplicatedMangleNames},
          directExtends{calc.directExtends},
          order{calc.order},
          fileMap{calc.fileMap}
    {
    }

    void Walk()
    {
        for (auto& file : p.files) {
            gid = 0;
            invp = false;
            curFile = &fileMap[GetTrimmedPath(file.get())];
            for (size_t i{0}; i < file->decls.size(); ++i) {
                auto& decl = *file->decls[i];
                if (i == 0 && DynamicCast<VarDeclAbstract>(&decl)) {
                    --gid; // the first decl of a file always has gid 0
                }
                WalkDecl(decl);
            }
        }

        for (auto& [mangle, extends] : directExtends) {
            mangled2Decl[mangle] = extends.cbegin()->first;
            ret.emplace(mangle, ComputeDirectExtend(std::move(extends)));
        }
    }

private:
    const AST::Package& p;
    ASTMangler mangler;
    // Note: polish the implementation here
    // We have two cases where we need to care about the code position info
    // 1) debug mode, we need all code position info
    // 2) stacktrace, we need code position info of expressions and functions
    std::pair<bool, bool> srcInfo;
    int gid{0}; // current gvid to increment when visiting a global or static variable or to use otherwise
    bool invp{false}; // in the visit of varwithpattern
    ASTCacheCalculator& calc;

    // these fields are all output of cache computation
    RawMangled2DeclMap& mangled2Decl; // RawMangledName -> Ptr<AST::Decl> map
    ASTCache& ret;
    std::unordered_set<Ptr<const AST::Decl>>& duplicatedMangleNames; // decls with duplicate RawMangledName
    std::unordered_map<RawMangledName, std::list<std::pair<Ptr<AST::ExtendDecl>, int>>>& directExtends;
    std::vector<const Decl*>& order;
    FileMap& fileMap;
    // output fields end

    FileMap::mapped_type* curFile{nullptr};

    TopLevelDeclCache ComputeDirectExtend(std::list<std::pair<Ptr<AST::ExtendDecl>, int>>&& extends)
    {
        TopLevelDeclCache r{};
        std::vector<Ptr<const Decl>> members;
        // collect public funcs & props
        for (auto extend : std::as_const(extends)) {
            for (auto& member : extend.first->members) {
                r.members.push_back(RevisitDirectExtendMember(*member, extend.second));
                if (member->TestAttr(Attribute::PUBLIC)) {
                    members.push_back(member.get());
                }
            }
        }
        auto& decl = *extends.begin()->first;
        decl.hash.srcUse = r.srcUse = ASTHasher::SrcUseHash(decl);
        decl.hash.sig = r.sigHash = ASTHasher::SigHash(decl);
        decl.hash.bodyHash = r.bodyHash = ASTHasher::HashMemberAPIs(std::move(members));
        r.gvid = {GetTrimmedPath(decl.curFile.get()), extends.begin()->second};
        decl.hash.gvid = r.gvid.id;
        r.astKind = static_cast<uint8_t>(decl.astKind);
        r.isGV = false;
        return r;
    }

    void WalkDecl(Decl& decl)
    {
        if (auto extend = DynamicCast<ExtendDecl*>(&decl); extend && extend->inheritedTypes.empty()) {
            // collect direct extends but do not compute the cache; delay the computation until all direct extends
            // with the same name are seen
            extend->rawMangleName = mangler.Mangle(*extend);
            auto& it = directExtends[extend->rawMangleName];
            if (it.empty()) {
                order.push_back(&decl); // ensure direct extends are written to the cache only once
            }
            it.push_back(std::make_pair(extend, gid));
            for (auto member : extend->GetMemberDeclPtrs()) {
                PrevisitDirectExtendMember(*member); // visit extend members purely for their side effects
            }
            return;
        }
        // global vars inside varwithpattern are also treated as global var
        order.push_back(&decl);
        auto declCache = VisitTopLevel(decl);
        (void)ret.emplace(decl.rawMangleName, std::move(declCache));
        CollectDecl(decl);
    }

    static void CombineDeclHash(size_t& acc, const Decl& decl)
    {
        acc = ASTHasher::CombineHash(acc, Utils::SipHash::GetHashValue(decl.rawMangleName));
        acc = ASTHasher::CombineHash(acc, ASTHasher::SigHash(decl));
        acc = ASTHasher::CombineHash(acc, ASTHasher::SrcUseHash(decl));
    }

    size_t VisitMemberVariables(const Decl& decl) const
    {
        size_t hashed{0};
        Ptr<const PrimaryCtorDecl> pc{nullptr};
        for (auto member : decl.GetMemberDeclPtrs()) {
            switch (member->astKind) {
                case ASTKind::VAR_DECL:
                    if (IsInstance(*member)) {
                        CombineDeclHash(hashed, *member);
                        // For those member without explicit type annotation,
                        // we need to also hash its body
                        auto memberVar = StaticCast<VarDecl*>(member);
                        if (memberVar->type == nullptr) {
                            // here we hardcode the srcinfo is true
                            hashed = ASTHasher::CombineHash(hashed, ASTHasher::BodyHash(*memberVar, {true, true}));
                        }
                    }
                    break;
                case ASTKind::PRIMARY_CTOR_DECL:
                    pc = StaticCast<PrimaryCtorDecl*>(member);
                    break;
                default:
                    break;
            }
        }
        if (pc && pc->funcBody) {
            for (auto& pl : pc->funcBody->paramLists) {
                for (auto& param : pl->params) {
                    if (param->isMemberParam) {
                        CombineDeclHash(hashed, *param);
                    }
                }
            }
        }
        return hashed;
    }

    size_t VisitEnumConstructors(const EnumDecl& decl) const
    {
        size_t hashed = Utils::SipHash::GetHashValue(decl.hasEllipsis);
        for (auto& cons : decl.constructors) {
            CombineDeclHash(hashed, *cons);
        }
        return hashed;
    }

    size_t VirtualHashOfInterface(const Decl& decl) const
    {
        size_t hashed{0};
        std::vector<Ptr<const Decl>> allMemberFuncs{};
        for (auto member : decl.GetMemberDeclPtrs()) {
            switch (member->astKind) {
                case ASTKind::FUNC_DECL: {
                    allMemberFuncs.emplace_back(member);
                    break;
                }
                case ASTKind::PROP_DECL: {
                    auto propDecl = StaticCast<const PropDecl*>(member);
                    allMemberFuncs.emplace_back(propDecl);
                    for (auto& getter : propDecl->getters) {
                        allMemberFuncs.emplace_back(getter.get());
                    }
                    for (auto& setter : propDecl->setters) {
                        allMemberFuncs.emplace_back(setter.get());
                    }
                    break;
                }
                default:
                    break;
            }
        }
        std::stable_sort(allMemberFuncs.begin(), allMemberFuncs.end(),
            [](auto a, auto b) { return a->rawMangleName < b->rawMangleName; });
        for (auto member : std::as_const(allMemberFuncs)) {
            CombineDeclHash(hashed, *member);
            if (IsUntyped(*member)) {
                hashed = ASTHasher::CombineHash(hashed, ASTHasher::BodyHash(*member, {false, false}));
            }
        }
        return hashed;
    }

    void VisitVarWithPattern(const VarWithPatternDecl& decl)
    {
        CODEC_NULLPTR_CHECK(decl.irrefutablePattern);
        Walker(decl.irrefutablePattern.get(), [this](Ptr<Node> node) {
            if (auto varDecl = DynamicCast<VarDecl*>(node)) {
                WalkDecl(*varDecl);
            }
            return VisitAction::WALK_CHILDREN;
        }).Walk();
    }

    void VisitTopLevelDeclMembers(const Decl& decl, TopLevelDeclCache& result)
    {
        for (auto member : decl.GetMemberDeclPtrs()) {
            // visit static var first so their gvid is smaller than other decls
            if (IsStaticVar(*member)) {
                result.members.push_back(VisitMember(*member));
            }
        }
        for (auto member : decl.GetMemberDeclPtrs()) {
            // then visit other member decls
            if (!IsStaticVar(*member)) {
                result.members.push_back(VisitMember(*member));
                // visit member parameters
                if (auto pd = DynamicCast<PrimaryCtorDecl*>(member);
                    pd && pd->funcBody && !pd->funcBody->paramLists.empty()) {
                    for (auto& param : pd->funcBody->paramLists[0]->params) {
                        if (param->isMemberParam) {
                            result.members.push_back(VisitMember(*param));
                        }
                    }
                }
            }
        }
    }

    static bool IsDirectExtend(const Decl& decl)
    {
        if (decl.astKind != ASTKind::EXTEND_DECL) {
            return false;
        }
        return static_cast<const ExtendDecl&>(decl).inheritedTypes.empty();
    }

    // Visit a direct extend member as all members are visited.
    // This function is only needed for its side effects; the result during this visit is to be discarded, as
    // direct extend members are revisited after other decls.
    void PrevisitDirectExtendMember(Decl& decl)
    {
        for (auto member : GetMembers(decl)) {
            PrevisitDirectExtendMember(*member);
        }
        decl.rawMangleName = mangler.Mangle(decl);
        CollectDecl(decl);
        decl.hash.srcUse = ASTHasher::SrcUseHash(decl);
        decl.hash.bodyHash = ASTHasher::BodyHash(decl, srcInfo);
        decl.hash.sig = ASTHasher::SigHash(decl);
        decl.hash.gvid = gid;
        if (IsOOEAffectedDecl(decl)) {
            CODEC_NULLPTR_CHECK(curFile);
            curFile->push_back(&decl);
        }
    }

    // Implementation of visit top level decl that varies on ast kind of decl. Note that this function does not
    // process for direct extend; they are specially processed later.
    void VisitTopLevelImpl1(const Decl& decl, TopLevelDeclCache& result)
    {
        switch (decl.astKind) {
            case ASTKind::ENUM_DECL:
                result.instVarHash = VisitEnumConstructors(static_cast<const EnumDecl&>(decl));
                break;
            case ASTKind::INTERFACE_DECL:
                // Instantce interface members are also treated as open.
                result.virtHash = VirtualHashOfInterface(decl);
                break;
            case ASTKind::CLASS_DECL:
                result.virtHash = ASTHasher::VirtualHash(static_cast<const ClassDecl&>(decl));
                result.instVarHash = VisitMemberVariables(decl);
                break;
            case ASTKind::STRUCT_DECL:
                result.instVarHash = VisitMemberVariables(decl);
                break;
            case ASTKind::VAR_WITH_PATTERN_DECL:
                ++gid;
                invp = true;
                VisitVarWithPattern(static_cast<const VarWithPatternDecl&>(decl));
                invp = false;
                result.isGV = true;
                CODEC_ASSERT(IsOOEAffectedDecl(decl));
                CODEC_NULLPTR_CHECK(curFile);
                curFile->push_back(&decl); // global varwithpattern always has an initialiser
                break;
            case ASTKind::VAR_DECL:
                result.isGV = !invp;
                if (!invp) {
                    ++gid;
                    if (IsOOEAffectedDecl(decl)) {
                        CODEC_NULLPTR_CHECK(curFile);
                        curFile->push_back(&decl);
                    }
                }
                break;
            case ASTKind::FUNC_DECL:
                if (IsOOEAffectedDecl(decl)) { // foreign global func does not have a body
                    CODEC_NULLPTR_CHECK(curFile);
                    curFile->push_back(&decl);
                }
                break;
            case ASTKind::MAIN_DECL:
            case ASTKind::MACRO_DECL:
                CODEC_NULLPTR_CHECK(curFile);
                curFile->push_back(&decl);
                break;
            default:
                break;
        }
    }

    Codira::TopLevelDeclCache VisitTopLevel(Decl& decl)
    {
        TopLevelDeclCache result{};
        VisitTopLevelDeclMembers(decl, result);

        decl.rawMangleName = mangler.Mangle(decl);
        decl.hash.srcUse = result.srcUse = ASTHasher::SrcUseHash(decl);
        decl.hash.bodyHash = result.bodyHash = ASTHasher::BodyHash(decl, srcInfo);
        decl.hash.sig = result.sigHash = ASTHasher::SigHash(decl);
        VisitTopLevelImpl1(decl, result);
        decl.hash.instVar = result.instVarHash;
        decl.hash.virt = result.virtHash;
        result.gvid = {GetTrimmedPath(decl.curFile.get()), decl.hash.gvid = gid};
        result.astKind = static_cast<uint8_t>(decl.astKind);
        return result;
    }

    Codira::MemberDeclCache VisitMember(Decl& decl)
    {
        MemberDeclCache result{};
        for (auto member : GetMembers(decl)) {
            result.members.push_back(VisitMember(*member));
        }
        result.rawMangle = decl.rawMangleName = mangler.Mangle(decl);
        CollectDecl(decl);
        decl.hash.srcUse = result.srcUse = ASTHasher::SrcUseHash(decl);
        decl.hash.bodyHash = result.bodyHash = ASTHasher::BodyHash(decl, srcInfo);
        decl.hash.sig = result.sigHash = ASTHasher::SigHash(decl);
        if (IsStaticVar(decl)) {
            ++gid; // only increment gid, not basegid
        }
        result.gvid = {GetTrimmedPath(decl.curFile.get()), decl.hash.gvid = gid};
        if (IsOOEAffectedDecl(decl)) {
            CODEC_NULLPTR_CHECK(curFile);
            curFile->push_back(&decl);
        }
        if (IsMemberParam(decl)) {
            // Member param in primary ctor should be counted as varDecl.
            result.astKind = static_cast<uint8_t>(ASTKind::VAR_DECL);
            result.isGV = false;
        } else {
            result.astKind = static_cast<uint8_t>(decl.astKind);
            result.isGV = decl.astKind == ASTKind::VAR_DECL && decl.TestAttr(Attribute::STATIC);
        }

        return result;
    }

    // direct extends are first collected and then visited later, but their gvid should follow that of the outer decl
    // where it is collected, so the gvid must be passed manually
    // it happens that extend cannot have variable member, so a single gvid is enough for all inner decls of an extend
    MemberDeclCache RevisitDirectExtendMember(Decl& decl, int gvid)
    {
        MemberDeclCache result{};
        for (auto member : GetMembers(decl)) {
            result.members.push_back(RevisitDirectExtendMember(*member, gvid));
        }
        result.rawMangle = decl.rawMangleName;
        result.srcUse = decl.hash.srcUse;
        result.bodyHash = decl.hash.bodyHash;
        result.sigHash = decl.hash.sig;
        result.astKind = static_cast<uint8_t>(decl.astKind);
        result.gvid.file = GetTrimmedPath(decl.curFile.get());
        decl.hash.gvid = result.gvid.id = gvid;
        for (auto member : decl.GetMemberDeclPtrs()) {
            member->hash.gvid = gvid;
        }
        for (auto& member : result.members) {
            member.gvid.id = gvid;
        }
        return result;
    }

    void CollectDecl(const Decl& decl)
    {
        const RawMangledName& mangled = decl.rawMangleName;
        if (auto it{mangled2Decl.find(mangled)}; it != mangled2Decl.cend()) {
            (void)duplicatedMangleNames.insert(&decl);
            (void)duplicatedMangleNames.insert(it->second);
        }
        mangled2Decl[mangled] = &decl;
    }
};

ASTCacheCalculator::ASTCacheCalculator(const AST::Package& p, const std::pair<bool, bool>& srcInfo)
    : impl{MakeOwned<ASTCacheCalculatorImpl>(*this, p, srcInfo)}
{
}
ASTCacheCalculator::~ASTCacheCalculator() = default;
void ASTCacheCalculator::Walk() const { impl->Walk(); }
}
