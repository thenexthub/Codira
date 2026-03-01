/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "debug_data_builder.h"

#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/DIBuilder.h>

namespace ark::llvmbackend {

using llvm::DIBuilder;
using llvm::DICompileUnit;
using llvm::DILocation;
using llvm::DISubprogram;
using llvm::Function;
using llvm::Instruction;

DebugDataBuilder::DebugDataBuilder(llvm::Module *module, const std::string &filename) : builder_(new DIBuilder(*module))
{
    auto file = builder_->createFile(filename, "/");
    builder_->createCompileUnit(llvm::dwarf::DW_LANG_C_plus_plus_14, file, "ark-llvm-backend", true, "", 0, "",
                                DICompileUnit::DebugEmissionKind::NoDebug, 0, true, false,
                                DICompileUnit::DebugNameTableKind::None);
}

void DebugDataBuilder::BeginSubprogram(Function *function, const std::string &filename, uint32_t line)
{
    ASSERT(builder_ != nullptr);
    auto file = builder_->createFile(filename, "/");
    auto flags = DISubprogram::SPFlagDefinition | DISubprogram::SPFlagOptimized;
    auto type = builder_->createSubroutineType(builder_->getOrCreateTypeArray(llvm::None));
    auto sp = builder_->createFunction(file, function->getName(), function->getName(), file, line, type, line,
                                       llvm::DINode::FlagZero, flags);
    function->setSubprogram(sp);
}

void DebugDataBuilder::EndSubprogram(Function *function)
{
    ASSERT(builder_ != nullptr);
    builder_->finalizeSubprogram(function->getSubprogram());
}

void DebugDataBuilder::SetLocation(Instruction *inst, uint32_t line, uint32_t column)
{
    auto func = inst->getFunction();
    inst->setDebugLoc(DILocation::get(func->getContext(), line, column, func->getSubprogram()));
}

void DebugDataBuilder::AppendInlinedAt(llvm::Instruction *inst, llvm::Function *function, uint32_t line,
                                       uint32_t column)
{
    auto original = inst->getDebugLoc();

    auto inlinedAtNode = DILocation::getDistinct(function->getContext(), line, column, original.getScope());
    llvm::DenseMap<const llvm::MDNode *, llvm::MDNode *> cache;
    auto inlinedAt = llvm::DebugLoc::appendInlinedAt(original, inlinedAtNode, function->getContext(), cache);
    auto newDebugInfo = DILocation::getDistinct(function->getContext(), original.getLine(), original->getColumn(),
                                                original.getScope(), inlinedAt);
    inst->setDebugLoc(newDebugInfo);
}

void DebugDataBuilder::Finalize()
{
    builder_->finalize();
    delete builder_;
    builder_ = nullptr;
}

DebugDataBuilder::~DebugDataBuilder()
{
    delete builder_;
}

}  // namespace ark::llvmbackend
