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

#ifndef CODIRA_CHIR_CHECKER_VAR_INIT_CHECK_H
#define CODIRA_CHIR_CHECKER_VAR_INIT_CHECK_H

#include "Codira/CHIR/Analysis/MaybeInitAnalysis.h"
#include "Codira/CHIR/Analysis/MaybeUninitAnalysis.h"
#include "Codira/CHIR/CHIRCasting.h"
#include "Codira/CHIR/DiagAdapter.h"
#include "Codira/CHIR/Expression/Terminator.h"
#include "Codira/CHIR/Package.h"

namespace Codira::CHIR {

class VarInitCheck {
public:
    explicit VarInitCheck(DiagAdapter* diag);

    void RunOnPackage(const Package* package, size_t threadNum);

    void RunOnFunc(const Func* func);

private:
    // ================================================================= //
    void UseBeforeInitCheck(const Func* func, const ConstructorInitInfo* ctorInitInfo,
        const std::vector<MemberVarInfo>& members);

    bool CheckLoadToUninitedAllocation(const MaybeUninitDomain& state, const Load& load) const;

    bool CheckGetElementRefToUninitedAllocation(
        const MaybeUninitDomain& state, const GetElementRef& getElementRef) const;

    void CheckLoadToUninitedCustomDefMember(const MaybeUninitDomain& state, const Func* func, const Load* load,
        const std::vector<MemberVarInfo>& members) const;

    void CheckStoreToUninitedCustomDefMember(const MaybeUninitDomain& state, const Func* func,
        const StoreElementRef* store, const std::vector<MemberVarInfo>& members) const;

    void AddMaybeInitedPosNote(
        DiagnosticBuilder& builder, const std::string& identifier, const std::set<unsigned>& maybeInitedPos) const;

    void CheckUninitedDefMember(const MaybeUninitDomain& state, const Expression* expr,
        const std::vector<MemberVarInfo>& members, size_t index, bool onlyCheckSuper = false) const;

    void RaiseUninitedDefMemberError(const MaybeUninitDomain& state, const Func* func,
        const std::vector<MemberVarInfo>& members, const std::vector<size_t>& uninitedMemberIdx) const;

    template <typename TApply>
    void CheckMemberFuncCall(const MaybeUninitDomain& state, const Func& initFunc, const TApply& apply) const;

    void RaiseIllegalMemberFunCallError(const Expression* apply, const Func* memberFunc) const;

    // ================================================================= //
    void ReassignInitedLetVarCheck(const Func* func, const ConstructorInitInfo* ctorInitInfo,
        const std::vector<MemberVarInfo>& members) const;

    void CheckStoreToInitedCustomDefMember(const MaybeInitDomain& state, const Func* func, const StoreElementRef* store,
        const std::vector<MemberVarInfo>& members) const;

private:
    DiagAdapter* diag;
};

} // namespace Codira::CHIR

#endif
