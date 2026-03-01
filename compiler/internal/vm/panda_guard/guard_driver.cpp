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

#include "guard_driver.h"

#include "abc2program_compiler.h"
#include "assembly-emitter.h"
#include "program_dump.h"
#include "utils/logger.h"

#include "configs/guard_context.h"
#include "guard4program.h"
#include "util/assert_util.h"

namespace {
constexpr std::string_view TAG = "[Guard_Driver]";

void Dump(const panda::pandasm::Program &program, const std::string &paFilePath)
{
    std::ofstream ofs;
    ofs.open(paFilePath, std::ios::trunc | std::ios::out);
    panda::abc2program::PandasmProgramDumper dumper;
    dumper.Dump(ofs, program);
    ofs.close();
}
}  // namespace

void panda::guard::GuardDriver::Run(int argc, const char **argv)
{
    const auto context = GuardContext::GetInstance();
    context->Init(argc, argv);
    LOG(INFO, PANDAGUARD) << TAG << "guard context init success";

    auto options = context->GetGuardOptions();
    if (options->DisableObfuscation()) {
        LOG(INFO, PANDAGUARD) << TAG << "obfuscation is disabled";
        return;
    }

    abc2program::Abc2ProgramCompiler compiler;
    bool status = compiler.OpenAbcFile(options->GetAbcFilePath());
    PANDA_GUARD_ASSERT_PRINT(!status, TAG, ErrorCode::GENERIC_ERROR,
                             "abc to program, open abc file failed" << options->GetAbcFilePath());

    auto program = std::move(*compiler.CompileAbcFile());
    LOG(INFO, PANDAGUARD) << TAG << "abc to program success";

    context->CreateGraphContext(compiler.GetAbcFile());

    ProgramGuard::GuardProgram(program);
    LOG(INFO, PANDAGUARD) << TAG << "guard for program success";

    if (context->IsDebugMode() && !options->GetObfPaFilePath().empty()) {
        Dump(program, options->GetObfPaFilePath());
        LOG(INFO, PANDAGUARD) << TAG << "program to pa success";
    }

    status = pandasm::AsmEmitter::Emit(options->GetObfAbcFilePath(), program, nullptr, nullptr, true, nullptr,
                                       options->GetTargetApiVersion(), options->GetTargetApiSubVersion());
    PANDA_GUARD_ASSERT_PRINT(!status, TAG, ErrorCode::GENERIC_ERROR,
                             "program to abc failed" << pandasm::AsmEmitter::GetLastError());
    LOG(INFO, PANDAGUARD) << TAG << "program to abc success";

    context->Finalize();
}
