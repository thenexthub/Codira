/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef LIBABCKIT_FUNCTION_UTIL_H
#define LIBABCKIT_FUNCTION_UTIL_H

#include <string>

#include "static_core/assembler/assembly-function.h"
#include "static_core/assembler/assembly-program.h"

#include "libabckit/src/metadata_inspect_impl.h"
namespace abckit::util {

std::string ReplaceFunctionModuleName(const std::string &oldModuleName, const std::string &newName);
bool UpdateFunctionTableKey(ark::pandasm::Program *prog, ark::pandasm::Function *impl, const std::string &newName,
                            std::string &oldMangleName, std::string &newMangleName, AbckitArktsFunction *arktsFunc);
bool UpdateFileMap(AbckitFile *file, const std::string &oldMangleName, const std::string &newMangleName);
void ReplaceInstructionIds(ark::pandasm::Program *prog, ark::pandasm::Function *impl, const std::string &oldId,
                           const std::string &newId);
std::string GenerateFunctionMangleName(const std::string &moduleName, const ark::pandasm::Function &func);
std::string GetTypeName(const AbckitType *type, bool useComponentFormat = false);
void AddFunctionParameterImpl(AbckitCoreFunction *coreFunc, ark::pandasm::Function *impl,
                              const AbckitCoreFunctionParam *paramCore, const std::string &componentName);

bool RemoveFunctionParameterByIndexImpl(AbckitCoreFunction *coreFunc, ark::pandasm::Function *impl, size_t index);

}  // namespace abckit::util

#endif  // LIBABCKIT_FUNCTION_UTIL_H