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
 * This file declares the CompileStrategy related classes, which provide real compile ablility.
 */

#ifndef CODIRA_FRONTEND_COMPILESTRATEGY_H
#define CODIRA_FRONTEND_COMPILESTRATEGY_H

#include <future>
#include <memory>
#include <queue>
#include <unordered_set>

namespace Codira {
class CompilerInstance;
class TypeChecker;

/**
 * Strategy type.
 */
enum class StrategyType {
    DEFAULT,             /**< Default compile strategy. */
    FULL_COMPILE,        /**< Full compile strategy. */
    INCREMENTAL_COMPILE, /**< Increamental compile strategy. */
};

/**
 * Compile Strategy use different compilation unit to compile.
 */
class CompileStrategy {
public:
    explicit CompileStrategy(CompilerInstance* ci) : ci(ci)
    {
    }
    virtual ~CompileStrategy()
    {
    }
    virtual bool Parse() = 0;
    bool ConditionCompile() const;
    void DesugarAfterSema() const;
    bool ImportPackages() const;
    bool MacroExpand() const;
    virtual bool Sema() = 0;
    bool OverflowStrategy() const;
    StrategyType type{StrategyType::DEFAULT};

protected:
    /**
     * Desugar Syntactic sugar.
     */
    void PerformDesugar() const;
    /**
     * Do TypeCheck and Generic Instantiation.
     */
    void TypeCheck() const;
    CompilerInstance* ci = nullptr;
    // A collection of file ids, used to determine whether the id is conflicted.
    std::unordered_set<unsigned int> fileIds;

private:
    /**
     * @brief Do merge custom annotations from '.code.d' file to ast tree.
     * @details If the coded file is in the same directory as the codeo file and the file name is the same as it,
     *          this function will do parse and macroexpand for it.
     * @note This function will modify the ast tree in compiler instance.
     */
    void ParseAndMergeCodeds() const;
};

/**
 * Full Compile Strategy will compile the whole module.
 */
class FullCompileStrategy : public CompileStrategy {
public:
    explicit FullCompileStrategy(CompilerInstance* ci);
    ~FullCompileStrategy();
    bool Parse() override;
    bool Sema() override;

private:
    friend class FullCompileStrategyImpl;
    class FullCompileStrategyImpl* impl;
};
} // namespace Codira
#endif // CODIRA_FRONTEND_COMPILESTRATEGY_H
