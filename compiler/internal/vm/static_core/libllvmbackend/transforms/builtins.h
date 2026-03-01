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

#ifndef LIBLLVMBACKEND_TRANSFORMS_BUILTINS_H
#define LIBLLVMBACKEND_TRANSFORMS_BUILTINS_H

#include <llvm/ADT/Triple.h>
#include <llvm/IR/IRBuilder.h>

namespace ark::llvmbackend {
class LLVMArkInterface;
}  // namespace ark::llvmbackend

namespace ark::llvmbackend::builtins {
llvm::Function *LenArray(llvm::Module *module);
llvm::Function *LoadClass(llvm::Module *module);
llvm::Function *LoadInitClass(llvm::Module *module);
llvm::Function *PreWRB(llvm::Module *module, unsigned addrSpace);
llvm::Function *PostWRB(llvm::Module *module, unsigned addrSpace);
llvm::Function *LoadString(llvm::Module *module);
llvm::Function *ResolveVirtual(llvm::Module *module);
llvm::Function *BarrierReturnVoid(llvm::Module *module);
llvm::Function *KeepThis(llvm::Module *module);
llvm::Value *LowerBuiltin(llvm::IRBuilder<> *builder, llvm::CallInst *inst,
                          ark::llvmbackend::LLVMArkInterface *arkInterface);
constexpr auto BUILTIN_SECTION = ".builtins";
constexpr auto LEN_ARRAY_BUILTIN = "__builtin_lenarray";
constexpr auto KEEP_THIS_BUILTIN = "__builtin_keep_this";
constexpr auto LOAD_CLASS_BUILTIN = "__builtin_load_class";
constexpr auto LOAD_INIT_CLASS_BUILTIN = "__builtin_load_init_class";
constexpr auto PRE_WRB_BUILTIN = "__builtin_pre_wrb";
constexpr auto PRE_WRB_GCADR_BUILTIN = "__builtin_pre_wrb_gcadr";
constexpr auto POST_WRB_BUILTIN = "__builtin_post_wrb";
constexpr auto POST_WRB_GCADR_BUILTIN = "__builtin_post_wrb_gcadr";
constexpr auto LOAD_STRING_BUILTIN = "__builtin_load_string";
constexpr auto RESOLVE_VIRTUAL_BUILTIN = "__builtin_resolve_virtual";
constexpr auto BARRIER_RETURN_VOID_BUILTIN = "__builtin_barrier_return_void";
}  // namespace ark::llvmbackend::builtins

#endif  // LIBLLVMBACKEND_TRANSFORMS_BUILTINS_H
