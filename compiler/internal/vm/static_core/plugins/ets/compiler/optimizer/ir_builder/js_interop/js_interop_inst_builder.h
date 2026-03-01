/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_COMPILER_OPTIMIZER_IR_BUILDER_JS_INTEROP_JS_INTEROP_INST_BUILDER_H
#define PANDA_PLUGINS_ETS_COMPILER_OPTIMIZER_IR_BUILDER_JS_INTEROP_JS_INTEROP_INST_BUILDER_H

template <RuntimeInterface::IntrinsicId ID, DataType::Type RET_TYPE, DataType::Type... PARAM_TYPES>
friend struct IntrinsicBuilder;

template <size_t N>
IntrinsicInst *BuildInteropIntrinsic(size_t pc, RuntimeInterface::IntrinsicId id, DataType::Type retType,
                                     const std::array<DataType::Type, N> &types,
                                     const std::array<Inst *, N + 1> &inputs);
Inst *BuildInitJSCallClass(RuntimeInterface::MethodPtr method, size_t pc, SaveStateInst *saveState);
// CC-OFFNXT(G.NAM.01) namespace identifier
std::pair<Inst *, Inst *> BuildResolveInteropCallIntrinsic(RuntimeInterface::InteropCallKind callKind, size_t pc,
                                                           RuntimeInterface::MethodPtr method, Inst *arg0, Inst *arg1,
                                                           Inst *arg2, Inst *cpOffsetForClass,
                                                           SaveStateInst *saveState);
IntrinsicInst *CreateInteropCallIntrinsic(size_t pc, RuntimeInterface::InteropCallKind callKind);
void BuildReturnValueConvertInteropIntrinsic(compiler::RuntimeInterface::InteropCallKind callKind, size_t pc,
                                             compiler::RuntimeInterface::MethodPtr method, Inst *jsCall,
                                             SaveStateInst *saveState);
void BuildInteropCall(const BytecodeInstruction *bcInst, compiler::RuntimeInterface::InteropCallKind callKind,
                      compiler::RuntimeInterface::MethodPtr method, bool isRange, bool accRead);
bool TryBuildInteropCall(const BytecodeInstruction *bcInst, bool isRange, bool accRead);
#endif  // PANDA_PLUGINS_ETS_COMPILER_OPTIMIZER_IR_BUILDER_JS_INTEROP_JS_INTEROP_INST_BUILDER_H
