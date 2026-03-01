/*
 * Copyright (C) 2023 Midokura Japan KK.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://toolchain.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "vm/core/ExecutionEngine/Orc/CompileUtils.h"
#include "vm/core/ExecutionEngine/Orc/LLJIT.h"
#include "vm/core/IR/LegacyPassManager.h"
#include "vm/core/Object/ObjectFile.h"
#include "vm/core/Support/SmallVectorMemoryBuffer.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"

#include "aot_orc_extra.h"
#include "bh_log.h"

typedef void (*cb_t)(void *, const char *, size_t, size_t);

class MyCompiler : public toolchain::orc::IRCompileLayer::IRCompiler
{
  public:
    MyCompiler(toolchain::orc::JITTargetMachineBuilder JTMB, cb_t cb, void *cb_data);
    toolchain::Expected<toolchain::orc::SimpleCompiler::CompileResult> operator()(
        toolchain::Module &M) override;

  private:
    toolchain::orc::JITTargetMachineBuilder JTMB;

    cb_t cb;
    void *cb_data;
};

MyCompiler::MyCompiler(toolchain::orc::JITTargetMachineBuilder JTMB, cb_t cb,
                       void *cb_data)
  : IRCompiler(toolchain::orc::irManglingOptionsFromTargetOptions(JTMB.getOptions()))
  , JTMB(std::move(JTMB))
  , cb(cb)
  , cb_data(cb_data)
{}

class PrintStackSizes : public toolchain::MachineFunctionPass
{
  public:
    PrintStackSizes(cb_t cb, void *cb_data);
    bool runOnMachineFunction(toolchain::MachineFunction &MF) override;
    static char ID;

  private:
    cb_t cb;
    void *cb_data;
};

PrintStackSizes::PrintStackSizes(cb_t cb, void *cb_data)
  : MachineFunctionPass(ID)
  , cb(cb)
  , cb_data(cb_data)
{}

char PrintStackSizes::ID = 0;

bool
PrintStackSizes::runOnMachineFunction(toolchain::MachineFunction &MF)
{
    auto name = MF.getName();
    auto MFI = &MF.getFrameInfo();
    size_t sz = MFI->getStackSize();
    cb(cb_data, name.data(), name.size(), sz);
    return false;
}

class MyPassManager : public toolchain::legacy::PassManager
{
  public:
    void add(toolchain::Pass *P) override;
};

void
MyPassManager::add(toolchain::Pass *P)
{
    // a hack to avoid having a copy of the whole addPassesToEmitMC.
    // we want to add PrintStackSizes before FreeMachineFunctionPass.
    if (P->getPassName() == "Free MachineFunction") {
        return;
    }
    toolchain::legacy::PassManager::add(P);
}

// a modified copy from toolchain/lib/ExecutionEngine/Orc/CompileUtils.cpp
toolchain::Expected<toolchain::orc::SimpleCompiler::CompileResult>
MyCompiler::operator()(toolchain::Module &M)
{
    auto TM = cantFail(JTMB.createTargetMachine());
    toolchain::SmallVector<char, 0> ObjBufferSV;

    {
        toolchain::raw_svector_ostream ObjStream(ObjBufferSV);

        MyPassManager PM;
        toolchain::MCContext *Ctx;
        if (TM->addPassesToEmitMC(PM, Ctx, ObjStream))
            return toolchain::make_error<toolchain::StringError>(
                "Target does not support MC emission",
                toolchain::inconvertibleErrorCode());
        PM.add(new PrintStackSizes(cb, cb_data));
        dynamic_cast<toolchain::legacy::PassManager *>(&PM)->add(
            toolchain::createFreeMachineFunctionPass());
        PM.run(M);
    }

#if LLVM_VERSION_MAJOR > 13
    auto ObjBuffer = std::make_unique<toolchain::SmallVectorMemoryBuffer>(
        std::move(ObjBufferSV),
        M.getModuleIdentifier() + "-jitted-objectbuffer",
        /*RequiresNullTerminator=*/false);
#else
    auto ObjBuffer = std::make_unique<toolchain::SmallVectorMemoryBuffer>(
        std::move(ObjBufferSV),
        M.getModuleIdentifier() + "-jitted-objectbuffer");
#endif

    return std::move(ObjBuffer);
}

DEFINE_SIMPLE_CONVERSION_FUNCTIONS(toolchain::orc::LLLazyJITBuilder,
                                   LLVMOrcLLLazyJITBuilderRef)

void
LLVMOrcLLJITBuilderSetCompileFunctionCreatorWithStackSizesCallback(
    LLVMOrcLLLazyJITBuilderRef Builder,
    void (*cb)(void *, const char *, size_t, size_t), void *cb_data)
{
    auto b = unwrap(Builder);
    b->setCompileFunctionCreator(
        [cb, cb_data](toolchain::orc::JITTargetMachineBuilder JTMB)
            -> toolchain::Expected<
                std::unique_ptr<toolchain::orc::IRCompileLayer::IRCompiler>> {
            return std::make_unique<MyCompiler>(
                MyCompiler(std::move(JTMB), cb, cb_data));
        });
}
