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
 * This file declares the constant evaluation pass using the interpreter
 */

#ifndef CODIRA_CHIR_INTERP_CONST_EVAL_H
#define CODIRA_CHIR_INTERP_CONST_EVAL_H

#include <iostream>

#include <codira/CHIR/CHIRBuilder.h>
#include <codira/CHIR/Expression/Terminator.h>
#include <codira/CHIR/Interpreter/BCHIR.h>
#include <codira/CHIR/Interpreter/BCHIRInterpreter.h>
#include <codira/CHIR/Interpreter/BCHIRLinker.h>
#include <codira/CHIR/Interpreter/InterpreterValueUtils.h>
#include <codira/CHIR/Package.h>
#include <codira/Frontend/CompilerInstance.h>
#include <codira/Mangle/BaseMangler.h>
#include <codira/Option/Option.h>

namespace Codira::CHIR::Interpreter {

class IVal2CHIR {
public:
    IVal2CHIR(CHIRBuilder& chirBuilder, const Bchir& bchir, const Package& package)
        : chirBuilder(chirBuilder), bchir(bchir), package(package)
    {
    }
    IVal2CHIR(const IVal2CHIR&) = delete;
    IVal2CHIR& operator=(const IVal2CHIR&) = delete;

    // Returns a Constant that represents the same value as `val`.
    // Returns nullptr if `val` does not have an equivalent constant.
    Constant* TryConvertToConstant(Type& ty, const IVal& val, Block& parent);
    // Converts `val` to chir that produces an equivalent value.
    // Returns the value for the expression that holds the full value.
    // Returns nullptr if the value cannot be converted because the type is
    // unsupported or if it references a function that has not been imported.
    Value* ConvertToChir(Type& ty, const IVal& val, std::function<void(Expression*)>& insertExpr, Block& parent);

private:
    Value* ConvertStringToChir(
        Type& ty, const ITuple& val, const std::function<void(Expression*)>& insertExpr, Block& parent);
    Value* ConvertTupleToChir(Type& ty, const ITuple& val, std::function<void(Expression*)>& insertExpr, Block& parent);
    Value* ConvertEnumToChir(
        EnumType& ty, const IVal& val, std::function<void(Expression*)>& insertExpr, Block& parent);
    Value* ConvertRefToChir(RefType& ty, const IVal& val, std::function<void(Expression*)>& insertExpr, Block& parent);
    Value* ConvertFuncToChir(const FuncType& ty, const IFunc& val);
    Value* ConvertArrayToChir(
        VArrayType& ty, const IArray& val, std::function<void(Expression*)>& insertExpr, Block& parent);

    ClassType* FindClassType(const std::string& mangledName);

    CHIRBuilder& chirBuilder;
    const Bchir& bchir;
    const Package& package;
};

class ConstEvalPass {
public:
    explicit ConstEvalPass(CompilerInstance& ci, CHIRBuilder& builder, SourceManager& sourceManager,
        const Codira::GlobalOptions& opts, DiagnosticEngine& diag)
        : ci(ci), builder(builder), sourceManager(sourceManager), opts(opts), diag(diag)
    {
    }
    ConstEvalPass(const ConstEvalPass&) = delete;
    ConstEvalPass& operator=(const ConstEvalPass&) = delete;

    // Evaluates constants (variables declared with `const`) and simplifies
    // their intializers.
    void RunOnPackage(Package& package,
        const std::vector<CHIR::FuncBase*>& initFuncsForConstVar, std::vector<Bchir>& bchirPackages);

private:
    void RunInterpreter(Package& package, std::vector<Bchir>& bchirPackages,
        const std::vector<CHIR::FuncBase*>& initFuncsForConstVar,
        std::function<void(Package&, BCHIRInterpreter&, BCHIRLinker&)> onSuccess);
    void ReplaceGlobalConstantInitializers(Package& package, BCHIRInterpreter& interpreter, BCHIRLinker& linker);
    void RemoveConstructorCalls(const Value& value);
    std::optional<BlockGroup*> CreateNewInitializer(
        Func& oldInitializer, const BCHIRInterpreter& interpreter, const BCHIRLinker& linker, const Package& package);

    void PrintDebugMessage(
        const DebugLocation& loc, const Func& oldInit, const std::optional<BlockGroup*>& newInit) const;

    CompilerInstance& ci;
    CHIRBuilder& builder;
    SourceManager& sourceManager;
    const Codira::GlobalOptions& opts;
    DiagnosticEngine& diag;
};

} // namespace Codira::CHIR::Interpreter

#endif
