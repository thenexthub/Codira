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
 * This file declares the class for calculating the smallest common supertype (or join, or least upper bound) and the
 * greatest common subtype (or meet, or greatest lower bound) of the given set of types.
 */
#ifndef CODIRA_SEMA_JOINANDMEET_H
#define CODIRA_SEMA_JOINANDMEET_H

#include <variant>
#include <functional>

#include "Codira/AST/Types.h"
#include "Codira/Sema/TypeManager.h"
#include "Codira/Modules/ImportManager.h"

namespace Codira {
struct DualMode {
    Ptr<AST::Ty> bound; // Any for join, Nothing for meet
    std::function<Ptr<AST::Ty>(const std::set<Ptr<AST::Ty>>&)> coFunc; // join for join, meet for meet
    std::function<Ptr<AST::Ty>(const std::set<Ptr<AST::Ty>>&)> contraFunc; // meet for join, join for meet
    std::function<bool(Ptr<AST::Ty>, Ptr<AST::Ty>)> coSubtyFunc; // is-subtype for join, is-supertype for meet
};

class JoinAndMeet {
    using ErrMsg = std::stack<std::string>;
    using ErrOrTy = std::variant<ErrMsg, Ptr<AST::Ty>>;

public:
    // if curFile is given, impMgr must also be given
    JoinAndMeet(TypeManager& tyMgr, const std::initializer_list<Ptr<AST::Ty>> tySet,
        const std::initializer_list<Ptr<TyVar>> ignoredTyVars = {}, Ptr<const ImportManager> impMgr = nullptr,
        Ptr<AST::File> curFile = nullptr)
        : tyMgr(tyMgr), tySet(tySet), ignoredTyVars(ignoredTyVars), impMgr(impMgr), curFile(curFile)
    {
    }
    // if curFile is given, impMgr must also be given
    JoinAndMeet(TypeManager& tyMgr, const std::set<Ptr<AST::Ty>> tySet, const std::set<Ptr<TyVar>> ignoredTyVars = {},
        Ptr<const ImportManager> impMgr = nullptr, Ptr<AST::File> curFile = nullptr)
        : tyMgr(tyMgr), tySet(tySet), ignoredTyVars(ignoredTyVars), impMgr(impMgr), curFile(curFile)
    {
    }

    /**
     * Calculate the join (i.e. least upper bound) of two types.
     * sprsErr: suppress error messages. We opt in reporting the summary of errors after the join (meet) finishes and
     * the error messages produced along the calculation are regarded as logs for debuging.
     * Turn the sprsErr from true to false when debuging this module.
     */
    ErrOrTy Join(bool sprsErr = true);
    ErrOrTy JoinAsVisibleTy();
    /**
     * Calculate the meet (i.e. greatest lower bound) of two types.
     */
    ErrOrTy Meet(bool sprsErr = true);
    ErrOrTy MeetAsVisibleTy();
    static std::string CombineErrMsg(ErrMsg& msgs);

    // Caution! The serial of functions modifies the first argument.
    // The first argument is guaranteed to be not null after the invocation.
    static std::optional<std::string> SetJoinedType(
        Ptr<AST::Ty>& ty, std::variant<std::stack<std::string>, Ptr<AST::Ty>>& joinRes);
    static std::optional<std::string> SetMetType(
        Ptr<AST::Ty>& ty, std::variant<std::stack<std::string>, Ptr<AST::Ty>>& metRes);

    // Convert the input type to a user-visible one by eliminating intersection and union types.
    // Use a boolean value isJoin to distinguish the join and meet mode.
    Ptr<AST::Ty> ToUserVisibleTy(Ptr<AST::Ty> ty);

private:
    TypeManager& tyMgr;
    const std::set<Ptr<AST::Ty>> tySet;
    const TyVars ignoredTyVars;
    Ptr<const ImportManager> impMgr;
    Ptr<AST::File> curFile;
    ErrMsg errMsg;
    bool isForcedToUserVisible = false;

    Ptr<AST::Ty> BatchJoin(const std::set<Ptr<AST::Ty>>& tys);
    Ptr<AST::Ty> BatchMeet(const std::set<Ptr<AST::Ty>>& tys);

    Ptr<AST::Ty> JoinOrMeetFuncTy(const DualMode& mode, const std::set<Ptr<AST::Ty>>& tys);
    Ptr<AST::Ty> JoinOrMeetTupleTy(const DualMode& mode, const std::set<Ptr<AST::Ty>>& tys);

    void AddFinalErrMsgs(const AST::Ty& ty, bool isJoin);
    bool IsInputValid() const;
};
} // namespace Codira
#endif // CODIRA_SEMA_JOINANDMEET_H
