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

LLVMArkInterface::IntrinsicId LLVMArkInterface::GetEtsIntrinsicId(const llvm::Instruction *inst) const
{
    if (!llvm::isa<llvm::IntrinsicInst>(inst)) {
        return NO_INTRINSIC_ID_CONTINUE;
    }
    int64_t srcLang = -1;
    [[maybe_unused]] bool error = inst->getFunction()
        ->getFnAttribute(ark::llvmbackend::LLVMArkInterface::SOURCE_LANG_ATTR)
        .getValueAsString().getAsInteger(0, srcLang);
    ASSERT(!error);
    ASSERT(srcLang >= 0);
    if (static_cast<ark::SourceLanguage>(srcLang) != ark::SourceLanguage::ETS) {
        return NO_INTRINSIC_ID_CONTINUE;
    }

    auto type = inst->getType();
    [[maybe_unused]] auto etype = !type->isVectorTy() ? type : llvm::cast<llvm::VectorType>(type)->getElementType();
    switch (llvm::cast<llvm::IntrinsicInst>(inst)->getIntrinsicID()) {
        case llvm::Intrinsic::ceil: {
            ASSERT(etype->isFloatTy() || etype->isDoubleTy());
            if (X86NoSSE(triple_.getArch())) {
                ASSERT(etype->isDoubleTy());
                return static_cast<IntrinsicId>(RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_CEIL);
            }
            return NO_INTRINSIC_ID;
        }
        case llvm::Intrinsic::floor: {
            ASSERT(etype->isFloatTy() || etype->isDoubleTy());
            if (X86NoSSE(triple_.getArch())) {
                ASSERT(etype->isDoubleTy());
                return static_cast<IntrinsicId>(RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_FLOOR);
            }
            return NO_INTRINSIC_ID;
        }
        case llvm::Intrinsic::round: {
            ASSERT(etype->isFloatTy() || etype->isDoubleTy());
            if (X86NoSSE(triple_.getArch())) {
                ASSERT(etype->isDoubleTy());
                return static_cast<IntrinsicId>(RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_ROUND);
            }
            return NO_INTRINSIC_ID;
        }
        case llvm::Intrinsic::log: {
            ASSERT(etype->isDoubleTy());
            EVENT_PAOC("Lowering @llvm.log");
            return static_cast<IntrinsicId>(RuntimeInterface::IntrinsicId::INTRINSIC_STD_MATH_LOG);
        }
        default:
            break;
    }
    return NO_INTRINSIC_ID_CONTINUE;
}
