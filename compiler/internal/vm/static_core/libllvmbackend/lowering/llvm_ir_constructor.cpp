/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include <llvm/IR/Intrinsics.h>
#include "optimizer/code_generator/codegen.h"
#include "intrinsic_string_flat_check.inl"
#include "irtoc/backend/compiler/constants.h"
#include "runtime/include/coretypes/string.h"

#include "llvm_ir_constructor.h"

#include "gc_barriers.h"
#include "irtoc_function_utils.h"
#include "llvm_logger.h"
#include "llvm_options.h"
#include "metadata.h"
#include "utils.h"
#include "transforms/builtins.h"
#include "transforms/gc_utils.h"
#include "transforms/runtime_calls.h"

namespace ark::compiler {
#define ONLY_NEEDSAFEPOINT
#include <intrinsics_ir_build.inl>
#undef ONLY_NEEDSAFEPOINT
}  // namespace ark::compiler

#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/IntrinsicsAArch64.h>
#include <llvm/IR/MDBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

using ark::llvmbackend::DebugDataBuilder;
using ark::llvmbackend::LLVMArkInterface;
using ark::llvmbackend::builtins::BarrierReturnVoid;
using ark::llvmbackend::builtins::KeepThis;
using ark::llvmbackend::builtins::LenArray;
using ark::llvmbackend::builtins::LoadClass;
using ark::llvmbackend::builtins::LoadInitClass;
using ark::llvmbackend::builtins::LoadString;
using ark::llvmbackend::builtins::ResolveVirtual;
using ark::llvmbackend::irtoc_function_utils::IsNoAliasIrtocFunction;
#ifndef NDEBUG
using ark::llvmbackend::irtoc_function_utils::IsPtrIgnIrtocFunction;
#endif
using ark::llvmbackend::utils::CreateLoadClassFromObject;

static constexpr unsigned VECTOR_SIZE_2 = 2;
static constexpr unsigned VECTOR_SIZE_8 = 8;
static constexpr unsigned VECTOR_SIZE_16 = 16;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ASSERT_TYPE(input, expectedType)                                                                   \
    ASSERT_DO((input)->getType() == (expectedType),                                                        \
              std::cerr << "Unexpected data type: " << GetTypeName((input)->getType()) << ". Should be a " \
                        << GetTypeName(expectedType) << "." << std::endl)

// Max integer that can be represented in float/double without losing precision
constexpr float MaxIntAsExactFloat()
{
    return static_cast<float>((1U << static_cast<unsigned>(std::numeric_limits<float>::digits)) - 1);
}

constexpr double MaxIntAsExactDouble()
{
    return static_cast<double>((1ULL << static_cast<unsigned>(std::numeric_limits<double>::digits)) - 1);
}

// arm64: { dispatch: 24, pc: 20, frame: 23, acc: 21, accTag: 22, moffset: 25, methodPtr: 26 },
static constexpr auto AARCH64_PC = 20;
static constexpr auto AARCH64_ACC = 21;
static constexpr auto AARCH64_ACC_TAG = 22;
static constexpr auto AARCH64_FP = 23;
static constexpr auto AARCH64_DISPATCH = 24;
static constexpr auto AARCH64_MOFFSET = 25;
static constexpr auto AARCH64_METHOD_PTR = 26;
static constexpr auto AARCH64_REAL_FP = 29;

// x86_64: { dispatch: 8, pc: 4, frame: 5, acc: 11, accTag: 3 }
static constexpr auto X86_64_PC = 4;       // renamed r10
static constexpr auto X86_64_ACC = 11;     // renamed r3 (rbx)
static constexpr auto X86_64_ACC_TAG = 3;  // renamed r11
static constexpr auto X86_64_FP = 5;       // renamed r9
static constexpr auto X86_64_DISPATCH = 8;
static constexpr auto X86_64_REAL_FP = 9;  // renamed r5 (rbp)

namespace {
inline llvm::Function *CreateFunctionDeclaration(llvm::FunctionType *functionType, const std::string &name,
                                                 llvm::Module *module)
{
    ASSERT(functionType != nullptr);
    ASSERT(!name.empty());
    ASSERT(module != nullptr);

    auto function = module->getFunction(name);
    if (function != nullptr) {
        ASSERT(function->getVisibility() == llvm::GlobalValue::ProtectedVisibility);
        ASSERT(function->doesNotThrow());
        return function;
    }

    function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, name, module);
    function->setDoesNotThrow();
    function->setVisibility(llvm::GlobalValue::ProtectedVisibility);
    function->setSectionPrefix(name);

    return function;
}

inline void CreateBlackBoxAsm(llvm::IRBuilder<> *builder, const std::string &inlineAsm)
{
    auto iasmType = llvm::FunctionType::get(builder->getVoidTy(), {}, false);
    builder->CreateCall(iasmType, llvm::InlineAsm::get(iasmType, inlineAsm, "", true), {});
}

inline void CreateInt32ImmAsm(llvm::IRBuilder<> *builder, const std::string &inlineAsm, uint32_t imm)
{
    auto oneInt = llvm::FunctionType::get(builder->getVoidTy(), {builder->getInt32Ty()}, false);
    builder->CreateCall(oneInt, llvm::InlineAsm::get(oneInt, inlineAsm, "i", true), {builder->getInt32(imm)});
}

inline llvm::AtomicOrdering ToAtomicOrdering(bool isVolatile)
{
    return isVolatile ? LLVMArkInterface::VOLATILE_ORDER : LLVMArkInterface::NOT_ATOMIC_ORDER;
}

#ifndef NDEBUG
inline std::string GetTypeName(llvm::Type *type)
{
    std::string name;
    auto stream = llvm::raw_string_ostream(name);
    type->print(stream);
    return stream.str();
}
#endif
}  // namespace

namespace ark::compiler {

#include <can_compile_intrinsics_gen.inl>

namespace {
class MemCharSimdLowering {
public:
    NO_COPY_SEMANTIC(MemCharSimdLowering);
    NO_MOVE_SEMANTIC(MemCharSimdLowering);
    MemCharSimdLowering() = delete;
    ~MemCharSimdLowering() = default;

    MemCharSimdLowering(llvm::Value *ch, llvm::Value *addr, llvm::IRBuilder<> *builder, llvm::Function *func);

    template <bool MEM_BLOCK_SIZE_256_BITS>
    llvm::Value *Generate(llvm::VectorType *vecTy);

    llvm::VectorType *GetU64X2Ty() const
    {
        return llvm::VectorType::get(builder_->getInt64Ty(), VECTOR_SIZE_2, false);
    }
    llvm::VectorType *GetU16X8Ty() const
    {
        return llvm::VectorType::get(builder_->getInt16Ty(), VECTOR_SIZE_8, false);
    }
    llvm::VectorType *GetU8X16Ty() const
    {
        return llvm::VectorType::get(builder_->getInt8Ty(), VECTOR_SIZE_16, false);
    }

private:
    static const uint64_t UL64 = 64UL;
    static const uint64_t UL128 = 128UL;
    static const uint64_t UL192 = 192UL;

    void GenLoadAndFastCheck128(llvm::VectorType *vecTy);
    void GenLoadAndFastCheck256(llvm::VectorType *vecTy);
    llvm::Value *GenFindChar128(llvm::IntegerType *charTy);
    llvm::Value *GenFindChar256(llvm::IntegerType *charTy);
    static llvm::SmallVector<int> ShuffleMask(llvm::Type *charTy)
    {
        ASSERT(charTy->isIntegerTy(8U) || charTy->isIntegerTy(16U));
        static constexpr std::initializer_list<int> MASK_U8 {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
        static constexpr std::initializer_list<int> MASK_U16 {7, 6, 5, 4, 3, 2, 1, 0};
        return {charTy->isIntegerTy(8U) ? MASK_U8 : MASK_U16};
    }

private:
    llvm::Value *ch_;
    llvm::Value *addr_;
    llvm::IRBuilder<> *builder_;
    llvm::Function *func_;

    llvm::BasicBlock *lastBb_ = nullptr;
    llvm::BasicBlock *foundBb_ = nullptr;
    llvm::BasicBlock *inspectV1D0Bb_ = nullptr;
    llvm::BasicBlock *inspectV1D1Bb_ = nullptr;
    llvm::BasicBlock *inspectV2D0Bb_ = nullptr;
    llvm::BasicBlock *inspectV2D1Bb_ = nullptr;
    llvm::BasicBlock *clzV1D0Bb_ = nullptr;
    llvm::BasicBlock *clzV1D1Bb_ = nullptr;
    llvm::BasicBlock *clzV2D0Bb_ = nullptr;
    llvm::Value *vcmpeq1_ = nullptr;
    llvm::Value *vcmpeq2_ = nullptr;
};

class MemLastCharSimdLowering {
public:
    NO_COPY_SEMANTIC(MemLastCharSimdLowering);
    NO_MOVE_SEMANTIC(MemLastCharSimdLowering);
    MemLastCharSimdLowering() = delete;
    ~MemLastCharSimdLowering() = default;

    MemLastCharSimdLowering(llvm::Value *ch, llvm::Value *addr, llvm::IRBuilder<> *builder, llvm::Function *func);

    llvm::Value *Generate(llvm::VectorType *vecTy);

    llvm::VectorType *GetU64X2Ty() const
    {
        return llvm::VectorType::get(builder_->getInt64Ty(), VECTOR_SIZE_2, false);
    }
    llvm::VectorType *GetU16X8Ty() const
    {
        return llvm::VectorType::get(builder_->getInt16Ty(), VECTOR_SIZE_8, false);
    }
    llvm::VectorType *GetU8X16Ty() const
    {
        return llvm::VectorType::get(builder_->getInt8Ty(), VECTOR_SIZE_16, false);
    }

private:
    void GenLoadAndFastCheck128(llvm::VectorType *vecTy);
    llvm::Value *GenFindChar128(llvm::IntegerType *charTy);
    static llvm::SmallVector<int> ShuffleMask(llvm::Type *charTy)
    {
        ASSERT(charTy->isIntegerTy(8U) || charTy->isIntegerTy(16U));
        static constexpr std::initializer_list<int> MASK_U8 {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
        static constexpr std::initializer_list<int> MASK_U16 {7, 6, 5, 4, 3, 2, 1, 0};
        return {charTy->isIntegerTy(8U) ? MASK_U8 : MASK_U16};
    }

private:
    llvm::Value *ch_;
    llvm::Value *addr_;
    llvm::IRBuilder<> *builder_;
    llvm::Function *func_;

    llvm::BasicBlock *lastBb_ = nullptr;
    llvm::BasicBlock *foundBb_ = nullptr;
    llvm::BasicBlock *inspectV1D0Bb_ = nullptr;
    llvm::BasicBlock *inspectV1D1Bb_ = nullptr;
    llvm::BasicBlock *clzV1D1Bb_ = nullptr;
    llvm::Value *vcmpeq1_ = nullptr;
};

MemCharSimdLowering::MemCharSimdLowering(llvm::Value *ch, llvm::Value *addr, llvm::IRBuilder<> *builder,
                                         llvm::Function *func)
    : ch_(ch), addr_(addr), builder_(builder), func_(func)
{
    ASSERT(addr_ != nullptr);
    ASSERT(builder_ != nullptr);
    ASSERT(func_ != nullptr);
}

void MemCharSimdLowering::GenLoadAndFastCheck128(llvm::VectorType *vecTy)
{
    auto *module = func_->getParent();
    auto addpId = llvm::Intrinsic::AARCH64Intrinsics::aarch64_neon_addp;
    auto addp = llvm::Intrinsic::getDeclaration(module, addpId, {GetU64X2Ty()});
    // Read 16-byte chunk of memory
    auto vld1 = builder_->CreateLoad(vecTy, addr_);
    // Prepare the search pattern
    auto insert = builder_->CreateInsertElement(vecTy, ch_, 0UL);
    auto pattern = builder_->CreateShuffleVector(insert, ShuffleMask(vecTy->getElementType()));
    // Compare
    vcmpeq1_ = builder_->CreateSExt(builder_->CreateICmpEQ(vld1, pattern), vecTy);
    // Do fast check and give up if char is not there
    auto v64x2 = builder_->CreateBitCast(vcmpeq1_, GetU64X2Ty());
    auto vaddp = builder_->CreateCall(addp, {v64x2, v64x2});
    auto low64 = builder_->CreateBitCast(builder_->CreateExtractElement(vaddp, 0UL), builder_->getInt64Ty());
    auto charIsNotThere = builder_->CreateICmpEQ(low64, llvm::Constant::getNullValue(low64->getType()));
    builder_->CreateCondBr(charIsNotThere, lastBb_, inspectV1D0Bb_);
}

void MemCharSimdLowering::GenLoadAndFastCheck256(llvm::VectorType *vecTy)
{
    auto *module = func_->getParent();
    auto ld1Id = llvm::Intrinsic::AARCH64Intrinsics::aarch64_neon_ld1x2;
    auto addpId = llvm::Intrinsic::AARCH64Intrinsics::aarch64_neon_addp;
    auto ld1 = llvm::Intrinsic::getDeclaration(module, ld1Id, {vecTy, addr_->getType()});
    auto addp1 = llvm::Intrinsic::getDeclaration(module, addpId, {vecTy});
    auto addp2 = llvm::Intrinsic::getDeclaration(module, addpId, {GetU64X2Ty()});
    // Read 32-byte chunk of memory
    auto vld1 = builder_->CreateCall(ld1, {addr_});
    auto v1 = builder_->CreateExtractValue(vld1, {0});
    auto v2 = builder_->CreateExtractValue(vld1, {1});
    // Prepare the search pattern
    auto insert = builder_->CreateInsertElement(vecTy, ch_, 0UL);
    auto pattern = builder_->CreateShuffleVector(insert, ShuffleMask(vecTy->getElementType()));
    // Compare
    vcmpeq1_ = builder_->CreateSExt(builder_->CreateICmpEQ(v1, pattern), vecTy);
    vcmpeq2_ = builder_->CreateSExt(builder_->CreateICmpEQ(v2, pattern), vecTy);
    // Do fast check and give up if char is not there
    auto vaddp = builder_->CreateCall(addp1, {vcmpeq1_, vcmpeq2_});
    auto v64x2 = builder_->CreateBitCast(vaddp, GetU64X2Ty());
    vaddp = builder_->CreateCall(addp2, {v64x2, v64x2});
    auto low64 = builder_->CreateBitCast(builder_->CreateExtractElement(vaddp, 0UL), builder_->getInt64Ty());
    auto charIsNotThere = builder_->CreateICmpEQ(low64, llvm::Constant::getNullValue(low64->getType()));
    builder_->CreateCondBr(charIsNotThere, lastBb_, inspectV1D0Bb_);
}

llvm::Value *MemCharSimdLowering::GenFindChar128(llvm::IntegerType *charTy)
{
    ASSERT(vcmpeq1_ != nullptr && vcmpeq2_ == nullptr);
    auto i64Ty = builder_->getInt64Ty();
    constexpr uint32_t DWORD_SIZE = 64U;
    // Inspect low 64-bit part of vcmpeq1
    builder_->SetInsertPoint(inspectV1D0Bb_);
    auto vcmpeq1 = builder_->CreateBitCast(vcmpeq1_, GetU64X2Ty());
    auto v1d0 = builder_->CreateBitCast(builder_->CreateExtractElement(vcmpeq1, 0UL), i64Ty);
    auto v1d0IsZero = builder_->CreateICmpEQ(v1d0, llvm::Constant::getNullValue(v1d0->getType()));
    builder_->CreateCondBr(v1d0IsZero, inspectV1D1Bb_, clzV1D0Bb_);
    builder_->SetInsertPoint(clzV1D0Bb_);
    auto rev10 = builder_->CreateUnaryIntrinsic(llvm::Intrinsic::bswap, v1d0, nullptr);
    auto pos10 = builder_->CreateBinaryIntrinsic(llvm::Intrinsic::ctlz, rev10, builder_->getFalse(), nullptr);
    builder_->CreateBr(foundBb_);
    // Inspect high 64-bit part of vcmpeq1
    builder_->SetInsertPoint(inspectV1D1Bb_);
    auto v1d1 = builder_->CreateBitCast(builder_->CreateExtractElement(vcmpeq1, 1UL), i64Ty);
    auto rev11 = builder_->CreateUnaryIntrinsic(llvm::Intrinsic::bswap, v1d1, nullptr);
    auto clz11 = builder_->CreateBinaryIntrinsic(llvm::Intrinsic::ctlz, rev11, builder_->getFalse(), nullptr);
    auto pos11 = builder_->CreateAdd(clz11, llvm::Constant::getIntegerValue(i64Ty, llvm::APInt(DWORD_SIZE, UL64)));
    builder_->CreateBr(foundBb_);
    // Compute a pointer to the char
    builder_->SetInsertPoint(foundBb_);
    auto nbits = builder_->CreatePHI(i64Ty, 2U);
    nbits->addIncoming(pos10, clzV1D0Bb_);
    nbits->addIncoming(pos11, inspectV1D1Bb_);
    auto nbytes = builder_->CreateLShr(nbits, charTy->isIntegerTy(8U) ? 3UL : 4UL);
    auto foundCharPtr = builder_->CreateInBoundsGEP(charTy, addr_, nbytes);
    builder_->CreateBr(lastBb_);
    return foundCharPtr;
}

llvm::Value *MemCharSimdLowering::GenFindChar256(llvm::IntegerType *charTy)
{
    ASSERT(vcmpeq1_ != nullptr);
    ASSERT(vcmpeq2_ != nullptr);
    auto i64Ty = builder_->getInt64Ty();
    constexpr uint32_t DWORD_SIZE = 64U;
    // Inspect low 64-bit part of vcmpeq1
    builder_->SetInsertPoint(inspectV1D0Bb_);
    auto vcmpeq1 = builder_->CreateBitCast(vcmpeq1_, GetU64X2Ty());
    auto v1d0 = builder_->CreateBitCast(builder_->CreateExtractElement(vcmpeq1, 0UL), i64Ty);
    auto v1d0IsZero = builder_->CreateICmpEQ(v1d0, llvm::Constant::getNullValue(v1d0->getType()));
    builder_->CreateCondBr(v1d0IsZero, inspectV1D1Bb_, clzV1D0Bb_);
    builder_->SetInsertPoint(clzV1D0Bb_);
    auto rev10 = builder_->CreateUnaryIntrinsic(llvm::Intrinsic::bswap, v1d0, nullptr);
    auto pos10 = builder_->CreateBinaryIntrinsic(llvm::Intrinsic::ctlz, rev10, builder_->getFalse(), nullptr);
    builder_->CreateBr(foundBb_);
    // Inspect high 64-bit part of vcmpeq1
    builder_->SetInsertPoint(inspectV1D1Bb_);
    auto v1d1 = builder_->CreateBitCast(builder_->CreateExtractElement(vcmpeq1, 1UL), i64Ty);
    auto v1d1IsZero = builder_->CreateICmpEQ(v1d1, llvm::Constant::getNullValue(v1d1->getType()));
    builder_->CreateCondBr(v1d1IsZero, inspectV2D0Bb_, clzV1D1Bb_);
    builder_->SetInsertPoint(clzV1D1Bb_);
    auto rev11 = builder_->CreateUnaryIntrinsic(llvm::Intrinsic::bswap, v1d1, nullptr);
    auto clz11 = builder_->CreateBinaryIntrinsic(llvm::Intrinsic::ctlz, rev11, builder_->getFalse(), nullptr);
    auto pos11 = builder_->CreateAdd(clz11, llvm::Constant::getIntegerValue(i64Ty, llvm::APInt(DWORD_SIZE, UL64)));
    builder_->CreateBr(foundBb_);
    // Inspect low 64-bit part of vcmpeq2
    builder_->SetInsertPoint(inspectV2D0Bb_);
    auto vcmpeq2 = builder_->CreateBitCast(vcmpeq2_, GetU64X2Ty());
    auto v2d0 = builder_->CreateBitCast(builder_->CreateExtractElement(vcmpeq2, 0UL), i64Ty);
    auto v2d0IsZero = builder_->CreateICmpEQ(v2d0, llvm::Constant::getNullValue(v2d0->getType()));
    builder_->CreateCondBr(v2d0IsZero, inspectV2D1Bb_, clzV2D0Bb_);
    builder_->SetInsertPoint(clzV2D0Bb_);
    auto rev20 = builder_->CreateUnaryIntrinsic(llvm::Intrinsic::bswap, v2d0, nullptr);
    auto clz20 = builder_->CreateBinaryIntrinsic(llvm::Intrinsic::ctlz, rev20, builder_->getFalse(), nullptr);
    auto pos20 = builder_->CreateAdd(clz20, llvm::Constant::getIntegerValue(i64Ty, llvm::APInt(DWORD_SIZE, UL128)));
    builder_->CreateBr(foundBb_);
    // Inspect high 64-bit part of vcmpeq2
    builder_->SetInsertPoint(inspectV2D1Bb_);
    auto v2d1 = builder_->CreateBitCast(builder_->CreateExtractElement(vcmpeq2, 1UL), i64Ty);
    auto rev21 = builder_->CreateUnaryIntrinsic(llvm::Intrinsic::bswap, v2d1, nullptr);
    auto clz21 = builder_->CreateBinaryIntrinsic(llvm::Intrinsic::ctlz, rev21, builder_->getFalse(), nullptr);
    auto pos21 = builder_->CreateAdd(clz21, llvm::Constant::getIntegerValue(i64Ty, llvm::APInt(DWORD_SIZE, UL192)));
    builder_->CreateBr(foundBb_);
    // Compute a pointer to the char
    builder_->SetInsertPoint(foundBb_);
    auto nbits = builder_->CreatePHI(i64Ty, 4U);
    nbits->addIncoming(pos10, clzV1D0Bb_);
    nbits->addIncoming(pos11, clzV1D1Bb_);
    nbits->addIncoming(pos20, clzV2D0Bb_);
    nbits->addIncoming(pos21, inspectV2D1Bb_);
    auto nbytes = builder_->CreateLShr(nbits, charTy->isIntegerTy(8U) ? 3UL : 4UL);
    auto foundCharPtr = builder_->CreateInBoundsGEP(charTy, addr_, nbytes);
    builder_->CreateBr(lastBb_);
    return foundCharPtr;
}

template <bool MEM_BLOCK_SIZE_256_BITS>
llvm::Value *MemCharSimdLowering::Generate(llvm::VectorType *vecTy)
{
    auto *charTy = llvm::cast<llvm::IntegerType>(vecTy->getElementType());
    ASSERT(vecTy == GetU8X16Ty() || vecTy == GetU16X8Ty());
    auto &context = func_->getContext();
    auto firstBb = builder_->GetInsertBlock();
    lastBb_ = llvm::BasicBlock::Create(context, "mem_char_using_simd_last", func_);
    foundBb_ = llvm::BasicBlock::Create(context, "mem_char_using_simd_found", func_);
    inspectV1D0Bb_ = llvm::BasicBlock::Create(context, "mem_char_using_simd_inspect_v1d0", func_);
    inspectV1D1Bb_ = llvm::BasicBlock::Create(context, "mem_char_using_simd_inspect_v1d1", func_);
    clzV1D0Bb_ = llvm::BasicBlock::Create(context, "mem_char_using_simd_clz_v1d0", func_);
    llvm::Value *foundCharPtr = nullptr;
    if constexpr (MEM_BLOCK_SIZE_256_BITS) {
        inspectV2D0Bb_ = llvm::BasicBlock::Create(context, "mem_char_using_simd_inspect_v2d0", func_);
        inspectV2D1Bb_ = llvm::BasicBlock::Create(context, "mem_char_using_simd_inspect_v2d1", func_);
        clzV1D1Bb_ = llvm::BasicBlock::Create(context, "mem_char_using_simd_clz_v1d1", func_);
        clzV2D0Bb_ = llvm::BasicBlock::Create(context, "mem_char_using_simd_clz_v2d0", func_);
        GenLoadAndFastCheck256(vecTy);
        foundCharPtr = GenFindChar256(charTy);
    } else {
        GenLoadAndFastCheck128(vecTy);
        foundCharPtr = GenFindChar128(charTy);
    }
    ASSERT(foundCharPtr != nullptr);
    // The result is either a pointer to the char or null
    builder_->SetInsertPoint(lastBb_);
    auto result = builder_->CreatePHI(builder_->getPtrTy(), 2U);
    result->addIncoming(llvm::Constant::getNullValue(builder_->getPtrTy()), firstBb);
    result->addIncoming(foundCharPtr, foundBb_);
    // Cleanup and return
    lastBb_ = nullptr;
    foundBb_ = nullptr;
    inspectV1D0Bb_ = nullptr;
    inspectV1D1Bb_ = nullptr;
    inspectV2D0Bb_ = nullptr;
    inspectV2D1Bb_ = nullptr;
    clzV1D0Bb_ = nullptr;
    clzV1D1Bb_ = nullptr;
    clzV2D0Bb_ = nullptr;
    vcmpeq1_ = nullptr;
    vcmpeq2_ = nullptr;
    return result;
}

MemLastCharSimdLowering::MemLastCharSimdLowering(llvm::Value *ch, llvm::Value *addr, llvm::IRBuilder<> *builder,
                                                 llvm::Function *func)
    : ch_(ch), addr_(addr), builder_(builder), func_(func)
{
    ASSERT(addr_ != nullptr);
    ASSERT(builder_ != nullptr);
    ASSERT(func_ != nullptr);
}

void MemLastCharSimdLowering::GenLoadAndFastCheck128(llvm::VectorType *vecTy)
{
    auto *module = func_->getParent();
    auto addpId = llvm::Intrinsic::AARCH64Intrinsics::aarch64_neon_addp;
    auto addp = llvm::Intrinsic::getDeclaration(module, addpId, {GetU64X2Ty()});
    // Read 16-byte chunk of memory
    auto vld1 = builder_->CreateLoad(vecTy, addr_);
    // Prepare the search pattern
    auto insert = builder_->CreateInsertElement(vecTy, ch_, 0UL);
    auto pattern = builder_->CreateShuffleVector(insert, ShuffleMask(vecTy->getElementType()));
    // Compare
    vcmpeq1_ = builder_->CreateSExt(builder_->CreateICmpEQ(vld1, pattern), vecTy);
    // Do fast check and give up if char is not there
    auto v64x2 = builder_->CreateBitCast(vcmpeq1_, GetU64X2Ty());
    auto vaddp = builder_->CreateCall(addp, {v64x2, v64x2});
    auto low64 = builder_->CreateBitCast(builder_->CreateExtractElement(vaddp, 0UL), builder_->getInt64Ty());
    auto charIsNotThere = builder_->CreateICmpEQ(low64, llvm::Constant::getNullValue(low64->getType()));
    builder_->CreateCondBr(charIsNotThere, lastBb_, inspectV1D1Bb_);
}

llvm::Value *MemLastCharSimdLowering::GenFindChar128(llvm::IntegerType *charTy)
{
    static constexpr uint32_t sixtyFour = 64U;
    static constexpr int32_t zeroBasedMaxIndexU8 = 15;
    static constexpr int32_t zeroBasedMaxIndexU16 = 7;
    ASSERT(vcmpeq1_ != nullptr);
    auto i64Ty = builder_->getInt64Ty();
    // Inspect high 64-bit part of vcmpeq1 first as we are looking for the last occurrence of 'ch'
    builder_->SetInsertPoint(inspectV1D1Bb_);
    auto vcmpeq1 = builder_->CreateBitCast(vcmpeq1_, GetU64X2Ty());
    auto v1d1 = builder_->CreateBitCast(builder_->CreateExtractElement(vcmpeq1, 1UL), i64Ty);
    auto v1d1IsZero = builder_->CreateICmpEQ(v1d1, llvm::Constant::getNullValue(v1d1->getType()));
    builder_->CreateCondBr(v1d1IsZero, inspectV1D0Bb_, clzV1D1Bb_);
    builder_->SetInsertPoint(clzV1D1Bb_);
    auto pos11 = builder_->CreateBinaryIntrinsic(llvm::Intrinsic::ctlz, v1d1, builder_->getFalse(), nullptr);
    builder_->CreateBr(foundBb_);
    // Inspect low 64-bit part of vcmpeq1
    builder_->SetInsertPoint(inspectV1D0Bb_);
    auto v1d0 = builder_->CreateBitCast(builder_->CreateExtractElement(vcmpeq1, 0UL), i64Ty);
    auto clz10 = builder_->CreateBinaryIntrinsic(llvm::Intrinsic::ctlz, v1d0, builder_->getFalse(), nullptr);
    auto pos10 = builder_->CreateAdd(clz10, llvm::Constant::getIntegerValue(i64Ty, llvm::APInt(sixtyFour, sixtyFour)));
    builder_->CreateBr(foundBb_);
    // Compute a pointer to the char
    builder_->SetInsertPoint(foundBb_);
    auto nbits = builder_->CreatePHI(i64Ty, 2U);
    nbits->addIncoming(pos11, clzV1D1Bb_);
    nbits->addIncoming(pos10, inspectV1D0Bb_);
    uint32_t bitsShiftWidth = charTy->isIntegerTy(8U) ? 3UL : 4UL;
    auto nchars = builder_->CreateNeg(builder_->CreateLShr(nbits, bitsShiftWidth));
    auto maxIndex = charTy->isIntegerTy(8U) ? zeroBasedMaxIndexU8 : zeroBasedMaxIndexU16;
    auto maxIndexValue = llvm::Constant::getIntegerValue(i64Ty, llvm::APInt(sixtyFour, maxIndex));
    auto offsetInChars = builder_->CreateAdd(nchars, maxIndexValue);
    auto foundCharPtr = builder_->CreateInBoundsGEP(charTy, addr_, offsetInChars);
    builder_->CreateBr(lastBb_);
    return foundCharPtr;
}

llvm::Value *MemLastCharSimdLowering::Generate(llvm::VectorType *vecTy)
{
    auto *charTy = llvm::cast<llvm::IntegerType>(vecTy->getElementType());
    ASSERT(vecTy == GetU8X16Ty() || vecTy == GetU16X8Ty());
    auto &context = func_->getContext();
    auto firstBb = builder_->GetInsertBlock();
    lastBb_ = llvm::BasicBlock::Create(context, "last", func_);
    foundBb_ = llvm::BasicBlock::Create(context, "found", func_);
    inspectV1D0Bb_ = llvm::BasicBlock::Create(context, "inspect_v1d0", func_);
    inspectV1D1Bb_ = llvm::BasicBlock::Create(context, "inspect_v1d1", func_);
    clzV1D1Bb_ = llvm::BasicBlock::Create(context, "clz_v1d1", func_);
    llvm::Value *foundCharPtr = nullptr;
    GenLoadAndFastCheck128(vecTy);
    foundCharPtr = GenFindChar128(charTy);
    ASSERT(foundCharPtr != nullptr);
    // The result is either a pointer to the char or null
    builder_->SetInsertPoint(lastBb_);
    auto result = builder_->CreatePHI(builder_->getPtrTy(), 2U);
    result->addIncoming(llvm::Constant::getNullValue(builder_->getPtrTy()), firstBb);
    result->addIncoming(foundCharPtr, foundBb_);
    // Cleanup and return
    lastBb_ = nullptr;
    foundBb_ = nullptr;
    inspectV1D0Bb_ = nullptr;
    inspectV1D1Bb_ = nullptr;
    clzV1D1Bb_ = nullptr;
    vcmpeq1_ = nullptr;
    return result;
}
class CharsCaseConversionSimdLowering final {
public:
    llvm::VectorType *GetU8X8Ty() const
    {
        return llvm::VectorType::get(builder_->getInt8Ty(), VECTOR_SIZE_8, false);
    }
    llvm::VectorType *GetU8X16Ty() const
    {
        return llvm::VectorType::get(builder_->getInt8Ty(), VECTOR_SIZE_16, false);
    }

    NO_COPY_SEMANTIC(CharsCaseConversionSimdLowering);
    NO_MOVE_SEMANTIC(CharsCaseConversionSimdLowering);

    CharsCaseConversionSimdLowering() = delete;
    ~CharsCaseConversionSimdLowering() = default;

    CharsCaseConversionSimdLowering(llvm::Value *src, llvm::Value *dst, llvm::IRBuilder<> *builder);

    void Generate(llvm::VectorType *vecTy, uint8_t begin, uint8_t end) const;

private:
    llvm::Value *src_;
    llvm::Value *dst_;
    llvm::IRBuilder<> *builder_;
};

CharsCaseConversionSimdLowering::CharsCaseConversionSimdLowering(llvm::Value *src, llvm::Value *dst,
                                                                 llvm::IRBuilder<> *builder)
    : src_(src), dst_(dst), builder_(builder)
{
    ASSERT(src_ != nullptr);
    ASSERT(dst_ != nullptr);
    ASSERT(builder_ != nullptr);
}

void CharsCaseConversionSimdLowering::Generate(llvm::VectorType *vecTy, uint8_t begin, uint8_t end) const
{
    constexpr uint32_t UINT8_SIZE = 8U;                     // CC-OFF(G.NAM.03-CPP) project code style
    constexpr uint8_t CASE_CONVERSION_FLIP_BIT = 1U << 5U;  // CC-OFF(G.NAM.03-CPP) project code style
    ASSERT(vecTy == GetU8X16Ty() || vecTy == GetU8X8Ty());
    auto ld = builder_->CreateLoad(vecTy, src_);
    auto flipper = llvm::Constant::getIntegerValue(vecTy, llvm::APInt(UINT8_SIZE, CASE_CONVERSION_FLIP_BIT));
    auto minAscii = llvm::Constant::getIntegerValue(vecTy, llvm::APInt(UINT8_SIZE, begin));
    auto comparing = builder_->CreateSub(ld, minAscii);
    auto maxAllowed = llvm::Constant::getIntegerValue(vecTy, llvm::APInt(UINT8_SIZE, end - begin));
    auto xored = builder_->CreateXor(ld, flipper);
    auto cond = builder_->CreateICmpULE(comparing, maxAllowed);
    auto res = builder_->CreateSelect(cond, xored, ld);
    builder_->CreateStore(res, dst_);
}

}  // namespace

static void MarkNormalBlocksRecursive(BasicBlock *block, Marker normal)
{
    [[maybe_unused]] size_t expected = 0;
    bool processSucc = true;
    auto last = block->GetLastInst();
    if (last != nullptr) {
        // Any successors of blocks with terminators are either TryEnd or Catch blocks
        if (last->GetFlag(inst_flags::TERMINATOR)) {
            processSucc = false;
        }
        if (last->GetOpcode() == Opcode::IfImm || last->GetOpcode() == Opcode::If) {
            expected = 1;
        }
    }
    for (size_t i = 0; i < block->GetSuccsBlocks().size(); i++) {
        auto succ = block->GetSuccessor(i);
        if (succ->IsCatch()) {
            ASSERT_DO(i > expected,
                      (std::cerr << "Catch block found too early in successors: at index " << i << std::endl));
            continue;
        }
        ASSERT_DO(i <= expected, (std::cerr << "Unexpected non-catch successor block at index " << i << std::endl));
        if (processSucc && !succ->SetMarker(normal)) {
            MarkNormalBlocksRecursive(succ, normal);
        }
    }
}

// Use that only to pass it into method like rvalue
static inline std::string CreateBasicBlockName(Inst *inst, const std::string &bbName)
{
    std::stringstream name;
    name << "bb" << std::to_string(inst->GetBasicBlock()->GetId()) << "_i" << std::to_string(inst->GetId()) << ".."
         << bbName << "..";
    return name.str();
}

static inline std::string CreateNameForInst(Inst *inst)
{
    return std::string("v") + std::to_string(inst->GetId());
}

static inline bool IsInteger(DataType::Type type)
{
    return DataType::IsTypeNumeric(type) && !DataType::IsFloatType(type) && type != DataType::POINTER;
}

static inline bool IsSignedInteger(const DataType::Type &type)
{
    return IsInteger(type) && DataType::IsTypeSigned(type);
}

static inline bool IsUnsignedInteger(DataType::Type type)
{
    return IsInteger(type) && !DataType::IsTypeSigned(type);
}

static inline bool IsAlwaysThrowBasicBlock(Inst *inst)
{
    if (!g_options.IsCompilerInliningSkipThrowBlocks()) {
        return false;
    }

    auto bbLastInst = inst->GetBasicBlock()->GetLastInst();
    return bbLastInst->GetOpcode() == Opcode::Throw || bbLastInst->GetOpcode() == Opcode::Deoptimize;
}

static llvm::ICmpInst::Predicate ICmpCodeConvert(ConditionCode cc)
{
    switch (cc) {
        case ConditionCode::CC_EQ:
            return llvm::CmpInst::Predicate::ICMP_EQ;
        case ConditionCode::CC_NE:
            return llvm::CmpInst::Predicate::ICMP_NE;
        case ConditionCode::CC_LT:
            return llvm::CmpInst::Predicate::ICMP_SLT;
        case ConditionCode::CC_GT:
            return llvm::CmpInst::Predicate::ICMP_SGT;
        case ConditionCode::CC_LE:
            return llvm::CmpInst::Predicate::ICMP_SLE;
        case ConditionCode::CC_GE:
            return llvm::CmpInst::Predicate::ICMP_SGE;
        case ConditionCode::CC_B:
            return llvm::CmpInst::Predicate::ICMP_ULT;
        case ConditionCode::CC_A:
            return llvm::CmpInst::Predicate::ICMP_UGT;
        case ConditionCode::CC_BE:
            return llvm::CmpInst::Predicate::ICMP_ULE;
        case ConditionCode::CC_AE:
            return llvm::CmpInst::Predicate::ICMP_UGE;
        default:
            UNREACHABLE();
            return llvm::CmpInst::Predicate::ICMP_NE;
    }
}

static llvm::FCmpInst::Predicate FCmpCodeConvert(ConditionCode conditionCode)
{
    switch (conditionCode) {
        case ConditionCode::CC_EQ:
            return llvm::FCmpInst::Predicate::FCMP_UEQ;
        case ConditionCode::CC_NE:
            return llvm::FCmpInst::Predicate::FCMP_UNE;
        case ConditionCode::CC_LT:
            return llvm::FCmpInst::Predicate::FCMP_ULT;
        case ConditionCode::CC_GT:
            return llvm::FCmpInst::Predicate::FCMP_UGT;
        case ConditionCode::CC_LE:
            return llvm::FCmpInst::Predicate::FCMP_ULE;
        case ConditionCode::CC_GE:
            return llvm::FCmpInst::Predicate::FCMP_UGE;
        case ConditionCode::CC_B:
            return llvm::FCmpInst::Predicate::FCMP_ULT;
        case ConditionCode::CC_A:
            return llvm::FCmpInst::Predicate::FCMP_UGT;
        case ConditionCode::CC_BE:
            return llvm::FCmpInst::Predicate::FCMP_ULE;
        case ConditionCode::CC_AE:
            return llvm::FCmpInst::Predicate::FCMP_UGE;
        default:
            ASSERT_DO(false, (std::cerr << "Unexpected condition_code = " << conditionCode << std::endl));
            UNREACHABLE();
    }
}

static DeoptimizeType GetDeoptimizationType(Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::NullCheck:
            return DeoptimizeType::NULL_CHECK;
        case Opcode::DeoptimizeIf:
            return inst->CastToDeoptimizeIf()->GetDeoptimizeType();
        case Opcode::BoundsCheck:
            return DeoptimizeType::BOUNDS_CHECK_WITH_DEOPT;
        case Opcode::NegativeCheck:
            return DeoptimizeType::NEGATIVE_CHECK;
        case Opcode::ZeroCheck:
            return DeoptimizeType::ZERO_CHECK;
        case Opcode::SubOverflowCheck:
            return DeoptimizeType::OVERFLOW_TYPE;
        case Opcode::CheckCast:
            return DeoptimizeType::CHECK_CAST;
        case Opcode::RefTypeCheck:
        default:
            ASSERT_DO(false, (std::cerr << "Unexpected inst to GetDeoptimizationType, inst:" << std::endl,
                              inst->Dump(&std::cerr, true)));
            UNREACHABLE();
    }
}

static llvm::CallingConv::ID GetFastPathCallingConv(uint32_t numArgs)
{
    switch (numArgs) {
        case 0U:
            return llvm::CallingConv::ArkFast0;
        case 1U:
            return llvm::CallingConv::ArkFast1;
        case 2U:
            return llvm::CallingConv::ArkFast2;
        case 3U:
            return llvm::CallingConv::ArkFast3;
        case 4U:
            return llvm::CallingConv::ArkFast4;
        case 5U:
            return llvm::CallingConv::ArkFast5;
        case 6U:
            return llvm::CallingConv::ArkFast6;
        default:
            UNREACHABLE();
    }
}

static RuntimeInterface::EntrypointId GetAllocateArrayTlabEntrypoint(size_t elementSize)
{
    switch (elementSize) {
        case sizeof(uint8_t):
            return RuntimeInterface::EntrypointId::ALLOCATE_ARRAY_TLAB8;
        case sizeof(uint16_t):
            return RuntimeInterface::EntrypointId::ALLOCATE_ARRAY_TLAB16;
        case sizeof(uint32_t):
            return RuntimeInterface::EntrypointId::ALLOCATE_ARRAY_TLAB32;
        case sizeof(uint64_t):
            return RuntimeInterface::EntrypointId::ALLOCATE_ARRAY_TLAB64;
        default:
            UNREACHABLE();
    }
}

static size_t GetRealFrameReg(Arch arch)
{
    switch (arch) {
        case Arch::AARCH64:
            return AARCH64_REAL_FP;
        case Arch::X86_64:
            return X86_64_REAL_FP;
        default:
            UNREACHABLE();
    }
}

/**
 * Call when we are sure that instruction shouldn't appear for translating but
 * eventually we've tried to translate it.
 */
static void UnexpectedLowering([[maybe_unused]] Inst *inst)
{
    ASSERT_DO(false, (std::cerr << "Unexpected attempt to lower: ", inst->Dump(&std::cerr, true)));
    UNREACHABLE();
}

bool LLVMIrConstructor::IsSafeCast(Inst *inst, unsigned int index)
{
    auto trueType = inst->GetInput(index).GetInst()->GetType();
    auto instType = inst->GetInputType(index);
    bool signTheSame = IsSignedInteger(trueType) == IsSignedInteger(instType);
    bool extending = DataType::GetTypeSize(trueType, GetGraph()->GetArch()) <=
                     DataType::GetTypeSize(instType, GetGraph()->GetArch());
    return signTheSame || extending;
}

bool LLVMIrConstructor::TryEmitIntrinsic(Inst *inst, RuntimeInterface::IntrinsicId arkId)
{
    if (GetGraph()->IsStringFlatCheckConstraint() && GetStringFlatCheckArgMask(arkId) > 0) {
        // Tree strings are enabled and StringFlatCheck is disabled but IRTOC implementation requires flat string, so
        // call CPP implementation
        return false;
    }

    auto module = func_->getParent();
    auto f32Ty = builder_.getFloatTy();
    auto f64Ty = builder_.getDoubleTy();
    llvm::Function *llvmId = nullptr;

    switch (arkId) {
#include "intrinsics_llvm_codegen.inl"
#ifndef NDEBUG
        // Must be lowered earlier in IrBuilder, impossible to meet
        case RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_ENTER:
        case RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_EXIT:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_I32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_I64:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_F32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_F64:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_SQRT_F32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_SQRT_F64:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_I32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_I64:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_F32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_F64:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_I32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_I64:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_F32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_F64:
            UNREACHABLE();
        // Can appear only after LLVM optimizations
        case RuntimeInterface::IntrinsicId::LIB_CALL_MEM_COPY:
        case RuntimeInterface::IntrinsicId::LIB_CALL_MEM_SET:
        case RuntimeInterface::IntrinsicId::LIB_CALL_MEM_MOVE:
            UNREACHABLE();
#include "emit_intrinsic_llvm_ir_constructor_gen.inl"
#endif
        default:
            return false;
    }

    ASSERT(llvmId != nullptr);
    ASSERT(!inst->CanThrow());

    arkInterface_->GetOrCreateRuntimeFunctionType(func_->getContext(), func_->getParent(),
                                                  LLVMArkInterface::RuntimeCallType::INTRINSIC,
                                                  static_cast<LLVMArkInterface::EntrypointId>(arkId));

    auto arguments = GetIntrinsicArguments(llvmId->getFunctionType(), inst->CastToIntrinsic());
    auto result = llvm::CallInst::Create(llvmId, arguments, "", GetCurrentBasicBlock());
    SetIntrinsicParamAttrs(result, inst->CastToIntrinsic(), arguments);
    ValueMapAdd(inst, result);
    return true;
}

// Specific intrinsic Emitters

bool LLVMIrConstructor::EmitFastPath(Inst *inst, RuntimeInterface::EntrypointId eid, uint32_t numArgs)
{
    ArenaVector<llvm::Value *> args(GetGraph()->GetLocalAllocator()->Adapter());
    for (uint32_t i = 0; i < numArgs; i++) {
        args.push_back(GetInputValue(inst, i));
    }
    auto call = CreateFastPathCall(inst, eid, args);

    auto retType = GetType(inst->GetType());
    if (!retType->isVoidTy()) {
        ValueMapAdd(inst, call);
    }
    return true;
}

bool LLVMIrConstructor::EmitStringEquals(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::STRING_EQUALS, 2U);
}

bool LLVMIrConstructor::EmitStringBuilderBool(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::STRING_BUILDER_BOOL, 2U);
}

bool LLVMIrConstructor::EmitStringBuilderChar(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::STRING_BUILDER_CHAR, 2U);
}

bool LLVMIrConstructor::EmitStringBuilderString(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::STRING_BUILDER_STRING_COMPRESSED, 2U);
}

bool LLVMIrConstructor::EmitStringCompareTo(Inst *inst)
{
    return EmitFastPath(inst, RuntimeInterface::EntrypointId::STRING_COMPARE_TO, 2U);
}

bool LLVMIrConstructor::EmitIsInf(Inst *inst)
{
    auto result = CreateIsInf(GetInputValue(inst, 0));
    ValueMapAdd(inst, result);
    return true;
}

bool LLVMIrConstructor::EmitMemmoveUnchecked(Inst *inst)
{
    switch (inst->CastToIntrinsic()->GetIntrinsicId()) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_MEMMOVE_UNCHECKED_1_BYTE:
            return EmitFastPath(inst, RuntimeInterface::EntrypointId::ARRAY_COPY_TO_UNCHECKED_1_BYTE, 5U);
        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_MEMMOVE_UNCHECKED_2_BYTE:
            return EmitFastPath(inst, RuntimeInterface::EntrypointId::ARRAY_COPY_TO_UNCHECKED_2_BYTE, 5U);
        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_MEMMOVE_UNCHECKED_4_BYTE:
            return EmitFastPath(inst, RuntimeInterface::EntrypointId::ARRAY_COPY_TO_UNCHECKED_4_BYTE, 5U);
        case RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_MEMMOVE_UNCHECKED_8_BYTE:
            return EmitFastPath(inst, RuntimeInterface::EntrypointId::ARRAY_COPY_TO_UNCHECKED_8_BYTE, 5U);
        default:
            UNREACHABLE();
    }
}

bool LLVMIrConstructor::EmitUnreachable([[maybe_unused]] Inst *inst)
{
    auto bb = GetCurrentBasicBlock();
    if (bb->empty() || !llvm::isa<llvm::ReturnInst>(*(bb->rbegin()))) {
        auto trap = llvm::Intrinsic::getDeclaration(func_->getParent(), llvm::Intrinsic::trap, {});
        builder_.CreateCall(trap, {});
        builder_.CreateUnreachable();
    }
    return true;
}

bool LLVMIrConstructor::EmitNothing([[maybe_unused]] Inst *inst)
{
    return true;
}

#ifndef NDEBUG
static void CheckSlowPathName(const std::string &name, size_t funcArgsNum, size_t callArgsNum)
{
    ASSERT_DO(std::string_view {name}.find("SlowPath") == std::string_view::npos,
              std::cerr << "Bad bridge: SlowPath bridge not allowed in LLVM FastPath: " << name << std::endl);
    ASSERT(callArgsNum <= funcArgsNum);
    if (callArgsNum < funcArgsNum) {
        funcArgsNum -= 2U;  // exclude fake arguments for these asserts
        ASSERT(funcArgsNum <= 4U);
        ASSERT_DO((std::string_view {name}.find("1ArgBridge") != std::string_view::npos) == (funcArgsNum == 1U),
                  std::cerr << "Bad bridge: OddSaved1 for FastPath with 1 arguments "
                            << "and SlowPath with zero arguments: " << name << std::endl);
        ASSERT_DO((std::string_view {name}.find("2ArgBridge") != std::string_view::npos) == (funcArgsNum == 2U),
                  std::cerr << "Bad bridge: OddSaved2 for FastPath with 2 arguments "
                            << "and SlowPath with 0-1 arguments: " << name << std::endl);
        ASSERT_DO((std::string_view {name}.find("3ArgBridge") != std::string_view::npos) == (funcArgsNum == 3U),
                  std::cerr << "Bad bridge: OddSaved3 for FastPath with 3 arguments "
                            << "and SlowPath with 0-2 arguments: " << name << std::endl);
        ASSERT_DO((std::string_view {name}.find("4ArgBridge") != std::string_view::npos) == (funcArgsNum == 4U),
                  std::cerr << "Bad bridge: OddSaved4 for FastPath with 4 arguments "
                            << "and SlowPath with 0-3 arguments: " << name << std::endl);
    } else {  // callArgsNum == funcArgsNum
        ASSERT_DO((std::string_view {name}.find("OddSaved") != std::string_view::npos) == (funcArgsNum % 2U == 1U),
                  std::cerr << "Bad bridge: OddSaved <=> amount of arguments is odd: " << name << std::endl);
    }
}
#endif

bool LLVMIrConstructor::EmitSlowPathEntry(Inst *inst)
{
    ASSERT(GetGraph()->GetMode().IsFastPath());
    ASSERT(func_->getCallingConv() == llvm::CallingConv::ArkFast0 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast1 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast2 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast3 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast4 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast5 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast6);

    // Arguments
    ArenaVector<llvm::Value *> args(GetGraph()->GetLocalAllocator()->Adapter());
    for (size_t i = 0; i < inst->GetInputs().Size(); i++) {
        args.push_back(GetInputValue(inst, i));
    }
    auto threadRegPtr = builder_.CreateIntToPtr(GetThreadRegValue(), builder_.getPtrTy());
    auto frameRegPtr = builder_.CreateIntToPtr(GetRealFrameRegValue(), builder_.getPtrTy());
    args.push_back(threadRegPtr);
    args.push_back(frameRegPtr);

    ASSERT(inst->CastToIntrinsic()->HasImms() && inst->CastToIntrinsic()->GetImms().size() == 2U);
    uint32_t externalId = inst->CastToIntrinsic()->GetImms()[1];
    auto externalName = GetGraph()->GetRuntime()->GetExternalMethodName(GetGraph()->GetMethod(), externalId);
#ifndef NDEBUG
    CheckSlowPathName(externalName, func_->arg_size(), args.size());
#endif
    auto callee = func_->getParent()->getFunction(externalName);
    if (callee == nullptr) {
        ArenaVector<llvm::Type *> argTypes(GetGraph()->GetLocalAllocator()->Adapter());
        for (const auto &input : inst->GetInputs()) {
            argTypes.push_back(GetExactType(input.GetInst()->GetType()));
        }
        argTypes.push_back(builder_.getPtrTy());
        argTypes.push_back(builder_.getPtrTy());
        auto ftype = llvm::FunctionType::get(GetType(inst->GetType()), argTypes, false);
        callee = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, externalName, func_->getParent());
        callee->setCallingConv(GetFastPathCallingConv(inst->GetInputs().Size()));
    }

    auto call = builder_.CreateCall(callee->getFunctionType(), callee, args);
    call->setCallingConv(callee->getCallingConv());
    call->setTailCallKind(llvm::CallInst::TailCallKind::TCK_Tail);
    call->addFnAttr(llvm::Attribute::get(call->getContext(), "ark-tail-call"));
    CreateReturn(call);
    return true;
}

bool LLVMIrConstructor::EmitExclusiveLoadWithAcquire(Inst *inst)
{
    ASSERT(GetGraph()->GetArch() == Arch::AARCH64);
    ASSERT(inst->GetInputType(0) == DataType::POINTER);
    auto &ctx = func_->getContext();
    auto addr = GetInputValue(inst, 0);
    auto dstType = GetExactType(inst->GetType());
    auto intrinsicId = llvm::Intrinsic::AARCH64Intrinsics::aarch64_ldaxr;
    auto load = builder_.CreateUnaryIntrinsic(intrinsicId, addr);
    load->addParamAttr(0, llvm::Attribute::get(ctx, llvm::Attribute::ElementType, dstType));
    ValueMapAdd(inst, load);
    return true;
}

bool LLVMIrConstructor::EmitExclusiveStoreWithRelease(Inst *inst)
{
    ASSERT(GetGraph()->GetArch() == Arch::AARCH64);
    ASSERT(inst->GetInputType(0) == DataType::POINTER);
    auto &ctx = func_->getContext();
    auto addr = GetInputValue(inst, 0);
    auto value = GetInputValue(inst, 1);
    auto type = value->getType();
    auto intrinsicId = llvm::Intrinsic::AARCH64Intrinsics::aarch64_stlxr;
    auto stlxr = llvm::Intrinsic::getDeclaration(func_->getParent(), intrinsicId, builder_.getPtrTy());
    value = builder_.CreateZExtOrBitCast(value, stlxr->getFunctionType()->getParamType(0));
    auto store = builder_.CreateCall(stlxr, {value, addr});
    store->addParamAttr(1, llvm::Attribute::get(ctx, llvm::Attribute::ElementType, type));
    ValueMapAdd(inst, store);
    return true;
}

bool LLVMIrConstructor::EmitInterpreterReturn([[maybe_unused]] Inst *inst)
{
    // We only support it for Irtoc interpreters on AArch64
    ASSERT(GetGraph()->GetMode().IsInterpreter());

    CFrameLayout fl(GetGraph()->GetArch(), IRTOC_NUM_SPILL_SLOTS);
    constexpr bool SAVE_UNUSED_CALLEE_REGS = true;

    // Restore callee-registers
    auto calleeRegsMask = GetCalleeRegsMask(GetGraph()->GetArch(), false, SAVE_UNUSED_CALLEE_REGS);
    auto calleeVregsMask = GetCalleeRegsMask(GetGraph()->GetArch(), true, SAVE_UNUSED_CALLEE_REGS);
    if (GetGraph()->GetArch() == Arch::AARCH64) {
        constexpr bool SAVE_FRAME_AND_LINK_REGS = true;

        size_t slotSize = fl.GetSlotSize();
        size_t dslotSize = slotSize * 2U;

        auto lastCalleeReg = fl.GetRegsSlotsCount() - calleeRegsMask.Count();
        auto lastCalleeVreg = fl.GetRegsSlotsCount() - fl.GetCalleeRegistersCount(false) - calleeVregsMask.Count();
        CreateInterpreterReturnRestoreRegs(calleeRegsMask, lastCalleeReg, false);
        CreateInterpreterReturnRestoreRegs(calleeVregsMask, lastCalleeVreg, true);

        // Adjust SP
        auto spToFrameTopSlots = fl.GetRegsSlotsCount() + CFrameRegs::Start() - CFrameReturnAddr::Start();
        if (SAVE_FRAME_AND_LINK_REGS) {
            spToFrameTopSlots -= CFrameLayout::GetFpLrSlotsCount();
        }

        CreateInt32ImmAsm(&builder_,
                          std::string("add  sp, sp, $0").append(LLVMArkInterface::PATCH_STACK_ADJUSTMENT_COMMENT),
                          spToFrameTopSlots * slotSize);
        CreateInt32ImmAsm(&builder_, "ldp  x29, x30, [sp], $0", dslotSize);
        CreateBlackBoxAsm(&builder_, "ret");
    } else {
        // Currently there is no vector regs usage at x86_64 handlers
        ASSERT(calleeVregsMask.count() == 0);
        auto regShift = DOUBLE_WORD_SIZE_BYTES *
                        (fl.GetSpillsCount() + fl.GetCallerRegistersCount(false) + fl.GetCallerRegistersCount(true));
        auto fpShift = DOUBLE_WORD_SIZE_BYTES * (2 + CFrameSlots::Start() - CFrameData::Start());

        std::string iasmStr =
            std::string("leaq  ${0:c}(%rsp), %rsp").append(LLVMArkInterface::PATCH_STACK_ADJUSTMENT_COMMENT);
        CreateInt32ImmAsm(&builder_, iasmStr, regShift);
        Target target {GetGraph()->GetArch()};
        while (calleeRegsMask.count() > 0) {
            auto reg = calleeRegsMask.GetMinRegister();
            calleeRegsMask ^= 1U << reg;
            iasmStr = "pop  %" + target.GetRegName(reg, false);
            CreateBlackBoxAsm(&builder_, iasmStr);
        }
        iasmStr = "leaq  " + std::to_string(fpShift) + "(%rsp), %rsp";
        CreateBlackBoxAsm(&builder_, iasmStr);
        CreateBlackBoxAsm(&builder_, "pop  %rbp");
        CreateBlackBoxAsm(&builder_, "retq");
    }
    builder_.CreateUnreachable();

    return true;
}

bool LLVMIrConstructor::EmitInitInterpreterCallRecord([[maybe_unused]] Inst *inst)
{
    ASSERT(GetGraph()->GetMode().IsInterpreter());
    UNREACHABLE();  // Embedded call record not supported in irtoc+LLVM currently.
}

bool LLVMIrConstructor::EmitPushInterpreterCallRecord([[maybe_unused]] Inst *inst)
{
    ASSERT(GetGraph()->GetMode().IsInterpreter());
    UNREACHABLE();  // Embedded call record not supported in irtoc+LLVM currently.
}

bool LLVMIrConstructor::EmitPopInterpreterCallRecord([[maybe_unused]] Inst *inst)
{
    ASSERT(GetGraph()->GetMode().IsInterpreter());
    UNREACHABLE();  // Embedded call record not supported in irtoc+LLVM currently.
}

bool LLVMIrConstructor::EmitPopMultipleInterpreterCallRecords([[maybe_unused]] Inst *inst)
{
    ASSERT(GetGraph()->GetMode().IsInterpreter());
    UNREACHABLE();  // Embedded call record not supported in irtoc+LLVM currently.
}

bool LLVMIrConstructor::EmitUpdateInterpreterCallRecord([[maybe_unused]] Inst *inst)
{
    ASSERT(GetGraph()->GetMode().IsInterpreter());
    UNREACHABLE();  // Embedded call record not supported in irtoc+LLVM currently.
}

bool LLVMIrConstructor::EmitTailCall(Inst *inst)
{
    ASSERT(func_->getCallingConv() == llvm::CallingConv::ArkFast0 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast1 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast2 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast3 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast4 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast5 ||
           func_->getCallingConv() == llvm::CallingConv::ArkFast6 ||
           func_->getCallingConv() == llvm::CallingConv::ArkInt);
    llvm::CallInst *call;

    if (GetGraph()->GetMode().IsFastPath()) {
        call = CreateTailCallFastPath(inst);
    } else if (GetGraph()->GetMode().IsInterpreter()) {
        call = CreateTailCallInterpreter(inst);
    } else {
        UNREACHABLE();
    }
    call->setTailCallKind(llvm::CallInst::TailCallKind::TCK_Tail);
    call->addFnAttr(llvm::Attribute::get(call->getContext(), "ark-tail-call"));
    CreateReturn(call);
    std::fill(ccValues_.begin(), ccValues_.end(), nullptr);
    return true;
}

bool LLVMIrConstructor::EmitCompressEightUtf16ToUtf8CharsUsingSimd(Inst *inst)
{
    CreateCompressUtf16ToUtf8CharsUsingSimd<VECTOR_SIZE_8>(inst);
    return true;
}

bool LLVMIrConstructor::EmitCompressSixteenUtf16ToUtf8CharsUsingSimd(Inst *inst)
{
    CreateCompressUtf16ToUtf8CharsUsingSimd<VECTOR_SIZE_16>(inst);
    return true;
}

bool LLVMIrConstructor::EmitMemCharU8X16UsingSimd(Inst *inst)
{
    ASSERT(GetGraph()->GetArch() == Arch::AARCH64);
    ASSERT(GetGraph()->GetMode().IsFastPath());
    ASSERT(inst->GetInputType(0) == DataType::UINT8);
    ASSERT(inst->GetInputType(1) == DataType::POINTER);

    MemCharSimdLowering memCharLowering(GetInputValue(inst, 0), GetInputValue(inst, 1), GetBuilder(), GetFunc());
    ValueMapAdd(inst, memCharLowering.Generate<false>(memCharLowering.GetU8X16Ty()));
    return true;
}

bool LLVMIrConstructor::EmitMemCharU8X32UsingSimd(Inst *inst)
{
    ASSERT(GetGraph()->GetArch() == Arch::AARCH64);
    ASSERT(GetGraph()->GetMode().IsFastPath());
    ASSERT(inst->GetInputType(0) == DataType::UINT8);
    ASSERT(inst->GetInputType(1) == DataType::POINTER);

    MemCharSimdLowering memCharLowering(GetInputValue(inst, 0), GetInputValue(inst, 1), GetBuilder(), GetFunc());
    ValueMapAdd(inst, memCharLowering.Generate<true>(memCharLowering.GetU8X16Ty()));
    return true;
}

bool LLVMIrConstructor::EmitMemCharU16X8UsingSimd(Inst *inst)
{
    ASSERT(GetGraph()->GetArch() == Arch::AARCH64);
    ASSERT(GetGraph()->GetMode().IsFastPath());
    ASSERT(inst->GetInputType(0) == DataType::UINT16);
    ASSERT(inst->GetInputType(1) == DataType::POINTER);

    MemCharSimdLowering memCharLowering(GetInputValue(inst, 0), GetInputValue(inst, 1), GetBuilder(), GetFunc());
    ValueMapAdd(inst, memCharLowering.Generate<false>(memCharLowering.GetU16X8Ty()));
    return true;
}

bool LLVMIrConstructor::EmitMemCharU16X16UsingSimd(Inst *inst)
{
    ASSERT(GetGraph()->GetArch() == Arch::AARCH64);
    ASSERT(GetGraph()->GetMode().IsFastPath());
    ASSERT(inst->GetInputType(0) == DataType::UINT16);
    ASSERT(inst->GetInputType(1) == DataType::POINTER);

    MemCharSimdLowering memCharLowering(GetInputValue(inst, 0), GetInputValue(inst, 1), GetBuilder(), GetFunc());
    ValueMapAdd(inst, memCharLowering.Generate<true>(memCharLowering.GetU16X8Ty()));
    return true;
}

bool LLVMIrConstructor::EmitMemLastCharU8X16UsingSimd(Inst *inst)
{
    ASSERT(GetGraph()->GetArch() == Arch::AARCH64);
    ASSERT(GetGraph()->GetMode().IsFastPath());
    ASSERT(inst->GetInputType(0) == DataType::UINT8);
    ASSERT(inst->GetInputType(1) == DataType::POINTER);

    auto ch = GetInputValue(inst, 0);
    auto addr = GetInputValue(inst, 1);
    MemLastCharSimdLowering memLastCharLowering(ch, addr, GetBuilder(), GetFunc());
    ValueMapAdd(inst, memLastCharLowering.Generate(memLastCharLowering.GetU8X16Ty()));
    return true;
}

bool LLVMIrConstructor::EmitMemLastCharU16X8UsingSimd(Inst *inst)
{
    ASSERT(GetGraph()->GetArch() == Arch::AARCH64);
    ASSERT(GetGraph()->GetMode().IsFastPath());
    ASSERT(inst->GetInputType(0) == DataType::UINT16);
    ASSERT(inst->GetInputType(1) == DataType::POINTER);

    auto ch = GetInputValue(inst, 0);
    auto addr = GetInputValue(inst, 1);
    MemLastCharSimdLowering memLastCharLowering(ch, addr, GetBuilder(), GetFunc());
    ValueMapAdd(inst, memLastCharLowering.Generate(memLastCharLowering.GetU16X8Ty()));
    return true;
}

template <uint32_t VECTOR_SIZE>
void LLVMIrConstructor::CreateCharsToCaseU8UsingSimd(Inst *inst, uint8_t begin, uint8_t end)
{
    ASSERT(GetGraph()->GetArch() == Arch::AARCH64);
    ASSERT(GetGraph()->GetMode().IsFastPath());
    ASSERT(inst->GetInputsCount() == 2U);
    ASSERT(inst->GetInputType(0) == DataType::POINTER);
    ASSERT(inst->GetInputType(1) == DataType::POINTER);

    static_assert(VECTOR_SIZE == VECTOR_SIZE_8 || VECTOR_SIZE == VECTOR_SIZE_16, "Unexpected vector size");

    CharsCaseConversionSimdLowering charsCaseConversionLowering(GetInputValue(inst, 0), GetInputValue(inst, 1),
                                                                GetBuilder());
    auto vecTy = llvm::VectorType::get(builder_.getInt8Ty(), VECTOR_SIZE, false);
    charsCaseConversionLowering.Generate(vecTy, begin, end);
}

bool LLVMIrConstructor::EmitCharsToUpperCaseU8X16UsingSimd(Inst *inst)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    CreateCharsToCaseU8UsingSimd<VECTOR_SIZE_16>(inst, 0x61U, 0x7aU);
    return true;
}

bool LLVMIrConstructor::EmitCharsToUpperCaseU8X8UsingSimd(Inst *inst)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    CreateCharsToCaseU8UsingSimd<VECTOR_SIZE_8>(inst, 0x61U, 0x7aU);
    return true;
}

bool LLVMIrConstructor::EmitCharsToLowerCaseU8X16UsingSimd(Inst *inst)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    CreateCharsToCaseU8UsingSimd<VECTOR_SIZE_16>(inst, 0x41U, 0x5aU);
    return true;
}

bool LLVMIrConstructor::EmitCharsToLowerCaseU8X8UsingSimd(Inst *inst)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    CreateCharsToCaseU8UsingSimd<VECTOR_SIZE_8>(inst, 0x41U, 0x5aU);
    return true;
}

bool LLVMIrConstructor::EmitReverseBytes(Inst *inst)
{
    ASSERT(IsSafeCast(inst, 0));
    auto result = builder_.CreateUnaryIntrinsic(llvm::Intrinsic::bswap, GetInputValue(inst, 0), nullptr);
    ValueMapAdd(inst, result);
    return true;
}

bool LLVMIrConstructor::EmitMemoryFenceFull([[maybe_unused]] Inst *inst)
{
    CreateMemoryFence(memory_order::FULL);
    return true;
}

bool LLVMIrConstructor::EmitMemoryFenceRelease([[maybe_unused]] Inst *inst)
{
    CreateMemoryFence(memory_order::RELEASE);
    return true;
}

bool LLVMIrConstructor::EmitMemoryFenceAcquire([[maybe_unused]] Inst *inst)
{
    CreateMemoryFence(memory_order::ACQUIRE);
    return true;
}

bool LLVMIrConstructor::EmitRoundToPInf(Inst *inst)
{
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr double HALF = 0.5;
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr double NEG_HALF = -0.5;
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr double ONE = 1.0;
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr double ZERO = 0.0;
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr double NEG_ZERO = -0.0;

    auto input = GetInputValue(inst, 0);
    ASSERT_TYPE(input, builder_.getDoubleTy());

    auto ceil = builder_.CreateIntrinsic(llvm::Intrinsic::ceil, {builder_.getDoubleTy()}, {input});
    auto diff = builder_.CreateFSub(ceil, input);
    auto roundBias = llvm::ConstantFP::get(builder_.getDoubleTy(), HALF);
    auto cmp = builder_.CreateFCmpOGT(diff, roundBias);
    auto compensation = llvm::ConstantFP::get(builder_.getDoubleTy(), ONE);
    auto adjusted = builder_.CreateFSub(ceil, compensation);
    auto roundRes = builder_.CreateSelect(cmp, adjusted, ceil);

    auto negHalfConst = llvm::ConstantFP::get(builder_.getDoubleTy(), NEG_HALF);
    auto zeroConst = llvm::ConstantFP::get(builder_.getDoubleTy(), ZERO);
    auto negZeroConst = llvm::ConstantFP::get(builder_.getDoubleTy(), NEG_ZERO);

    auto cmpLow = builder_.CreateFCmpOGE(input, negHalfConst);
    auto cmpHigh = builder_.CreateFCmpOLT(input, roundBias);
    auto inRange = builder_.CreateAnd(cmpLow, cmpHigh);

    auto cmpNeg = builder_.CreateFCmpOLT(input, zeroConst);
    auto zeroSel = builder_.CreateSelect(cmpNeg, negZeroConst, zeroConst);

    auto result = builder_.CreateSelect(inRange, zeroSel, roundRes);

    ValueMapAdd(inst, result);
    return true;
}

bool LLVMIrConstructor::EmitFround(Inst *inst)
{
    llvm::Value *input = GetInputValue(inst, 0);
    ASSERT_TYPE(input, builder_.getDoubleTy());
    auto isNan = CreateIsNan(input);
    auto floatCasted = builder_.CreateCast(llvm::Instruction::FPTrunc, input, builder_.getFloatTy());
    auto casted = builder_.CreateCast(llvm::Instruction::FPExt, floatCasted, builder_.getDoubleTy());
    llvm::Value *nan = llvm::ConstantFP::getQNaN(builder_.getDoubleTy());
    auto result = builder_.CreateSelect(isNan, nan, casted);
    ValueMapAdd(inst, result);
    return true;
}

bool LLVMIrConstructor::EmitJsCastDoubleToChar([[maybe_unused]] Inst *inst)
{
    ASSERT(GetGraph()->GetArch() == Arch::AARCH64);
    ASSERT(GetGraph()->GetMode().IsFastPath());
    ASSERT_DO(!g_options.IsCpuFeatureEnabled(CpuFeature::JSCVT),
              std::cerr << "The LLVM backend doesn't support the aarch64_fjcvtzs intrinsic yet." << std::endl);
    llvm::Value *input = GetInputValue(inst, 0);
    auto sourceType = input->getType();
    ASSERT_DO(sourceType->isDoubleTy(), std::cerr << "Unexpected source type: " << GetTypeName(sourceType)
                                                  << ". Should be a double." << std::endl);
    auto targetType = inst->GetType();
    ASSERT_DO(targetType == DataType::UINT32,
              std::cerr << "Unexpected target type: " << targetType << ". Should be a uint32_t." << std::endl);

    // infinite and big numbers will overflow here to INT64_MIN or INT64_MAX, but NaN casts to 0
    auto *doubleToInt =
        builder_.CreateIntrinsic(llvm::Intrinsic::fptosi_sat, {builder_.getInt64Ty(), sourceType}, {input}, nullptr);

    auto *int64min = builder_.CreateICmpEQ(doubleToInt, builder_.getInt64(std::numeric_limits<int64_t>::min()));
    auto *int64max = builder_.CreateICmpEQ(doubleToInt, builder_.getInt64(std::numeric_limits<int64_t>::max()));
    auto *overflow = builder_.CreateLogicalOr(int64min, int64max);

    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr uint64_t UTF16_CHAR_MASK = 0xffff;
    auto *character = builder_.CreateTrunc(builder_.CreateAnd(doubleToInt, builder_.getInt64(UTF16_CHAR_MASK)),
                                           GetExactType(targetType));

    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr uint32_t FAILURE_RESULT_FLAG = (1U << 16U);
    auto *result = builder_.CreateSelect(overflow, builder_.getInt32(FAILURE_RESULT_FLAG), character);
    ValueMapAdd(inst, result);
    return true;
}

bool LLVMIrConstructor::EmitCtlz(Inst *inst)
{
    auto result = CreateZerosCount(inst, llvm::Intrinsic::ctlz);
    ValueMapAdd(inst, result);
    return true;
}

bool LLVMIrConstructor::EmitCttz(Inst *inst)
{
    auto result = CreateZerosCount(inst, llvm::Intrinsic::cttz);
    ValueMapAdd(inst, result);
    return true;
}

bool LLVMIrConstructor::EmitSignbit(Inst *inst)
{
    auto num = GetInputValue(inst, 0);
    auto bitcast = builder_.CreateBitCast(num, builder_.getInt64Ty());
    auto cmp = builder_.CreateICmpSLT(bitcast, builder_.getInt64(0));
    ValueMapAdd(inst, cmp);
    return true;
}

bool LLVMIrConstructor::EmitIsInteger(Inst *inst)
{
    auto result = CreateIsInteger(inst, GetInputValue(inst, 0));
    ValueMapAdd(inst, result);
    return true;
}

bool LLVMIrConstructor::EmitIsSafeInteger(Inst *inst)
{
    auto &ctx = func_->getContext();
    auto input = GetInputValue(inst, 0);
    ASSERT(input->getType()->isDoubleTy() || input->getType()->isFloatTy());
    auto isInteger = CreateIsInteger(inst, input);

    auto maxSafe = input->getType()->isDoubleTy() ? llvm::ConstantFP::get(builder_.getDoubleTy(), MaxIntAsExactDouble())
                                                  : llvm::ConstantFP::get(builder_.getFloatTy(), MaxIntAsExactFloat());

    auto initialBb = GetCurrentBasicBlock();
    auto isSafeIntegerBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "is_safe_integer"), func_);
    auto continueBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "is_safe_integer_continue"), func_);

    builder_.CreateCondBr(isInteger, isSafeIntegerBb, continueBb);

    SetCurrentBasicBlock(isSafeIntegerBb);
    // fabs(v) <= MaxSafeInteger
    auto inputAbs = builder_.CreateUnaryIntrinsic(llvm::Intrinsic::fabs, input);
    auto cmp = builder_.CreateFCmp(llvm::CmpInst::FCMP_OLE, inputAbs, maxSafe);
    builder_.CreateBr(continueBb);

    SetCurrentBasicBlock(continueBb);
    auto result = builder_.CreatePHI(builder_.getInt1Ty(), 2U);
    result->addIncoming(builder_.getInt1(false), initialBb);
    result->addIncoming(cmp, isSafeIntegerBb);

    ValueMapAdd(inst, result);
    return true;
}

bool LLVMIrConstructor::EmitRawBitcastToInt(Inst *inst)
{
    llvm::Value *input = GetInputValue(inst, 0);
    ASSERT_TYPE(input, builder_.getFloatTy());
    auto result = builder_.CreateBitCast(input, builder_.getInt32Ty());
    ValueMapAdd(inst, result);
    return true;
}

bool LLVMIrConstructor::EmitRawBitcastToLong(Inst *inst)
{
    llvm::Value *input = GetInputValue(inst, 0);
    ASSERT_TYPE(input, builder_.getDoubleTy());
    auto result = builder_.CreateBitCast(input, builder_.getInt64Ty());
    ValueMapAdd(inst, result);
    return true;
}

bool LLVMIrConstructor::EmitRawBitcastFromInt(Inst *inst)
{
    llvm::Value *input = GetInputValue(inst, 0);
    ASSERT_TYPE(input, builder_.getInt32Ty());
    auto result = builder_.CreateBitCast(input, builder_.getFloatTy());
    ValueMapAdd(inst, result);
    return true;
}

bool LLVMIrConstructor::EmitRawBitcastFromLong(Inst *inst)
{
    llvm::Value *input = GetInputValue(inst, 0);
    ASSERT_TYPE(input, builder_.getInt64Ty());
    auto result = builder_.CreateBitCast(input, builder_.getDoubleTy());
    ValueMapAdd(inst, result);
    return true;
}

bool LLVMIrConstructor::EmitStringGetCharsTlab(Inst *inst)
{
    auto offset = GetGraph()->GetRuntime()->GetArrayU16ClassPointerTlsOffset(GetGraph()->GetArch());
    auto klass = llvmbackend::runtime_calls::LoadTLSValue(&builder_, arkInterface_, offset, builder_.getPtrTy());
    auto eid = RuntimeInterface::EntrypointId::STRING_GET_CHARS_TLAB_COMPRESSED;
    auto result = CreateEntrypointCall(eid, inst,
                                       {GetInputValue(inst, 0), GetInputValue(inst, 1), GetInputValue(inst, 2), klass});
    ASSERT(result->getCallingConv() == llvm::CallingConv::C);
    result->setCallingConv(llvm::CallingConv::ArkFast4);
    result->addRetAttr(llvm::Attribute::NonNull);
    result->addRetAttr(llvm::Attribute::NoAlias);
    ValueMapAdd(inst, result);
    return true;
}

bool LLVMIrConstructor::EmitStringHashCode(Inst *inst)
{
    ASSERT(GetGraph()->GetRuntime()->IsCompressedStringsEnabled());
    auto string = GetInputValue(inst, 0);
    auto offset = coretypes::String::GetHashcodeOffset();
    auto gep = builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), string, offset);
    auto hashCode = builder_.CreateLoad(builder_.getInt32Ty(), gep);
    auto isZero = builder_.CreateICmpEQ(hashCode, llvm::Constant::getNullValue(hashCode->getType()));
    auto fastPath = GetCurrentBasicBlock();
    auto slowPath = llvm::BasicBlock::Create(func_->getContext(), "hash_code_slow_path", func_);
    auto continuation = llvm::BasicBlock::Create(func_->getContext(), "hash_code_continuation", func_);
    auto branchWeights = llvm::MDBuilder(func_->getContext())
                             .createBranchWeights(llvmbackend::Metadata::BranchWeights::UNLIKELY_BRANCH_WEIGHT,
                                                  llvmbackend::Metadata::BranchWeights::LIKELY_BRANCH_WEIGHT);
    builder_.CreateCondBr(isZero, slowPath, continuation, branchWeights);
    SetCurrentBasicBlock(slowPath);

    auto newHash = CreateEntrypointCall(RuntimeInterface::EntrypointId::STRING_HASH_CODE_COMPRESSED, inst, {string});
    ASSERT(newHash->getCallingConv() == llvm::CallingConv::C);
    newHash->setCallingConv(llvm::CallingConv::ArkFast1);
    builder_.CreateBr(continuation);
    SetCurrentBasicBlock(continuation);

    auto result = builder_.CreatePHI(hashCode->getType(), 2U);
    result->addIncoming(hashCode, fastPath);
    result->addIncoming(newHash, slowPath);
    ValueMapAdd(inst, result);

    return true;
}

bool LLVMIrConstructor::EmitWriteTlabStatsSafe(Inst *inst)
{
    auto addr = GetInputValue(inst, 0);
    auto size = GetInputValue(inst, 1);
    CreateEntrypointCall(RuntimeInterface::EntrypointId::WRITE_TLAB_STATS_NO_BRIDGE, inst, {addr, size});

    return true;
}

bool LLVMIrConstructor::EmitExpandU8U16(Inst *inst)
{
    auto input = GetInputValue(inst, 0);
    ASSERT(input->getType()->getScalarSizeInBits() == 32U);  // has to be f32

    auto srcTy = llvm::VectorType::get(builder_.getInt8Ty(), 4U, false);
    auto dstTy = llvm::VectorType::get(builder_.getInt16Ty(), 4U, false);

    auto val = builder_.CreateBitCast(input, srcTy);
    auto result = builder_.CreateZExt(val, dstTy);
    ValueMapAdd(inst, result);

    return true;
}

bool LLVMIrConstructor::EmitReverseHalfWords(Inst *inst)
{
    auto input = GetInputValue(inst, 0);
    ASSERT(input->getType()->getScalarSizeInBits() == 64U);  // has to be f64
    auto srcTy = llvm::VectorType::get(builder_.getInt16Ty(), 4U, false);
    auto val = builder_.CreateBitCast(input, srcTy);

    const llvm::SmallVector<int, 4> indices = {3, 2, 1, 0};
    auto result = builder_.CreateShuffleVector(val, indices);
    ValueMapAdd(inst, result);

    return true;
}

bool LLVMIrConstructor::EmitAtomicByteOr(Inst *inst)
{
    auto addr = GetInputValue(inst, 0);
    auto value = GetInputValue(inst, 1);
    auto byteVal = builder_.CreateTrunc(value, builder_.getInt8Ty());
    auto op = llvm::AtomicRMWInst::BinOp::Or;
    builder_.CreateAtomicRMW(op, addr, byteVal, llvm::MaybeAlign(0), llvm::AtomicOrdering::Monotonic);

    return true;
}

llvm::Value *LLVMIrConstructor::GetMappedValue(Inst *inst, DataType::Type type)
{
    ASSERT(inputMap_.count(inst) == 1);
    auto &typeMap = inputMap_.at(inst);
    ASSERT(typeMap.count(type) == 1);
    auto result = typeMap.at(type);
    ASSERT(result != nullptr);
    return result;
}

llvm::Value *LLVMIrConstructor::GetInputValue(Inst *inst, size_t index, bool skipCoerce)
{
    auto input = inst->GetInput(index).GetInst();
    auto type = inst->GetInputType(index);
    ASSERT(type != DataType::NO_TYPE);

    if (skipCoerce) {
        ASSERT(input->GetType() == DataType::UINT64 || input->GetType() == DataType::INT64);
        type = input->GetType();
    }

    if (input->IsConst()) {
        return GetInputValueFromConstant(input->CastToConstant(), type);
    }
    if (input->GetOpcode() == Opcode::NullPtr) {
        auto llvmType = GetExactType(DataType::REFERENCE);
        ASSERT(llvmType == builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));
        return llvm::Constant::getNullValue(llvmType);
    }
    return GetMappedValue(input, type);
}

llvm::Value *LLVMIrConstructor::GetInputValueFromConstant(ConstantInst *constant, DataType::Type pandaType)
{
    auto llvmType = GetExactType(pandaType);
    if (pandaType == DataType::FLOAT64) {
        double value = constant->GetDoubleValue();
        return llvm::ConstantFP::get(llvmType, value);
    }
    if (pandaType == DataType::FLOAT32) {
        float value = constant->GetFloatValue();
        return llvm::ConstantFP::get(llvmType, value);
    }
    if (pandaType == DataType::POINTER) {
        auto cval = static_cast<int64_t>(constant->GetIntValue());
        auto integer = builder_.getInt64(cval);
        return builder_.CreateIntToPtr(integer, builder_.getPtrTy());
    }
    if (DataType::IsTypeNumeric(pandaType)) {
        auto isSigned = DataType::IsTypeSigned(pandaType);
        auto cval = static_cast<int64_t>(constant->GetIntValue());
        return llvm::ConstantInt::get(llvmType, cval, isSigned);
    }
    if (DataType::IsReference(pandaType) && constant->GetRawValue() == 0) {
        return llvm::Constant::getNullValue(llvmType);
    }
    UNREACHABLE();
}

// Initializers. BuildIr calls them

void LLVMIrConstructor::BuildBasicBlocks(Marker normal)
{
    auto &context = func_->getContext();
    for (auto block : graph_->GetBlocksRPO()) {
        if (block->IsEndBlock()) {
            continue;
        }
        if (!block->IsMarked(normal)) {
            continue;
        }
        auto bb = llvm::BasicBlock::Create(context, llvm::StringRef("bb") + llvm::Twine(block->GetId()), func_);
        AddBlock(block, bb);
        // Checking that irtoc handler contains a return instruction
        if (!graph_->GetMode().IsInterpreter()) {
            continue;
        }
        for (auto inst : block->AllInsts()) {
            if (inst->IsIntrinsic() && inst->CastToIntrinsic()->GetIntrinsicId() ==
                                           RuntimeInterface::IntrinsicId::INTRINSIC_INTERPRETER_RETURN) {
                arkInterface_->AppendIrtocReturnHandler(func_->getName());
            }
        }
    }
}

void LLVMIrConstructor::BuildInstructions(Marker normal)
{
    for (auto block : graph_->GetBlocksRPO()) {
        if (block->IsEndBlock() || !block->IsMarked(normal)) {
            continue;
        }
        SetCurrentBasicBlock(GetTailBlock(block));
        for (auto inst : block->AllInsts()) {
            auto bb = GetCurrentBasicBlock();
            if (!bb->empty() && llvm::isa<llvm::UnreachableInst>(*(bb->rbegin()))) {
                break;
            }
            VisitInstruction(inst);
        }

        if (block->IsTryBegin()) {
            ASSERT(block->GetSuccsBlocks().size() > 1);
            ASSERT(block->GetSuccessor(0)->IsMarked(normal) && !block->GetSuccessor(1)->IsMarked(normal));
            ASSERT(!block->GetLastInst()->IsControlFlow());
            builder_.CreateBr(GetHeadBlock(block->GetSuccessor(0)));
        }
        if (((block->GetSuccsBlocks().size() == 1 && !block->GetSuccessor(0)->IsEndBlock()) || block->IsTryEnd()) &&
            block->GetSuccessor(0)->IsMarked(normal)) {
            ASSERT(block->IsTryEnd() ? !block->GetSuccessor(1)->IsMarked(normal) : true);
            builder_.CreateBr(GetHeadBlock(block->GetSuccessor(0)));
        }
        ReplaceTailBlock(block, GetCurrentBasicBlock());
    }
}

void LLVMIrConstructor::FillPhiInputs(BasicBlock *block, Marker normal)
{
    if (block->IsStartBlock() || block->IsEndBlock() || !block->IsMarked(normal)) {
        return;
    }
    for (auto inst : block->PhiInsts()) {
        auto phi = llvm::cast<llvm::PHINode>(GetMappedValue(inst, inst->GetType()));
        for (size_t i = 0; i < inst->GetInputsCount(); i++) {
            auto inputBlock = inst->CastToPhi()->GetPhiInputBb(i);
            if (!inputBlock->IsMarked(normal)) {
                continue;
            }

            auto input = GetInputValue(inst, i);
            phi->addIncoming(input, GetTailBlock(inputBlock));
        }
    }
}

// Creator functions for internal usage

llvm::CallInst *LLVMIrConstructor::CreateEntrypointCall(RuntimeInterface::EntrypointId eid, Inst *inst,
                                                        llvm::ArrayRef<llvm::Value *> args)
{
    arkInterface_->GetOrCreateRuntimeFunctionType(func_->getContext(), func_->getParent(),
                                                  LLVMArkInterface::RuntimeCallType::ENTRYPOINT,
                                                  static_cast<LLVMArkInterface::EntrypointId>(eid));

    // Sanity assert to not misuse this scenario
    ASSERT(inst != nullptr);

    llvm::CallInst *call;
    auto threadReg = GetThreadRegValue();
    if (GetGraph()->SupportManagedCode() && (inst->CanThrow() || inst->CanDeoptimize())) {
        bool noReturn = GetGraph()->GetRuntime()->IsEntrypointNoreturn(eid);
        call = llvmbackend::runtime_calls::CreateEntrypointCallCommon(
            &builder_, threadReg, arkInterface_, static_cast<llvmbackend::runtime_calls::EntrypointId>(eid), args,
            CreateSaveStateBundle(inst, noReturn));
    } else {
        call = llvmbackend::runtime_calls::CreateEntrypointCallCommon(
            &builder_, threadReg, arkInterface_, static_cast<llvmbackend::runtime_calls::EntrypointId>(eid), args);
    }
    if (inst->RequireState()) {
        WrapArkCall(inst, call);
    }
    return call;
}

llvm::CallInst *LLVMIrConstructor::CreateIntrinsicCall(Inst *inst)
{
    auto entryId = inst->CastToIntrinsic()->GetIntrinsicId();
    auto rtFunctionTy = arkInterface_->GetOrCreateRuntimeFunctionType(
        func_->getContext(), func_->getParent(), LLVMArkInterface::RuntimeCallType::INTRINSIC,
        static_cast<LLVMArkInterface::EntrypointId>(entryId));
    auto arguments = GetIntrinsicArguments(rtFunctionTy, inst->CastToIntrinsic());
    return CreateIntrinsicCall(inst, entryId, arguments);
}

llvm::CallInst *LLVMIrConstructor::CreateIntrinsicCall(Inst *inst, RuntimeInterface::IntrinsicId entryId,
                                                       llvm::ArrayRef<llvm::Value *> arguments)
{
    auto rtFunctionTy = arkInterface_->GetOrCreateRuntimeFunctionType(
        func_->getContext(), func_->getParent(), LLVMArkInterface::RuntimeCallType::INTRINSIC,
        static_cast<LLVMArkInterface::EntrypointId>(entryId));
    auto rtFunctionName = arkInterface_->GetRuntimeFunctionName(LLVMArkInterface::RuntimeCallType::INTRINSIC,
                                                                static_cast<LLVMArkInterface::EntrypointId>(entryId));
    auto intrinsicOffset = static_cast<int>(entryId);
    auto callee = llvmbackend::runtime_calls::GetPandaRuntimeFunctionCallee(intrinsicOffset, rtFunctionTy, &builder_,
                                                                            rtFunctionName);
    llvm::CallInst *result;
    if (inst->CanThrow()) {
        ASSERT_PRINT(inst->GetSaveState() != nullptr, "Intrinsic with can_throw does not have a savestate");
        result = builder_.CreateCall(callee, arguments, CreateSaveStateBundle(inst));
    } else {
        result = builder_.CreateCall(callee, arguments);
    }
    SetIntrinsicParamAttrs(result, inst->CastToIntrinsic(), arguments);

    if (inst->RequireState()) {
        WrapArkCall(inst, result);
    }
    if (NeedSafePointAfterIntrinsic(entryId) && g_options.IsCompilerUseSafepoint()) {
        result->addFnAttr(llvm::Attribute::get(result->getContext(), "needs-extra-safepoint"));
        result->getFunction()->addFnAttr("needs-extra-safepoint");
    }

    return result;
}

// Helper function. Regardless of where we use `alloca` to pass args, we want to do all of them in the
// first basic block. This should allow LLVM to combine allocas into prologue
llvm::Value *LLVMIrConstructor::CreateAllocaForArgs(llvm::Type *type, uint32_t arraySize)
{
    auto currentBb = GetCurrentBasicBlock();
    auto &firstBb = func_->getEntryBlock();
    auto inst = firstBb.getFirstNonPHI();
    builder_.SetInsertPoint(inst);
    llvm::AllocaInst *result;

    if (llvm::isa<llvm::AllocaInst>(inst)) {
        auto alloca = llvm::cast<llvm::AllocaInst>(inst);
        ASSERT(alloca->getAllocatedType() == type);
        ASSERT(llvm::isa<llvm::ConstantInt>(alloca->getArraySize()));
        auto allocaSize = llvm::cast<llvm::ConstantInt>(alloca->getArraySize())->getZExtValue();
        if (allocaSize < arraySize) {
            alloca->setOperand(0, builder_.getInt32(arraySize));
        }
        result = alloca;
    } else {
        result = builder_.CreateAlloca(type, builder_.getInt32(arraySize), "call_arg_buffer");
    }

    SetCurrentBasicBlock(currentBb);
    return result;
}

llvm::CallInst *LLVMIrConstructor::CreateFastPathCall(Inst *inst, RuntimeInterface::EntrypointId eid,
                                                      llvm::ArrayRef<llvm::Value *> args)
{
    auto call = CreateEntrypointCall(eid, inst, args);
    ASSERT(call->getCallingConv() == llvm::CallingConv::C);
    call->setCallingConv(GetFastPathCallingConv(args.size()));
    return call;
}

// IsInstance Helpers

llvm::Value *LLVMIrConstructor::CreateIsInstanceEntrypointCall(Inst *inst)
{
    auto object = GetInputValue(inst, 0);
    auto klass = GetInputValue(inst, 1);
    return CreateEntrypointCall(RuntimeInterface::EntrypointId::IS_INSTANCE, inst, {object, klass});
}

llvm::Value *LLVMIrConstructor::CreateIsInstanceObject(llvm::Value *klassObj)
{
    auto typeOffset = GetGraph()->GetRuntime()->GetClassTypeOffset(GetGraph()->GetArch());
    auto typeMask = GetGraph()->GetRuntime()->GetReferenceTypeMask();
    auto typePtr = builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), klassObj, typeOffset);
    auto typeLdr = builder_.CreateLoad(builder_.getInt8Ty(), typePtr);
    auto cmpLocal =
        builder_.CreateICmpEQ(builder_.getInt32(typeMask), builder_.CreateZExt(typeLdr, builder_.getInt32Ty()));
    return builder_.CreateZExt(cmpLocal, builder_.getInt8Ty(), "isinstance_object_out");
}

llvm::Value *LLVMIrConstructor::CreateIsInstanceOther(Inst *inst, llvm::Value *klassObj, llvm::Value *klassId)
{
    auto initialBb = GetCurrentBasicBlock();
    auto &ctx = func_->getContext();
    auto loopHeaderBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "other_loop_h"), func_);
    auto loopBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "other_loop"), func_);
    auto outBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "other_out"), func_);
    builder_.CreateBr(loopHeaderBb);

    SetCurrentBasicBlock(loopHeaderBb);
    auto typeOffset = GetGraph()->GetRuntime()->GetClassBaseOffset(GetGraph()->GetArch());
    auto loopPhi = builder_.CreatePHI(builder_.getPtrTy(), 2U, "loop_in");
    auto typePtr = builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), loopPhi, typeOffset);
    auto typeLdr = builder_.CreateLoad(builder_.getPtrTy(), typePtr);
    auto cmpLocal = builder_.CreateIsNotNull(typeLdr);
    loopPhi->addIncoming(klassObj, initialBb);
    loopPhi->addIncoming(typeLdr, loopBb);
    builder_.CreateCondBr(cmpLocal, loopBb, outBb);

    SetCurrentBasicBlock(loopBb);
    cmpLocal = builder_.CreateICmpEQ(typeLdr, klassId);
    builder_.CreateCondBr(cmpLocal, outBb, loopHeaderBb);

    SetCurrentBasicBlock(outBb);
    auto outPhi = builder_.CreatePHI(builder_.getInt8Ty(), 2U, "isinstance_other_out");
    outPhi->addIncoming(builder_.getInt8(1), loopBb);
    outPhi->addIncoming(builder_.getInt8(0), loopHeaderBb);
    return outPhi;
}

llvm::Value *LLVMIrConstructor::CreateIsInstanceArray(Inst *inst, llvm::Value *klassObj, llvm::Value *klassId)
{
    auto &ctx = func_->getContext();
    auto initialBb = GetCurrentBasicBlock();
    auto secondLoadBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "array_second_load"), func_);
    auto slowPath = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "array_slow_path"), func_);
    auto outBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "array_out"), func_);

    auto componentOffset = GetGraph()->GetRuntime()->GetClassComponentTypeOffset(GetGraph()->GetArch());
    auto typePtrObj = builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), klassObj, componentOffset);
    auto typeLdrObj = builder_.CreateLoad(builder_.getPtrTy(), typePtrObj);
    auto cmpLocal = builder_.CreateIsNotNull(typeLdrObj);
    builder_.CreateCondBr(cmpLocal, secondLoadBb, outBb);

    SetCurrentBasicBlock(secondLoadBb);
    auto typePtrKlass = builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), klassId, componentOffset);
    auto typeLdrKlass = builder_.CreateLoad(builder_.getPtrTy(), typePtrKlass);
    cmpLocal = builder_.CreateICmpEQ(typeLdrObj, typeLdrKlass);
    auto branchWeights = llvm::MDBuilder(ctx).createBranchWeights(
        llvmbackend::Metadata::BranchWeights::LIKELY_BRANCH_WEIGHT,     // if other comparisons are enough
        llvmbackend::Metadata::BranchWeights::UNLIKELY_BRANCH_WEIGHT);  // else
    builder_.CreateCondBr(cmpLocal, outBb, slowPath, branchWeights);

    SetCurrentBasicBlock(slowPath);
    auto slowPathResult = CreateIsInstanceEntrypointCall(inst);
    builder_.CreateBr(outBb);

    SetCurrentBasicBlock(outBb);
    auto outPhi = builder_.CreatePHI(builder_.getInt8Ty(), 3U, "isinstance_array_out");
    outPhi->addIncoming(builder_.getInt8(0), initialBb);
    outPhi->addIncoming(builder_.getInt8(1), secondLoadBb);
    outPhi->addIncoming(slowPathResult, slowPath);
    return outPhi;
}

llvm::Value *LLVMIrConstructor::CreateIsInstanceArrayObject(Inst *inst, llvm::Value *klassObj)
{
    auto &ctx = func_->getContext();
    auto initialBb = GetCurrentBasicBlock();
    auto checkMaskBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "array_object_check_mask"), func_);
    auto outBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "array_object_out"), func_);

    auto componentOffset = GetGraph()->GetRuntime()->GetClassComponentTypeOffset(GetGraph()->GetArch());
    auto typePtr = builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), klassObj, componentOffset);
    auto typeLdr = builder_.CreateLoad(builder_.getPtrTy(), typePtr);
    auto cmpLocal = builder_.CreateIsNotNull(typeLdr);
    builder_.CreateCondBr(cmpLocal, checkMaskBb, outBb);

    SetCurrentBasicBlock(checkMaskBb);
    auto typeOffset = GetGraph()->GetRuntime()->GetClassTypeOffset(GetGraph()->GetArch());
    auto typeMask = GetGraph()->GetRuntime()->GetReferenceTypeMask();
    auto typePtrElem = builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), typeLdr, typeOffset);
    auto typeLdrElem = builder_.CreateLoad(builder_.getInt8Ty(), typePtrElem);
    cmpLocal =
        builder_.CreateICmpEQ(builder_.getInt32(typeMask), builder_.CreateZExt(typeLdrElem, builder_.getInt32Ty()));
    auto cmpExt = builder_.CreateZExt(cmpLocal, builder_.getInt8Ty());
    builder_.CreateBr(outBb);

    SetCurrentBasicBlock(outBb);
    auto outPhi = builder_.CreatePHI(builder_.getInt8Ty(), 2U, "isinstance_array_object_out");
    outPhi->addIncoming(builder_.getInt8(0), initialBb);
    outPhi->addIncoming(cmpExt, checkMaskBb);
    return outPhi;
}

llvm::Value *LLVMIrConstructor::CreateIsInstanceInnerBlock(Inst *inst, llvm::Value *klassObj, llvm::Value *klassId)
{
    auto klassType = inst->CastToIsInstance()->GetClassType();
    switch (klassType) {
        case ClassType::OBJECT_CLASS:
            return CreateIsInstanceObject(klassObj);
        case ClassType::OTHER_CLASS:
            return CreateIsInstanceOther(inst, klassObj, klassId);
        case ClassType::ARRAY_CLASS:
            return CreateIsInstanceArray(inst, klassObj, klassId);
        case ClassType::ARRAY_OBJECT_CLASS:
            return CreateIsInstanceArrayObject(inst, klassObj);
        case ClassType::INTERFACE_CLASS:
            return CreateIsInstanceEntrypointCall(inst);
        default:
            UNREACHABLE();
    }
}

// IsInstance Helpers End

// CheckCast Helpers

void LLVMIrConstructor::CreateCheckCastEntrypointCall(Inst *inst)
{
    auto object = GetInputValue(inst, 0);
    auto klass = GetInputValue(inst, 1);
    if (inst->CanDeoptimize()) {
        auto call = CreateEntrypointCall(RuntimeInterface::EntrypointId::CHECK_CAST_DEOPTIMIZE, inst, {object, klass});
        call->addFnAttr(llvm::Attribute::get(call->getContext(), "may-deoptimize"));
    } else {
        CreateEntrypointCall(RuntimeInterface::EntrypointId::CHECK_CAST, inst, {object, klass});
    }
}

void LLVMIrConstructor::CreateCheckCastObject(Inst *inst, llvm::Value *klassObj, llvm::Value *klassId)
{
    auto typeOffset = GetGraph()->GetRuntime()->GetClassTypeOffset(GetGraph()->GetArch());
    auto typeMask = GetGraph()->GetRuntime()->GetReferenceTypeMask();
    auto typePtr = builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), klassObj, typeOffset);
    auto typeLdr = builder_.CreateLoad(builder_.getInt8Ty(), typePtr);
    auto src = GetInputValue(inst, 0);
    auto zext = builder_.CreateZExt(typeLdr, builder_.getInt32Ty());
    auto deoptimize = builder_.CreateICmpNE(builder_.getInt32(typeMask), zext);

    auto exception = RuntimeInterface::EntrypointId::CLASS_CAST_EXCEPTION;
    CreateDeoptimizationBranch(inst, deoptimize, exception, {klassId, src});
}

void LLVMIrConstructor::CreateCheckCastOther(Inst *inst, llvm::Value *klassObj, llvm::Value *klassId)
{
    auto initialBb = GetCurrentBasicBlock();
    auto src = GetInputValue(inst, 0);

    auto &ctx = func_->getContext();
    auto loopHeaderBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "other_loop_h"), func_);
    auto loopBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "other_loop"), func_);
    auto outBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "other_out"), func_);
    builder_.CreateBr(loopHeaderBb);

    SetCurrentBasicBlock(loopHeaderBb);
    auto typeOffset = GetGraph()->GetRuntime()->GetClassBaseOffset(GetGraph()->GetArch());
    auto loopPhi = builder_.CreatePHI(builder_.getPtrTy(), 2U, "loop_in");
    auto typePtr = builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), loopPhi, typeOffset);
    auto typeLdr = builder_.CreateLoad(builder_.getPtrTy(), typePtr);
    auto deoptimize = builder_.CreateIsNull(typeLdr);
    loopPhi->addIncoming(klassObj, initialBb);
    loopPhi->addIncoming(typeLdr, loopBb);

    auto exception = RuntimeInterface::EntrypointId::CLASS_CAST_EXCEPTION;
    CreateDeoptimizationBranch(inst, deoptimize, exception, {klassId, src});
    builder_.CreateBr(loopBb);

    SetCurrentBasicBlock(loopBb);
    auto cmp = builder_.CreateICmpEQ(typeLdr, klassId);
    builder_.CreateCondBr(cmp, outBb, loopHeaderBb);

    SetCurrentBasicBlock(outBb);
}

void LLVMIrConstructor::CreateCheckCastArray(Inst *inst, llvm::Value *klassObj, llvm::Value *klassId)
{
    auto src = GetInputValue(inst, 0);

    auto &ctx = func_->getContext();
    auto slowPath = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "array_slow_path"), func_);
    auto outBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "array_out"), func_);

    auto componentOffset = GetGraph()->GetRuntime()->GetClassComponentTypeOffset(GetGraph()->GetArch());
    auto typePtrObj = builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), klassObj, componentOffset);
    auto typeLdrObj = builder_.CreateLoad(builder_.getPtrTy(), typePtrObj);

    auto deoptimize = builder_.CreateIsNull(typeLdrObj);
    auto exception = RuntimeInterface::EntrypointId::CLASS_CAST_EXCEPTION;
    CreateDeoptimizationBranch(inst, deoptimize, exception, {klassId, src});

    auto typePtrKlass = builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), klassId, componentOffset);
    auto typeLdrKlass = builder_.CreateLoad(builder_.getPtrTy(), typePtrKlass);
    auto cmpLocal = builder_.CreateICmpEQ(typeLdrObj, typeLdrKlass);
    auto branchWeights = llvm::MDBuilder(ctx).createBranchWeights(
        llvmbackend::Metadata::BranchWeights::LIKELY_BRANCH_WEIGHT,     // if other comparisons are enough
        llvmbackend::Metadata::BranchWeights::UNLIKELY_BRANCH_WEIGHT);  // else
    builder_.CreateCondBr(cmpLocal, outBb, slowPath, branchWeights);

    SetCurrentBasicBlock(slowPath);
    CreateCheckCastEntrypointCall(inst);
    builder_.CreateBr(outBb);

    SetCurrentBasicBlock(outBb);
}

void LLVMIrConstructor::CreateCheckCastArrayObject(Inst *inst, llvm::Value *klassObj, llvm::Value *klassId)
{
    auto src = GetInputValue(inst, 0);

    auto componentOffset = GetGraph()->GetRuntime()->GetClassComponentTypeOffset(GetGraph()->GetArch());
    auto typePtr = builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), klassObj, componentOffset);
    auto typeLdr = builder_.CreateLoad(builder_.getPtrTy(), typePtr);

    auto deoptimize = builder_.CreateIsNull(typeLdr);
    auto exception = RuntimeInterface::EntrypointId::CLASS_CAST_EXCEPTION;
    CreateDeoptimizationBranch(inst, deoptimize, exception, {klassId, src});

    auto typeOffset = GetGraph()->GetRuntime()->GetClassTypeOffset(GetGraph()->GetArch());
    auto typeMask = GetGraph()->GetRuntime()->GetReferenceTypeMask();
    auto typePtrElem = builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), typeLdr, typeOffset);
    auto typeLdrElem = builder_.CreateLoad(builder_.getInt8Ty(), typePtrElem);
    deoptimize =
        builder_.CreateICmpNE(builder_.getInt32(typeMask), builder_.CreateZExt(typeLdrElem, builder_.getInt32Ty()));
    CreateDeoptimizationBranch(inst, deoptimize, exception, {klassId, src});
}

void LLVMIrConstructor::CreateCheckCastInner(Inst *inst, llvm::Value *klassObj, llvm::Value *klassId)
{
    auto klassType = inst->CastToCheckCast()->GetClassType();
    switch (klassType) {
        case ClassType::OBJECT_CLASS:
            CreateCheckCastObject(inst, klassObj, klassId);
            break;
        case ClassType::OTHER_CLASS:
            CreateCheckCastOther(inst, klassObj, klassId);
            break;
        case ClassType::ARRAY_CLASS:
            CreateCheckCastArray(inst, klassObj, klassId);
            break;
        case ClassType::ARRAY_OBJECT_CLASS:
            CreateCheckCastArrayObject(inst, klassObj, klassId);
            break;
        case ClassType::INTERFACE_CLASS:
        default:
            UNREACHABLE();
    }
}

// CheckCast Helpers End

void LLVMIrConstructor::CreateInterpreterReturnRestoreRegs(RegMask &regMask, size_t offset, bool fp)
{
    int32_t slotSize = PointerSize(GetGraph()->GetArch());
    int32_t dslotSize = slotSize * 2U;
    int32_t totalSize = regMask.count() * slotSize;
    auto startRegOffset = offset * DOUBLE_WORD_SIZE_BYTES;
    auto endRegOffset = startRegOffset + std::max(0, totalSize - dslotSize);

    constexpr uint32_t MAX_REPR_VAL = 504U;
    bool representable = startRegOffset <= MAX_REPR_VAL && (startRegOffset & 0x7U) == 0 &&
                         endRegOffset <= MAX_REPR_VAL && (endRegOffset & 0x7U) == 0;

    std::string baseReg = representable ? "sp" : "x16";
    if (!representable) {
        CreateInt32ImmAsm(&builder_,
                          std::string("add  x16, sp, $0").append(LLVMArkInterface::PATCH_STACK_ADJUSTMENT_COMMENT),
                          startRegOffset);
        startRegOffset = 0;
    }

    while (regMask.count() > 0) {
        std::string asmString = regMask.count() / 2U > 0 ? "ldp " : "ldr ";
        auto first = regMask.GetMinRegister();
        asmString += (fp ? "d" : "x") + std::to_string(first);
        regMask ^= 1U << first;
        if (regMask.count() > 0) {
            asmString += ", ";
            auto second = regMask.GetMinRegister();
            asmString += (fp ? "d" : "x") + std::to_string(second);
            regMask ^= 1U << second;
        }
        asmString += ", [";
        asmString += baseReg;
        asmString += ", $0]";
        if (representable) {
            asmString += LLVMArkInterface::PATCH_STACK_ADJUSTMENT_COMMENT;
        }
        CreateInt32ImmAsm(&builder_, asmString, startRegOffset);
        startRegOffset += dslotSize;
    }
}

llvm::Value *LLVMIrConstructor::CreateLoadClassById(Inst *inst, uint32_t typeId, bool init)
{
    auto builtin = init ? LoadInitClass(func_->getParent()) : LoadClass(func_->getParent());
    auto slotIdVal = builder_.getInt32(arkInterface_->GetClassIndexInAotGot(GetGraph()->GetAotData(), typeId, init));

    // remember two functions, later we will use it in panda_runtime_lowering pass
    arkInterface_->GetOrCreateRuntimeFunctionType(
        func_->getContext(), func_->getParent(), LLVMArkInterface::RuntimeCallType::ENTRYPOINT,
        static_cast<LLVMArkInterface::EntrypointId>(RuntimeInterface::EntrypointId::CLASS_RESOLVER));
    arkInterface_->GetOrCreateRuntimeFunctionType(
        func_->getContext(), func_->getParent(), LLVMArkInterface::RuntimeCallType::ENTRYPOINT,
        static_cast<LLVMArkInterface::EntrypointId>(RuntimeInterface::EntrypointId::CLASS_INIT_RESOLVER));

    auto callInst = builder_.CreateCall(builtin, {builder_.getInt64(typeId), slotIdVal}, CreateSaveStateBundle(inst));
    WrapArkCall(inst, callInst);
    return callInst;
}

llvm::Value *LLVMIrConstructor::CreateBinaryOp(Inst *inst, llvm::Instruction::BinaryOps opcode)
{
    llvm::Value *x = GetInputValue(inst, 0);
    llvm::Value *y = GetInputValue(inst, 1);

    if (x->getType()->isPointerTy()) {
        if (y->getType()->isPointerTy()) {
            ASSERT(opcode == llvm::Instruction::Sub);
            x = builder_.CreatePtrToInt(x, builder_.getInt64Ty());
            y = builder_.CreatePtrToInt(y, builder_.getInt64Ty());
            return builder_.CreateBinOp(opcode, x, y);
        }
        if (y->getType()->isIntegerTy()) {
            ASSERT(opcode == llvm::Instruction::Add);
            ASSERT(x->getType()->isPointerTy());
            return builder_.CreateInBoundsGEP(builder_.getInt8Ty(), x, y);
        }
        UNREACHABLE();
    }
    if (IsTypeNumeric(inst->GetType())) {
        // Peephole can remove casts and instead put a constant with the wrong type
        // so we need to create them here.
        x = CoerceValue(x, inst->GetInputType(0), inst->GetType());
        y = CoerceValue(y, inst->GetInputType(1), inst->GetType());
    }
    return builder_.CreateBinOp(opcode, x, y);
}

llvm::Value *LLVMIrConstructor::CreateBinaryImmOp(Inst *inst, llvm::Instruction::BinaryOps opcode, uint64_t c)
{
    ASSERT(IsTypeNumeric(inst->GetType()));
    llvm::Value *x = GetInputValue(inst, 0);
    if (x->getType()->isPointerTy()) {
        ASSERT(x->getType()->isPointerTy());
        ASSERT(opcode == llvm::Instruction::Add || opcode == llvm::Instruction::Sub);
        if (opcode == llvm::Instruction::Sub) {
            c = -c;
        }
        return builder_.CreateConstInBoundsGEP1_64(builder_.getInt8Ty(), x, c);
    }
    llvm::Value *y = CoerceValue(builder_.getInt64(c), DataType::INT64, inst->GetType());
    return builder_.CreateBinOp(opcode, x, y);
}

llvm::Value *LLVMIrConstructor::CreateShiftOp(Inst *inst, llvm::Instruction::BinaryOps opcode)
{
    llvm::Value *x = GetInputValue(inst, 0);
    llvm::Value *y = GetInputValue(inst, 1);
    auto targetType = inst->GetType();
    bool target64 = (targetType == DataType::UINT64) || (targetType == DataType::INT64);
    auto constexpr SHIFT32_RANGE = 0x1f;
    auto constexpr SHIFT64_RANGE = 0x3f;

    y = builder_.CreateBinOp(llvm::Instruction::And, y,
                             llvm::ConstantInt::get(y->getType(), target64 ? SHIFT64_RANGE : SHIFT32_RANGE));

    return builder_.CreateBinOp(opcode, x, y);
}

llvm::Value *LLVMIrConstructor::CreateSignDivMod(Inst *inst, llvm::Instruction::BinaryOps opcode)
{
    ASSERT(opcode == llvm::Instruction::SDiv || opcode == llvm::Instruction::SRem);
    llvm::Value *x = GetInputValue(inst, 0);
    llvm::Value *y = GetInputValue(inst, 1);
    auto &ctx = func_->getContext();
    auto eqM1 = builder_.CreateICmpEQ(y, llvm::ConstantInt::get(y->getType(), -1));
    auto m1Result = opcode == llvm::Instruction::SDiv ? builder_.CreateNeg(x) : llvm::ConstantInt::get(y->getType(), 0);

    // Select for AArch64, as 'sdiv' correctly handles the INT_MIN / -1 case
    if (GetGraph()->GetArch() == Arch::AARCH64) {
        auto result = builder_.CreateBinOp(opcode, x, y);
        auto selectVal = builder_.CreateSelect(eqM1, m1Result, result);
        if (auto selectInst = llvm::dyn_cast<llvm::SelectInst>(selectVal)) {
            auto *metadata = llvm::MDNode::get(ctx, {});
            auto sdiv = ark::llvmbackend::LLVMArkInterface::AARCH64_SDIV_INST;
            selectInst->setMetadata(sdiv, metadata);
        }
        return selectVal;
    }

    // X86_64 solution with control flow
    auto currBb = GetCurrentBasicBlock();
    auto notM1Bb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "divmod_normal"), func_);
    auto contBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "divmod_cont"), func_);
    builder_.CreateCondBr(eqM1, contBb, notM1Bb);

    SetCurrentBasicBlock(notM1Bb);
    auto result = builder_.CreateBinOp(opcode, x, y);
    builder_.CreateBr(contBb);

    SetCurrentBasicBlock(contBb);
    auto resultPhi = builder_.CreatePHI(y->getType(), 2U);
    resultPhi->addIncoming(m1Result, currBb);
    resultPhi->addIncoming(result, notM1Bb);
    return resultPhi;
}

llvm::Value *LLVMIrConstructor::CreateFloatComparison(CmpInst *cmpInst, llvm::Value *x, llvm::Value *y)
{
    // if x is less than y then return -1
    // else return zero extend of (x > y)
    llvm::CmpInst::Predicate greaterThanPredicate;
    llvm::CmpInst::Predicate lessThanPredicate;
    if (cmpInst->IsFcmpg()) {
        // if x or y is nan then greaterThanPredicate yields true
        greaterThanPredicate = llvm::CmpInst::FCMP_UGT;
        lessThanPredicate = llvm::CmpInst::FCMP_OLT;
    } else if (cmpInst->IsFcmpl()) {
        greaterThanPredicate = llvm::CmpInst::FCMP_OGT;
        // if x or y is nan then lessThanPredicate yields true
        lessThanPredicate = llvm::CmpInst::FCMP_ULT;
    } else {
        ASSERT_PRINT(false, "cmpInst must be either Fcmpg, or Fcmpl");
        UNREACHABLE();
    }
    // x > y || (inst == Fcmpg && (x == NaN || y == NaN))
    auto greaterThan = builder_.CreateFCmp(greaterThanPredicate, x, y);
    // x < y || (inst == Fcmpl && (x == NaN || y == NaN))
    auto lessThan = builder_.CreateFCmp(lessThanPredicate, x, y);
    auto comparison = builder_.CreateZExt(greaterThan, builder_.getInt32Ty());
    auto negativeOne = builder_.getInt32(-1);
    return builder_.CreateSelect(lessThan, negativeOne, comparison);
}

llvm::Value *LLVMIrConstructor::CreateIntegerComparison(CmpInst *inst, llvm::Value *x, llvm::Value *y)
{
    ASSERT(x->getType() == y->getType());
    llvm::Value *greaterThan;
    llvm::Value *lessThan;

    if (DataType::IsTypeSigned(inst->GetOperandsType())) {
        greaterThan = builder_.CreateICmpSGT(x, y);
        lessThan = builder_.CreateICmpSLT(x, y);
    } else {
        greaterThan = builder_.CreateICmpUGT(x, y);
        lessThan = builder_.CreateICmpULT(x, y);
    }
    auto castComparisonResult = builder_.CreateZExt(greaterThan, builder_.getInt32Ty());
    auto negativeOne = builder_.getInt32(-1);
    return builder_.CreateSelect(lessThan, negativeOne, castComparisonResult);
}

llvm::Value *LLVMIrConstructor::CreateNewArrayWithRuntime(Inst *inst)
{
    auto type = GetInputValue(inst, 0);
    auto size = ToSizeT(GetInputValue(inst, 1));
    auto eid = RuntimeInterface::EntrypointId::CREATE_ARRAY;
    auto result = CreateEntrypointCall(eid, inst, {type, size});
    MarkAsAllocation(result);
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        result->addFnAttr(llvm::Attribute::get(result->getContext(), "needs-mem-barrier"));
    }
    return result;
}

llvm::Value *LLVMIrConstructor::CreateNewObjectWithRuntime(Inst *inst)
{
    auto initClass = GetInputValue(inst, 0);
    auto eid = RuntimeInterface::EntrypointId::CREATE_OBJECT_BY_CLASS;
    auto result = CreateEntrypointCall(eid, inst, {initClass});
    auto srcInst = inst->GetInput(0).GetInst();
    if (srcInst->GetOpcode() != Opcode::LoadAndInitClass ||
        GetGraph()->GetRuntime()->CanUseTlabForClass(srcInst->CastToLoadAndInitClass()->GetClass())) {
        MarkAsAllocation(result);
    }
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        result->addFnAttr(llvm::Attribute::get(result->getContext(), "needs-mem-barrier"));
    }
    return result;
}

llvm::Value *LLVMIrConstructor::CreateResolveVirtualCallBuiltin(Inst *inst, llvm::Value *thiz, uint32_t methodId)
{
    ASSERT(thiz->getType()->isPointerTy());

    auto builtin = ResolveVirtual(func_->getParent());
    arkInterface_->GetOrCreateRuntimeFunctionType(
        func_->getContext(), func_->getParent(), LLVMArkInterface::RuntimeCallType::ENTRYPOINT,
        static_cast<LLVMArkInterface::EntrypointId>(RuntimeInterface::EntrypointId::RESOLVE_VIRTUAL_CALL_AOT));
    arkInterface_->GetOrCreateRuntimeFunctionType(
        func_->getContext(), func_->getParent(), LLVMArkInterface::RuntimeCallType::ENTRYPOINT,
        static_cast<LLVMArkInterface::EntrypointId>(RuntimeInterface::EntrypointId::INTF_INLINE_CACHE));

    auto zero = builder_.getInt64(0);
    auto arrayType = llvm::ArrayType::get(builder_.getInt64Ty(), 0);
    auto offset = builder_.CreateIntToPtr(zero, arrayType->getPointerTo());
    auto callInst =
        builder_.CreateCall(builtin, {thiz, ToSizeT(builder_.getInt64(methodId)), offset}, CreateSaveStateBundle(inst));
    WrapArkCall(inst, callInst);
    return builder_.CreateIntToPtr(callInst, builder_.getPtrTy());
}

llvm::Value *LLVMIrConstructor::CreateLoadManagedClassFromClass(llvm::Value *klass)
{
    ASSERT(klass->getType()->isPointerTy());
    auto dataOff = GetGraph()->GetRuntime()->GetManagedClassOffset(GetGraph()->GetArch());
    auto ptrData = builder_.CreateConstInBoundsGEP1_32(builder_.getInt8Ty(), klass, dataOff);
    return builder_.CreateLoad(builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE), ptrData);
}

llvm::Value *LLVMIrConstructor::CreateIsInf(llvm::Value *input)
{
    llvm::Type *type = nullptr;
    uint64_t infMaskInt;
    if (input->getType()->isFloatTy()) {
        constexpr uint32_t INF_MASK_FLOAT = 0xff000000;
        infMaskInt = INF_MASK_FLOAT;
        type = builder_.getInt32Ty();
    } else {
        ASSERT_TYPE(input, builder_.getDoubleTy());
        constexpr uint64_t INF_MASK_DOUBLE = 0xffe0000000000000;
        infMaskInt = INF_MASK_DOUBLE;
        type = builder_.getInt64Ty();
    }

    auto infMask = llvm::ConstantInt::get(type, infMaskInt);
    auto one = llvm::ConstantInt::get(type, 1);
    auto castedInput = builder_.CreateBitCast(input, type);
    auto shiftedInput = builder_.CreateShl(castedInput, one);
    auto result = builder_.CreateICmpEQ(shiftedInput, infMask);
    return result;
}

llvm::Value *LLVMIrConstructor::CreateIsInteger(Inst *inst, llvm::Value *input)
{
    auto &ctx = func_->getContext();
    ASSERT(input->getType()->isDoubleTy() || input->getType()->isFloatTy());

    auto isInf = CreateIsInf(input);
    auto epsilon = input->getType()->isDoubleTy()
                       ? llvm::ConstantFP::get(builder_.getDoubleTy(), std::numeric_limits<double>::epsilon())
                       : llvm::ConstantFP::get(builder_.getFloatTy(), std::numeric_limits<float>::epsilon());

    auto initialBb = GetCurrentBasicBlock();
    auto notInfBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "is_integer_not_inf"), func_);
    auto continueBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "is_integer_continue"), func_);

    builder_.CreateCondBr(isInf, continueBb, notInfBb);

    SetCurrentBasicBlock(notInfBb);
    // fabs(v - trunc(v)) <= epsilon
    auto truncated = builder_.CreateUnaryIntrinsic(llvm::Intrinsic::trunc, input);
    auto diff = builder_.CreateFSub(input, truncated);
    auto diffAbs = builder_.CreateUnaryIntrinsic(llvm::Intrinsic::fabs, diff);
    auto cmp = builder_.CreateFCmp(llvm::CmpInst::FCMP_OLE, diffAbs, epsilon);
    builder_.CreateBr(continueBb);

    SetCurrentBasicBlock(continueBb);
    auto result = builder_.CreatePHI(builder_.getInt1Ty(), 2U);
    result->addIncoming(builder_.getInt1(false), initialBb);
    result->addIncoming(cmp, notInfBb);

    return result;
}

llvm::Value *LLVMIrConstructor::CreateCastToInt(Inst *inst)
{
    llvm::Value *input = GetInputValue(inst, 0);
    auto sourceType = input->getType();
    auto targetType = inst->GetType();

    ASSERT_DO(sourceType->isFloatTy() || sourceType->isDoubleTy(),
              std::cerr << "Unexpected source type: " << GetTypeName(sourceType) << ". Should be a float or double."
                        << std::endl);

    auto llvmId = DataType::IsTypeSigned(targetType) ? llvm::Intrinsic::fptosi_sat : llvm::Intrinsic::fptoui_sat;
    ArenaVector<llvm::Type *> intrinsicTypes(GetGraph()->GetLocalAllocator()->Adapter());
    intrinsicTypes.assign({GetExactType(targetType), sourceType});
    return builder_.CreateIntrinsic(llvmId, intrinsicTypes, {input}, nullptr);
}

llvm::Value *LLVMIrConstructor::CreateLoadWithOrdering(Inst *inst, llvm::Value *value, llvm::AtomicOrdering ordering,
                                                       const llvm::Twine &name)
{
    auto pandaType = inst->GetType();
    llvm::Type *type = GetExactType(pandaType);

    auto load = builder_.CreateLoad(type, value, false, name);  // C-like volatile is not applied
    if (ordering != LLVMArkInterface::NOT_ATOMIC_ORDER) {
        auto alignment = func_->getParent()->getDataLayout().getPrefTypeAlignment(type);
        load->setOrdering(ordering);
        load->setAlignment(llvm::Align(alignment));
    }

    return load;
}

llvm::Value *LLVMIrConstructor::CreateStoreWithOrdering(llvm::Value *value, llvm::Value *ptr,
                                                        llvm::AtomicOrdering ordering)
{
    auto store = builder_.CreateStore(value, ptr, false);  // C-like volatile is not applied
    if (ordering != LLVMArkInterface::NOT_ATOMIC_ORDER) {
        auto alignment = func_->getParent()->getDataLayout().getPrefTypeAlignment(value->getType());
        store->setAlignment(llvm::Align(alignment));
        store->setOrdering(ordering);
    }
    return store;
}

llvm::Value *LLVMIrConstructor::CreateZerosCount(Inst *inst, llvm::Intrinsic::ID llvmId)
{
    ASSERT(IsSafeCast(inst, 0));
    auto zeroDefined = llvm::ConstantInt::getFalse(func_->getContext());
    return builder_.CreateBinaryIntrinsic(llvmId, GetInputValue(inst, 0), zeroDefined, nullptr);
}

llvm::Value *LLVMIrConstructor::CreateRoundArm64(Inst *inst, bool is64)
{
    auto input = GetInputValue(inst, 0);

    auto sourceType = is64 ? builder_.getDoubleTy() : builder_.getFloatTy();
    auto targetType = is64 ? builder_.getInt64Ty() : builder_.getInt32Ty();

    double constexpr HALF = 0.5;
    auto half = llvm::ConstantFP::get(sourceType, HALF);
    auto zero = is64 ? builder_.getInt64(0) : builder_.getInt32(0);

    auto initialBb = GetCurrentBasicBlock();
    auto &ctx = func_->getContext();
    auto module = func_->getParent();

    // lround - fcvtas instruction (positive solved fine, NaN mapped to 0, but negatives ties wrong way)
    auto decl = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::lround, {targetType, sourceType});
    llvm::Value *round = llvm::CallInst::Create(decl, input, "", initialBb);

    // Check if rounded value less than zero (if not negative rounding is done)
    auto negative = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "neg"), func_);
    auto done = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "cont"), func_);
    auto lessThan = builder_.CreateICmpSLT(round, zero);
    builder_.CreateCondBr(lessThan, negative, done);

    // CC-OFFNXT(C_RULE_ID_HORIZON_SPACE_SHIELD) false-positive
    // Negative input case, add 1 iff "input - round(input) == 0.5"
    SetCurrentBasicBlock(negative);
    // frinta instruction
    auto floatRound = builder_.CreateUnaryIntrinsic(llvm::Intrinsic::round, input, nullptr);
    auto sub = builder_.CreateBinOp(llvm::Instruction::FSub, input, floatRound);
    auto one = is64 ? builder_.getInt64(1) : builder_.getInt32(1);
    auto add = builder_.CreateBinOp(llvm::Instruction::Add, round, one);
    auto equal = builder_.CreateFCmp(llvm::CmpInst::FCMP_OEQ, sub, half);
    auto roundMayInc = builder_.CreateSelect(equal, add, round);
    builder_.CreateBr(done);

    // Continue block
    SetCurrentBasicBlock(done);
    auto roundPhi = builder_.CreatePHI(targetType, 2U);
    roundPhi->addIncoming(round, initialBb);
    roundPhi->addIncoming(roundMayInc, negative);
    return roundPhi;
}

llvm::Value *LLVMIrConstructor::CreateNewStringFromCharsTlab(Inst *inst, llvm::Value *offset, llvm::Value *length,
                                                             llvm::Value *array)
{
    auto entryId = RuntimeInterface::EntrypointId::CREATE_STRING_FROM_CHAR_ARRAY_TLAB_COMPRESSED;
    ArenaVector<llvm::Value *> arguments(GetGraph()->GetLocalAllocator()->Adapter());
    auto callConv = llvm::CallingConv::ArkFast3;
    if (llvm::isa<llvm::Constant>(offset) && llvm::cast<llvm::Constant>(offset)->isNullValue()) {
        entryId = RuntimeInterface::EntrypointId::CREATE_STRING_FROM_ZERO_BASED_CHAR_ARRAY_TLAB_COMPRESSED;
    } else {
        arguments.push_back(offset);
        callConv = llvm::CallingConv::ArkFast4;
    }
    arguments.push_back(length);
    arguments.push_back(array);
    auto klassOffset = GetGraph()->GetRuntime()->GetStringClassPointerTlsOffset(GetGraph()->GetArch());
    auto klass = llvmbackend::runtime_calls::LoadTLSValue(&builder_, arkInterface_, klassOffset, builder_.getPtrTy());
    arguments.push_back(klass);
    auto result = CreateEntrypointCall(entryId, inst, arguments);
    ASSERT(result->getCallingConv() == llvm::CallingConv::C);
    result->setCallingConv(callConv);
    MarkAsAllocation(result);
    return result;
}

llvm::Value *LLVMIrConstructor::CreateNewStringFromStringTlab(Inst *inst, llvm::Value *stringVal)
{
    auto entryId = RuntimeInterface::EntrypointId::CREATE_STRING_FROM_STRING_TLAB_COMPRESSED;
    auto result = CreateEntrypointCall(entryId, inst, {stringVal});
    ASSERT(result->getCallingConv() == llvm::CallingConv::C);
    result->setCallingConv(llvm::CallingConv::ArkFast1);
    MarkAsAllocation(result);
    return result;
}

void LLVMIrConstructor::CreateDeoptimizationBranch(Inst *inst, llvm::Value *deoptimize,
                                                   RuntimeInterface::EntrypointId exception,
                                                   llvm::ArrayRef<llvm::Value *> arguments)
{
    ASSERT_TYPE(deoptimize, builder_.getInt1Ty());
    ASSERT(exception != RuntimeInterface::EntrypointId::DEOPTIMIZE || inst->CanDeoptimize());
    auto &ctx = func_->getContext();

    /* Create basic blocks for continuation and throw */
    auto continuation = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "cont"), func_);
    auto throwPath = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "deopt"), func_);

    /* Creating branch */
    auto branchWeights = llvm::MDBuilder(ctx).createBranchWeights(
        llvmbackend::Metadata::BranchWeights::UNLIKELY_BRANCH_WEIGHT,  // if unlikely(deoptimize) then throw
        llvmbackend::Metadata::BranchWeights::LIKELY_BRANCH_WEIGHT);   // else continue
    auto branch = builder_.CreateCondBr(deoptimize, throwPath, continuation, branchWeights);

    /* Creating throw block */
    SetCurrentBasicBlock(throwPath);

    auto call = CreateDeoptimizeCall(inst, arguments, exception);

    /* Set metadata for implicit null check */
    if (!inst->CanDeoptimize() && exception == RuntimeInterface::EntrypointId::NULL_POINTER_EXCEPTION &&
        g_options.IsCompilerImplicitNullCheck()) {
        ASSERT(inst->IsNullCheck());
        auto *metadata = llvm::MDNode::get(ctx, {});
        branch->setMetadata(llvm::LLVMContext::MD_make_implicit, metadata);
    }

    /* Create 'ret' after llvm.experimental.deoptimize call */
    CreateReturn(call);
    WrapArkCall(inst, call);

    /* Continue */
    SetCurrentBasicBlock(continuation);
}

llvm::CallInst *LLVMIrConstructor::CreateDeoptimizeCall(Inst *inst, llvm::ArrayRef<llvm::Value *> arguments,
                                                        RuntimeInterface::EntrypointId exception)
{
    auto deoptimizeDeclaration = llvm::Intrinsic::getDeclaration(
        func_->getParent(), llvm::Intrinsic::experimental_deoptimize, {func_->getReturnType()});
    llvm::CallInst *call;
    if (inst->CanDeoptimize()) {
        // If inst CanDeoptimize then call Deoptimize to bail out into interpreter, do not throw exception
        exception = RuntimeInterface::EntrypointId::DEOPTIMIZE;
        auto type = helpers::ToUnderlying(GetDeoptimizationType(inst)) |
                    (inst->GetId() << MinimumBitsToStore(DeoptimizeType::COUNT));
        ASSERT(GetGraph()->GetRuntime()->IsEntrypointNoreturn(exception));
        call = builder_.CreateCall(
            {deoptimizeDeclaration}, {builder_.getInt64(type), builder_.getInt32(static_cast<uint32_t>(exception))},
            CreateSaveStateBundle(inst, GetGraph()->GetRuntime()->IsEntrypointNoreturn(exception)));
        call->addFnAttr(llvm::Attribute::get(call->getContext(), "may-deoptimize"));
    } else {
        std::vector<llvm::Value *> args = arguments;
        args.push_back(builder_.getInt32(static_cast<uint32_t>(exception)));
        call =
            builder_.CreateCall({deoptimizeDeclaration}, args,
                                CreateSaveStateBundle(inst, GetGraph()->GetRuntime()->IsEntrypointNoreturn(exception)));
    }
    arkInterface_->GetOrCreateRuntimeFunctionType(func_->getContext(), func_->getParent(),
                                                  LLVMArkInterface::RuntimeCallType::ENTRYPOINT,
                                                  static_cast<LLVMArkInterface::EntrypointId>(exception));
    return call;
}

ArenaVector<llvm::OperandBundleDef> LLVMIrConstructor::CreateSaveStateBundle(Inst *inst, bool noReturn)
{
    ASSERT_PRINT(inst->CanThrow() || inst->CanDeoptimize(),
                 "Attempt to create a regmap for instruction that doesn't throw (or deoptimize)");
    ArenaVector<llvm::OperandBundleDef> bundle(GetGraph()->GetLocalAllocator()->Adapter());
    if (!arkInterface_->DeoptsEnabled()) {
        return bundle;
    }
    ArenaVector<llvm::Value *> vals(GetGraph()->GetLocalAllocator()->Adapter());
    ArenaVector<SaveStateInst *> saveStates(GetGraph()->GetLocalAllocator()->Adapter());

    auto saveState = inst->GetSaveState();
    while (saveState != nullptr) {
        saveStates.push_back(saveState);
        auto caller = saveState->GetCallerInst();
        saveState = caller == nullptr ? nullptr : caller->GetSaveState();
    }

    std::reverse(saveStates.begin(), saveStates.end());
    for (auto ss : saveStates) {
        auto method = ss->GetMethod();
        auto caller = ss->GetCallerInst();
        if (caller != nullptr) {
            method = caller->GetCallMethod();
        }
        ASSERT(method != nullptr);
        // Put a function as a delimiter in inlining chain
        auto function = GetOrCreateFunctionForCall(caller, method);
        ASSERT(function != nullptr);
        vals.push_back(function);
        // Put methodId needed for inline info
        vals.push_back(builder_.getInt32(GetGraph()->GetRuntime()->GetMethodId(method)));
        // Put bytecode pc for inlining chain as well
        vals.push_back(builder_.getInt32(ss->GetPc()));
        // Put a marker if catch has been met
        uint32_t flags = (inst->RequireRegMap() ? 1U : 0U) | (noReturn ? 2U : 0U);
        vals.push_back(builder_.getInt32(flags));
        // Put a number of interpreter registers for the method
        auto vregCount = arkInterface_->GetVirtualRegistersCount(method);
        vals.push_back(builder_.getInt32(vregCount));

        EncodeSaveStateInputs(&vals, ss);
    }
    bundle.assign({llvm::OperandBundleDef {"deopt", vals}});
    return bundle;
}

void LLVMIrConstructor::EncodeSaveStateInputs(ArenaVector<llvm::Value *> *vals, SaveStateInst *ss)
{
    for (size_t i = 0; i < ss->GetInputsCount(); ++i) {
        if (ss->GetVirtualRegister(i).Value() == VirtualRegister::BRIDGE) {
            continue;
        }
        // Put a virtual register index
        vals->push_back(builder_.getInt32(ss->GetVirtualRegister(i).Value()));
        // Put a virtual register type
        auto metatype = IrTypeToMetainfoType(ss->GetInputType(i));
        uint32_t undertype = static_cast<std::underlying_type_t<VRegInfo::Type>>(metatype);
        vals->push_back(builder_.getInt32(undertype));
        // Put a virtual register value
        auto value = GetInputValue(ss, i);
        if (!value->getType()->isPointerTy()) {
            ASSERT(value->getType()->getScalarSizeInBits() <= 64U);
            auto intVal = builder_.CreateBitCast(value, builder_.getIntNTy(value->getType()->getScalarSizeInBits()));
            if (metatype == VRegInfo::Type::INT32) {
                intVal = CoerceValue(intVal, ss->GetInputType(i), DataType::INT32);
            }
            vals->push_back(builder_.CreateZExt(intVal, builder_.getInt64Ty()));
        } else {
            vals->push_back(value);
        }
    }
}

void LLVMIrConstructor::EncodeInlineInfo(Inst *inst, llvm::Instruction *instruction)
{
    SaveStateInst *saveState = inst->GetSaveState();
    llvm::SmallVector<SaveStateInst *> saveStates;
    bool first = true;
    while (saveState != nullptr) {
        if (!first) {
            saveStates.push_back(saveState);
        }
        first = false;
        saveState = saveState->GetCallerInst() == nullptr ? nullptr : saveState->GetCallerInst()->GetSaveState();
    }
    llvm::reverse(saveStates);
    for (auto ss : saveStates) {
        auto method = ss->GetMethod();
        auto methodName = arkInterface_->GetUniqMethodName(method);
        auto function = func_->getParent()->getFunction(methodName);
        auto caller = ss->GetCallerInst();
        if (caller != nullptr) {
            method = ss->GetCallerInst()->GetCallMethod();
            function = GetOrCreateFunctionForCall(caller, method);
        }
        ASSERT(function != nullptr);
        debugData_->AppendInlinedAt(instruction, function, ss->GetPc());
    }
}

void LLVMIrConstructor::CreatePreWRB(Inst *inst, llvm::Value *mem)
{
    auto barrierType = GetGraph()->GetRuntime()->GetPreType();
    auto isVolatile = IsVolatileMemInst(inst);
    if (barrierType == mem::BarrierType::PRE_WRB_NONE) {
        ASSERT(GetGraph()->SupportManagedCode());
        return;
    }
    ASSERT(barrierType == mem::BarrierType::PRE_SATB_BARRIER);

    if (llvmbackend::g_options.IsLlvmBuiltinWrb() && !arkInterface_->IsIrtocMode()) {
        auto builtin = llvmbackend::builtins::PreWRB(func_->getParent(), mem->getType()->getPointerAddressSpace());
        builder_.CreateCall(builtin, {mem, builder_.getInt1(isVolatile)});
        return;
    }
    auto &ctx = func_->getContext();
    auto outBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "pre_wrb_out"), func_);
    llvmbackend::gc_barriers::EmitPreWRB(&builder_, mem, isVolatile, outBb, arkInterface_, GetThreadRegValue());
}

void LLVMIrConstructor::CreatePostWRB(Inst *inst, llvm::Value *mem, llvm::Value *offset, llvm::Value *value)
{
    auto barrierType = GetGraph()->GetRuntime()->GetPostType();
    if (barrierType == mem::BarrierType::POST_WRB_NONE) {
        ASSERT(GetGraph()->SupportManagedCode());
        return;
    }
    ASSERT(barrierType == mem::BarrierType::POST_INTERREGION_BARRIER);

    Inst *secondValue;
    Inst *val = InstStoredValue(inst, &secondValue);
    ASSERT(secondValue == nullptr);

    if (val->GetOpcode() == Opcode::NullPtr) {
        return;
    }

    ASSERT(offset->getType()->isIntegerTy());
    offset = builder_.CreateSExtOrTrunc(offset, builder_.getInt32Ty());

    bool irtoc = arkInterface_->IsIrtocMode();
    if (!irtoc && llvmbackend::g_options.IsLlvmBuiltinWrb()) {
        auto builtin = llvmbackend::builtins::PostWRB(func_->getParent(), mem->getType()->getPointerAddressSpace());
        builder_.CreateCall(builtin, {mem, offset, value});
        return;
    }
    auto frame = (irtoc && GetGraph()->GetArch() == Arch::X86_64) ? GetRealFrameRegValue() : nullptr;
    llvmbackend::gc_barriers::EmitPostWRB(&builder_, mem, offset, value, arkInterface_, GetThreadRegValue(), frame);
}

llvm::Value *LLVMIrConstructor::CreateMemoryFence(memory_order::Order order)
{
    llvm::AtomicOrdering ordering;
    switch (order) {
        case memory_order::RELEASE:
            ordering = llvm::AtomicOrdering::Release;
            break;
        case memory_order::ACQUIRE:
            ordering = llvm::AtomicOrdering::Acquire;
            break;
        case memory_order::FULL:
            ordering = llvm::AtomicOrdering::SequentiallyConsistent;
            break;
        default:
            UNREACHABLE();
    }
    return builder_.CreateFence(ordering);
}

llvm::Value *LLVMIrConstructor::CreateCondition(ConditionCode cc, llvm::Value *x, llvm::Value *y)
{
    if (cc == CC_TST_EQ || cc == CC_TST_NE) {
        auto tst = builder_.CreateBinOp(llvm::Instruction::And, x, y);
        return (cc == CC_TST_EQ) ? builder_.CreateIsNull(tst) : builder_.CreateIsNotNull(tst);
    }
    return builder_.CreateICmp(ICmpCodeConvert(cc), x, y);
}

void LLVMIrConstructor::CreateIf(Inst *inst, llvm::Value *cond, bool likely, bool unlikely)
{
    llvm::MDNode *weights = nullptr;
    auto constexpr LIKELY = llvmbackend::Metadata::BranchWeights::LIKELY_BRANCH_WEIGHT;
    auto constexpr UNLIKELY = llvmbackend::Metadata::BranchWeights::UNLIKELY_BRANCH_WEIGHT;
    if (likely) {
        weights = llvm::MDBuilder(func_->getContext()).createBranchWeights(LIKELY, UNLIKELY);
    } else if (unlikely) {
        weights = llvm::MDBuilder(func_->getContext()).createBranchWeights(UNLIKELY, LIKELY);
    }
    builder_.CreateCondBr(cond, GetHeadBlock(inst->GetBasicBlock()->GetTrueSuccessor()),
                          GetHeadBlock(inst->GetBasicBlock()->GetFalseSuccessor()), weights);
}

llvm::Value *LLVMIrConstructor::CreateReturn(llvm::Value *value)
{
    ASSERT(value != nullptr);
    if (value->getType()->isVoidTy()) {
        return builder_.CreateRetVoid();
    }
    return builder_.CreateRet(value);
}

llvm::CallInst *LLVMIrConstructor::CreateTailCallFastPath(Inst *inst)
{
    ASSERT(inst->GetInputs().Size() == 0);
    ASSERT(inst->CastToIntrinsic()->HasImms() && inst->CastToIntrinsic()->GetImms().size() == 2U);
    ASSERT(ccValues_.size() == func_->arg_size());

    ArenaVector<llvm::Value *> args(GetGraph()->GetLocalAllocator()->Adapter());
    uint32_t externalId = inst->CastToIntrinsic()->GetImms()[1];
    auto externalName = GetGraph()->GetRuntime()->GetExternalMethodName(GetGraph()->GetMethod(), externalId);
    auto callee = func_->getParent()->getFunction(externalName);
    llvm::CallingConv::ID cc = 0;
    if (callee == nullptr) {
        ArenaVector<llvm::Type *> argTypes(GetGraph()->GetLocalAllocator()->Adapter());
        for (size_t i = 0; i < func_->arg_size(); i++) {
            args.push_back(i < ccValues_.size() && ccValues_.at(i) != nullptr ? ccValues_.at(i) : func_->getArg(i));
            argTypes.push_back(args.at(i)->getType());
        }
        auto ftype = llvm::FunctionType::get(GetType(inst->GetType()), argTypes, false);
        callee = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, externalName, func_->getParent());
        cc = func_->getCallingConv();
    } else {
        size_t size = func_->arg_size();
        ASSERT(callee->arg_size() <= size);
        for (size_t i = 0; i < callee->arg_size() - 2U; i++) {
            args.push_back(i < ccValues_.size() && ccValues_.at(i) != nullptr ? ccValues_.at(i) : func_->getArg(i));
        }
        args.push_back(func_->getArg(size - 2U));
        args.push_back(func_->getArg(size - 1U));
        cc = callee->getCallingConv();
    }
    auto call = builder_.CreateCall(callee->getFunctionType(), callee, args);
    call->setCallingConv(cc);
    return call;
}

llvm::CallInst *LLVMIrConstructor::CreateTailCallInterpreter(Inst *inst)
{
    auto ptr = GetInputValue(inst, 0);
    ASSERT_TYPE(ptr, builder_.getPtrTy());
    ASSERT(ccValues_.size() == (GetGraph()->GetArch() == Arch::AARCH64 ? 8U : 7U));
    ASSERT(ccValues_.at(0) != nullptr);  // pc
    static constexpr unsigned ACC = 1U;
    static constexpr unsigned ACC_TAG = 2U;
    ArenaVector<llvm::Type *> argTypes(GetGraph()->GetLocalAllocator()->Adapter());
    for (size_t i = 0; i < cc_.size(); i++) {
        if (ccValues_.at(i) != nullptr) {
            argTypes.push_back(ccValues_.at(i)->getType());
        } else {
            argTypes.push_back(func_->getFunctionType()->getParamType(i));
        }
    }
    if (ccValues_.at(ACC) == nullptr) {
        ccValues_[ACC] = llvm::Constant::getNullValue(argTypes[ACC]);
    }
    if (ccValues_.at(ACC_TAG) == nullptr) {
        ccValues_[ACC_TAG] = llvm::Constant::getNullValue(argTypes[ACC_TAG]);
    }
    ASSERT(ccValues_.at(3U) != nullptr);  // frame
    ASSERT(ccValues_.at(4U) != nullptr);  // dispatch
    if (GetGraph()->GetArch() == Arch::AARCH64) {
        ASSERT(ccValues_.at(5U) != nullptr);  // moffset
        ASSERT(ccValues_.at(6U) != nullptr);  // methodPtr
        ASSERT(ccValues_.at(7U) != nullptr);  // thread
    } else {
        static constexpr unsigned REAL_FRAME_POINER = 6U;
        ASSERT(ccValues_.at(5U) != nullptr);                 // thread
        ASSERT(ccValues_.at(REAL_FRAME_POINER) == nullptr);  // real frame pointer
        ccValues_[REAL_FRAME_POINER] = func_->getArg(REAL_FRAME_POINER);
    }

    auto functionType = llvm::FunctionType::get(func_->getReturnType(), argTypes, false);
    auto call = builder_.CreateCall(functionType, ptr, ccValues_);
    call->setCallingConv(func_->getCallingConv());
    return call;
}

template <uint32_t VECTOR_SIZE>
void LLVMIrConstructor::CreateCompressUtf16ToUtf8CharsUsingSimd(Inst *inst)
{
    ASSERT(GetGraph()->GetArch() == Arch::AARCH64);
    ASSERT(GetGraph()->GetMode().IsFastPath());
    ASSERT(inst->GetInputType(0) == DataType::POINTER);
    ASSERT(inst->GetInputType(1) == DataType::POINTER);
    static_assert(VECTOR_SIZE == VECTOR_SIZE_8 || VECTOR_SIZE == VECTOR_SIZE_16, "Unexpected vector size");
    auto vecInTy = llvm::VectorType::get(builder_.getInt16Ty(), VECTOR_SIZE, false);
    auto vecOutTy = llvm::VectorType::get(builder_.getInt8Ty(), VECTOR_SIZE, false);

    auto u16Ptr = GetInputValue(inst, 0);  // ptr to src array of utf16 chars
    auto u8Ptr = GetInputValue(inst, 1);   // ptr to dst array of utf8 chars
    auto inVec = builder_.CreateLoad(vecInTy, u16Ptr);
    auto outVec = builder_.CreateTrunc(inVec, vecOutTy);
    builder_.CreateStore(outVec, u8Ptr);
}

// Getters

llvm::FunctionType *LLVMIrConstructor::GetEntryFunctionType()
{
    ArenaVector<llvm::Type *> argTypes(graph_->GetLocalAllocator()->Adapter());

    // Method*
    if (graph_->SupportManagedCode()) {
        argTypes.push_back(builder_.getPtrTy());
    }

    // ArkInt have fake parameters
    if (graph_->GetMode().IsInterpreter()) {
        for (size_t i = 0; i < cc_.size(); ++i) {
            argTypes.push_back(builder_.getPtrTy());
        }
    }

    // Actual function arguments
    auto method = graph_->GetMethod();
    for (size_t i = 0; i < graph_->GetRuntime()->GetMethodTotalArgumentsCount(method); i++) {
        ASSERT(!graph_->GetMode().IsInterpreter());
        auto type = graph_->GetRuntime()->GetMethodTotalArgumentType(method, i);
        if (graph_->GetMode().IsFastPath()) {
            argTypes.push_back(GetExactType(type));
        } else {
            argTypes.push_back(GetType(type));
        }
    }

    // ThreadReg and RealFP for FastPaths
    if (graph_->GetMode().IsFastPath()) {
        argTypes.push_back(builder_.getPtrTy());
        argTypes.push_back(builder_.getPtrTy());
    }

    auto retType = graph_->GetRuntime()->GetMethodReturnType(method);
    ASSERT(graph_->GetMode().IsInterpreter() || retType != DataType::NO_TYPE);
    retType = retType == DataType::NO_TYPE ? DataType::VOID : retType;
    return llvm::FunctionType::get(GetType(retType), makeArrayRef(argTypes.data(), argTypes.size()), false);
}

llvm::Value *LLVMIrConstructor::ToSizeT(llvm::Value *value)
{
    auto entrypointSizeType = GetEntrypointSizeType();
    if (value->getType() == entrypointSizeType) {
        return value;
    }
    ASSERT(value->getType()->getIntegerBitWidth() < entrypointSizeType->getBitWidth());
    return builder_.CreateZExt(value, entrypointSizeType);
}

llvm::Value *LLVMIrConstructor::ToSSizeT(llvm::Value *value)
{
    auto entrypointSizeType = GetEntrypointSizeType();
    if (value->getType() == entrypointSizeType) {
        return value;
    }
    ASSERT(value->getType()->getIntegerBitWidth() < entrypointSizeType->getBitWidth());
    return builder_.CreateSExt(value, entrypointSizeType);
}

ArenaVector<llvm::Value *> LLVMIrConstructor::GetArgumentsForCall(llvm::Value *callee, CallInst *call, bool skipFirst)
{
    ASSERT(callee->getType()->isPointerTy());
    ArenaVector<llvm::Value *> args(GetGraph()->GetLocalAllocator()->Adapter());
    args.push_back(callee);

    // SaveState skipping - last arg
    for (size_t i = skipFirst ? 1 : 0; i < call->GetInputsCount() - 1; i++) {
        auto arg = GetInputValue(call, i);
        auto type = call->GetInputType(i);
        if (DataType::IsLessInt32(type)) {
            arg = CoerceValue(arg, type, DataType::INT32);
        }
        args.push_back(arg);
    }

    return args;
}

ArenaVector<llvm::Value *> LLVMIrConstructor::GetIntrinsicArguments(llvm::FunctionType *intrinsicFunctionType,
                                                                    IntrinsicInst *inst)
{
    ASSERT(intrinsicFunctionType != nullptr);
    ASSERT(inst != nullptr);

    ArenaVector<llvm::Value *> args(GetGraph()->GetLocalAllocator()->Adapter());

    if (inst->IsMethodFirstInput()) {
        args.push_back(GetMethodArgument());
    }
    if (inst->HasImms()) {
        for (uint64_t imm : inst->GetImms()) {
            size_t index = args.size();
            auto type = intrinsicFunctionType->getParamType(index);
            args.push_back(llvm::ConstantInt::get(type, imm));
        }
    }
    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        // Skip SaveState
        if (inst->GetInput(i).GetInst()->IsSaveState()) {
            continue;
        }
        args.push_back(GetInputValue(inst, i));
    }
    ASSERT(intrinsicFunctionType->getNumParams() == args.size());
    return args;
}

void LLVMIrConstructor::SetIntrinsicParamAttrs(llvm::CallInst *call, IntrinsicInst *inst,
                                               [[maybe_unused]] llvm::ArrayRef<llvm::Value *> args)
{
    size_t i = inst->IsMethodFirstInput() ? 1U : 0;
    if (inst->HasImms()) {
        i += inst->GetImms().size();
    }
#ifndef NDEBUG
    for (size_t j = 0; j < i; j++) {
        ASSERT(!args[j]->getType()->isIntegerTy() || args[j]->getType()->getIntegerBitWidth() > VECTOR_SIZE_16);
    }
#endif
    for (size_t arkIndex = 0; arkIndex < inst->GetInputsCount(); arkIndex++) {
        // Skip SaveState
        if (inst->GetInput(arkIndex).GetInst()->IsSaveState()) {
            continue;
        }
        auto arkType = inst->GetInputType(arkIndex);
        switch (arkType) {
            case DataType::UINT8:
                ASSERT(args[i]->getType()->isIntegerTy() && args[i]->getType()->getIntegerBitWidth() == VECTOR_SIZE_8);
                call->addParamAttr(i, llvm::Attribute::ZExt);
                break;
            case DataType::UINT16:
                ASSERT(args[i]->getType()->isIntegerTy() && args[i]->getType()->getIntegerBitWidth() == VECTOR_SIZE_16);
                call->addParamAttr(i, llvm::Attribute::ZExt);
                break;
            case DataType::INT8:
                ASSERT(args[i]->getType()->isIntegerTy() && args[i]->getType()->getIntegerBitWidth() == VECTOR_SIZE_8);
                call->addParamAttr(i, llvm::Attribute::SExt);
                break;
            case DataType::INT16:
                ASSERT(args[i]->getType()->isIntegerTy() && args[i]->getType()->getIntegerBitWidth() == VECTOR_SIZE_16);
                call->addParamAttr(i, llvm::Attribute::SExt);
                break;
            case DataType::BOOL:
                break;
            default:
                ASSERT(!args[i]->getType()->isIntegerTy() || args[i]->getType()->getIntegerBitWidth() > VECTOR_SIZE_16);
                break;
        }
        i++;
    }
    ASSERT(i == args.size());
}

template <typename T>
llvm::FunctionType *LLVMIrConstructor::GetFunctionTypeForCall(T *inst)
{
    ArenaVector<llvm::Type *> argTypes(GetGraph()->GetLocalAllocator()->Adapter());

    if (GetGraph()->SupportManagedCode()) {
        // Callee
        argTypes.push_back(builder_.getPtrTy());
    }

    auto runtime = GetGraph()->GetRuntime();
    auto methodPtr = GetGraph()->GetMethod();
    auto methodId = inst->GetCallMethodId();
    // For instance methods pass implicit object argument
    if (!runtime->IsMethodStatic(methodPtr, methodId)) {
        argTypes.push_back(builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));
    }

    for (size_t i = 0; i < runtime->GetMethodArgumentsCount(methodPtr, methodId); i++) {
        auto ptype = runtime->GetMethodArgumentType(methodPtr, methodId, i);
        argTypes.push_back(GetType(ptype));
    }

    auto retType = runtime->GetMethodReturnType(methodPtr, methodId);
    // Ugly fix CallVirtual opcode for SaveState-excluded run codegen statistics
    if (methodPtr == nullptr) {
        retType = inst->GetType();
    }

    if constexpr (std::is_same_v<T, CallInst>) {
        ASSERT(inst->IsInlined() || inst->GetType() == retType);
    }

    return llvm::FunctionType::get(GetType(retType), argTypes, false);
}

llvm::Value *LLVMIrConstructor::GetThreadRegValue()
{
    if (GetGraph()->SupportManagedCode()) {
        return llvmbackend::runtime_calls::GetThreadRegValue(&builder_, arkInterface_);
    }
    auto regInput = std::find(cc_.begin(), cc_.end(), GetThreadReg(GetGraph()->GetArch()));
    ASSERT(regInput != cc_.end());
    auto threadRegValue = func_->arg_begin() + std::distance(cc_.begin(), regInput);
    return threadRegValue;
}

llvm::Value *LLVMIrConstructor::GetRealFrameRegValue()
{
    if (GetGraph()->SupportManagedCode()) {
        return llvmbackend::runtime_calls::GetRealFrameRegValue(&builder_, arkInterface_);
    }
    ASSERT(GetGraph()->GetMode().IsFastPath() || GetGraph()->GetArch() == Arch::X86_64);
    auto regInput = std::find(cc_.begin(), cc_.end(), GetRealFrameReg(GetGraph()->GetArch()));
    ASSERT(regInput != cc_.end());
    auto frameRegValue = func_->arg_begin() + std::distance(cc_.begin(), regInput);
    return frameRegValue;
}

llvm::Function *LLVMIrConstructor::GetOrCreateFunctionForCall(ark::compiler::CallInst *call, void *method)
{
    ASSERT(method != nullptr);
    auto module = func_->getParent();
    auto methodName = arkInterface_->GetUniqMethodName(method);
    auto function = module->getFunction(methodName);
    if (function == nullptr) {
        auto functionProto = GetFunctionTypeForCall(call);
        function = CreateFunctionDeclaration(functionProto, methodName, module);
        function->addFnAttr("frame-pointer", "all");
        function->addFnAttr(
            ark::llvmbackend::LLVMArkInterface::SOURCE_LANG_ATTR,
            std::to_string(static_cast<uint8_t>(GetGraph()->GetRuntime()->GetMethodSourceLanguage(method))));
    }
    return function;
}

llvm::Type *LLVMIrConstructor::GetType(DataType::Type pandaType)
{
    switch (pandaType) {
        case DataType::VOID:
            return builder_.getVoidTy();
        case DataType::POINTER:
            return builder_.getPtrTy();
        case DataType::REFERENCE:
            return builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE);
        case DataType::BOOL:
        case DataType::UINT8:
        case DataType::INT8:
        case DataType::UINT16:
        case DataType::INT16:
        case DataType::UINT32:
        case DataType::INT32:
            return builder_.getInt32Ty();
        case DataType::UINT64:
        case DataType::INT64:
            return builder_.getInt64Ty();
        case DataType::FLOAT32:
            return builder_.getFloatTy();
        case DataType::FLOAT64:
            return builder_.getDoubleTy();
        default:
            ASSERT_DO(false, (std::cerr << "No handler for panda type = '" << DataType::ToString(pandaType)
                                        << "' to llvm type conversion." << std::endl));
            UNREACHABLE();
    }
}

/**
 * Return exact llvm::Type corresponding to the panda type.
 *
 * Use this method when exact panda type is indeed required.
 * It is the case for:
 * - array loads and stores. If 32-bit version were used, then the neighbour array elements would be overwritten or read
 * - object field loads and stores. The reason the same as in the case above.
 * - object static field loads and stores. The reason the same as in the cases above.
 * - comparisons. Sometimes boolean is compared with i32 or other integral type.
 *   The exact type could be obtained from the compareInst->GetOperandsType(),
 *   which should be used to coerce its operands
 * - Runtime calls. Some runtime call function declarations have narrower types than 32-bit version. To invoke them
 *   the argument should be coerced to the exact type.
 */
llvm::Type *LLVMIrConstructor::GetExactType(DataType::Type targetType)
{
    switch (targetType) {
        case DataType::VOID:
            return builder_.getVoidTy();
        case DataType::POINTER:
            return builder_.getPtrTy();
        case DataType::REFERENCE:
            return builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE);
        case DataType::BOOL:
        case DataType::UINT8:
        case DataType::INT8:
            return builder_.getInt8Ty();
        case DataType::UINT16:
        case DataType::INT16:
            return builder_.getInt16Ty();
        case DataType::UINT32:
        case DataType::INT32:
            return builder_.getInt32Ty();
        case DataType::UINT64:
        case DataType::INT64:
            return builder_.getInt64Ty();
        case DataType::FLOAT32:
            return builder_.getFloatTy();
        case DataType::FLOAT64:
            return builder_.getDoubleTy();
        default:
            ASSERT_DO(false, (std::cerr << "No handler for panda type = '" << DataType::ToString(targetType)
                                        << "' to llvm type conversion." << std::endl));
            UNREACHABLE();
    }
}

llvm::Instruction::CastOps LLVMIrConstructor::GetCastOp(DataType::Type from, DataType::Type to)
{
    Arch arch = GetGraph()->GetArch();
    if (IsInteger(from) && IsInteger(to) && DataType::GetTypeSize(from, arch) > DataType::GetTypeSize(to, arch)) {
        // narrowing, e.g. U32TOU8, I64TOI32
        return llvm::Instruction::Trunc;
    }
    if (IsSignedInteger(from) && IsInteger(to) && DataType::GetTypeSize(from, arch) < DataType::GetTypeSize(to, arch)) {
        // signed int widening, e.g. I32TOI64, I32TOU64
        return llvm::Instruction::SExt;
    }
    if (IsUnsignedInteger(from) && IsInteger(to) &&
        DataType::GetTypeSize(from, arch) < DataType::GetTypeSize(to, arch)) {
        // unsigned int widening, e.g. U32TOI64, U8TOU64
        return llvm::Instruction::ZExt;
    }
    if (IsUnsignedInteger(from) && DataType::IsFloatType(to)) {
        // unsigned int to float, e.g. U32TOF64, U64TOF64
        return llvm::Instruction::UIToFP;
    }
    if (IsSignedInteger(from) && DataType::IsFloatType(to)) {
        // signed int to float e.g. I32TOF64, I64TOF64
        return llvm::Instruction::SIToFP;
    }
    if (DataType::IsFloatType(from) && DataType::IsFloatType(to)) {
        if (DataType::GetTypeSize(from, arch) < DataType::GetTypeSize(to, arch)) {
            return llvm::Instruction::FPExt;
        }
        return llvm::Instruction::FPTrunc;
    }
    if (DataType::IsReference(from) && to == DataType::POINTER) {
        return llvm::Instruction::AddrSpaceCast;
    }
    ASSERT_DO(false, (std::cerr << "Cast from " << DataType::ToString(from) << " to " << DataType::ToString(to))
                         << " is not supported" << std::endl);
    UNREACHABLE();
}

// Various other helpers

/**
 * Coerce given {@code value} with {@code sourceType} to the {@code targetType}.
 *
 * The method may perform truncation or widening cast, or leave the original
 * {@code value}, if no cast is necessary.
 *
 * For integer {@code value} when widening cast is performed the sign of the {@code sourceType} is taken
 * into account:
 * * {@code value} is zero extended if the {@code sourceType} is unsigned integer
 * * {@code value} is sign extended if the {@code sourceType} is signed integer
 *
 * Reference types are returned as is.
 *
 * Currently Ark Bytecode:
 * * does not differentiate between ints of sizes less than 32 bits, and treats them all as i32/u32
 * * leaves resolution of such conversions to the discretion of bytecodes accepting them
 * * assumes implicit casts between small integers
 *
 * Sometimes it causes inconsistencies in LLVM since Ark Compiler IR input has implicit casts too,
 * but LLVM does not permit such conversions.  This function perform those casts if necessary.
 */
llvm::Value *LLVMIrConstructor::CoerceValue(llvm::Value *value, DataType::Type sourceType, DataType::Type targetType)
{
    ASSERT(value != nullptr);
    // Other non-integer mistyping prohibited
    ASSERT_DO(!IsInteger(targetType) || value->getType()->isIntegerTy(),
              std::cerr << "Unexpected data type: " << GetTypeName(value->getType()) << ". Should be an integer."
                        << std::endl);
    ASSERT_DO(!DataType::IsReference(targetType) || value->getType()->isPointerTy(),
              std::cerr << "Unexpected data type: " << GetTypeName(value->getType()) << ". Should be a pointer."
                        << std::endl);
    ASSERT_DO(targetType != DataType::FLOAT64 || value->getType()->isDoubleTy(),
              std::cerr << "Unexpected data type: " << GetTypeName(value->getType()) << ". Should be a double."
                        << std::endl);
    ASSERT_DO(targetType != DataType::FLOAT32 || value->getType()->isFloatTy(),
              std::cerr << "Unexpected data type: " << GetTypeName(value->getType()) << ". Should be a float."
                        << std::endl);

    if (!IsInteger(targetType)) {
        return value;
    }
    ASSERT(value->getType()->isIntegerTy());

    auto targetLlvmType = llvm::cast<llvm::IntegerType>(GetExactType(targetType));
    auto originalLlvmType = llvm::cast<llvm::IntegerType>(value->getType());
    ASSERT(originalLlvmType->getBitWidth() == DataType::GetTypeSize(sourceType, GetGraph()->GetArch()));

    llvm::CastInst::CastOps castOp;
    if (originalLlvmType->getBitWidth() > targetLlvmType->getBitWidth()) {
        castOp = llvm::Instruction::Trunc;
    } else if (originalLlvmType->getBitWidth() < targetLlvmType->getBitWidth()) {
        if (IsSignedInteger(sourceType)) {
            castOp = llvm::Instruction::SExt;
        } else {
            castOp = llvm::Instruction::ZExt;
        }
    } else {
        return value;
    }
    return builder_.CreateCast(castOp, value, targetLlvmType);
}

/**
 * Used in irtoc C++ inlining.
 *
 * When we compile irtoc handlers, we do not have ark's types.
 * For example, ark::Frame is missing.
 * LLVM AOT uses i8* or i64 instead
 *
 * For example, the irtoc handler could look like:
 *
 * @code
 * void MyHandler(i8* myMbject) {
 *    var clone = CloneObjectEntrypoint(myObject);
 * }
 * @endcode
 *
 * When we compile interpreter handlers with cpp inlining we have the definition of CloneObjectEntrypoint:
 *
 * @code
 * ObjectHeader *CloneObjectEntrypoint(ObjectHeader *obj) {
 *   ...
 * }
 * @endcode
 *
 * and we must invoke the CloneObjectEntrypoint with ObjectHeader* argument, not i8*.
 * The CoerceValue method converts i8* to ObjectHeader*
 */
llvm::Value *LLVMIrConstructor::CoerceValue(llvm::Value *value, llvm::Type *targetType)
{
    auto valueType = value->getType();
    if (valueType == targetType) {
        return value;
    }

    if (!valueType->isPointerTy() && targetType->isPointerTy()) {
        // DataType::POINTER to targetType.
        // Example: i64 -> %"class.ark::Frame"*
        return builder_.CreateIntToPtr(value, targetType);
    }
    if (valueType->isPointerTy() && !targetType->isPointerTy()) {
        // valueType to DataType::POINTER
        // Example: %"class.ark::coretypes::String"* -> i64
        return builder_.CreatePtrToInt(value, targetType);
    }

    if (valueType->isIntegerTy() && targetType->isIntegerTy()) {
        auto valueWidth = llvm::cast<llvm::IntegerType>(valueType)->getBitWidth();
        auto targetWidth = llvm::cast<llvm::IntegerType>(targetType)->getBitWidth();
        if (valueWidth > targetWidth) {
            return builder_.CreateTrunc(value, targetType);
        }
        if (valueWidth < targetWidth) {
            return builder_.CreateZExt(value, targetType);
        }
    }
    if (valueType->isPointerTy() && targetType->isPointerTy()) {
        return builder_.CreateAddrSpaceCast(value, targetType);
    }
    UNREACHABLE();
}

void LLVMIrConstructor::ValueMapAdd(Inst *inst, llvm::Value *value, bool setName)
{
    if (!inst->IsMovableObject() && !inst->IsCheck() && llvmbackend::gc_utils::IsGcRefType(value->getType())) {
        auto llvmInst = llvm::dyn_cast<llvm::Instruction>(value);
        if (llvmInst != nullptr) {
            llvmbackend::gc_utils::MarkAsNonMovable(llvmInst);
        }
    }

    auto type = inst->GetType();
    auto ltype = GetExactType(type);
    ASSERT(inputMap_.count(inst) == 0);
    auto it = inputMap_.emplace(inst, GetGraph()->GetLocalAllocator()->Adapter());
    ASSERT(it.second);
    ArenaUnorderedMap<DataType::Type, llvm::Value *> &typeMap = it.first->second;

    if (value == nullptr) {
        typeMap.insert({type, nullptr});
        return;
    }
    if (setName) {
        value->setName(CreateNameForInst(inst));
    }
    if (inst->GetOpcode() == Opcode::LiveOut || !ltype->isIntegerTy()) {
        typeMap.insert({type, value});
        if (type == DataType::POINTER) {
            FillValueMapForUsers(&typeMap, inst, value);
        }
        return;
    }
    ASSERT(value->getType()->isIntegerTy());
    if (value->getType()->getIntegerBitWidth() > ltype->getIntegerBitWidth()) {
        value = builder_.CreateTrunc(value, ltype);
    } else if (value->getType()->getIntegerBitWidth() < ltype->getIntegerBitWidth()) {
        value = builder_.CreateZExt(value, ltype);
    }
    typeMap.insert({type, value});
    FillValueMapForUsers(&typeMap, inst, value);
}

void LLVMIrConstructor::FillValueMapForUsers(ArenaUnorderedMap<DataType::Type, llvm::Value *> *map, Inst *inst,
                                             llvm::Value *value)
{
    auto type = inst->GetType();
    ASSERT(type != DataType::REFERENCE);
    for (auto &userItem : inst->GetUsers()) {
        auto user = userItem.GetInst();
        for (unsigned i = 0; i < user->GetInputsCount(); i++) {
            auto itype = user->GetInputType(i);
            auto input = user->GetInput(i).GetInst();
            if (input != inst || itype == type || map->count(itype) != 0) {
                continue;
            }
            /*
             * When Ark Compiler implicitly converts something -> LLVM side:
             * 1. POINTER to REFERENCE (user LiveOut or Store) -> AddrSpaceCast
             * 2. POINTER to UINT64 (user is LiveOut)          -> no conversion necessary
             * 3. LiveIn to REFERENCE                          -> no conversion necessary
             * 4. INT64/UINT64 to REFERENCE (user is LiveOut)  -> IntToPtr
             * 5. Integers                                     -> use coercing
             */
            llvm::Value *cvalue;
            if (type == DataType::POINTER && itype == DataType::REFERENCE) {
                ASSERT(user->GetOpcode() == Opcode::LiveOut || user->GetOpcode() == Opcode::Store);
                cvalue = builder_.CreateAddrSpaceCast(value, builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));
            } else if (type == DataType::POINTER && itype == DataType::UINT64) {
                ASSERT(user->GetOpcode() == Opcode::LiveOut);
                cvalue = value;
            } else if (type == DataType::POINTER) {
                continue;
            } else if (inst->GetOpcode() == Opcode::LiveIn && itype == DataType::REFERENCE) {
                cvalue = value;
            } else if ((type == DataType::INT64 || type == DataType::UINT64) && itype == DataType::REFERENCE) {
                ASSERT(user->GetOpcode() == Opcode::LiveOut);
                cvalue = builder_.CreateIntToPtr(value, builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));
            } else {
                cvalue = CoerceValue(value, type, itype);
            }
            map->insert({itype, cvalue});
        }
    }
}

void LLVMIrConstructor::WrapArkCall(Inst *orig, llvm::CallInst *call)
{
    ASSERT(orig->RequireState());
    ASSERT_PRINT(!call->getDebugLoc(), "Debug info must be unset");
    // Ark calls may call GC inside, so add statepoint
    debugData_->SetLocation(call, orig->GetPc());
    EncodeInlineInfo(orig, call);
}

void LLVMIrConstructor::InitializeEntryBlock(bool noInline)
{
    if (noInline) {
        ASSERT(!arkInterface_->IsIrtocMode() && GetGraph()->SupportManagedCode());
        func_->addFnAttr(llvm::Attribute::NoInline);
        // This type of linkage prevents return value propagation.
        // llvm::GlobalValue::isDefinitionExact becomes false and as a result
        // llvm::canTrackReturnsInterprocedurally() also false.
        func_->setLinkage(llvm::Function::WeakAnyLinkage);
    }

    if (GetGraph()->SupportManagedCode()) {
        func_->addParamAttr(GetMethodArgument()->getArgNo(), llvm::Attribute::NonNull);
        if (!GetGraph()->GetRuntime()->IsMethodStatic(GetGraph()->GetMethod())) {
            func_->addParamAttr(GetArgument(0)->getArgNo(), llvm::Attribute::NonNull);
        }
    }

    if (func_->hasMetadata(LLVMArkInterface::FUNCTION_MD_INLINE_MODULE) &&
        !GetGraph()->GetRuntime()->IsMethodStatic(GetGraph()->GetMethod())) {
        SetCurrentBasicBlock(&func_->getEntryBlock());
        builder_.CreateCall(KeepThis(func_->getParent()), GetArgument(0));
    }
}

void LLVMIrConstructor::MarkAsAllocation(llvm::CallInst *call)
{
    llvm::AttrBuilder builder {call->getContext()};
    /**
     * When we add allockind(alloc) attribute, then llvm can assume that the function is allocation function.
     * With this assumption llvm can remove dead allocations
     */
    builder.addAllocKindAttr(llvm::AllocFnKind::Alloc);
    call->addFnAttr(builder.getAttribute(llvm::Attribute::AllocKind));
    call->addRetAttr(llvm::Attribute::NonNull);
    call->addRetAttr(llvm::Attribute::NoAlias);
}

// Instruction Visitors

// Constant and NullPtr are processed directly in GetInputValue
void LLVMIrConstructor::VisitConstant([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT(inst->GetBasicBlock()->IsStartBlock());
}

void LLVMIrConstructor::VisitNullPtr([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst)
{
    ASSERT(inst->GetBasicBlock()->IsStartBlock());
}

void LLVMIrConstructor::VisitLiveIn(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    ASSERT(inst->GetBasicBlock()->IsStartBlock());
    ASSERT(!ctor->GetGraph()->SupportManagedCode());

    auto regInput = std::find(ctor->cc_.begin(), ctor->cc_.end(), inst->CastToLiveIn()->GetDstReg());
    ASSERT(regInput != ctor->cc_.end());
    auto idx = std::distance(ctor->cc_.begin(), regInput);
    auto n = ctor->func_->arg_begin() + idx;
    ctor->ValueMapAdd(inst, ctor->CoerceValue(n, ctor->GetExactType(inst->GetType())));
}

void LLVMIrConstructor::VisitParameter(GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetBasicBlock()->IsStartBlock());
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    ASSERT(ctor->GetGraph()->SupportManagedCode() || ctor->GetGraph()->GetMode().IsFastPath());
    auto n = ctor->GetArgument(inst->CastToParameter()->GetArgNumber());
    ctor->ValueMapAdd(inst, n, false);
}

void LLVMIrConstructor::VisitReturnVoid(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        auto builtin = BarrierReturnVoid(ctor->func_->getParent());
        auto builtinCall = ctor->builder_.CreateCall(builtin);
        builtinCall->addFnAttr(llvm::Attribute::get(builtinCall->getContext(), "needs-mem-barrier"));
    }
    ctor->builder_.CreateRetVoid();
}

void LLVMIrConstructor::VisitReturn(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto ret = ctor->GetInputValue(inst, 0);

    auto type = inst->GetType();
    if (DataType::IsLessInt32(type)) {
        ret = ctor->CoerceValue(ret, type, DataType::INT32);
    }

    ctor->builder_.CreateRet(ret);
}

void LLVMIrConstructor::VisitReturnInlined(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);

    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        auto builtin = BarrierReturnVoid(ctor->func_->getParent());
        auto builtinCall = ctor->builder_.CreateCall(builtin);
        builtinCall->addFnAttr(llvm::Attribute::get(builtinCall->getContext(), "needs-mem-barrier"));
    }
}

void LLVMIrConstructor::VisitReturnI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    llvm::Value *ret = ctor->builder_.getInt64(inst->CastToReturnI()->GetImm());

    auto type = inst->GetType();
    if (DataType::IsInt32Bit(type)) {
        ret = ctor->CoerceValue(ret, DataType::INT64, DataType::INT32);
    }

    ctor->builder_.CreateRet(ret);
}

// No-op "pseudo" instructions
void LLVMIrConstructor::VisitTry([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst) {}
void LLVMIrConstructor::VisitSaveState([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst) {}
void LLVMIrConstructor::VisitSaveStateDeoptimize([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst) {}
void LLVMIrConstructor::VisitSafePoint([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst) {}
// NOP and Deoptimize* required after adding CheckElim* passes
void LLVMIrConstructor::VisitNOP([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst) {}

void LLVMIrConstructor::VisitLiveOut(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    ASSERT(!ctor->GetGraph()->SupportManagedCode());
    auto input = ctor->GetInputValue(inst, 0);

    auto regInput = std::find(ctor->cc_.begin(), ctor->cc_.end(), inst->GetDstReg());
    ASSERT(regInput != ctor->cc_.end());
    size_t idx = std::distance(ctor->cc_.begin(), regInput);
    ASSERT(ctor->ccValues_[idx] == nullptr);

    // LiveOut not allowed for real frame register
    ASSERT(ctor->GetGraph()->GetArch() == Arch::AARCH64 || idx + 1 != ctor->cc_.size());
    auto value = ctor->CoerceValue(input, ctor->GetExactType(inst->GetType()));
    ctor->ccValues_[idx] = value;
    ctor->ValueMapAdd(inst, value, false);
}

void LLVMIrConstructor::VisitSubOverflowCheck(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto dtype = inst->GetType();
    auto ltype = ctor->GetExactType(dtype);
    auto src0 = ctor->GetInputValue(inst, 0);
    auto src1 = ctor->GetInputValue(inst, 1);
    ASSERT(inst->GetInputType(0) == inst->GetInputType(1));

    auto arch = ctor->GetGraph()->GetArch();
    auto dtypeSize = DataType::GetTypeSize(dtype, arch);
    auto srcTypeSize = DataType::GetTypeSize(inst->GetInputType(0), arch);
    ASSERT(DataType::Is32Bits(dtype, arch) || DataType::Is64Bits(dtype, arch));
    if (srcTypeSize < dtypeSize) {
        src0 = ctor->builder_.CreateSExt(src0, ltype);
        src1 = ctor->builder_.CreateSExt(src1, ltype);
    }
    if (dtypeSize < srcTypeSize) {
        src0 = ctor->builder_.CreateTrunc(src0, ltype);
        src1 = ctor->builder_.CreateTrunc(src1, ltype);
    }

    auto ssubOverflow = ctor->builder_.CreateBinaryIntrinsic(llvm::Intrinsic::ssub_with_overflow, src0, src1);
    auto result = ctor->builder_.CreateExtractValue(ssubOverflow, {0}, "ssub");
    auto deoptimize = ctor->builder_.CreateExtractValue(ssubOverflow, {1}, "obit");

    auto exception = RuntimeInterface::EntrypointId::DEOPTIMIZE;
    ctor->CreateDeoptimizationBranch(inst, deoptimize, exception);

    ctor->ValueMapAdd(inst, result, false);
}

void LLVMIrConstructor::VisitDeoptimize(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto type = inst->CastToDeoptimize()->GetDeoptimizeType();
    auto exception = RuntimeInterface::EntrypointId::DEOPTIMIZE;
    uint64_t value = static_cast<uint64_t>(type) | (inst->GetId() << MinimumBitsToStore(DeoptimizeType::COUNT));
    auto call = ctor->CreateEntrypointCall(exception, inst, {ctor->builder_.getInt64(value)});
    call->addFnAttr(llvm::Attribute::get(call->getContext(), "may-deoptimize"));
    ctor->builder_.CreateUnreachable();
}

void LLVMIrConstructor::VisitDeoptimizeIf(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto exception = RuntimeInterface::EntrypointId::DEOPTIMIZE;
    auto deoptimize = ctor->builder_.CreateIsNotNull(ctor->GetInputValue(inst, 0));
    ctor->CreateDeoptimizationBranch(inst, deoptimize, exception);
}

void LLVMIrConstructor::VisitNegativeCheck(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto val = ctor->GetInputValue(inst, 0);

    auto deoptimize = ctor->builder_.CreateICmpSLT(val, llvm::Constant::getNullValue(val->getType()));
    auto exception = RuntimeInterface::EntrypointId::NEGATIVE_ARRAY_SIZE_EXCEPTION;
    ctor->CreateDeoptimizationBranch(inst, deoptimize, exception, {ctor->ToSSizeT(val)});

    ctor->ValueMapAdd(inst, val, false);
}

void LLVMIrConstructor::VisitZeroCheck(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto val = ctor->GetInputValue(inst, 0);

    auto deoptimize = ctor->builder_.CreateIsNull(val);
    auto exception = RuntimeInterface::EntrypointId::ARITHMETIC_EXCEPTION;
    ctor->CreateDeoptimizationBranch(inst, deoptimize, exception);

    ctor->ValueMapAdd(inst, val, false);
}

void LLVMIrConstructor::VisitNullCheck(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto obj = ctor->GetInputValue(inst, 0);
    auto obj64 = obj;

    if (compiler::g_options.IsCompilerImplicitNullCheck()) {
        // LLVM's ImplicitNullChecks pass can't operate with 32-bit pointers, but it is enough
        // to create address space cast to an usual 64-bit pointer before comparing with null.
        obj64 = ctor->builder_.CreateAddrSpaceCast(obj, ctor->builder_.getPtrTy());
    }

    auto deoptimize = ctor->builder_.CreateIsNull(obj64);
    auto exception = RuntimeInterface::EntrypointId::NULL_POINTER_EXCEPTION;
    ctor->CreateDeoptimizationBranch(inst, deoptimize, exception);

    ctor->ValueMapAdd(inst, obj, false);
}

void LLVMIrConstructor::VisitBoundsCheck(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto length = ctor->GetInputValue(inst, 0);
    ASSERT_TYPE(length, ctor->builder_.getInt32Ty());
    auto index = ctor->GetInputValue(inst, 1);
    ASSERT(index->getType()->isIntegerTy());

    auto deoptimize = ctor->builder_.CreateICmpUGE(index, length);
    auto exception = inst->CastToBoundsCheck()->IsArray()
                         ? RuntimeInterface::EntrypointId::ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION
                         : RuntimeInterface::EntrypointId::STRING_INDEX_OUT_OF_BOUNDS_EXCEPTION;
    ctor->CreateDeoptimizationBranch(inst, deoptimize, exception, {ctor->ToSSizeT(index), ctor->ToSizeT(length)});

    ctor->ValueMapAdd(inst, index, false);
}

void LLVMIrConstructor::VisitRefTypeCheck(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);

    auto array = ctor->GetInputValue(inst, 0);
    auto ref = ctor->GetInputValue(inst, 1);

    auto &ctx = ctor->func_->getContext();
    auto compareBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "comparison"), ctor->func_);
    auto compBaseBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "comp_base"), ctor->func_);
    auto slowPathBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "slow_path"), ctor->func_);
    auto outBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "out"), ctor->func_);

    auto runtime = ctor->GetGraph()->GetRuntime();
    auto arch = ctor->GetGraph()->GetArch();

    auto cmp = ctor->builder_.CreateIsNotNull(ref);
    ctor->builder_.CreateCondBr(cmp, compareBb, outBb);

    // Get element class from array
    ctor->SetCurrentBasicBlock(compareBb);
    auto arrayClass = CreateLoadClassFromObject(array, &ctor->builder_, ctor->arkInterface_);
    auto elementTypeOffset = runtime->GetClassComponentTypeOffset(arch);
    auto int8Ty = ctor->builder_.getInt8Ty();
    auto elementClassPtr = ctor->builder_.CreateConstInBoundsGEP1_32(int8Ty, arrayClass, elementTypeOffset);
    auto elementClass = ctor->builder_.CreateLoad(ctor->builder_.getPtrTy(), elementClassPtr);
    // And class from stored object
    auto refClass = CreateLoadClassFromObject(ref, &ctor->builder_, ctor->arkInterface_);

    // Unlike other checks, there's another check in the runtime function, so don't use CreateDeoptimizationBranch
    cmp = ctor->builder_.CreateICmpNE(elementClass, refClass);
    auto branchWeights =
        llvm::MDBuilder(ctx).createBranchWeights(llvmbackend::Metadata::BranchWeights::LIKELY_BRANCH_WEIGHT,
                                                 llvmbackend::Metadata::BranchWeights::UNLIKELY_BRANCH_WEIGHT);
    ctor->builder_.CreateCondBr(cmp, compBaseBb, outBb, branchWeights);

    // If the array's element class is Object (Base class is null) - no further check needed
    ctor->SetCurrentBasicBlock(compBaseBb);
    auto baseTypeOffset = runtime->GetClassBaseOffset(arch);
    auto baseClassPtr = ctor->builder_.CreateConstInBoundsGEP1_32(int8Ty, elementClass, baseTypeOffset);
    auto baseClass = ctor->builder_.CreateLoad(ctor->builder_.getPtrTy(), baseClassPtr);
    auto notObjectArray = ctor->builder_.CreateIsNotNull(baseClass);
    ctor->builder_.CreateCondBr(notObjectArray, slowPathBb, outBb);

    ctor->SetCurrentBasicBlock(slowPathBb);
    if (inst->CanDeoptimize()) {
        auto entrypoint = RuntimeInterface::EntrypointId::CHECK_STORE_ARRAY_REFERENCE_DEOPTIMIZE;
        auto call = ctor->CreateEntrypointCall(entrypoint, inst, {array, ref});
        call->addFnAttr(llvm::Attribute::get(call->getContext(), "may-deoptimize"));
    } else {
        ctor->CreateEntrypointCall(RuntimeInterface::EntrypointId::CHECK_STORE_ARRAY_REFERENCE, inst, {array, ref});
    }
    ctor->builder_.CreateBr(outBb);

    ctor->SetCurrentBasicBlock(outBb);
    ctor->ValueMapAdd(inst, ref, false);
}

void LLVMIrConstructor::VisitLoadString(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);

    llvm::Value *result;
    if (g_options.IsCompilerAotLoadStringPlt() &&
        !ctor->GetGraph()->GetRuntime()->IsMethodStaticConstructor(ctor->GetGraph()->GetMethod())) {
        auto aotData = ctor->GetGraph()->GetAotData();
        ASSERT(aotData != nullptr);

        auto typeId = inst->CastToLoadString()->GetTypeId();
        auto typeVal = ctor->builder_.getInt64(typeId);
        auto slotVal = ctor->builder_.getInt32(ctor->arkInterface_->GetStringSlotId(aotData, typeId));
        ctor->arkInterface_->GetOrCreateRuntimeFunctionType(
            ctor->func_->getContext(), ctor->func_->getParent(), LLVMArkInterface::RuntimeCallType::ENTRYPOINT,
            static_cast<LLVMArkInterface::EntrypointId>(RuntimeInterface::EntrypointId::RESOLVE_STRING_AOT));

        auto builtin = LoadString(ctor->func_->getParent());
        auto call = ctor->builder_.CreateCall(builtin, {typeVal, slotVal}, ctor->CreateSaveStateBundle(inst));
        ctor->WrapArkCall(inst, call);
        result = call;
    } else {
        auto stringType = ctor->builder_.getInt64(inst->CastToLoadString()->GetTypeId());
        auto entrypointId = RuntimeInterface::EntrypointId::RESOLVE_STRING;
        result = ctor->CreateEntrypointCall(entrypointId, inst, {ctor->GetMethodArgument(), stringType});
    }
    ctor->ValueMapAdd(inst, result);
}

void LLVMIrConstructor::VisitLenArray(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto array = ctor->GetInputValue(inst, 0);
    auto runtime = ctor->GetGraph()->GetRuntime();
    bool isString = !inst->CastToLenArray()->IsArray();
    auto &builder = ctor->builder_;

    auto arrayInput = inst->GetDataFlowInput(0);
    // Try to extract array length from constructor
    if (arrayInput->GetOpcode() == Opcode::NewArray) {
        auto size = ctor->GetInputValue(arrayInput, NewArrayInst::INDEX_SIZE);
        ctor->ValueMapAdd(inst, size);
        return;
    }
    auto builtin = LenArray(ctor->func_->getParent());
    auto arch = ctor->GetGraph()->GetArch();
    auto offset = isString ? runtime->GetStringLengthOffset(arch) : runtime->GetArrayLengthOffset(arch);
    auto len = ctor->builder_.CreateCall(builtin, {array, builder.getInt32(offset)});

    ctor->ValueMapAdd(inst, len);
}

void LLVMIrConstructor::VisitLoadArray(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto loadArray = inst->CastToLoadArray();

    auto array = ctor->GetInputValue(inst, 0);
    ASSERT_TYPE(array, ctor->builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));

    auto dtype = inst->GetType();
    auto ltype = ctor->GetExactType(dtype);
    auto arch = ctor->GetGraph()->GetArch();
    uint32_t dataOffset = ctor->GetGraph()->GetRuntime()->GetArrayDataOffset(arch);
    if (!loadArray->IsArray()) {
        dataOffset = ctor->GetGraph()->GetRuntime()->GetStringDataOffset(arch);
    }
    auto ptrData = ctor->builder_.CreateConstInBoundsGEP1_32(ctor->builder_.getInt8Ty(), array, dataOffset);

    llvm::Value *ptrElem = ctor->builder_.CreateInBoundsGEP(ltype, ptrData, ctor->GetInputValue(inst, 1));

    llvm::Value *n = ctor->builder_.CreateLoad(ltype, ptrElem);
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitLoadCompressedStringChar(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto loadString = inst->CastToLoadCompressedStringChar();

    ASSERT(inst->GetType() == DataType::UINT16);

    auto array = ctor->GetInputValue(loadString, 0);
    auto index = ctor->GetInputValue(loadString, 1);
    auto length = ctor->GetInputValue(loadString, 2);

    ASSERT(ctor->GetGraph()->GetRuntime()->GetStringCompressionMask() == 1U);
    auto compressionMask = ctor->builder_.getInt32(ctor->GetGraph()->GetRuntime()->GetStringCompressionMask());
    auto dataOff = ctor->GetGraph()->GetRuntime()->GetStringDataOffset(ctor->GetGraph()->GetArch());
    auto chars = ctor->builder_.CreateConstInBoundsGEP1_64(ctor->builder_.getInt8Ty(), array, dataOff);
    auto isCompressed = ctor->builder_.CreateIsNull(ctor->builder_.CreateAnd(length, compressionMask));

    /**
     * int32_t CompressedCharAt(uint8_t *string, int32_t index) {
     *     int32_t length = LenArray(string, LENGTH_OFFSET, SHIFT);
     *     bool isCompressed = (length & COMPRESSION_MASK) == 0;
     *     uint8_t *chars = string + DATA_OFFSET;
     *
     *     uint16_t c;
     *     if (isCompressed) {
     *         // compressedBb
     *         c = static_cast<uint16_t>(chars[index]);
     *     } else {
     *         // uncompressedBb
     *         c = reinterpret_cast<uint16_t *>(chars)[index];
     *     }
     *     // Coercing
     *     return static_cast<int32_t>(c);
     * }
     */
    auto compressedBb =
        llvm::BasicBlock::Create(ctor->func_->getContext(), CreateBasicBlockName(inst, "compressed_bb"), ctor->func_);
    auto uncompressedBb =
        llvm::BasicBlock::Create(ctor->func_->getContext(), CreateBasicBlockName(inst, "uncompressed_bb"), ctor->func_);
    auto continuation =
        llvm::BasicBlock::Create(ctor->func_->getContext(), CreateBasicBlockName(inst, "char_at_cont"), ctor->func_);
    ctor->builder_.CreateCondBr(isCompressed, compressedBb, uncompressedBb);
    llvm::Value *compressedChar;
    {
        ctor->SetCurrentBasicBlock(compressedBb);
        ASSERT_TYPE(chars, ctor->builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));
        auto charAt = ctor->builder_.CreateInBoundsGEP(ctor->builder_.getInt8Ty(), chars, index);
        auto character = ctor->builder_.CreateLoad(ctor->builder_.getInt8Ty(), charAt);
        compressedChar = ctor->builder_.CreateSExt(character, ctor->builder_.getInt16Ty());
        ctor->builder_.CreateBr(continuation);
    }

    llvm::Value *uncompressedChar;
    {
        ctor->SetCurrentBasicBlock(uncompressedBb);
        auto u16CharAt = ctor->builder_.CreateInBoundsGEP(ctor->builder_.getInt16Ty(), chars, index);
        uncompressedChar = ctor->builder_.CreateLoad(ctor->builder_.getInt16Ty(), u16CharAt);
        ctor->builder_.CreateBr(continuation);
    }
    ctor->SetCurrentBasicBlock(continuation);

    auto charAt = ctor->builder_.CreatePHI(ctor->builder_.getInt16Ty(), 2U);
    charAt->addIncoming(compressedChar, compressedBb);
    charAt->addIncoming(uncompressedChar, uncompressedBb);
    ctor->ValueMapAdd(inst, charAt);
}

void LLVMIrConstructor::VisitStoreArray(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto array = ctor->GetInputValue(inst, 0);
    ASSERT_TYPE(array, ctor->builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));
    auto value = ctor->GetInputValue(inst, 2U);

    auto dtype = inst->GetType();
    auto arch = ctor->GetGraph()->GetArch();
    auto ltype = ctor->GetExactType(dtype);
    auto dataOff = ctor->GetGraph()->GetRuntime()->GetArrayDataOffset(arch);
    auto ptrData = ctor->builder_.CreateConstInBoundsGEP1_32(ctor->builder_.getInt8Ty(), array, dataOff);
    auto index = ctor->GetInputValue(inst, 1);
    auto ptrElem = ctor->builder_.CreateInBoundsGEP(ltype, ptrData, index);

    // Pre
    if (inst->CastToStoreArray()->GetNeedBarrier()) {
        ctor->CreatePreWRB(inst, ptrElem);
    }
    // Write
    ctor->builder_.CreateStore(value, ptrElem);
    // Post
    if (inst->CastToStoreArray()->GetNeedBarrier()) {
        auto indexOffset = ctor->builder_.CreateBinOp(llvm::Instruction::Shl, index,
                                                      ctor->builder_.getInt32(DataType::ShiftByType(dtype, arch)));
        auto offset = ctor->builder_.CreateBinOp(llvm::Instruction::Add, indexOffset, ctor->builder_.getInt32(dataOff));
        ctor->CreatePostWRB(inst, array, offset, value);
    }
}

void LLVMIrConstructor::VisitLoad(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto srcPtr = ctor->GetInputValue(inst, 0);
    ASSERT(srcPtr->getType()->isPointerTy());

    llvm::Value *offset;
    auto offsetInput = inst->GetInput(1).GetInst();
    auto offsetItype = offsetInput->GetType();
    if (offsetItype == DataType::UINT64 || offsetItype == DataType::INT64) {
        ASSERT(offsetInput->GetOpcode() != Opcode::Load && offsetInput->GetOpcode() != Opcode::LoadI);
        offset = ctor->GetInputValue(inst, 1, true);
    } else {
        offset = ctor->GetInputValue(inst, 1);
    }

    ASSERT(srcPtr->getType()->isPointerTy());
    auto ptr = ctor->builder_.CreateInBoundsGEP(ctor->builder_.getInt8Ty(), srcPtr, offset);

    auto n = ctor->CreateLoadWithOrdering(inst, ptr, ToAtomicOrdering(inst->CastToLoad()->GetVolatile()));
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitLoadNative(GraphVisitor *v, Inst *inst)
{
    inst->SetOpcode(Opcode::Load);
    VisitLoad(v, inst);
}

void LLVMIrConstructor::VisitStore(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto srcPtr = ctor->GetInputValue(inst, 0);
    auto value = ctor->GetInputValue(inst, 2U);

    llvm::Value *offset;
    auto offsetInput = inst->GetInput(1).GetInst();
    auto offsetItype = offsetInput->GetType();
    if (offsetItype == DataType::UINT64 || offsetItype == DataType::INT64) {
        ASSERT(offsetInput->GetOpcode() != Opcode::Load && offsetInput->GetOpcode() != Opcode::LoadI);
        offset = ctor->GetInputValue(inst, 1, true);
    } else {
        offset = ctor->GetInputValue(inst, 1);
    }

    auto ptrPlus = ctor->builder_.CreateInBoundsGEP(ctor->builder_.getInt8Ty(), srcPtr, offset);

    // Pre
    if (inst->CastToStore()->GetNeedBarrier()) {
        ctor->CreatePreWRB(inst, ptrPlus);
    }
    // Write
    ctor->CreateStoreWithOrdering(value, ptrPlus, ToAtomicOrdering(inst->CastToStore()->GetVolatile()));
    // Post
    if (inst->CastToStore()->GetNeedBarrier()) {
        ctor->CreatePostWRB(inst, srcPtr, offset, value);
    }
}

void LLVMIrConstructor::VisitStoreNative(GraphVisitor *v, Inst *inst)
{
    inst->SetOpcode(Opcode::Store);
    VisitStore(v, inst);
}

void LLVMIrConstructor::VisitLoadI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto srcPtr = ctor->GetInputValue(inst, 0);
    auto index = inst->CastToLoadI()->GetImm();

    ASSERT(srcPtr->getType()->isPointerTy());
    auto ptrPlus = ctor->builder_.CreateConstInBoundsGEP1_64(ctor->builder_.getInt8Ty(), srcPtr, index);

    auto n = ctor->CreateLoadWithOrdering(inst, ptrPlus, ToAtomicOrdering(inst->CastToLoadI()->GetVolatile()));
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitStoreI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto srcPtr = ctor->GetInputValue(inst, 0);
    auto value = ctor->GetInputValue(inst, 1);

    auto index = inst->CastToStoreI()->GetImm();
    ASSERT(srcPtr->getType()->isPointerTy());
    auto ptrPlus = ctor->builder_.CreateConstInBoundsGEP1_64(ctor->builder_.getInt8Ty(), srcPtr, index);

    // Pre
    if (inst->CastToStoreI()->GetNeedBarrier()) {
        ctor->CreatePreWRB(inst, ptrPlus);
    }
    // Write
    ctor->CreateStoreWithOrdering(value, ptrPlus, ToAtomicOrdering(inst->CastToStoreI()->GetVolatile()));
    // Post
    if (inst->CastToStoreI()->GetNeedBarrier()) {
        ctor->CreatePostWRB(inst, srcPtr, ctor->builder_.getInt32(index), value);
    }
}

void LLVMIrConstructor::VisitLoadObject(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto obj = ctor->GetInputValue(inst, 0);
    ASSERT_TYPE(obj, ctor->builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));

    auto field = inst->CastToLoadObject()->GetObjField();
    auto dataOff = ctor->GetGraph()->GetRuntime()->GetFieldOffset(field);
    auto ptrData = ctor->builder_.CreateConstInBoundsGEP1_32(ctor->builder_.getInt8Ty(), obj, dataOff);

    auto n = ctor->CreateLoadWithOrdering(inst, ptrData, ToAtomicOrdering(inst->CastToLoadObject()->GetVolatile()));
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitStoreObject(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto obj = ctor->GetInputValue(inst, 0);
    ASSERT_TYPE(obj, ctor->builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));
    auto value = ctor->GetInputValue(inst, 1);

    auto field = inst->CastToStoreObject()->GetObjField();
    auto dataOff = ctor->GetGraph()->GetRuntime()->GetFieldOffset(field);

    auto ptrData = ctor->builder_.CreateConstInBoundsGEP1_32(ctor->builder_.getInt8Ty(), obj, dataOff);

    // Pre
    if (inst->CastToStoreObject()->GetNeedBarrier()) {
        ctor->CreatePreWRB(inst, ptrData);
    }
    // Write
    ctor->CreateStoreWithOrdering(value, ptrData, ToAtomicOrdering(inst->CastToStoreObject()->GetVolatile()));
    // Post
    if (inst->CastToStoreObject()->GetNeedBarrier()) {
        ctor->CreatePostWRB(inst, obj, ctor->builder_.getInt32(dataOff), value);
    }
}

void LLVMIrConstructor::VisitResolveObjectField(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);

    auto typeId = ctor->builder_.getInt64(inst->CastToResolveObjectField()->GetTypeId());

    auto entrypointId = RuntimeInterface::EntrypointId::GET_FIELD_OFFSET;
    auto offset = ctor->CreateEntrypointCall(entrypointId, inst, {ctor->GetMethodArgument(), typeId});

    ctor->ValueMapAdd(inst, offset);
}

void LLVMIrConstructor::VisitLoadResolvedObjectField(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto obj = ctor->GetInputValue(inst, 0);
    ASSERT_TYPE(obj, ctor->builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));

    auto offset = ctor->GetInputValue(inst, 1);
    auto ptrData = ctor->builder_.CreateInBoundsGEP(ctor->builder_.getInt8Ty(), obj, offset);

    auto n = ctor->CreateLoadWithOrdering(inst, ptrData, LLVMArkInterface::VOLATILE_ORDER);
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitStoreResolvedObjectField(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto obj = ctor->GetInputValue(inst, 0);
    ASSERT_TYPE(obj, ctor->builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));
    auto value = ctor->GetInputValue(inst, 1);

    auto offset = ctor->GetInputValue(inst, 2);
    auto ptrData = ctor->builder_.CreateInBoundsGEP(ctor->builder_.getInt8Ty(), obj, offset);

    // Pre
    if (inst->CastToStoreResolvedObjectField()->GetNeedBarrier()) {
        ctor->CreatePreWRB(inst, ptrData);
    }
    // Write
    ctor->CreateStoreWithOrdering(value, ptrData, LLVMArkInterface::VOLATILE_ORDER);
    // Post
    if (inst->CastToStoreResolvedObjectField()->GetNeedBarrier()) {
        ctor->CreatePostWRB(inst, obj, offset, value);
    }
}

void LLVMIrConstructor::VisitResolveObjectFieldStatic(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto resolverInst = inst->CastToResolveObjectFieldStatic();

    auto entrypoint = RuntimeInterface::EntrypointId::GET_UNKNOWN_STATIC_FIELD_MEMORY_ADDRESS;

    auto typeId = ctor->builder_.getInt64(resolverInst->GetTypeId());
    auto slotPtr = llvm::Constant::getNullValue(ctor->builder_.getPtrTy());

    auto ptrInt = ctor->CreateEntrypointCall(entrypoint, inst, {ctor->GetMethodArgument(), typeId, slotPtr});
    auto n = ctor->builder_.CreateIntToPtr(ptrInt, ctor->builder_.getPtrTy());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitLoadResolvedObjectFieldStatic(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto offset = ctor->GetInputValue(inst, 0);

    auto casted = ctor->builder_.CreateIntToPtr(offset, ctor->builder_.getPtrTy());
    auto n = ctor->CreateLoadWithOrdering(inst, casted, LLVMArkInterface::VOLATILE_ORDER);
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitStoreResolvedObjectFieldStatic(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    [[maybe_unused]] auto storeInst = inst->CastToStoreResolvedObjectFieldStatic();

    ASSERT(!DataType::IsReference(inst->GetType()));
    ASSERT(!storeInst->GetNeedBarrier());

    auto value = ctor->GetInputValue(inst, 1);
    auto destPtr = ctor->GetInputValue(inst, 0);

    [[maybe_unused]] auto dtype = inst->GetType();
    ASSERT(value->getType()->getScalarSizeInBits() == DataType::GetTypeSize(dtype, ctor->GetGraph()->GetArch()));
    ctor->CreateStoreWithOrdering(value, destPtr, LLVMArkInterface::VOLATILE_ORDER);
}

void LLVMIrConstructor::VisitBitcast(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto type = inst->GetType();
    auto llvmTargetType = ctor->GetExactType(type);
    auto input = ctor->GetInputValue(inst, 0);
    auto itype = inst->GetInputType(0);

    llvm::Value *n;
    if (itype == DataType::POINTER) {
        ASSERT(!llvmTargetType->isPointerTy());
        n = ctor->builder_.CreatePtrToInt(input, llvmTargetType);
    } else {
        if (type == DataType::REFERENCE) {
            n = ctor->builder_.CreateIntToPtr(input, ctor->builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));
        } else if (type == DataType::POINTER) {
            n = ctor->builder_.CreateIntToPtr(input, ctor->builder_.getPtrTy());
        } else {
            n = ctor->builder_.CreateBitCast(input, llvmTargetType);
        }
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitCast(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto x = ctor->GetInputValue(inst, 0);

    auto type = inst->GetInputType(0);
    auto targetType = inst->GetType();
    auto llvmTargetType = ctor->GetExactType(targetType);
    // Do not cast if either Ark or LLVM types are the same
    if (type == targetType || x->getType() == llvmTargetType) {
        ctor->ValueMapAdd(inst, x, false);
        return;
    }

    if (DataType::IsFloatType(type) && IsInteger(targetType)) {
        // float to int, e.g. F64TOI32, F32TOI64, F64TOU32, F32TOU64
        auto n = ctor->CreateCastToInt(inst);
        ctor->ValueMapAdd(inst, n);
        return;
    }
    auto op = ctor->GetCastOp(type, targetType);
    if (targetType == DataType::BOOL) {
        ASSERT(op == llvm::Instruction::Trunc);
        auto u1 = ctor->builder_.CreateIsNotNull(x, CreateNameForInst(inst));
        auto n = ctor->builder_.CreateZExt(u1, ctor->builder_.getInt8Ty());
        ctor->ValueMapAdd(inst, n, false);
        return;
    }
    auto n = ctor->builder_.CreateCast(op, x, llvmTargetType);
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitAnd(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryOp(inst, llvm::Instruction::And);
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitAndI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::And, inst->CastToAndI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitOr(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryOp(inst, llvm::Instruction::Or);
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitOrI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::Or, inst->CastToOrI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitXor(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryOp(inst, llvm::Instruction::Xor);
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitXorI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::Xor, inst->CastToXorI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitShl(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateShiftOp(inst, llvm::Instruction::Shl);
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitShlI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::Shl, inst->CastToShlI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitShr(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateShiftOp(inst, llvm::Instruction::LShr);
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitShrI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::LShr, inst->CastToShrI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitAShr(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateShiftOp(inst, llvm::Instruction::AShr);
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitAShrI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::AShr, inst->CastToAShrI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitAdd(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    llvm::Value *n;
    if (IsFloatType(inst->GetType())) {
        n = ctor->CreateBinaryOp(inst, llvm::Instruction::FAdd);
    } else if (IsTypeNumeric(inst->GetType())) {
        n = ctor->CreateBinaryOp(inst, llvm::Instruction::Add);
    } else {
        UNREACHABLE();
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitAddI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::Add, inst->CastToAddI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitSub(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    llvm::Value *n;
    if (IsFloatType(inst->GetType())) {
        n = ctor->CreateBinaryOp(inst, llvm::Instruction::FSub);
    } else if (IsTypeNumeric(inst->GetType())) {
        n = ctor->CreateBinaryOp(inst, llvm::Instruction::Sub);
    } else {
        UNREACHABLE();
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitSubI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::Sub, inst->CastToSubI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitMul(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    llvm::Value *n;
    if (IsFloatType(inst->GetType())) {
        n = ctor->CreateBinaryOp(inst, llvm::Instruction::FMul);
    } else if (IsTypeNumeric(inst->GetType())) {
        n = ctor->CreateBinaryOp(inst, llvm::Instruction::Mul);
    } else {
        UNREACHABLE();
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitMulI(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto n = ctor->CreateBinaryImmOp(inst, llvm::Instruction::Mul, inst->CastToMulI()->GetImm());
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitDiv(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto type = inst->GetType();
    llvm::Value *n;
    if (IsFloatType(type)) {
        n = ctor->CreateBinaryOp(inst, llvm::Instruction::FDiv);
    } else if (IsInteger(type)) {
        if (IsSignedInteger(type)) {
            n = ctor->CreateSignDivMod(inst, llvm::Instruction::SDiv);
        } else {
            n = ctor->CreateBinaryOp(inst, llvm::Instruction::UDiv);
        }
    } else {
        UNREACHABLE();
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitMod(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto type = inst->GetType();
    llvm::Value *n;
    if (IsFloatType(type)) {
        n = ctor->CreateBinaryOp(inst, llvm::Instruction::FRem);
    } else if (IsInteger(type)) {
        if (IsSignedInteger(type)) {
            n = ctor->CreateSignDivMod(inst, llvm::Instruction::SRem);
        } else {
            n = ctor->CreateBinaryOp(inst, llvm::Instruction::URem);
        }
    } else {
        UNREACHABLE();
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitMin(GraphVisitor *v, Inst *inst)
{
    ASSERT(g_options.IsCompilerEncodeIntrinsics());
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto operType = inst->CastToMin()->GetType();
    llvm::Value *x = ctor->GetInputValue(inst, 0);
    llvm::Value *y = ctor->GetInputValue(inst, 1);
    llvm::Intrinsic::ID llvmId = 0;

    if (DataType::IsFloatType(operType)) {
        llvmId = llvm::Intrinsic::minimum;
    } else if (IsInteger(operType)) {
        llvmId = DataType::IsTypeSigned(operType) ? llvm::Intrinsic::smin : llvm::Intrinsic::umin;
    } else {
        ASSERT_DO(false, (std::cerr << "Min is not supported for type " << DataType::ToString(operType) << std::endl));
        UNREACHABLE();
    }
    auto min = ctor->builder_.CreateBinaryIntrinsic(llvmId, x, y);
    ctor->ValueMapAdd(inst, min);
}

void LLVMIrConstructor::VisitMax(GraphVisitor *v, Inst *inst)
{
    ASSERT(g_options.IsCompilerEncodeIntrinsics());
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto operType = inst->CastToMax()->GetType();
    llvm::Value *x = ctor->GetInputValue(inst, 0);
    llvm::Value *y = ctor->GetInputValue(inst, 1);
    llvm::Intrinsic::ID llvmId = 0;

    if (DataType::IsFloatType(operType)) {
        llvmId = llvm::Intrinsic::maximum;
    } else if (IsInteger(operType)) {
        llvmId = DataType::IsTypeSigned(operType) ? llvm::Intrinsic::smax : llvm::Intrinsic::umax;
    } else {
        ASSERT_DO(false, (std::cerr << "Max is not supported for type " << DataType::ToString(operType) << std::endl));
        UNREACHABLE();
    }
    auto max = ctor->builder_.CreateBinaryIntrinsic(llvmId, x, y);
    ctor->ValueMapAdd(inst, max);
}

void LLVMIrConstructor::VisitCompare(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto compareInst = inst->CastToCompare();
    auto operandsType = compareInst->GetOperandsType();

    llvm::Value *x = ctor->GetInputValue(inst, 0);
    llvm::Value *y = ctor->GetInputValue(inst, 1);

    llvm::Value *n = nullptr;
    if (IsInteger(operandsType) || DataType::IsReference(operandsType)) {
        n = ctor->CreateCondition(compareInst->GetCc(), x, y);
    } else {
        n = ctor->builder_.CreateFCmp(FCmpCodeConvert(compareInst->GetCc()), x, y);
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitCmp(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    CmpInst *cmpInst = inst->CastToCmp();
    DataType::Type operandsType = cmpInst->GetOperandsType();

    auto x = ctor->GetInputValue(inst, 0);
    auto y = ctor->GetInputValue(inst, 1);
    llvm::Value *n;
    if (DataType::IsFloatType(operandsType)) {
        n = ctor->CreateFloatComparison(cmpInst, x, y);
    } else if (IsInteger(operandsType)) {
        n = ctor->CreateIntegerComparison(cmpInst, x, y);
    } else {
        ASSERT_DO(false, (std::cerr << "Unsupported comparison for operands of type = "
                                    << DataType::ToString(operandsType) << std::endl));
        UNREACHABLE();
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitNeg(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto inputType = inst->GetInputType(0);
    auto toNegate = ctor->GetInputValue(inst, 0);
    llvm::Value *n;
    if (inputType == DataType::Type::FLOAT64 || inputType == DataType::Type::FLOAT32) {
        n = ctor->builder_.CreateFNeg(toNegate);
    } else if (IsInteger(inputType)) {
        n = ctor->builder_.CreateNeg(toNegate);
    } else {
        ASSERT_DO(false, (std::cerr << "Negation is not supported for" << DataType::ToString(inputType) << std::endl));
        UNREACHABLE();
    }
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitNot(GraphVisitor *v, Inst *inst)
{
    ASSERT(IsInteger(inst->GetInputType(0)));

    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto input = ctor->GetInputValue(inst, 0);

    auto notOperator = ctor->builder_.CreateNot(input);
    ctor->ValueMapAdd(inst, notOperator);
}

void LLVMIrConstructor::VisitIfImm(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto x = ctor->GetInputValue(inst, 0);
    auto ifimm = inst->CastToIfImm();

    llvm::Value *cond = nullptr;
    if (ifimm->GetCc() == ConditionCode::CC_NE && ifimm->GetImm() == 0 && x->getType()->isIntegerTy()) {
        ASSERT(ifimm->GetOperandsType() == DataType::BOOL);
        cond = ctor->builder_.CreateTrunc(x, ctor->builder_.getInt1Ty());
    } else {
        ASSERT(x->getType()->isIntOrPtrTy());
        llvm::Constant *immCst;
        if (x->getType()->isPointerTy()) {
            if (ifimm->GetImm() == 0) {
                immCst = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(x->getType()));
            } else {
                immCst = llvm::ConstantInt::getSigned(x->getType(), ifimm->GetImm());
                immCst = llvm::ConstantExpr::getPointerCast(immCst, x->getType());
            }
        } else {
            immCst = llvm::ConstantInt::getSigned(x->getType(), ifimm->GetImm());
        }
        cond = ctor->CreateCondition(ifimm->GetCc(), x, immCst);
    }
    ctor->CreateIf(inst, cond, ifimm->IsLikely(), ifimm->IsUnlikely());
}

void LLVMIrConstructor::VisitIf(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto x = ctor->GetInputValue(inst, 0);
    auto y = ctor->GetInputValue(inst, 1);
    ASSERT(x->getType()->isIntOrPtrTy());
    ASSERT(y->getType()->isIntOrPtrTy());
    auto ifi = inst->CastToIf();
    auto cond = ctor->CreateCondition(ifi->GetCc(), x, y);
    ctor->CreateIf(inst, cond, ifi->IsLikely(), ifi->IsUnlikely());
}

void LLVMIrConstructor::VisitCallIndirect(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto ptr = ctor->GetInputValue(inst, 0);
    ASSERT_TYPE(ptr, ctor->builder_.getPtrTy());
    // Build FunctionType
    ArenaVector<llvm::Type *> argTypes(ctor->GetGraph()->GetLocalAllocator()->Adapter());
    ArenaVector<llvm::Value *> args(ctor->GetGraph()->GetLocalAllocator()->Adapter());
    for (size_t i = 1; i < inst->GetInputs().Size(); ++i) {
        argTypes.push_back(ctor->GetType(inst->GetInput(i).GetInst()->GetType()));
        args.push_back(ctor->GetInputValue(inst, i));
    }
    auto retType = ctor->GetType(inst->GetType());
    auto funcType = llvm::FunctionType::get(retType, argTypes, false);
    auto call = ctor->builder_.CreateCall(funcType, ptr, args);
    if (!retType->isVoidTy()) {
        ctor->ValueMapAdd(inst, call);
    }
}

void LLVMIrConstructor::VisitCall(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    ASSERT(!ctor->GetGraph()->SupportManagedCode());

    // Prepare external call if needed
    auto externalId = inst->CastToCall()->GetCallMethodId();
    auto runtime = ctor->GetGraph()->GetRuntime();
    auto externalName = runtime->GetExternalMethodName(ctor->GetGraph()->GetMethod(), externalId);
    auto function = ctor->func_->getParent()->getFunction(externalName);
    if (function == nullptr) {
        ArenaVector<llvm::Type *> argTypes(ctor->GetGraph()->GetLocalAllocator()->Adapter());
        for (size_t i = 0; i < inst->GetInputs().Size(); ++i) {
            argTypes.push_back(ctor->GetType(inst->GetInputType(i)));
        }
        auto ftype = llvm::FunctionType::get(ctor->GetType(inst->GetType()), argTypes, false);
        function =
            llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, externalName, ctor->func_->getParent());
    }
    // Arguments
    ArenaVector<llvm::Value *> args(ctor->GetGraph()->GetLocalAllocator()->Adapter());
    for (size_t i = 0; i < inst->GetInputs().Size(); ++i) {
        args.push_back(ctor->CoerceValue(ctor->GetInputValue(inst, i), function->getArg(i)->getType()));
    }
    // Call
    auto call = ctor->builder_.CreateCall(function->getFunctionType(), function, args);

    if (IsNoAliasIrtocFunction(externalName)) {
        ASSERT(call->getType()->isPointerTy());
        call->addRetAttr(llvm::Attribute::NoAlias);
    } else {
        ASSERT(call->getType()->isPointerTy() ^ !IsPtrIgnIrtocFunction(externalName));
    }

    // Check if function has debug info
    if (call->getFunction()->getSubprogram() != nullptr) {
        ctor->debugData_->SetLocation(call, inst->GetPc());
    }

    if (inst->GetType() != DataType::VOID) {
        ctor->ValueMapAdd(inst, ctor->CoerceValue(call, ctor->GetType(inst->GetType())));
    }
}

void LLVMIrConstructor::VisitPhi(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto ltype = ctor->GetExactType(inst->GetType());
    auto block = ctor->GetCurrentBasicBlock();

    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        auto srcOp = inst->GetInput(i).GetInst()->GetOpcode();
        if (inst->GetInput(i).GetInst()->IsClassInst() || srcOp == Opcode::LoadImmediate) {
            ASSERT(ltype == ctor->builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));
            ltype = ctor->builder_.getPtrTy();
            break;
        }
    }

    // PHI need adjusted insert point if ValueMapAdd already created coerced values for other PHIs
    auto nonPhi = block->getFirstNonPHI();
    if (nonPhi != nullptr) {
        ctor->builder_.SetInsertPoint(nonPhi);
    }

    auto phi = ctor->builder_.CreatePHI(ltype, inst->GetInputsCount());
    ctor->SetCurrentBasicBlock(block);
    ctor->ValueMapAdd(inst, phi);
}

void LLVMIrConstructor::VisitMultiArray(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);

    ArenaVector<llvm::Value *> args(ctor->GetGraph()->GetLocalAllocator()->Adapter());
    args.push_back(ctor->GetInputValue(inst, 0));

    auto sizesCount = inst->GetInputsCount() - 2U;
    args.push_back(ctor->builder_.getInt32(sizesCount));
    auto sizes = ctor->CreateAllocaForArgs(ctor->builder_.getInt64Ty(), sizesCount);

    // Store multi-array sizes
    for (size_t i = 1; i <= sizesCount; i++) {
        auto size = ctor->GetInputValue(inst, i);

        auto type = inst->GetInputType(i);
        if (type != DataType::INT64) {
            size = ctor->CoerceValue(size, type, DataType::INT64);
        }

        auto gep = ctor->builder_.CreateConstInBoundsGEP1_32(ctor->builder_.getInt64Ty(), sizes, i - 1);
        ctor->builder_.CreateStore(size, gep);
    }
    args.push_back(sizes);

    auto entrypointId = RuntimeInterface::EntrypointId::CREATE_MULTI_ARRAY;
    auto result = ctor->CreateEntrypointCall(entrypointId, inst, args);
    ctor->MarkAsAllocation(result);
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        result->addFnAttr(llvm::Attribute::get(result->getContext(), "needs-mem-barrier"));
    }
    ctor->ValueMapAdd(inst, result);
}

void LLVMIrConstructor::VisitInitEmptyString(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto eid = RuntimeInterface::EntrypointId::CREATE_EMPTY_STRING;
    auto result = ctor->CreateEntrypointCall(eid, inst);
    ctor->MarkAsAllocation(result);
    ctor->ValueMapAdd(inst, result);
}

void LLVMIrConstructor::VisitInitString(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto strInit = inst->CastToInitString();
    auto arg = ctor->GetInputValue(inst, 0);
    if (strInit->IsFromString()) {
        auto result = ctor->CreateNewStringFromStringTlab(inst, arg);
        ctor->ValueMapAdd(inst, result);
    } else {
        auto lengthOffset = ctor->GetGraph()->GetRuntime()->GetArrayLengthOffset(ctor->GetGraph()->GetArch());
        auto lengthPtr = ctor->builder_.CreateConstInBoundsGEP1_32(ctor->builder_.getInt8Ty(), arg, lengthOffset);
        auto length = ctor->builder_.CreateLoad(ctor->builder_.getInt32Ty(), lengthPtr);
        auto result = ctor->CreateNewStringFromCharsTlab(
            inst, llvm::Constant::getNullValue(ctor->builder_.getInt32Ty()), length, arg);
        ctor->ValueMapAdd(inst, result);
    }
}

void LLVMIrConstructor::VisitNewArray(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto method = inst->CastToNewArray()->GetMethod();

    auto type = ctor->GetInputValue(inst, 0);
    auto size = ctor->ToSizeT(ctor->GetInputValue(inst, 1));
    auto arrayType = inst->CastToNewArray()->GetTypeId();
    auto runtime = ctor->GetGraph()->GetRuntime();
    auto maxTlabSize = runtime->GetTLABMaxSize();
    if (maxTlabSize == 0) {
        auto result = ctor->CreateNewArrayWithRuntime(inst);
        ctor->ValueMapAdd(inst, result);
        return;
    }

    auto lenInst = inst->GetDataFlowInput(0);
    auto classArraySize = runtime->GetClassArraySize(ctor->GetGraph()->GetArch());
    uint64_t arraySize = 0;
    uint64_t elementSize = runtime->GetArrayElementSize(method, arrayType);
    uint64_t alignment = runtime->GetTLABAlignment();
    ASSERT(alignment != 0);

    if (lenInst->GetOpcode() == Opcode::Constant) {
        ASSERT(lenInst->GetType() == DataType::INT64);
        arraySize = lenInst->CastToConstant()->GetIntValue() * elementSize + classArraySize;
        arraySize = (arraySize & ~(alignment - 1U)) + ((arraySize % alignment) != 0U ? alignment : 0U);
        if (arraySize > maxTlabSize) {
            auto result = ctor->CreateNewArrayWithRuntime(inst);
            ctor->ValueMapAdd(inst, result);
            return;
        }
    }
    auto eid = GetAllocateArrayTlabEntrypoint(elementSize);
    auto result = ctor->CreateFastPathCall(inst, eid, {type, size});
    ctor->MarkAsAllocation(result);
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        result->addFnAttr(llvm::Attribute::get(result->getContext(), "needs-mem-barrier"));
    }
    ctor->ValueMapAdd(inst, result);
}

void LLVMIrConstructor::VisitNewObject(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);

    auto newObjInst = inst->CastToNewObject();
    auto srcInst = newObjInst->GetInput(0).GetInst();

    auto runtime = ctor->GetGraph()->GetRuntime();
    auto maxTlabSize = runtime->GetTLABMaxSize();
    if (maxTlabSize == 0 || srcInst->GetOpcode() != Opcode::LoadAndInitClass) {
        auto runtimeCall = ctor->CreateNewObjectWithRuntime(inst);
        ctor->ValueMapAdd(inst, runtimeCall);
        return;
    }

    auto klass = srcInst->CastToLoadAndInitClass()->GetClass();
    if (klass == nullptr || !runtime->CanUseTlabForClass(klass)) {
        auto runtimeCall = ctor->CreateNewObjectWithRuntime(inst);
        ctor->ValueMapAdd(inst, runtimeCall);
        return;
    }
    auto classSize = runtime->GetClassSize(klass);
    auto alignment = runtime->GetTLABAlignment();
    ASSERT(alignment != 0);

    classSize = (classSize & ~(alignment - 1U)) + ((classSize % alignment) != 0U ? alignment : 0U);
    if (classSize > maxTlabSize) {
        auto runtimeCall = ctor->CreateNewObjectWithRuntime(inst);
        ctor->ValueMapAdd(inst, runtimeCall);
        return;
    }

    auto initClass = ctor->GetInputValue(inst, 0);
    auto klassSize = ctor->ToSizeT(ctor->builder_.getInt32(classSize));
    auto eid = RuntimeInterface::EntrypointId::ALLOCATE_OBJECT_TLAB;
    auto result = ctor->CreateFastPathCall(inst, eid, {initClass, klassSize});
    ctor->MarkAsAllocation(result);
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        result->addFnAttr(llvm::Attribute::get(result->getContext(), "needs-mem-barrier"));
    }
    ctor->ValueMapAdd(inst, result);
}

void LLVMIrConstructor::VisitCallStatic(GraphVisitor *v, Inst *inst)
{
    auto call = inst->CastToCallStatic();
    if (call->IsInlined()) {
        return;
    }

    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto methodPtr = ctor->GetGraph()->GetMethod();
    auto methodId = call->GetCallMethodId();
    auto callee = ctor->GetGraph()->GetRuntime()->GetMethodById(methodPtr, methodId);
    ASSERT(callee != nullptr);
    // Create a declare statement if we haven't met this function yet
    auto function = ctor->GetOrCreateFunctionForCall(call, callee);
    ctor->arkInterface_->RememberFunctionCall(ctor->func_, function, methodId);

    // Replaced to real callee in the PandaRuntimeLowering
    auto args = ctor->GetArgumentsForCall(ctor->GetMethodArgument(), call);
    auto result = ctor->builder_.CreateCall(function, args, ctor->CreateSaveStateBundle(inst));
    ctor->WrapArkCall(inst, result);

    if (inst->GetType() != DataType::VOID) {
        ctor->ValueMapAdd(inst, result);
    }

    if (ctor->GetGraph()->GetRuntime()->IsMethodExternal(methodPtr, callee)) {
        result->addAttributeAtIndex(llvm::AttributeList::FunctionIndex, llvm::Attribute::NoInline);
    }
    if (IsAlwaysThrowBasicBlock(inst)) {
        result->addAttributeAtIndex(llvm::AttributeList::FunctionIndex, llvm::Attribute::NoInline);
        result->addFnAttr(llvm::Attribute::get(ctor->func_->getContext(), "keep-noinline"));
    }
}

void LLVMIrConstructor::VisitResolveStatic(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto call = inst->CastToResolveStatic();

    auto slotPtr = llvm::Constant::getNullValue(ctor->builder_.getPtrTy());
    auto methodPtr = ctor->CreateEntrypointCall(
        RuntimeInterface::EntrypointId::GET_UNKNOWN_CALLEE_METHOD, inst,
        {ctor->GetMethodArgument(), ctor->ToSizeT(ctor->builder_.getInt64(call->GetCallMethodId())), slotPtr});
    auto method = ctor->builder_.CreateIntToPtr(methodPtr, ctor->builder_.getPtrTy());

    ctor->ValueMapAdd(inst, method);
}

void LLVMIrConstructor::VisitCallResolvedStatic(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto call = inst->CastToCallResolvedStatic();
    if (call->IsInlined()) {
        return;
    }

    auto method = ctor->GetInputValue(inst, 0);

    llvm::FunctionType *fType = ctor->GetFunctionTypeForCall(call);
    auto args = ctor->GetArgumentsForCall(method, call, true);  // skip first input

    auto offset = ctor->GetGraph()->GetRuntime()->GetCompiledEntryPointOffset(ctor->GetGraph()->GetArch());
    auto entrypointPtr = ctor->builder_.CreateConstInBoundsGEP1_32(ctor->builder_.getInt8Ty(), method, offset);
    auto entrypoint = ctor->builder_.CreateLoad(ctor->builder_.getPtrTy(), entrypointPtr);

    auto result = ctor->builder_.CreateCall(fType, entrypoint, args, ctor->CreateSaveStateBundle(inst));
    if (inst->GetType() != DataType::VOID) {
        ctor->ValueMapAdd(inst, result);
    }
    ctor->WrapArkCall(inst, result);
}

template <typename T>
llvm::Function *CreateDeclForVirtualCall(T *inst, LLVMIrConstructor *ctor, LLVMArkInterface *arkInterface)
{
    arkInterface->GetOrCreateRuntimeFunctionType(
        ctor->GetFunc()->getContext(), ctor->GetFunc()->getParent(), LLVMArkInterface::RuntimeCallType::ENTRYPOINT,
        static_cast<LLVMArkInterface::EntrypointId>(RuntimeInterface::EntrypointId::RESOLVE_VIRTUAL_CALL_AOT));
    arkInterface->GetOrCreateRuntimeFunctionType(
        ctor->GetFunc()->getContext(), ctor->GetFunc()->getParent(), LLVMArkInterface::RuntimeCallType::ENTRYPOINT,
        static_cast<LLVMArkInterface::EntrypointId>(RuntimeInterface::EntrypointId::INTF_INLINE_CACHE));

    auto methodPtr = ctor->GetGraph()->GetMethod();
    auto methodId = inst->GetCallMethodId();
    auto callee = ctor->GetGraph()->GetRuntime()->GetMethodById(methodPtr, methodId);
    ASSERT(callee != nullptr);

    std::stringstream ssUniqName;
    ssUniqName << "f_" << std::hex << inst;
    auto uniqName = ssUniqName.str();
    auto methodName = arkInterface->GetUniqMethodName(callee) + "_" + uniqName;
    auto functionProto = ctor->GetFunctionTypeForCall(inst);
    auto func = CreateFunctionDeclaration(functionProto, methodName, ctor->GetFunc()->getParent());

    func->addFnAttr("frame-pointer", "all");
    arkInterface->PutVirtualFunction(inst->GetCallMethod(), func);
    return func;
}

void LLVMIrConstructor::VisitCallVirtual(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto call = inst->CastToCallVirtual();
    if (call->IsInlined()) {
        return;
    }
    ASSERT_PRINT(ctor->GetGraph()->GetAotData()->GetUseCha(),
                 std::string("GetUseCha must be 'true' but was 'false' for method = '") +
                     ctor->GetGraph()->GetRuntime()->GetMethodFullName(ctor->GetGraph()->GetMethod()) + "'");

    ASSERT(!ctor->GetGraph()->GetRuntime()->IsInterfaceMethod(call->GetCallMethod()));
    auto methodId = call->GetCallMethodId();

    auto func = CreateDeclForVirtualCall(call, ctor, ctor->arkInterface_);
    auto args = ctor->GetArgumentsForCall(ctor->GetMethodArgument(), call);
    auto result = ctor->builder_.CreateCall(func, args, ctor->CreateSaveStateBundle(inst));
    result->addFnAttr(llvm::Attribute::get(result->getContext(), "original-method-id", std::to_string(methodId)));
    if (inst->GetType() != DataType::VOID) {
        ctor->ValueMapAdd(inst, result);
    }
    ctor->WrapArkCall(inst, result);
}

void LLVMIrConstructor::VisitResolveVirtual(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto resolver = inst->CastToResolveVirtual();

    llvm::Value *method = nullptr;
    if (resolver->GetCallMethod() == nullptr) {
        llvm::Value *thiz = ctor->GetInputValue(inst, 0);
        method = ctor->CreateResolveVirtualCallBuiltin(inst, thiz, resolver->GetCallMethodId());
        ASSERT(method->getType()->isPointerTy());
    } else {
        ASSERT(ctor->GetGraph()->GetRuntime()->IsInterfaceMethod(resolver->GetCallMethod()));
        method = CreateDeclForVirtualCall(resolver, ctor, ctor->arkInterface_);
    }
    ctor->ValueMapAdd(inst, method, false);
}

void LLVMIrConstructor::VisitCallResolvedVirtual(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto call = inst->CastToCallResolvedVirtual();
    if (call->IsInlined()) {
        return;
    }
    auto runtime = ctor->GetGraph()->GetRuntime();
    auto method = ctor->GetInputValue(inst, 0);
    auto args = ctor->GetArgumentsForCall(method, call, true);

    llvm::CallInst *result = nullptr;
    if (call->GetCallMethod() == nullptr) {
        llvm::FunctionType *fType = ctor->GetFunctionTypeForCall(call);

        auto offset = runtime->GetCompiledEntryPointOffset(ctor->GetGraph()->GetArch());
        auto entrypointPtr = ctor->builder_.CreateConstInBoundsGEP1_32(ctor->builder_.getInt8Ty(), method, offset);
        auto entrypoint = ctor->builder_.CreateLoad(ctor->builder_.getPtrTy(), entrypointPtr);
        result = ctor->builder_.CreateCall(fType, entrypoint, args, ctor->CreateSaveStateBundle(inst));
    } else {
        ASSERT(runtime->IsInterfaceMethod(call->GetCallMethod()));
        auto *func = llvm::cast<llvm::Function>(method);
        result = ctor->builder_.CreateCall(func, args, ctor->CreateSaveStateBundle(inst));
        auto methodId = call->GetCallMethodId();
        result->addFnAttr(llvm::Attribute::get(result->getContext(), "original-method-id", std::to_string(methodId)));
    }
    if (inst->GetType() != DataType::VOID) {
        ctor->ValueMapAdd(inst, result);
    }
    ctor->WrapArkCall(inst, result);
}

void LLVMIrConstructor::VisitAbs(GraphVisitor *v, Inst *inst)
{
    ASSERT(g_options.IsCompilerEncodeIntrinsics());
    auto ctor = static_cast<LLVMIrConstructor *>(v);

    DataType::Type pandaType = inst->GetInputType(0);
    auto argument = ctor->GetInputValue(inst, 0);
    llvm::Value *result = nullptr;
    if (DataType::IsFloatType(pandaType)) {
        result = ctor->builder_.CreateUnaryIntrinsic(llvm::Intrinsic::fabs, argument, nullptr);
    } else if (IsInteger(pandaType)) {
        result = ctor->builder_.CreateBinaryIntrinsic(llvm::Intrinsic::abs, argument, ctor->builder_.getFalse());
    } else {
        ASSERT_DO(false, (std::cerr << "Abs is not supported for type " << DataType::ToString(pandaType) << std::endl));
        UNREACHABLE();
    }
    ASSERT(result != nullptr);
    ctor->ValueMapAdd(inst, result);
}

void LLVMIrConstructor::VisitIntrinsic(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto entryId = inst->CastToIntrinsic()->GetIntrinsicId();

    // Some Ark intrinsics are lowered into code or LLVM intrinsics, the IntrinsicsLowering pass
    // makes final desicion for them to be lowered into code or calling Ark entrypoint.
    if (g_options.IsCompilerEncodeIntrinsics()) {
        bool lowered = ctor->TryEmitIntrinsic(inst, entryId);
        if (lowered) {
            return;
        }
    }
    // Create call otherwise
    auto result = ctor->CreateIntrinsicCall(inst);
    if (inst->GetType() != DataType::VOID) {
        ctor->ValueMapAdd(inst, result);
    }
}

void LLVMIrConstructor::VisitMonitor(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    MonitorInst *monitor = inst->CastToMonitor();
    auto object = ctor->GetInputValue(inst, 0);
    auto eid = monitor->IsEntry() ? RuntimeInterface::EntrypointId::MONITOR_ENTER_FAST_PATH
                                  : RuntimeInterface::EntrypointId::MONITOR_EXIT_FAST_PATH;
    auto call = ctor->CreateEntrypointCall(eid, inst, {object});
    ASSERT(call->getCallingConv() == llvm::CallingConv::C);
    call->setCallingConv(llvm::CallingConv::ArkFast1);
}

void LLVMIrConstructor::VisitSqrt(GraphVisitor *v, Inst *inst)
{
    ASSERT(g_options.IsCompilerEncodeIntrinsics());
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto argument = ctor->GetInputValue(inst, 0);
    auto result = ctor->builder_.CreateUnaryIntrinsic(llvm::Intrinsic::sqrt, argument, nullptr);
    ctor->ValueMapAdd(inst, result);
}

void LLVMIrConstructor::VisitInitClass(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto classId = inst->CastToInitClass()->GetTypeId();

    auto constexpr INITIALIZED = true;
    ctor->CreateLoadClassById(inst, classId, INITIALIZED);
}

void LLVMIrConstructor::VisitLoadClass(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto classId = inst->CastToLoadClass()->GetTypeId();

    auto constexpr INITIALIZED = true;
    auto clsPtr = ctor->CreateLoadClassById(inst, classId, !INITIALIZED);
    ctor->ValueMapAdd(inst, clsPtr);
}

void LLVMIrConstructor::VisitLoadAndInitClass(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto classId = inst->CastToLoadAndInitClass()->GetTypeId();

    auto constexpr INITIALIZED = true;
    auto clsPtr = ctor->CreateLoadClassById(inst, classId, INITIALIZED);
    ctor->ValueMapAdd(inst, clsPtr);
}

void LLVMIrConstructor::VisitUnresolvedLoadAndInitClass(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto classId = inst->CastToUnresolvedLoadAndInitClass()->GetTypeId();

    auto constexpr INITIALIZED = true;
    auto clsPtr = ctor->CreateLoadClassById(inst, classId, INITIALIZED);
    ctor->ValueMapAdd(inst, clsPtr);
}

void LLVMIrConstructor::VisitLoadStatic(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto klass = ctor->GetInputValue(inst, 0);
    ASSERT_TYPE(klass, ctor->builder_.getPtrTy());

    auto offset = ctor->GetGraph()->GetRuntime()->GetFieldOffset(inst->CastToLoadStatic()->GetObjField());
    auto fieldPtr = ctor->builder_.CreateConstInBoundsGEP1_32(ctor->builder_.getInt8Ty(), klass, offset);

    auto n = ctor->CreateLoadWithOrdering(inst, fieldPtr, ToAtomicOrdering(inst->CastToLoadStatic()->GetVolatile()));
    ctor->ValueMapAdd(inst, n);
}

void LLVMIrConstructor::VisitStoreStatic(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto klass = ctor->GetInputValue(inst, 0);
    ASSERT_TYPE(klass, ctor->builder_.getPtrTy());
    auto value = ctor->GetInputValue(inst, 1);

    auto runtime = ctor->GetGraph()->GetRuntime();
    auto offset = runtime->GetFieldOffset(inst->CastToStoreStatic()->GetObjField());
    auto fieldPtr = ctor->builder_.CreateConstInBoundsGEP1_32(ctor->builder_.getInt8Ty(), klass, offset);

    // Pre
    if (inst->CastToStoreStatic()->GetNeedBarrier()) {
        ctor->CreatePreWRB(inst, fieldPtr);
    }
    // Write
    ctor->CreateStoreWithOrdering(value, fieldPtr, ToAtomicOrdering(inst->CastToStoreStatic()->GetVolatile()));
    // Post
    if (inst->CastToStoreStatic()->GetNeedBarrier()) {
        auto barrierType = runtime->GetPostType();
        if (barrierType == mem::BarrierType::POST_INTERREGION_BARRIER) {
            ctor->CreatePostWRB(inst, klass, ctor->builder_.getInt32(offset), value);
        } else {
            auto managed = ctor->CreateLoadManagedClassFromClass(klass);
            ctor->CreatePostWRB(inst, managed, ctor->builder_.getInt32(0), value);
        }
    }
}

void LLVMIrConstructor::VisitUnresolvedStoreStatic(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto unresolvedStore = inst->CastToUnresolvedStoreStatic();

    ASSERT(unresolvedStore->GetNeedBarrier());
    ASSERT(DataType::IsReference(inst->GetType()));

    auto typeId = ctor->builder_.getInt64(unresolvedStore->GetTypeId());
    auto value = ctor->GetInputValue(inst, 0);

    auto entrypoint = RuntimeInterface::EntrypointId::UNRESOLVED_STORE_STATIC_BARRIERED;
    ctor->CreateEntrypointCall(entrypoint, inst, {ctor->GetMethodArgument(), typeId, value});
}

void LLVMIrConstructor::VisitLoadConstArray(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto arrayType = inst->CastToLoadConstArray()->GetTypeId();

    llvm::Value *result = ctor->CreateEntrypointCall(RuntimeInterface::EntrypointId::RESOLVE_LITERAL_ARRAY, inst,
                                                     {ctor->GetMethodArgument(), ctor->builder_.getInt32(arrayType)});
    ctor->ValueMapAdd(inst, result);
}

void LLVMIrConstructor::VisitFillConstArray(GraphVisitor *v, Inst *inst)
{
    ASSERT(!DataType::IsReference(inst->GetType()));
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto &builder = ctor->builder_;

    auto runtime = ctor->GetGraph()->GetRuntime();
    auto arrayType = inst->CastToFillConstArray()->GetTypeId();
    auto arch = ctor->GetGraph()->GetArch();
    auto src = ctor->GetInputValue(inst, 0);
    auto offset = runtime->GetArrayDataOffset(arch);
    auto arraySize = inst->CastToFillConstArray()->GetImm() << DataType::ShiftByType(inst->GetType(), arch);
    auto arrayPtr = builder.CreateConstInBoundsGEP1_64(builder.getInt8Ty(), src, offset);

    ASSERT(arraySize != 0);

    auto arrOffset = runtime->GetOffsetToConstArrayData(inst->CastToFillConstArray()->GetMethod(), arrayType);
    auto pfOffset = runtime->GetPandaFileOffset(arch);
    auto fileOffset = runtime->GetBinaryFileBaseOffset(arch);

    auto pfPtrPtr = builder.CreateConstInBoundsGEP1_64(builder.getInt8Ty(), ctor->GetMethodArgument(), pfOffset);
    auto pfPtr = builder.CreateLoad(builder.getPtrTy(), pfPtrPtr);
    auto filePtrPtr = builder.CreateConstInBoundsGEP1_64(builder.getInt8Ty(), pfPtr, fileOffset);
    auto filePtr = builder.CreateLoad(builder.getPtrTy(), filePtrPtr);
    auto constArrPtr = builder.CreateConstInBoundsGEP1_64(builder.getInt8Ty(), filePtr, arrOffset);

    auto align = llvm::MaybeAlign(0);
    /**
     * LLVM AOT may replace `@llvm.memcpy.inline` with call to ark's `LIB_CALL_MEM_COPY`, see `MustLowerMemCpy` in
     * libllvmbackend/llvm_ark_interface.cpp.
     */
    builder.CreateMemCpyInline(arrayPtr, align, constArrPtr, align, builder.getInt64(arraySize));
}

// CC-OFFNXT(huge_method) solid logic
void LLVMIrConstructor::VisitIsInstance(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto isInstance = inst->CastToIsInstance();
    auto klassType = isInstance->GetClassType();
    auto object = ctor->GetInputValue(inst, 0);
    llvm::Value *result;

    // Note(#27161): implement FastPath for unions
    if (klassType == ClassType::UNRESOLVED_CLASS || klassType == ClassType::UNION_CLASS) {
        result = ctor->CreateIsInstanceEntrypointCall(inst);
    } else {
        auto &ctx = ctor->func_->getContext();
        auto preBb = ctor->GetCurrentBasicBlock();
        auto contBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "isinstance_cont"), ctor->func_);

        if (!inst->CastToIsInstance()->GetOmitNullCheck()) {
            auto notnullBb =
                llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "isinstance_notnull"), ctor->func_);
            auto isNullObj = ctor->builder_.CreateIsNull(object);
            ctor->builder_.CreateCondBr(isNullObj, contBb, notnullBb);
            ctor->SetCurrentBasicBlock(notnullBb);
        }

        llvm::Value *innerResult = nullptr;
        auto klassId = ctor->GetInputValue(inst, 1);
        auto klassObj = CreateLoadClassFromObject(object, &ctor->builder_, ctor->arkInterface_);
        auto notnullPostBb = ctor->GetCurrentBasicBlock();
        auto cmp = ctor->builder_.CreateICmpEQ(klassId, klassObj);
        if (klassType == ClassType::FINAL_CLASS) {
            innerResult = ctor->builder_.CreateZExt(cmp, ctor->builder_.getInt8Ty());
        } else {
            auto innerBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "isinstance_inner"), ctor->func_);
            ctor->builder_.CreateCondBr(cmp, contBb, innerBb);
            ctor->SetCurrentBasicBlock(innerBb);
            innerResult = ctor->CreateIsInstanceInnerBlock(inst, klassObj, klassId);
        }
        auto incomingBlock = ctor->GetCurrentBasicBlock();
        ctor->builder_.CreateBr(contBb);

        ctor->SetCurrentBasicBlock(contBb);
        unsigned amount = 1 + (preBb == notnullPostBb ? 0 : 1) + (notnullPostBb == incomingBlock ? 0 : 1);
        auto resultPhi = ctor->builder_.CreatePHI(ctor->builder_.getInt8Ty(), amount);
        if (preBb != notnullPostBb) {
            resultPhi->addIncoming(ctor->builder_.getInt8(0), preBb);
        }
        if (notnullPostBb != incomingBlock) {
            resultPhi->addIncoming(ctor->builder_.getInt8(1), notnullPostBb);
        }
        resultPhi->addIncoming(innerResult, incomingBlock);
        result = resultPhi;
    }

    ctor->ValueMapAdd(inst, result);
}

void LLVMIrConstructor::VisitCheckCast(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto checkCast = inst->CastToCheckCast();
    auto klassType = checkCast->GetClassType();
    auto src = ctor->GetInputValue(inst, 0);

    auto &ctx = ctor->func_->getContext();
    auto outBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "checkcast_out"), ctor->func_);

    // Nullptr check can be omitted sometimes
    if (!inst->CastToCheckCast()->GetOmitNullCheck()) {
        auto contBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "checkcast_cont"), ctor->func_);
        auto isNullptr = ctor->builder_.CreateIsNull(src);
        ctor->builder_.CreateCondBr(isNullptr, outBb, contBb);
        ctor->SetCurrentBasicBlock(contBb);
    }

    // Note(#27161): implement FastPath for unions
    if (klassType == ClassType::UNRESOLVED_CLASS || klassType == ClassType::UNION_CLASS ||
        (klassType == ClassType::INTERFACE_CLASS && inst->CanDeoptimize())) {
        ctor->CreateCheckCastEntrypointCall(inst);
    } else if (klassType == ClassType::INTERFACE_CLASS) {
        ASSERT(!inst->CanDeoptimize());
        auto entrypoint = RuntimeInterface::EntrypointId::CHECK_CAST_INTERFACE;
        ctor->CreateFastPathCall(inst, entrypoint, {src, ctor->GetInputValue(inst, 1)});
    } else {
        auto klassId = ctor->GetInputValue(inst, 1);
        auto klassObj = CreateLoadClassFromObject(src, &ctor->builder_, ctor->arkInterface_);
        if (klassType == ClassType::FINAL_CLASS) {
            auto cmpNe = ctor->builder_.CreateICmpNE(klassId, klassObj);
            auto exception = RuntimeInterface::EntrypointId::CLASS_CAST_EXCEPTION;
            ctor->CreateDeoptimizationBranch(inst, cmpNe, exception, {klassId, src});
        } else {
            auto cmpEq = ctor->builder_.CreateICmpEQ(klassId, klassObj);
            auto innerBb = llvm::BasicBlock::Create(ctx, CreateBasicBlockName(inst, "checkcast_inner"), ctor->func_);
            ctor->builder_.CreateCondBr(cmpEq, outBb, innerBb);
            ctor->SetCurrentBasicBlock(innerBb);
            ctor->CreateCheckCastInner(inst, klassObj, klassId);
        }
    }
    ctor->builder_.CreateBr(outBb);
    ctor->SetCurrentBasicBlock(outBb);
}

void LLVMIrConstructor::VisitLoadType(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);

    auto constexpr INITIALIZED = true;
    auto klass = ctor->CreateLoadClassById(inst, inst->CastToLoadType()->GetTypeId(), !INITIALIZED);
    auto result = ctor->CreateLoadManagedClassFromClass(klass);
    ctor->ValueMapAdd(inst, result);
}

void LLVMIrConstructor::VisitUnresolvedLoadType(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);

    auto constexpr INITIALIZED = true;
    auto klass = ctor->CreateLoadClassById(inst, inst->CastToUnresolvedLoadType()->GetTypeId(), !INITIALIZED);
    auto result = ctor->CreateLoadManagedClassFromClass(klass);
    ctor->ValueMapAdd(inst, result);
}

void LLVMIrConstructor::VisitGetInstanceClass(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);

    auto object = ctor->GetInputValue(inst, 0);
    auto klass = CreateLoadClassFromObject(object, &ctor->builder_, ctor->arkInterface_);
    ctor->ValueMapAdd(inst, klass);
}

void LLVMIrConstructor::VisitThrow(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto obj = ctor->GetInputValue(inst, 0);

    auto exception = RuntimeInterface::EntrypointId::THROW_EXCEPTION;
    ctor->CreateEntrypointCall(exception, inst, {obj});
    ctor->builder_.CreateUnreachable();
}

void LLVMIrConstructor::VisitCatchPhi([[maybe_unused]] GraphVisitor *v, Inst *inst)
{
    UnexpectedLowering(inst);
}

void LLVMIrConstructor::VisitLoadRuntimeClass(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);

    auto offset = ctor->GetGraph()->GetRuntime()->GetTlsPromiseClassPointerOffset(ctor->GetGraph()->GetArch());
    auto result = llvmbackend::runtime_calls::LoadTLSValue(&ctor->builder_, ctor->arkInterface_, offset,
                                                           ctor->builder_.getPtrTy());
    ctor->ValueMapAdd(inst, result);
}

void LLVMIrConstructor::VisitLoadUniqueObject(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);

    auto offset = ctor->GetGraph()->GetRuntime()->GetTlsUniqueObjectOffset(ctor->GetGraph()->GetArch());
    auto result = llvmbackend::runtime_calls::LoadTLSValue(&ctor->builder_, ctor->arkInterface_, offset,
                                                           ctor->builder_.getPtrTy(LLVMArkInterface::GC_ADDR_SPACE));
    ctor->ValueMapAdd(inst, result);
}

void LLVMIrConstructor::VisitLoadImmediate(GraphVisitor *v, Inst *inst)
{
    auto ctor = static_cast<LLVMIrConstructor *>(v);
    auto loadImm = inst->CastToLoadImmediate();
    ASSERT_DO(loadImm->IsTlsOffset(), (std::cerr << "Unsupported llvm lowering for \n", inst->Dump(&std::cerr, true)));
    ASSERT(inst->GetType() == DataType::POINTER);
    auto result = llvmbackend::runtime_calls::LoadTLSValue(&ctor->builder_, ctor->arkInterface_,
                                                           loadImm->GetTlsOffset(), ctor->builder_.getPtrTy());
    ctor->ValueMapAdd(inst, result);
}

void LLVMIrConstructor::VisitStringFlatCheck(GraphVisitor *v, Inst *inst)
{
    auto *ctor = static_cast<LLVMIrConstructor *>(v);
    ctor->EmitFastPath(inst, RuntimeInterface::EntrypointId::STRING_FLAT_CHECK, 1U);
}

void LLVMIrConstructor::VisitDefault([[maybe_unused]] Inst *inst)
{
    ASSERT_DO(false, (std::cerr << "Unsupported llvm lowering for \n", inst->Dump(&std::cerr, true)));
    UNREACHABLE();
}

LLVMIrConstructor::LLVMIrConstructor(Graph *graph, llvm::Module *module, llvm::LLVMContext *context,
                                     LLVMArkInterface *arkInterface, const std::unique_ptr<DebugDataBuilder> &debugData)
    : graph_(graph),
      builder_(llvm::IRBuilder<>(*context)),
      inputMap_(graph->GetLocalAllocator()->Adapter()),
      blockTailMap_(graph->GetLocalAllocator()->Adapter()),
      blockHeadMap_(graph->GetLocalAllocator()->Adapter()),
      arkInterface_(arkInterface),
      debugData_(debugData),
      cc_(graph->GetLocalAllocator()->Adapter()),
      ccValues_(graph->GetLocalAllocator()->Adapter())
{
    llvm::CallingConv::ID callingConv = llvm::CallingConv::C;
    // Assign regmaps
    if (graph->GetMode().IsInterpreter()) {
        if (graph->GetArch() == Arch::AARCH64) {
            cc_.assign({AARCH64_PC, AARCH64_ACC, AARCH64_ACC_TAG, AARCH64_FP, AARCH64_DISPATCH, AARCH64_MOFFSET,
                        AARCH64_METHOD_PTR, GetThreadReg(Arch::AARCH64)});
        } else if (graph->GetArch() == Arch::X86_64) {
            cc_.assign({X86_64_PC, X86_64_ACC, X86_64_ACC_TAG, X86_64_FP, X86_64_DISPATCH, GetThreadReg(Arch::X86_64),
                        X86_64_REAL_FP});
        } else {
            LLVM_LOG(FATAL, IR) << "Unsupported architecture for arkintcc";
        }
        callingConv = llvm::CallingConv::ArkInt;
    } else if (graph->GetMode().IsFastPath()) {
        ASSERT(graph->GetArch() == Arch::AARCH64);
        for (size_t i = 0; i < graph->GetRuntime()->GetMethodTotalArgumentsCount(graph->GetMethod()); i++) {
            cc_.push_back(i);
        }
        // Get calling convention excluding thread and frame registers
        callingConv = GetFastPathCallingConv(cc_.size());
        cc_.push_back(GetThreadReg(Arch::AARCH64));
        cc_.push_back(AARCH64_REAL_FP);
    }
    ccValues_.assign(cc_.size(), nullptr);

    // Create function
    auto funcProto = GetEntryFunctionType();
    auto methodName = arkInterface_->GetUniqMethodName(graph_->GetMethod());
    func_ = CreateFunctionDeclaration(funcProto, methodName, module);
    ASSERT(func_->getCallingConv() == llvm::CallingConv::C);
    func_->setCallingConv(callingConv);

    // Scenario of code generation for FastPath having zero arguments and return value is not tested
    ASSERT(callingConv != llvm::CallingConv::ArkFast0 || func_->getReturnType()->isVoidTy());

    if (graph->SupportManagedCode()) {
        func_->setGC(std::string {llvmbackend::LLVMArkInterface::GC_STRATEGY});
    }

    auto klassId = graph_->GetRuntime()->GetClassIdForMethod(graph_->GetMethod());
    auto klassIdMd = llvm::ConstantAsMetadata::get(builder_.getInt32(klassId));
    func_->addMetadata(llvmbackend::LLVMArkInterface::FUNCTION_MD_CLASS_ID, *llvm::MDNode::get(*context, {klassIdMd}));

    if (!arkInterface_->IsIrtocMode()) {
        func_->addMetadata("use-ark-frame", *llvm::MDNode::get(*context, {}));
    }
}

bool LLVMIrConstructor::BuildIr(bool preventInlining)
{
    LLVM_LOG(DEBUG, IR) << "Building IR for LLVM";

    // Set Argument Names
    // Special arguments
    auto it = func_->arg_begin();
    if (graph_->SupportManagedCode()) {
        (it++)->setName("method");
    }
    // Actual arguments
    auto idx = 0;
    while (it != func_->arg_end()) {
        std::stringstream name;
        name << "a" << idx++;
        (it++)->setName(name.str());
    }

    auto method = graph_->GetMethod();
    auto runtime = graph_->GetRuntime();
    arkInterface_->RememberFunctionOrigin(func_, method);
    func_->addFnAttr(ark::llvmbackend::LLVMArkInterface::SOURCE_LANG_ATTR,
                     std::to_string(static_cast<uint8_t>(runtime->GetMethodSourceLanguage(method))));

    if (!graph_->GetMode().IsFastPath()) {
        debugData_->BeginSubprogram(func_, runtime->GetFullFileName(method), runtime->GetMethodId(method));
    } else {
        func_->addFnAttr(llvm::Attribute::NoInline);
    }

    auto normalMarkerHolder = MarkerHolder(graph_);
    auto normal = normalMarkerHolder.GetMarker();

    graph_->GetStartBlock()->SetMarker(normal);
    MarkNormalBlocksRecursive(graph_->GetStartBlock(), normal);

    // First step - create blocks, leaving LLVM EntryBlock untouched
    BuildBasicBlocks(normal);
    InitializeEntryBlock(preventInlining);

    // Second step - visit all instructions, including StartBlock, but not filling PHI inputs
    BuildInstructions(normal);

    // Third step - fill the PHIs inputs
    for (auto block : graph_->GetBlocksRPO()) {
        FillPhiInputs(block, normal);
    }

    if (!graph_->GetMode().IsFastPath()) {
        debugData_->EndSubprogram(func_);
    }
    if (!arkInterface_->IsIrtocMode()) {
        func_->addFnAttr("frame-pointer", "all");
    }
#ifndef NDEBUG
    // Only for tests
    BreakIrIfNecessary();
#endif
    // verifyFunction returns false if there are no errors. But we return true if everything is ok.
    auto verified = !verifyFunction(*func_, &llvm::errs());
    if (!verified) {
        func_->print(llvm::errs());
    }
    return verified;
}

void LLVMIrConstructor::InsertArkFrameInfo(llvm::Module *module, Arch arch)
{
    constexpr std::string_view ARK_CALLER_SLOTS_MD = "ark.frame.info";
    ASSERT(module->getNamedMetadata(ARK_CALLER_SLOTS_MD) == nullptr);
    auto arkFrameInfoMd = module->getOrInsertNamedMetadata(ARK_CALLER_SLOTS_MD);
    auto builder = llvm::IRBuilder<>(module->getContext());

    // The fist param is a difference between Ark's fp and the start of LLVM frame.
    auto md = llvm::ConstantAsMetadata::get(builder.getInt32(0U));
    arkFrameInfoMd->addOperand(llvm::MDNode::get(module->getContext(), {md}));

    // The second param contains offsets of caller-saved registers inside the ark's frame
    std::vector<size_t> callParamsRegs;
    switch (arch) {
        case Arch::AARCH64: {
            auto src = ArchCallingConvention<Arch::AARCH64>::Target::CALL_PARAMS_REGS;
            callParamsRegs = std::vector<size_t>(src.begin(), src.end());
            break;
        }
        case Arch::X86_64: {
            auto src = ArchCallingConvention<Arch::X86_64>::Target::CALL_PARAMS_REGS;
            callParamsRegs = std::vector<size_t>(src.begin(), src.end());
            break;
        }
        default:
            UNREACHABLE();
    }

    CFrameLayout frameLayout(arch, 0);
    const auto callerRegsSlotStart = frameLayout.GetCallerFirstSlot(false);
    const auto callerRegsCount = frameLayout.GetCallerRegistersCount(false);
    std::vector<llvm::Metadata *> argOffsets;
    for (auto paramRegId : callParamsRegs) {
        int slot = callerRegsSlotStart + (callerRegsCount - 1 - paramRegId);
        slot += frameLayout.GetStackStartSlot();
        constexpr auto FP_ORIGIN = CFrameLayout::OffsetOrigin::FP;
        constexpr auto OFFSET_IN_BYTES = CFrameLayout::OffsetUnit::BYTES;
        auto offset = -frameLayout.GetOffset<FP_ORIGIN, OFFSET_IN_BYTES>(slot);
        ASSERT(std::numeric_limits<int32_t>::min() <= offset);
        ASSERT(offset <= std::numeric_limits<int32_t>::max());
        if (arch == Arch::AARCH64) {
            offset -= frameLayout.GetSlotSize() * 2U;
        }
        argOffsets.push_back(llvm::ConstantAsMetadata::get(builder.getInt32(offset)));
    }
    arkFrameInfoMd->addOperand(llvm::MDNode::get(module->getContext(), argOffsets));

    // The third param is actual frame size
    auto val = frameLayout.GetFrameSize<CFrameLayout::OffsetUnit::BYTES>();
    // LLVM will store LR & FP
    if (arch == Arch::AARCH64) {
        val -= frameLayout.GetSlotSize() * 2U;
    }
    auto vmd = llvm::ConstantAsMetadata::get(builder.getInt32(val));
    arkFrameInfoMd->addOperand(llvm::MDNode::get(module->getContext(), {vmd}));
}

void LLVMIrConstructor::ProvideSafepointPoll(llvm::Module *module, LLVMArkInterface *arkInterface, Arch arch)
{
    // Has been already provided
    ASSERT(module->getFunction(LLVMArkInterface::GC_SAFEPOINT_POLL_NAME) == nullptr);
    auto &ctx = module->getContext();
    auto builder = llvm::IRBuilder<>(ctx);

    // Create a gc.safepoint_poll itself
    auto pollFtype = llvm::FunctionType::get(builder.getVoidTy(), false);
    auto poll = llvm::Function::Create(pollFtype, llvm::Function::ExternalLinkage,
                                       LLVMArkInterface::GC_SAFEPOINT_POLL_NAME, module);
    poll->setDoesNotThrow();

    // Creating a body
    auto entry = llvm::BasicBlock::Create(ctx, "bb", poll);
    builder.SetInsertPoint(entry);

    int64_t flagAddrOffset = arkInterface->GetRuntime()->GetFlagAddrOffset(arch);
    auto trigger =
        llvmbackend::runtime_calls::LoadTLSValue(&builder, arkInterface, flagAddrOffset, builder.getInt16Ty());
    auto needSafepoint = builder.CreateICmpNE(trigger, builder.getInt16(0), "need_safepoint");
    // Create a ret instuction immediately to split bb right before it
    auto ret = builder.CreateRetVoid();

    // Split into IF-THEN before RET and insert a safepoint call into THEN block
    auto weights =
        llvm::MDBuilder(ctx).createBranchWeights(llvmbackend::Metadata::BranchWeights::UNLIKELY_BRANCH_WEIGHT,
                                                 llvmbackend::Metadata::BranchWeights::LIKELY_BRANCH_WEIGHT);

    builder.SetInsertPoint(llvm::SplitBlockAndInsertIfThen(needSafepoint, ret, false, weights));
    builder.GetInsertBlock()->setName("safepoint");
    auto eid = RuntimeInterface::EntrypointId::SAFEPOINT;
    arkInterface->GetOrCreateRuntimeFunctionType(ctx, module, LLVMArkInterface::RuntimeCallType::ENTRYPOINT,
                                                 static_cast<LLVMArkInterface::EntrypointId>(eid));
    auto threadReg = llvmbackend::runtime_calls::GetThreadRegValue(&builder, arkInterface);
    auto spCall = llvmbackend::runtime_calls::CreateEntrypointCallCommon(
        &builder, threadReg, arkInterface, static_cast<llvmbackend::runtime_calls::EntrypointId>(eid));

    spCall->addFnAttr(llvm::Attribute::get(ctx, "safepoint"));
}

std::string LLVMIrConstructor::CheckGraph(Graph *graph)
{
    ASSERT(!graph->IsDynamicMethod());
    for (auto basicBlock : graph->GetBlocksRPO()) {
        for (auto inst : basicBlock->AllInsts()) {
            bool canCompile = LLVMIrConstructor::CanCompile(inst);
            if (!canCompile) {
                // It means we have one of the following cases:
                // * meet some brand-new opcode in Ark Compiler IR
                // * dynamic intrinsic call (in non-dynamic method!)
                // * not yet patched SLOW_PATH_ENTRY call in Irtoc code
                std::stringstream sstream;
                sstream << GetOpcodeString(inst->GetOpcode()) << " unexpected in LLVM lowering. Method = "
                        << graph->GetRuntime()->GetMethodFullName(graph->GetMethod());
                std::string error = sstream.str();
                LLVM_LOG(ERROR, IR) << error;
                return error;
            }
        }
    }
    return "";
}

bool LLVMIrConstructor::CanCompile(Inst *inst)
{
    if (inst->IsIntrinsic()) {
        auto iid = inst->CastToIntrinsic()->GetIntrinsicId();
        // We support only slowpaths where the second immediate is an external function
        if (iid == RuntimeInterface::IntrinsicId::INTRINSIC_SLOW_PATH_ENTRY) {
            return inst->CastToIntrinsic()->GetImms().size() > 1;
        }
        return CanCompileIntrinsic(iid);
    }
    // Check if we have method that can handle it
    // CC-OFFNXT(C_RULE_SWITCH_BRANCH_CHECKER) autogenerated code
    switch (inst->GetOpcode()) {
        default:
            UNREACHABLE_CONSTEXPR();
            // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(OPCODE, ...)                                                     \
    case Opcode::OPCODE: {                                                        \
        /* CC-OFFNXT(G.PRE.05) function gen */                                    \
        return &LLVMIrConstructor::Visit##OPCODE != &GraphVisitor::Visit##OPCODE; \
    }
            OPCODE_LIST(INST_DEF)
    }
#undef INST_DEF
}

#ifndef NDEBUG
void LLVMIrConstructor::BreakIrIfNecessary()
{
    if (llvmbackend::g_options.GetLlvmBreakIrRegex().empty()) {
        return;
    }

    std::regex regex {llvmbackend::g_options.GetLlvmBreakIrRegex()};

    if (!std::regex_match(func_->getName().str(), regex)) {
        return;
    }

    LLVM_LOG(DEBUG, IR) << "Breaking IR for '" << func_->getName().str() << "' because it matches regex = '"
                        << llvmbackend::g_options.GetLlvmBreakIrRegex() << "'";

    for (auto &basicBlock : *func_) {
        basicBlock.getTerminator()->eraseFromParent();
    }
}
#endif

#include "llvm_ir_constructor_gen.inl"

}  // namespace ark::compiler
