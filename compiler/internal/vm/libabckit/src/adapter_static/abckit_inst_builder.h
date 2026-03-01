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

#ifndef ABCKIT_INST_BUILDER_H
#define ABCKIT_INST_BUILDER_H

namespace ark::compiler {

class Graph;

class AbcKitInstBuilder : public InstBuilder {
public:
    AbcKitInstBuilder(Graph *graph, RuntimeInterface::MethodPtr method, CallInst *callerInst, uint32_t inliningDepth)
        : InstBuilder(graph, method, callerInst, inliningDepth)
    {
    }

    NO_COPY_SEMANTIC(AbcKitInstBuilder);
    NO_MOVE_SEMANTIC(AbcKitInstBuilder);

    template <bool IS_ACC_WRITE>
    void AbcKitBuildLoadObject(const BytecodeInstruction *bcInst, DataType::Type type);
    template <bool IS_ACC_READ>
    void AbcKitBuildStoreObject(const BytecodeInstruction *bcInst, DataType::Type type);
    template <Opcode OPCODE>
    void AbcKitBuildLoadFromPool(const BytecodeInstruction *bcInst);
    void BuildDefaultAbcKitIntrinsic(const BytecodeInstruction *bcInst, RuntimeInterface::IntrinsicId intrinsicId);

private:
    void BuildNullcheck(const BytecodeInstruction *bcInst) override;
    void BuildIstrue(const BytecodeInstruction *bcInst) override;
    void BuildTypeof(const BytecodeInstruction *bcInst) override;
    void BuildIsNullValue(const BytecodeInstruction *bcInst) override;
    void BuildLoadStatic(const BytecodeInstruction *bcInst, DataType::Type type) override;
    void BuildStoreStatic(const BytecodeInstruction *bcInst, DataType::Type type) override;
    void BuildLoadArray(const BytecodeInstruction *bcInst, DataType::Type type) override;
    void BuildLoadConstArray(const BytecodeInstruction *bcInst) override;
    void BuildStoreArray(const BytecodeInstruction *bcInst, DataType::Type type) override;
    void BuildNewArray(const BytecodeInstruction *bcInst) override;
    void BuildNewObject(const BytecodeInstruction *bcInst) override;
    void BuildInitObject(const BytecodeInstruction *bcInst, bool isRange) override;
    void BuildCheckCast(const BytecodeInstruction *bcInst) override;
    void BuildIsInstance(const BytecodeInstruction *bcInst) override;
    void BuildThrow(const BytecodeInstruction *bcInst) override;
    void BuildAbcKitInitObjectIntrinsic(const BytecodeInstruction *bcInst, bool isRange = false);
    template <bool IS_ACC_WRITE>
    void BuildAbcKitLoadObjectIntrinsic(const BytecodeInstruction *bcInst, DataType::Type rawType);
    template <bool IS_ACC_READ>
    void BuildAbcKitStoreObjectIntrinsic(const BytecodeInstruction *bcInst, DataType::Type rawType);
    void BuildAbcKitLoadStaticIntrinsic(const BytecodeInstruction *bcInst, DataType::Type rawType);
    void BuildAbcKitStoreStaticIntrinsic(const BytecodeInstruction *bcInst, DataType::Type rawType);
    void BuildAbcKitLoadArrayIntrinsic(const BytecodeInstruction *bcInst, DataType::Type type);
    void BuildAbcKitStoreArrayIntrinsic(const BytecodeInstruction *bcInst, DataType::Type type);
};

};  // namespace ark::compiler

#endif  // ABCKIT_INST_BUILDER_H
