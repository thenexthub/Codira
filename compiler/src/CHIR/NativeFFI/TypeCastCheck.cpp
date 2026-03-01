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

#include "TypeCastCheck.h"

#include "Codira/CHIR/Analysis/Utils.h"
#include "Codira/CHIR/CHIRCasting.h"
#include "Codira/CHIR/Expression/Expression.h"
#include "Codira/CHIR/Utils.h"
#include "Codira/Utils/TaskQueue.h"

namespace Codira::CHIR::NativeFFI {
TypeCastCheck::TypeCastCheck(DiagAdapter& diag) : diag(diag)
{
}

void TypeCastCheck::RunOnPackage(const Package& package, size_t threadNum)
{
    std::vector<Func*> funcs;
    for (auto func : package.GetGlobalFuncs()) {
        if (!func->TestAttr(Attribute::UNSAFE)) {
            funcs.emplace_back(func);
        }
    }

    if (funcs.empty()) {
        return;
    }
    if (threadNum == 1) {
        for (auto func : funcs) {
            CODEC_NULLPTR_CHECK(func);
            RunOnFunc(*func);
        }
    } else {
        Utils::TaskQueue taskQueue(threadNum);
        for (auto func : funcs) {
            CODEC_NULLPTR_CHECK(func);
            taskQueue.AddTask<void>([this, func]() { return RunOnFunc(*func); });
        }
        taskQueue.RunAndWaitForAllTasksCompleted();
    }
}

namespace {
bool IsNativeFIIType(Type& type)
{
    auto valueType = type.StripAllRefs();
    if (!valueType->IsClass()) {
        return false;
    }

    auto classType = StaticCast<ClassType>(valueType);
    CODEC_NULLPTR_CHECK(classType);
    auto classDef = classType->GetClassDef();
    CODEC_NULLPTR_CHECK(classDef);
    return classDef->TestAttr(Attribute::JAVA_IMPL) || classDef->TestAttr(Attribute::JAVA_MIRROR);
}

std::string GetClassName(Type& type)
{
    auto valueType = type.StripAllRefs();
    CODEC_NULLPTR_CHECK(valueType);
    CODEC_ASSERT(valueType->IsClass());
    return StaticCast<ClassType>(valueType)->GetClassDef()->GetSrcCodeIdentifier();
}

std::optional<std::tuple<Type*, Type*>> TryExtractTypeCast(Expression& expr)
{
    if (expr.GetExprKind() == ExprKind::TYPECAST) {
        auto& typeCastExpr = StaticCast<TypeCast>(expr);
        return {{typeCastExpr.GetSourceTy(), typeCastExpr.GetTargetTy()}};
    }

    if (expr.GetExprKind() == ExprKind::TYPECAST_WITH_EXCEPTION) {
        auto& typeCastExpr = StaticCast<TypeCastWithException>(expr);
        return {{typeCastExpr.GetSourceTy(), typeCastExpr.GetTargetTy()}};
    }

    return std::nullopt;
}
} // namespace

void TypeCastCheck::RunOnFunc(const Func& func)
{
    std::function<VisitResult(Expression&)> visitor = [this, &func, &visitor](Expression& expr) {
        if (expr.IsLambda()) {
            Visitor::Visit(*StaticCast<Lambda>(expr).GetBody(), visitor);
            return VisitResult::CONTINUE;
        }

        auto typeCast = TryExtractTypeCast(expr);
        if (!typeCast) {
            return VisitResult::CONTINUE;
        }

        auto [sourceTy, targetTy] = *typeCast;
        if (!IsNativeFIIType(*sourceTy)) {
            return VisitResult::CONTINUE;
        }

        if (!IsNativeFIIType(*targetTy)) {
            auto [hasDebugPos, debugPos] = GetDebugPos(expr);
            if (!hasDebugPos) {
                debugPos = ToRange(func.GetDebugLocation());
            }

            CODEC_ASSERT(sourceTy->GetTypeKind());
            auto builder = diag.DiagnoseRefactor(DiagKindRefactor::chir_native_ffi_java_illegal_type_cast, debugPos,
                GetClassName(*sourceTy), GetClassName(*targetTy));
        }

        return VisitResult::CONTINUE;
    };
    Visitor::Visit(func, visitor);
}
} // namespace Codira::CHIR::NativeFFI
