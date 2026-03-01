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

#ifndef LIBLLVMBACKEND_LOWERING_GC_BARRIERS_H
#define LIBLLVMBACKEND_LOWERING_GC_BARRIERS_H

#include <llvm/IR/IRBuilder.h>

namespace ark::llvmbackend {
class LLVMArkInterface;
}  // namespace ark::llvmbackend

namespace ark::llvmbackend::gc_barriers {
void EmitPreWRB(llvm::IRBuilder<> *builder, llvm::Value *mem, bool isVolatileMem, llvm::BasicBlock *outBb,
                LLVMArkInterface *arkInterface, llvm::Value *threadRegValue);

void EmitPostWRB(llvm::IRBuilder<> *builder, llvm::Value *mem, llvm::Value *offset, llvm::Value *value,
                 LLVMArkInterface *arkInterface, llvm::Value *threadRegValue, llvm::Value *frameRegValue);
}  // namespace ark::llvmbackend::gc_barriers

#endif  // LIBLLVMBACKEND_LOWERING_GC_BARRIERS_H
