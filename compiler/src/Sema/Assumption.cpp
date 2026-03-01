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
 * This file defines functions for collecting type constraints recursively. This process
 * is called assumption.
 */

#include "TypeCheckerImpl.h"

#include "Codira/AST/Match.h"
#include "Codira/AST/Node.h"

using namespace Codira;
using namespace AST;

namespace {
/** Add the subtype relation subTy <: upperBoundTy to the type constraint collection @p typeConstraintCollection. */
void AddConstraint(TyVarEnv& typeConstraintCollection, Ty& subTy, Ty& upperBoundTy)
{
    if (!subTy.IsGeneric()) {
        return;
    }
    auto subGen = RawStaticCast<GenericsTy*>(&subTy);
    subGen->upperBounds.insert(&upperBoundTy);
    auto found = typeConstraintCollection.find(subGen);
    if (found == typeConstraintCollection.end()) {
        typeConstraintCollection.emplace(subGen, std::set<Ptr<Ty>>{&upperBoundTy});
    } else {
        found->second.insert(&upperBoundTy);
    }
}

/** Check whether the type constraint collection @p typeConstraintCollection has subtype relashion: subTy <: baseTy. */
bool LookUpConstraintCollection(Ty& subTy, Ty& baseTy, const TyVarEnv& typeConstraintCollection)
{
    if (&subTy == &baseTy) {
        return true;
    }
    if (!subTy.IsGeneric()) {
        return false;
    }
    auto found = typeConstraintCollection.find(StaticCast<TyVar*>(&subTy));
    // If the subTy cannot be found in typeConstraintCollection, return false.
    if (found == typeConstraintCollection.end()) {
        return false;
    }
    return found->second.find(&baseTy) != found->second.end();
}

/**
 * Checking Whether the Upper Bound of Generics Has InvalidTy.
 */
bool IsUpperBoundsValid(const std::vector<OwnedPtr<Type>>& upperBounds)
{
    for (auto& upper : upperBounds) {
        if (!Ty::IsTyCorrect(upper->ty)) {
            return false;
        }
    }
    return true;
}
} // namespace

void TypeChecker::TypeCheckerImpl::PerformAssumeReferenceTypeUpperBound(TyVarUB& typeConstraintCollection,
    GCBlames& blames, const AST::Type& referenceTypeUpperBound, const TypeSubst& typeMapping)
{
    auto upperBoundTy = referenceTypeUpperBound.ty;
    Ptr<Decl> baseDecl = Ty::GetDeclPtrOfTy(upperBoundTy);
    // If the upperBound is a generic Type and has associate declaration, perform assumption recursively.
    if (baseDecl != nullptr && Ty::IsTyCorrect(upperBoundTy) && upperBoundTy->HasGeneric()) {
        // 1. Create substitute Map between generic tys of upperBound's decl and current uppBound's tys which
        // can be generic or not.
        TypeSubst substituteMap = typeManager.GetSubstituteMapping(*upperBoundTy, typeMapping);
        // 2. Perform assumption recursively.
        Assumption(typeConstraintCollection, blames, *baseDecl, substituteMap);
    }
}

void TypeChecker::TypeCheckerImpl::AssumeOneUpperBound(
    TyVarUB& typeConstraintCollection, GCBlames& blames, const AST::Type& upperBound, const TypeSubst& typeMapping)
{
    switch (upperBound.astKind) {
        case ASTKind::REF_TYPE:
        case ASTKind::QUALIFIED_TYPE: {
            PerformAssumeReferenceTypeUpperBound(typeConstraintCollection, blames, upperBound, typeMapping);
            break;
        }
        default:
            break;
    }
}

void TypeChecker::TypeCheckerImpl::PerformAssumptionForOneGenericConstraint(
    TyVarUB& typeConstraintCollection, GCBlames& blames, const GenericConstraint& gc, const TypeSubst& typeMapping)
{
    auto subTypeTy = gc.type->ty;
    if (!Ty::IsTyCorrect(subTypeTy)) {
        return;
    }
    auto subTy = typeManager.GetInstantiatedTy(subTypeTy, typeMapping);
    for (const auto& upperBound : gc.upperBounds) {
        if (!upperBound) {
            continue;
        }
        auto upperBoundTy = upperBound->ty;
        if (!Ty::IsTyCorrect(upperBoundTy)) {
            continue;
        }
        auto baseTy = typeManager.GetInstantiatedTy(upperBoundTy, typeMapping);
        // If the constraint is already exist in typeConstraintCollection, no need to do assumption recursively.
        if (!subTy->IsGeneric() || LookUpConstraintCollection(*subTy, *baseTy, typeConstraintCollection)) {
            continue;
        }
        // Add the constraint to the typeConstraintCollection.
        AddConstraint(typeConstraintCollection, *subTy, *baseTy);
        blames[subTy][baseTy].emplace(&gc);
        AssumeOneUpperBound(typeConstraintCollection, blames, *upperBound, typeMapping);
    }
}

void TypeChecker::TypeCheckerImpl::Assumption(
    TyVarUB& typeConstraintCollection, GCBlames& blames, const AST::Decl& decl, const TypeSubst& typeMapping)
{
    Ptr<Generic> generic = decl.GetGeneric();
    if (generic == nullptr) {
        return;
    }
    for (auto& gc : generic->genericConstraints) {
        bool shouldCheckUpperBounds = gc && gc->type && !gc->upperBounds.empty() && IsUpperBoundsValid(gc->upperBounds);
        if (shouldCheckUpperBounds) {
            PerformAssumptionForOneGenericConstraint(typeConstraintCollection, blames, *gc, typeMapping);
        }
    }
}
