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

#ifndef CONST_FOLDING_H
#define CONST_FOLDING_H

namespace ark::compiler {

bool ConstFoldingCast(Inst *inst);
bool ConstFoldingNeg(Inst *inst);
bool ConstFoldingAbs(Inst *inst);
bool ConstFoldingNot(Inst *inst);
bool ConstFoldingAdd(Inst *inst);
bool ConstFoldingSub(Inst *inst);
bool ConstFoldingMul(Inst *inst);
bool ConstFoldingBinaryMathWithNan(Inst *inst);
bool ConstFoldingDiv(Inst *inst);
bool ConstFoldingDivWithNan(Inst *inst);
bool ConstFoldingMin(Inst *inst);
bool ConstFoldingMax(Inst *inst);
bool ConstFoldingMod(Inst *inst);
bool ConstFoldingShl(Inst *inst);
bool ConstFoldingShr(Inst *inst);
bool ConstFoldingAShr(Inst *inst);
bool ConstFoldingAnd(Inst *inst);
bool ConstFoldingOr(Inst *inst);
bool ConstFoldingXor(Inst *inst);
bool ConstFoldingCmp(Inst *inst);
bool ConstFoldingCompare(Inst *inst);
bool ConstFoldingSqrt(Inst *inst);
bool ConstFoldingLoadStatic(Inst *inst);

// NB: casting may be required to create constants of the desired array type for creating LiteralArrays (in Bytecode
// Optimizer ConstArrayResolver pass) If a constant is used to initialize an array (is_literal_data == true), it must be
// able to be cast to the appropriate types
PANDA_PUBLIC_API ConstantInst *ConstFoldingCastConst(Inst *inst, Inst *input, bool isLiteralData = false);

ConstantInst *ConstFoldingCreateIntConst(Inst *inst, uint64_t value, bool isLiteralData = false);
}  // namespace ark::compiler
#endif  // CONST_FOLDING_H
