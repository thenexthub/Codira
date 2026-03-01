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

#ifndef CODIRA_EMITPACKAGEIR_H
#define CODIRA_EMITPACKAGEIR_H

#include "llvm/IR/Module.h"

#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/CHIR/Package.h"
#include "Codira/Frontend/CompilerInstance.h"
#include "Codira/FrontendTool/IncrementalCompilerInstance.h"
#include "Codira/Option/Option.h"

namespace Codira::CodeGen {
/**
 * @brief This function generates the package modules.
 *        Note that after using llvm::Module, call the ClearPackageModules to clear the memory.
 *
 * @param chirBuilder A CHIRBuilder of CHIR.
 * @param chirData CHIRData of a complete package.
 * @param options GlobalOptions to compile a package.
 * @param compilerInstance DefaultCompilerInstance.
 * @param enableIncrement A falg, indicating whether incremental compilation is enabled.
 * @return A vector of std::unique_ptr<llvm::Module>. If --aggressive-parallel-compile is enabled,
 *         multiple llvm::Modules are returned. Otherwise, only one llvm::Module is returned.
 */
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
std::vector<std::unique_ptr<llvm::Module>> GenPackageModules(CHIR::CHIRBuilder& chirBuilder, const CHIRData& chirData,
    const GlobalOptions& options, DefaultCompilerInstance& compilerInstance, bool enableIncrement);
#endif

/**
 * @brief Save the LLVM module to the specified Bitcode file path
 *
 * @param module A llvm::Module to be cached.
 * @param bcFilePath CHIRData of a complete package
 * @return If the saving is successful, true is returned. Otherwise, false is returned.
 */
bool SavePackageModule(const llvm::Module& module, const std::string& bcFilePath);

/**
 * @brief Clear and release all modules. It ensures that all resources are properly released and cleaned up.
 *
 * @param packageModules A vector of unique pointers to LLVM modules to be cleared.
 */
void ClearPackageModules(std::vector<std::unique_ptr<llvm::Module>>& packageModules);
} // namespace Codira::CodeGen

#endif // CODIRA_EMITPACKAGEIR_H
