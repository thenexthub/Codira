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
 * This file implements the elimination of recursive types.
 */

#include "TypeCheckerImpl.h"

using namespace Codira;
using namespace AST;

namespace {
struct CmpNodeByPackagePos {
    bool operator()(Ptr<const AST::Node> n1, Ptr<const AST::Node> n2) const
    {
        if (n1 && n1->curFile && n1->curFile->curPackage && n2 && n2->curFile && n2->curFile->curPackage) {
            if (int cp1 = n1->curFile->curPackage->fullPackageName.compare(n2->curFile->curPackage->fullPackageName);
                cp1 != 0) {
                return cp1 < 0;
            }
        }
        return CompNodeByPos(n1, n2);
    }
};

// A dependency graph among value types.
// There are two kinds of vertices in the graph: `StructDecl` and `EnumDecl`.
// There is an arc from `u` to `v`, if `v` is a member variable of a `struct` `u`,
// or `v` is a parameter of a constructor of an `enum` `u`,
class Graph {
public:
    explicit Graph(const std::vector<Ptr<PackageDecl>>& pkgs)
    {
        for (auto pkg : pkgs) {
            CODEC_NULLPTR_CHECK(pkg);
            AddArcs(*pkg->srcPackage);
        }
    }

    const std::set<Ptr<const Decl>, CmpNodeByPackagePos>& InEdges(const EnumDecl& ed) const
    {
        return enumInEdges.at(&ed);
    }

    std::vector<Graph> StronglyConnectedComponents() const
    {
        TarjanContext ctx;
        for (auto u : vertices) {
            CODEC_NULLPTR_CHECK(u);
            if (!Utils::InKeys(u, ctx.indices)) {
                TarjanSCC(ctx, *u);
            }
        }
        return std::move(ctx.sccs);
    }

    // Peek the `enum` with the maximum in-degree to box.
    // Will peek the foremost one if there are more than one maximums.
    // Returns `nullptr` if there are no `enum`s in the graph or the SCC is trivial.
    Ptr<const EnumDecl> PeekEnumToBox() const
    {
        Ptr<const EnumDecl> candidate = nullptr;
        size_t maxInDegree = 0;
        for (auto& [ed, inEdges] : enumInEdges) {
            size_t inDegree = inEdges.size();
            if (inDegree > maxInDegree) {
                candidate = ed;
                maxInDegree = inDegree;
            }
        }
        return candidate;
    }

    void RemoveVertex(const EnumDecl& ed)
    {
        vertices.erase(&ed);
        for (auto u : enumInEdges.at(&ed)) {
            outEdges.at(u).erase(&ed);
        }
        outEdges.erase(&ed);
        enumInEdges.erase(&ed);
    }

private:
    Graph()
    {
    }

    void AddVertex(const Decl& v)
    {
        if (!Ty::IsTyCorrect(v.ty) || v.ty->HasGeneric()) {
            return;
        }
        vertices.emplace(&v);
        if (!Utils::InKeys(Ptr(&v), outEdges)) {
            outEdges[&v] = {};
        }
        if (v.astKind == ASTKind::ENUM_DECL) {
            auto& ed = StaticCast<const EnumDecl&>(v);
            if (!Utils::InKeys(Ptr(&ed), enumInEdges)) {
                enumInEdges[&ed] = {};
            }
        }
    }

    void AddArc(const Decl& u, const Decl& v)
    {
        if (!Ty::IsTyCorrect(u.ty) || u.ty->HasGeneric() || !Ty::IsTyCorrect(v.ty) || v.ty->HasGeneric()) {
            return;
        }
        AddVertex(u);
        AddVertex(v);
        outEdges[&u].emplace(&v);
        if (v.astKind == ASTKind::ENUM_DECL) {
            auto& ed = StaticCast<const EnumDecl&>(v);
            enumInEdges[&ed].emplace(&u);
        }
    }

    void AddArcs(const Decl& decl, const Ty& fieldTy)
    {
        if (!Ty::IsTyCorrect(&fieldTy)) {
            return;
        } else if ((fieldTy.IsStruct() || fieldTy.IsEnum()) && Ty::GetDeclOfTy(&fieldTy)) {
            AddArc(decl, *Ty::GetDeclOfTy(&fieldTy));
        } else if (fieldTy.IsTuple()) {
            for (auto typeArg : fieldTy.typeArgs) {
                if (Ty::IsTyCorrect(typeArg)) {
                    AddArcs(decl, *typeArg);
                }
            }
        }
    }

    void AddArcs(const StructDecl& sd)
    {
        CODEC_NULLPTR_CHECK(sd.body);
        for (auto& d : sd.body->decls) {
            CODEC_NULLPTR_CHECK(d);
            if (d->astKind != ASTKind::VAR_DECL || d->TestAttr(Attribute::STATIC) || !Ty::IsTyCorrect(d->ty)) {
                continue;
            }
            AddArcs(sd, *d->ty);
        }
    }

    void AddArcs(const EnumDecl& ed)
    {
        for (auto& ctor : ed.constructors) {
            CODEC_NULLPTR_CHECK(ctor);
            if (auto fd = DynamicCast<const FuncDecl*>(ctor.get())) {
                CODEC_ASSERT(fd->funcBody && fd->funcBody->paramLists.size() == 1 && fd->funcBody->paramLists.front());
                for (auto& param : fd->funcBody->paramLists.front()->params) {
                    CODEC_NULLPTR_CHECK(param);
                    if (Ty::IsTyCorrect(param->ty)) {
                        AddArcs(ed, *param->ty);
                    }
                }
            }
        }
    }

    void AddArcs(const Decl& decl)
    {
        if (decl.astKind == ASTKind::STRUCT_DECL) {
            AddArcs(StaticCast<const StructDecl&>(decl));
        } else if (decl.astKind == ASTKind::ENUM_DECL) {
            AddArcs(StaticCast<const EnumDecl&>(decl));
        }
    }

    void AddArcs(const Package& pkg)
    {
        IterateToplevelDecls(pkg, [this](auto& decl) {
            CODEC_NULLPTR_CHECK(decl);
            AddArcs(*decl);
        });
        for (auto& decl : pkg.genericInstantiatedDecls) {
            CODEC_NULLPTR_CHECK(decl);
            AddArcs(*decl);
        }
    }

    // Get the vertex-induced sub-graph.
    Graph SubGraph(const std::unordered_set<Ptr<const Decl>>& subVertices) const
    {
        Graph subGraph;
        for (auto& [u, uOutEdges] : outEdges) {
            CODEC_NULLPTR_CHECK(u);
            if (!Utils::In(u, subVertices)) {
                continue;
            }
            subGraph.AddVertex(*u);
            for (auto v : uOutEdges) {
                if (Utils::In(v, subVertices)) {
                    CODEC_ASSERT(u && v);
                    subGraph.AddArc(*u, *v);
                }
            }
        }
        return subGraph;
    }

    struct TarjanContext {
        size_t index = 0;
        std::vector<Ptr<const Decl>> stack;
        std::unordered_map<Ptr<const Decl>, size_t> indices;  /**< The discovered order of vertices in a DFS. */
        std::unordered_map<Ptr<const Decl>, size_t> lowlinks; /**< The smallest index reachable from the vertex. */
        std::unordered_map<Ptr<const Decl>, bool> onStack;    /**< Indicate whether the vertex is on stack. */
        std::vector<Graph> sccs;                              /**< Strongly connected components. */
    };

    void TarjanSCC(TarjanContext& ctx, const Decl& u) const
    {
        ctx.indices[&u] = ctx.index;
        ctx.lowlinks[&u] = ctx.index;
        ++ctx.index;
        ctx.stack.emplace_back(&u);
        ctx.onStack[&u] = true;
        CODEC_ASSERT(Utils::InKeys(Ptr(&u), outEdges));
        for (auto v : outEdges.at(&u)) {
            CODEC_NULLPTR_CHECK(v);
            if (!Utils::InKeys(v, ctx.indices)) {
                TarjanSCC(ctx, *v);
                ctx.lowlinks[&u] = std::min(ctx.lowlinks[&u], ctx.lowlinks[v]);
            } else if (ctx.onStack[v]) {
                ctx.lowlinks[&u] = std::min(ctx.lowlinks[&u], ctx.indices[v]);
            }
        }
        if (ctx.lowlinks[&u] == ctx.indices[&u]) {
            std::unordered_set<Ptr<const Decl>> subVertices;
            Ptr<const Decl> w = nullptr;
            do {
                w = ctx.stack.back();
                ctx.stack.pop_back();
                ctx.onStack[w] = false;
                subVertices.emplace(w);
            } while (w != &u);
            ctx.sccs.emplace_back(SubGraph(subVertices));
        }
    }

    std::set<Ptr<const Decl>, CmpNodeByPackagePos> vertices;
    std::map<Ptr<const Decl>, std::set<Ptr<const Decl>, CmpNodeByPackagePos>, CmpNodeByPackagePos> outEdges;
    // We don't care about the in-edges of `struct`s
    std::map<Ptr<const EnumDecl>, std::set<Ptr<const Decl>, CmpNodeByPackagePos>, CmpNodeByPackagePos> enumInEdges;
};

void CheckAndUpdateDeclTyWithNewTy(Decl& decl, const EnumTy& specifiedTy, TypeManager& typeManager)
{
    if (decl.ty->IsTuple()) {
        auto tupleTy = RawStaticCast<TupleTy*>(decl.ty);
        bool hasSpecifiedTy =
            std::find_if(tupleTy->typeArgs.begin(), tupleTy->typeArgs.end(),
                [&specifiedTy](auto& typeArg) { return typeArg == &specifiedTy; }) != tupleTy->typeArgs.end();
        if (!hasSpecifiedTy) {
            return;
        }
        std::vector<Ptr<Ty>> vec;
        for (auto elemTy : tupleTy->typeArgs) {
            if (elemTy == &specifiedTy) {
                auto enumTy = RawStaticCast<EnumTy*>(elemTy);
                auto newTy = typeManager.GetRefEnumTy(*enumTy->declPtr, enumTy->typeArgs);
                enumTy->hasCorrespondRefEnumTy = true;
                newTy->decl = enumTy->decl;
                vec.emplace_back(newTy);
            } else {
                vec.emplace_back(elemTy);
            }
        }
        decl.ty = typeManager.GetTupleTy(vec);
    } else if (decl.ty == &specifiedTy) {
        auto enumTy = RawStaticCast<EnumTy*>(decl.ty);
        auto newTy = typeManager.GetRefEnumTy(*enumTy->declPtr, enumTy->typeArgs);
        enumTy->hasCorrespondRefEnumTy = true;
        newTy->decl = enumTy->decl;
        decl.ty = newTy;
    }
}
} // namespace

void TypeChecker::TypeCheckerImpl::UpdateMemberVariableTy(const Decl& decl, const EnumTy& eTy)
{
    switch (decl.astKind) {
        case ASTKind::STRUCT_DECL: {
            auto rd = StaticCast<StructDecl*>(&decl);
            for (auto& d : rd->body->decls) {
                if (!d || d->astKind != ASTKind::VAR_DECL || d->TestAttr(Attribute::STATIC)) {
                    continue;
                }
                CheckAndUpdateDeclTyWithNewTy(*d, eTy, typeManager);
            }
            break;
        }
        case ASTKind::ENUM_DECL: {
            auto ed = StaticCast<EnumDecl*>(&decl);
            for (auto& ctor : ed->constructors) {
                if (ctor->astKind != ASTKind::FUNC_DECL) {
                    continue;
                }
                auto fd = StaticCast<FuncDecl*>(ctor.get());
                for (auto& d : fd->funcBody->paramLists[0]->params) {
                    CheckAndUpdateDeclTyWithNewTy(*d, eTy, typeManager);
                }
            }
            break;
        }
        default:
            CODEC_ASSERT(false && "wrong branch");
            break;
    }
}

void TypeChecker::TypeCheckerImpl::PerformRecursiveTypesElimination()
{
    // Since `RefEnumTy` and `EnumTy` cannot be distinguished in codeo, we have to check all the imported packages.
    // NOTE: This api will also contains current source package.
    Graph graph(importManager.GetAllImportedPackages());
    std::vector<Graph> sccs = graph.StronglyConnectedComponents();
    while (!sccs.empty()) {
        Graph scc = std::move(sccs.back());
        sccs.pop_back();
        auto ed = scc.PeekEnumToBox();
        if (ed == nullptr) {
            continue;
        }
        auto& inEdges = scc.InEdges(*ed);
        for (auto u : inEdges) {
            CODEC_NULLPTR_CHECK(u);
            UpdateMemberVariableTy(*u, *StaticCast<EnumTy*>(ed->ty));
        }
        scc.RemoveVertex(*ed);
        auto subSCCs = scc.StronglyConnectedComponents();
        std::copy(std::move_iterator{subSCCs.cbegin()}, std::move_iterator{subSCCs.cend()}, std::back_inserter(sccs));
    }
}
