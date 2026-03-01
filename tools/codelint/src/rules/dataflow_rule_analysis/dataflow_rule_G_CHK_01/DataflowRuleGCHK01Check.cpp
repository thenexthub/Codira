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

#include "DataflowRuleGCHK01Check.h"
#include "Codira/AST/Match.h"
#include "Codira/Utils/Utils.h"
#include "common/CommonFunc.h"

namespace Codira::CodeCheck {

using namespace Codira::CHIR;

using namespace Codira::CodeCheck::TaintData;

// Chech whether a exit node exist in a block in focus
static bool IsAnyReturnInBlock(CHIR::Block* block)
{
    auto exprs = block->GetExpressions();
    auto ret = std::find_if(
        exprs.begin(), exprs.end(), [](auto& expr) { return expr->GetExprKind() == CHIR::ExprKind::EXIT; });
    return ret != exprs.end();
}

static void GetDecontaminatBlockHelper(
    CHIR::Block* block, std::set<Codira::CHIR::Block*>& blockSet, std::set<CHIR::Block*>& visited)
{
    visited.insert(block);
    blockSet.insert(block);
    auto exprs = block->GetExpressions();
    for (auto& expr : exprs) {
        if (!expr->IsTerminator()) {
            continue;
        }
        auto terminator = StaticCast<CHIR::Terminator*>(expr);
        for (auto newBlock : terminator->GetSuccessors()) {
            if (visited.count(newBlock) == 0) {
                blockSet.insert(newBlock);
                GetDecontaminatBlockHelper(newBlock, blockSet, visited);
            }
        }
    }
}

// Obtains the block after decontamination.
static std::set<Codira::CHIR::Block*> GetDecontaminatBlock(
    CHIR::Value* value, std::set<Codira::CHIR::Block*>& blockSet, std::set<Codira::CHIR::Expression*> visitedExpr)
{
    auto users = value->GetUsers();
    for (auto user : users) {
        if (visitedExpr.count(user) != 0) {
            continue;
        }
        visitedExpr.insert(user);
        if (user->GetExprKind() == CHIR::ExprKind::BRANCH) {
            auto branch = StaticCast<CHIR::Branch*>(user);
            auto trueBlock = branch->GetTrueBlock();
            auto falseBlock = branch->GetFalseBlock();
            if (branch->GetCondition() == value && (IsAnyReturnInBlock(trueBlock) || IsAnyReturnInBlock(falseBlock))) {
                std::set<Codira::CHIR::Block*> visited;
                if (IsAnyReturnInBlock(trueBlock)) {
                    GetDecontaminatBlockHelper(falseBlock, blockSet, visited);
                }
                if (IsAnyReturnInBlock(falseBlock)) {
                    GetDecontaminatBlockHelper(trueBlock, blockSet, visited);
                }
            }
        } else {
            GetDecontaminatBlock(user->GetResult(), blockSet, visitedExpr);
        }
    }
    return blockSet;
}

static std::set<Codira::CHIR::Block*> GetDecontaminatBlock(CHIR::Value* value)
{
    std::set<Codira::CHIR::Block*> blockSet;
    std::set<Codira::CHIR::Expression*> visitedExpr;
    GetDecontaminatBlock(value, blockSet, visitedExpr);
    return blockSet;
}

static bool IsMemberFunc(CHIR::Value* value)
{
    if (value->IsFuncWithBody()) {
        auto func = VirtualCast<CHIR::Func*>(value);
        if (func->TestAttr(Attribute::STATIC)) {
            return false;
        }
        if (func->GetParentCustomTypeDef() || func->GetFuncKind() == CHIR::FuncKind::LAMBDA) {
            return true;
        }
    }
    return false;
}

static std::string GetArgName(CHIR::Value* value)
{
    if (value->IsLocalVar()) {
        auto localVar = StaticCast<CHIR::LocalVar*>(value);
        auto debug = localVar->GetDebugExpr();
        if (debug) {
            return debug->GetSrcCodeIdentifier();
        }
        auto expr = localVar->GetExpr();
        if (expr->GetExprKind() == CHIR::ExprKind::LOAD) {
            auto load = StaticCast<CHIR::Load*>(expr);
            return GetArgName(load->GetLocation());
        }
        return localVar->GetIdentifier();
    }
    return value->GetSrcCodeIdentifier();
}

template <typename T> bool DataflowRuleGCHK01Check::IsAlarmNotNeeded(T* apply)
{
    auto funcName = apply->GetCallee()->GetSrcCodeIdentifier();
    for (auto taintedFunc : taintPropagate) {
        if (taintedFunc.first.funcName == funcName) {
            return true;
        }
    }
    return false;
}

template <typename T> void DataflowRuleGCHK01Check::CheckApplyInUsers(T* apply, CHIR::Value* arg)
{
    if (IsAlarmNotNeeded<T>(apply)) {
        return;
    }
    auto args = apply->GetArgs();
    for (uint32_t i = 0; i < args.size(); ++i) {
        auto identifier = GetArgName(arg);
        auto block = apply->GetParentBlock();
        if (args[i] == arg &&
            (varInblockWithoutTainted.count(identifier) == 0 ||
                (varInblockWithoutTainted.count(identifier) > 0 &&
                    varInblockWithoutTainted[identifier].count(block) == 0))) {
            auto index = IsMemberFunc(apply->GetCallee()) ? i : i + 1;
            std::string tips = "the " + std::to_string(index) + "st argument is untrusted, we can't deliver it outter.";
            std::string identifier;
            if (auto callee = apply->GetCallee(); callee->IsLocalVar()) {
                auto lambda = StaticCast<const CHIR::LocalVar*>(callee)->GetExpr();
                CODEC_ASSERT(lambda->GetExprKind() == CHIR::ExprKind::LAMBDA);
                identifier = StaticCast<const CHIR::Lambda*>(lambda)->GetSrcCodeIdentifier();
            } else {
                identifier = callee->GetSrcCodeIdentifier();
            }
            auto applyLoc = CommonFunc::GetCodePosition(apply);
            diagEngine->Diagnose(
                applyLoc.first, applyLoc.second, CodeCheckDiagKind::G_CHK_01_illegal_func_arg, identifier, tips);
        }
    }
}

template <typename T> void DataflowRuleGCHK01Check::CheckInvokeInUsers(T* invoke, CHIR::Value* arg)
{
    auto args = invoke->GetArgs();
    for (uint32_t i = 0; i < args.size(); ++i) {
        auto identifier = GetArgName(arg);
        auto block = invoke->GetParentBlock();
        if (args[i] == arg &&
            (varInblockWithoutTainted.count(identifier) == 0 ||
                (varInblockWithoutTainted.count(identifier) > 0 &&
                    varInblockWithoutTainted[identifier].count(block) == 0))) {
            auto index = IsMemberFunc(invoke->GetOperand(0)) ? i : i + 1;
            std::string tips = "the " + std::to_string(index) + "st argument is untrusted, we can't deliver it outter.";
            auto invokeLoc = CommonFunc::GetCodePosition(invoke);
            diagEngine->Diagnose(invokeLoc.first, invokeLoc.second, CodeCheckDiagKind::G_CHK_01_illegal_func_arg,
                invoke->GetMethodName(), tips);
        }
    }
}

static CommonFunc::PositionPair GetCorrectCodeLoc(CHIR::Store* store)
{
    auto pos = CommonFunc::GetCodePosition(store);
    if (pos.first == Codira::Position{0, 0, 0}) {
        auto block = store->GetParentBlock();
        return CommonFunc::GetCodePosition(block->GetTerminator());
    }
    return pos;
}

void DataflowRuleGCHK01Check::CheckStoreInUsers(CHIR::Store* store, std::set<std::string>& taintedVars)
{
    auto localVar = DynamicCast<CHIR::LocalVar*>(store->GetLocation());
    if (localVar && localVar->IsRetValue()) {
        auto storeLoc = GetCorrectCodeLoc(store);
        diagEngine->Diagnose(storeLoc.first, storeLoc.second, CodeCheckDiagKind::G_CHK_01_illegal_return);
    } else {
        auto identifier = store->GetLocation()->GetSrcCodeIdentifier();
        taintedVars.insert(identifier);
        auto blockSet = GetDecontaminatBlock(store->GetLocation());
        varInblockWithoutTainted[identifier] = blockSet;
    }
}

void DataflowRuleGCHK01Check::CheckApplyUser(CHIR::Expression* user, CHIR::Value* result)
{
    if (user->GetExprKind() == CHIR::ExprKind::APPLY) {
        auto newApply = StaticCast<CHIR::Apply*>(user);
        CheckApplyInUsers<CHIR::Apply>(newApply, result);
        return;
    }
    if (user->GetExprKind() == CHIR::ExprKind::APPLY_WITH_EXCEPTION) {
        auto newApply = StaticCast<CHIR::ApplyWithException*>(user);
        CheckApplyInUsers<CHIR::ApplyWithException>(newApply, result);
        return;
    }
    if (user->GetExprKind() == CHIR::ExprKind::INVOKE) {
        auto invoke = StaticCast<CHIR::Invoke*>(user);
        CheckInvokeInUsers<CHIR::Invoke>(invoke, result);
        return;
    }
    if (user->GetExprKind() == CHIR::ExprKind::INVOKESTATIC) {
        auto invoke = StaticCast<CHIR::InvokeStatic*>(user);
        CheckInvokeInUsers<CHIR::InvokeStatic>(invoke, result);
        return;
    }
    if (user->GetExprKind() == CHIR::ExprKind::INVOKE_WITH_EXCEPTION) {
        auto invoke = StaticCast<CHIR::InvokeWithException*>(user);
        CheckInvokeInUsers<CHIR::InvokeWithException>(invoke, result);
        return;
    }
}

template <typename T> void DataflowRuleGCHK01Check::CheckApplyUsers(T* apply, std::set<std::string>& taintedVars)
{
    auto users = apply->GetResult()->GetUsers();
    for (auto user : users) {
        if (user->GetExprKind() == CHIR::ExprKind::STORE) {
            auto store = StaticCast<CHIR::Store*>(user);
            CheckStoreInUsers(store, taintedVars);
            continue;
        }
        // Let-type Warning
        CheckApplyUser(user, apply->GetResult());
    }
}

template <typename T> void DataflowRuleGCHK01Check::CheckApply(T* apply, std::set<std::string>& taintedVars)
{
    for (auto taintedFunc : taintSource) {
        if (CommonFunc::FindCHIRFunction(apply->GetCallee(), taintedFunc.first) && apply->GetResult()) {
            auto loc = CommonFunc::GetCodePosition(apply);
            auto identifier = apply->GetResult()->GetIdentifier();
            taintedVars.insert(identifier);
            auto blockSet = GetDecontaminatBlock(apply->GetResult());
            varInblockWithoutTainted[identifier] = blockSet;
            CheckApplyUsers(apply, taintedVars);
            return;
        }
    }
    for (auto taintedFunc : taintPropagate) {
        if (CommonFunc::FindCHIRFunction(apply->GetCallee(), taintedFunc.first)) {
            auto args = apply->GetArgs();
            auto arg = std::find_if(args.begin(), args.end(), [&taintedVars](auto& arg) {
                auto identifier = GetArgName(arg);
                return taintedVars.count(identifier) > 0;
            });
            if (arg != args.end()) {
                auto result = taintedFunc.second.dst == RET_VAL ? apply->GetResult() : args[taintedFunc.second.dst - 1];
                auto identifier = GetArgName(result);
                taintedVars.insert(identifier);
                auto blockSet = GetDecontaminatBlock(result);
                varInblockWithoutTainted[identifier] = blockSet;
                auto users = result->GetUsers();
                auto user = std::find_if(users.begin(), users.end(),
                    [](auto& user) { return user->GetExprKind() == CHIR::ExprKind::STORE; });
                if (user != users.end()) {
                    auto store = StaticCast<CHIR::Store*>(*user);
                    CheckStoreInUsers(store, taintedVars);
                }
                return;
            }
        }
    }
}

void DataflowRuleGCHK01Check::CheckExpr(
    CHIR::Value* value, CHIR::LocalVar* result, std::set<std::string>& taintedVars)
{
    auto identifier = value->GetSrcCodeIdentifier().empty() ? value->GetIdentifier() : value->GetSrcCodeIdentifier();
    if (taintedVars.count(identifier) == 0) {
        return;
    }
    taintedVars.insert(result->GetIdentifier());
    auto users = result->GetUsers();
    for (auto user : users) {
        if (user->GetExprKind() != CHIR::ExprKind::STORE) {
            CheckApplyUser(user, result);
            continue;
        }
        auto store = StaticCast<CHIR::Store*>(user);
        auto storeLoc = store->GetLocation();
        if (auto localVar = DynamicCast<CHIR::LocalVar*>(storeLoc); localVar && localVar->IsRetValue()) {
            auto loc = GetCorrectCodeLoc(store);
            diagEngine->Diagnose(loc.first, loc.second, CodeCheckDiagKind::G_CHK_01_illegal_return);
            continue;
        }
        if (store->GetLocation() == result) {
            taintedVars.erase(identifier);
        } else {
            auto newVar = store->GetLocation()->GetSrcCodeIdentifier();
            taintedVars.insert(newVar);
            auto blockSet = GetDecontaminatBlock(store->GetLocation());
            varInblockWithoutTainted[newVar] = blockSet;
        }
    }
}

void DataflowRuleGCHK01Check::CheckStore(CHIR::Store* store, std::set<std::string>& taintedVars)
{
    auto value = store->GetValue();
    auto identifier = GetArgName(value);
    if (taintedVars.count(identifier) > 0) {
        auto storeLoc = store->GetLocation();
        auto localVar = DynamicCast<CHIR::LocalVar*>(storeLoc);
        if (localVar && localVar->IsRetValue()) {
            auto loc = GetCorrectCodeLoc(store);
            diagEngine->Diagnose(loc.first, loc.second, CodeCheckDiagKind::G_CHK_01_illegal_return);
        }
        taintedVars.insert(GetArgName(storeLoc));
    }
}

/**
 * Added support for tuple type data checking, for example
 * '''
 * var a1: (String,String,String,String) = (getArgs()[0],getArgs()[1],getArgs()[2],getArgs()[3])
 * var a2 = a1[0]
 * return a2
 * '''
 */
void DataflowRuleGCHK01Check::CheckTuple(CHIR::Tuple* tuple, std::set<std::string>& taintedVars)
{
    auto operands = tuple->GetOperands();
    auto iter = std::find_if(operands.begin(), operands.end(), [&taintedVars](auto& operand) {
        auto identifier = GetArgName(operand);
        return taintedVars.count(identifier) > 0;
    });
    if (iter != operands.end()) {
        auto users = tuple->GetResult()->GetUsers();
        if (users.size() == 1 && users[0]->GetExprKind() == CHIR::ExprKind::STORE) {
            auto store = StaticCast<CHIR::Store*>(users[0]);
            CheckStoreInUsers(store, taintedVars);
        }
    }
}

void DataflowRuleGCHK01Check::CheckField(CHIR::Field* field, std::set<std::string>& taintedVars)
{
    auto identifier = GetArgName(field->GetBase());
    if (taintedVars.count(identifier) > 0) {
        taintedVars.insert(GetArgName(field->GetResult()));
    }
}

void DataflowRuleGCHK01Check::CheckFuncBody(CHIR::Block& entryBlock)
{
    std::unordered_set<Block*> visited;
    std::function<void(CHIR::Block*, std::set<std::string>)> visitBlock =
        [this, &visitBlock, &visited](CHIR::Block* block, std::set<std::string> taintedVars) {
            if (visited.find(block) != visited.end()) {
                return;
            }
            visited.emplace(block);
            for (auto expr : block->GetExpressions()) {
                if (expr->GetExprKind() == CHIR::ExprKind::LAMBDA) {
                    auto lambda = StaticCast<CHIR::Lambda*>(expr);
                    CheckFuncBody(*lambda->GetEntryBlock());
                    continue;
                }
                if (expr->GetExprKind() == CHIR::ExprKind::APPLY) {
                    auto apply = StaticCast<CHIR::Apply*>(expr);
                    CheckApply<CHIR::Apply>(apply, taintedVars);
                    continue;
                }
                if (expr->GetExprKind() == CHIR::ExprKind::LOAD) {
                    auto load = StaticCast<CHIR::Load*>(expr);
                    CheckExpr(load->GetLocation(), load->GetResult(), taintedVars);
                    continue;
                }
                if (expr->GetExprKind() == CHIR::ExprKind::STORE) {
                    auto load = StaticCast<CHIR::Store*>(expr);
                    CheckStore(load, taintedVars);
                    continue;
                }
                if (expr->GetExprKind() == CHIR::ExprKind::TYPECAST) {
                    auto typeCast = StaticCast<CHIR::TypeCast*>(expr);
                    CheckExpr(typeCast->GetSourceValue(), typeCast->GetResult(), taintedVars);
                    continue;
                }
                if (expr->GetExprKind() == CHIR::ExprKind::BOX) {
                    auto box = StaticCast<CHIR::Box*>(expr);
                    CheckExpr(box->GetSourceValue(), box->GetResult(), taintedVars);
                    continue;
                }
                if (expr->GetExprKind() == CHIR::ExprKind::TUPLE) {
                    auto tuple = StaticCast<CHIR::Tuple*>(expr);
                    CheckTuple(tuple, taintedVars);
                    continue;
                }
                if (expr->GetExprKind() == CHIR::ExprKind::FIELD) {
                    auto field = StaticCast<CHIR::Field*>(expr);
                    CheckField(field, taintedVars);
                    continue;
                }
                if (expr->IsTerminator()) {
                    auto term = StaticCast<CHIR::Terminator*>(expr);
                    if (expr->GetExprKind() == CHIR::ExprKind::APPLY_WITH_EXCEPTION) {
                        auto applyWithException = StaticCast<CHIR::ApplyWithException*>(expr);
                        CheckApply<CHIR::ApplyWithException>(applyWithException, taintedVars);
                    }
                    for (auto newblock : term->GetSuccessors()) {
                        visitBlock(newblock, taintedVars);
                    }
                }
            }
        };
    std::set<std::string> taintedVars = perilousGlobalVars;
    visitBlock(&entryBlock, taintedVars);
}

void DataflowRuleGCHK01Check::CheckBlock(CHIR::Block* block)
{
    if (!block) {
        return;
    }
    for (auto expr : block->GetExpressions()) {
        if (expr->GetExprKind() == CHIR::ExprKind::APPLY) {
            auto apply = StaticCast<CHIR::Apply*>(expr);
            CheckApply<CHIR::Apply>(apply, perilousGlobalVars);
        }
    }
}

void DataflowRuleGCHK01Check::GetPerilousGlobalVar(CHIR::GlobalVar* globalVar)
{
    auto func = globalVar->GetInitFunc();
    if (func) {
        CheckBlock(func->GetEntryBlock());
    }
}

void DataflowRuleGCHK01Check::CheckBasedOnCHIR(CHIR::Package& package)
{
    auto globalVars = package.GetGlobalVars();
    for (auto var : globalVars) {
        GetPerilousGlobalVar(var);
    }
    auto funcs = package.GetGlobalFuncs();
    for (auto& func : funcs) {
        if (func->TestAttr(CHIR::Attribute::IMPORTED)) {
            continue;
        }
        auto identifier = func->GetIdentifier();
        // Check whether the function is the initialization function of global variables.
        // If yes, skip this step.
        // 5 means the length of the string "@_CGV$"
        if (identifier.size() > 5 && identifier.compare(0, 5, "@_CGV")) {
            CheckFuncBody(*func->GetEntryBlock());
        }
    }
}

} // namespace Codira::CodeCheck
