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
 * This file declares some type alias used by the semantic check modules.
 */

#ifndef CODIRA_SEMA_COMMONTYPEALIAS_H
#define CODIRA_SEMA_COMMONTYPEALIAS_H

#include "Codira/AST/Node.h"
#include "Codira/AST/Types.h"
#include "Codira/Utils/PartiallyPersistent.h"

namespace Codira {
enum class BlameStyle {
    ARGUMENT,
    RETURN,
    CONSTRAINT
};

struct Blame {
    Ptr<const AST::Node> src;
    Ptr<const AST::Ty> lb;
    Ptr<const AST::Ty> ub;
    BlameStyle style{BlameStyle::ARGUMENT};
    bool operator<(const Blame& rhs) const
    {
        if (!rhs.src) {
            return false;
        }
        if (!src) {
            return true;
        }
        return src->begin < rhs.src->begin ||
            (src->begin == rhs.src->begin &&
            src->end < rhs.src->end);
    }
};

using AST::TyVar;
using TyVars = std::set<Ptr<TyVar>>;
using TyVarUB = std::map<Ptr<TyVar>, std::set<Ptr<AST::Ty>>>;

using LowerBounds = PSet<Ptr<AST::Ty>>;
using UpperBounds = PSet<Ptr<AST::Ty>>;

template <typename C>
inline TyVars StaticToTyVars(C tys)
{
    TyVars ret;
    for (auto ty : tys) {
        ret.emplace(static_cast<TyVar*>(ty.get()));
    }
    return ret;
}

struct TyVarBounds {
    LowerBounds lbs{};
    UpperBounds ubs{};
    // must pick from one of these types, will kick out elements as more info becomes available
    // when initialized: will have a single element Any
    UpperBounds sum{};
    // may greedily decide a ty var's solution, should have only 0 or 1 element : the solution
    // PSet here only to reuse its backtracking
    PSet<Ptr<AST::Ty>> eq{};
    // currently don't track source for non-local ty var solving failure,
    // therefore don't need backtrackable data structure for blames
    std::map<Ptr<AST::Ty>, std::set<Blame>> lb2Blames;
    std::map<Ptr<AST::Ty>, std::set<Blame>> ub2Blames;
    std::string ToString() const;
};

using Constraint = std::map<Ptr<TyVar>, TyVarBounds>;
using Constraints = std::vector<Constraint>;

// Substitution from type variables to types.
using TypeSubst = std::map<Ptr<TyVar>, Ptr<AST::Ty>>;
using TypeSubsts = std::set<TypeSubst>;

// Substitution from type variables to their multiple instantiated types.
// Eg: Given interface I3<Ti> and class C<T, V> <: I3<T> & I3<V>,  then [Ti |-> [T, V]]
using MultiTypeSubst = std::map<Ptr<TyVar>, std::set<Ptr<AST::Ty>>>;

std::string ToStringC(const Constraint& c);
std::string ToStringS(const TypeSubst& m);
std::string ToStringMS(const MultiTypeSubst& m);

/*
 * Maps possibly needed to instantiate any type.
 *
 * Theoretically, there should be the following 3 levels of mapping:
 * u2i map: map from universal ty var to instance ty var, should be applied first
 * promote maps: map from ty var of supertype to types consisting of ty var of subtype,
 *              should be applied in order, after applying u2i map
 * inst map: map from instance ty var to type argument, should be applied last
 *
 * An example:
 * open class A<T> { func foo<S>(x: T, y: S) {} }
 * open class B<R> <: A<Option<R>> {}
 * class C<U> <: B<Array<U>> {}
 * let v = C<Int>().foo(Some([1]), 2).
 *
 * For the instance of `foo` in the last line, the u2i map is:
 * [T |-> T', R |-> R', U |-> U', S |-> S']
 * where T', R', U', S' are instance ty vars, which are placeholders for real type args.
 *
 * The promote maps are:
 * [[T' |-> Option<R'>],
 *  [R' |-> Array<U'>]]
 *
 * The inst map is:
 * [U' |-> Int]
 *
 * foo's declared type is:
 *   (T, S)->Unit
 * after applying u2i map:
 *   (T', S')->Unit
 * after applying promote maps one by one
 *   (Option<R'>, S')->Unit
 *   (Option<Array<U'>>, S')->Unit
 * after applying inst map:
 *   (Option<Array<Int>>, S')->Unit
 *
 * Now only S' is left to be solved, the solution of which is obviously Int.
 * Then we add this solution to inst map:
 * [U' |-> Int, S' |-> Int]
 *
 * So foo's instantiated type is:
 *   (Option<Array<Int>>, Int)->Unit
 *
 * promote maps and inst map can be merged into one if needed,
 * since there can't be loop in these substitutions.
 * But u2i map shouldn't be merged with the rest,
 * since its domain can overlap with the range of other substituions.
 * e.g.
 * func f<T>():Unit {
 *     f<T>() // here u2i map:[T |-> T'], inst map: [T' |-> T]
 * }
 *
 * In this struct, `inst` represents promote maps and inst map merged.
 */
struct SubstPack {
    TypeSubst u2i;
    MultiTypeSubst inst;
};

std::string ToStringP(const SubstPack& m);

struct StableTyCmp {
    bool operator ()(const Ptr<const AST::Ty> ty1, const Ptr<const AST::Ty> ty2) const
    {
        return AST::CompTyByNames(ty1, ty2);
    }
};
using StableTys = std::set<Ptr<AST::Ty>, StableTyCmp>;
using StableTyVars = std::set<Ptr<TyVar>, StableTyCmp>;

enum class SolvingErrStyle {
    DEFAULT,
    NO_CONSTRAINT,
    CONFLICTING_CONSTRAINTS,
    ARG_MISMATCH,
    RET_MISMATCH
};

struct SolvingErrInfo {
    SolvingErrStyle style = SolvingErrStyle::DEFAULT;
    Ptr<TyVar> tyVar;
    std::vector<Ptr<AST::Ty>> lbs;
    std::vector<Ptr<AST::Ty>> ubs;
    // in case of conflicting constraints, first blames for lbs, then blames for ubs
    std::vector<std::set<Blame>> blames;
};

using ErrOrSubst = std::variant<SolvingErrInfo, TypeSubst>;

// tyvar -> lb X ub X sum X eq
using CstVersionID = std::map<Ptr<TyVar>, std::tuple<VersionID, VersionID, VersionID, VersionID>>;

template<> void PData::Commit(Constraint& data);
template<> void PData::Reset(Constraint& data);
template<> CstVersionID PData::Stash(Constraint& data);
template<> void PData::Apply(Constraint& data, CstVersionID& version);
template<> void PData::ResetSoft(Constraint& data);
} // namespace Codira
#endif // CODIRA_SEMA_COMMONTYPEALIAS_H
