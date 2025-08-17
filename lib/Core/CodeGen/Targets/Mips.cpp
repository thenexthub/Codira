//===- Mips.cpp -----------------------------------------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "ABIInfoImpl.h"
#include "TargetInfo.h"

using namespace language::Core;
using namespace language::Core::CodeGen;

//===----------------------------------------------------------------------===//
// MIPS ABI Implementation.  This works for both little-endian and
// big-endian variants.
//===----------------------------------------------------------------------===//

namespace {
class MipsABIInfo : public ABIInfo {
  bool IsO32;
  const unsigned MinABIStackAlignInBytes, StackAlignInBytes;
  void CoerceToIntArgs(uint64_t TySize,
                       SmallVectorImpl<toolchain::Type *> &ArgList) const;
  toolchain::Type* HandleAggregates(QualType Ty, uint64_t TySize) const;
  toolchain::Type* returnAggregateInRegs(QualType RetTy, uint64_t Size) const;
  toolchain::Type* getPaddingType(uint64_t Align, uint64_t Offset) const;
public:
  MipsABIInfo(CodeGenTypes &CGT, bool _IsO32) :
    ABIInfo(CGT), IsO32(_IsO32), MinABIStackAlignInBytes(IsO32 ? 4 : 8),
    StackAlignInBytes(IsO32 ? 8 : 16) {}

  ABIArgInfo classifyReturnType(QualType RetTy) const;
  ABIArgInfo classifyArgumentType(QualType RetTy, uint64_t &Offset) const;
  void computeInfo(CGFunctionInfo &FI) const override;
  RValue EmitVAArg(CodeGenFunction &CGF, Address VAListAddr, QualType Ty,
                   AggValueSlot Slot) const override;
  ABIArgInfo extendType(QualType Ty) const;
};

class MIPSTargetCodeGenInfo : public TargetCodeGenInfo {
  unsigned SizeOfUnwindException;
public:
  MIPSTargetCodeGenInfo(CodeGenTypes &CGT, bool IsO32)
      : TargetCodeGenInfo(std::make_unique<MipsABIInfo>(CGT, IsO32)),
        SizeOfUnwindException(IsO32 ? 24 : 32) {}

  int getDwarfEHStackPointer(CodeGen::CodeGenModule &CGM) const override {
    return 29;
  }

  void setTargetAttributes(const Decl *D, toolchain::GlobalValue *GV,
                           CodeGen::CodeGenModule &CGM) const override {
    const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D);
    if (!FD) return;
    toolchain::Function *Fn = cast<toolchain::Function>(GV);

    if (FD->hasAttr<MipsLongCallAttr>())
      Fn->addFnAttr("long-call");
    else if (FD->hasAttr<MipsShortCallAttr>())
      Fn->addFnAttr("short-call");

    // Other attributes do not have a meaning for declarations.
    if (GV->isDeclaration())
      return;

    if (FD->hasAttr<Mips16Attr>()) {
      Fn->addFnAttr("mips16");
    }
    else if (FD->hasAttr<NoMips16Attr>()) {
      Fn->addFnAttr("nomips16");
    }

    if (FD->hasAttr<MicroMipsAttr>())
      Fn->addFnAttr("micromips");
    else if (FD->hasAttr<NoMicroMipsAttr>())
      Fn->addFnAttr("nomicromips");

    const MipsInterruptAttr *Attr = FD->getAttr<MipsInterruptAttr>();
    if (!Attr)
      return;

    const char *Kind;
    switch (Attr->getInterrupt()) {
    case MipsInterruptAttr::eic:     Kind = "eic"; break;
    case MipsInterruptAttr::sw0:     Kind = "sw0"; break;
    case MipsInterruptAttr::sw1:     Kind = "sw1"; break;
    case MipsInterruptAttr::hw0:     Kind = "hw0"; break;
    case MipsInterruptAttr::hw1:     Kind = "hw1"; break;
    case MipsInterruptAttr::hw2:     Kind = "hw2"; break;
    case MipsInterruptAttr::hw3:     Kind = "hw3"; break;
    case MipsInterruptAttr::hw4:     Kind = "hw4"; break;
    case MipsInterruptAttr::hw5:     Kind = "hw5"; break;
    }

    Fn->addFnAttr("interrupt", Kind);

  }

  bool initDwarfEHRegSizeTable(CodeGen::CodeGenFunction &CGF,
                               toolchain::Value *Address) const override;

  unsigned getSizeOfUnwindException() const override {
    return SizeOfUnwindException;
  }
};

class WindowsMIPSTargetCodeGenInfo : public MIPSTargetCodeGenInfo {
public:
  WindowsMIPSTargetCodeGenInfo(CodeGenTypes &CGT, bool IsO32)
      : MIPSTargetCodeGenInfo(CGT, IsO32) {}

  void getDependentLibraryOption(toolchain::StringRef Lib,
                                 toolchain::SmallString<24> &Opt) const override {
    Opt = "/DEFAULTLIB:";
    Opt += qualifyWindowsLibrary(Lib);
  }

  void getDetectMismatchOption(toolchain::StringRef Name, toolchain::StringRef Value,
                               toolchain::SmallString<32> &Opt) const override {
    Opt = "/FAILIFMISMATCH:\"" + Name.str() + "=" + Value.str() + "\"";
  }
};
}

void MipsABIInfo::CoerceToIntArgs(
    uint64_t TySize, SmallVectorImpl<toolchain::Type *> &ArgList) const {
  toolchain::IntegerType *IntTy =
    toolchain::IntegerType::get(getVMContext(), MinABIStackAlignInBytes * 8);

  // Add (TySize / MinABIStackAlignInBytes) args of IntTy.
  for (unsigned N = TySize / (MinABIStackAlignInBytes * 8); N; --N)
    ArgList.push_back(IntTy);

  // If necessary, add one more integer type to ArgList.
  unsigned R = TySize % (MinABIStackAlignInBytes * 8);

  if (R)
    ArgList.push_back(toolchain::IntegerType::get(getVMContext(), R));
}

// In N32/64, an aligned double precision floating point field is passed in
// a register.
toolchain::Type* MipsABIInfo::HandleAggregates(QualType Ty, uint64_t TySize) const {
  SmallVector<toolchain::Type*, 8> ArgList, IntArgList;

  if (IsO32) {
    CoerceToIntArgs(TySize, ArgList);
    return toolchain::StructType::get(getVMContext(), ArgList);
  }

  if (Ty->isComplexType())
    return CGT.ConvertType(Ty);

  const RecordType *RT = Ty->getAs<RecordType>();

  // Unions/vectors are passed in integer registers.
  if (!RT || !RT->isStructureOrClassType()) {
    CoerceToIntArgs(TySize, ArgList);
    return toolchain::StructType::get(getVMContext(), ArgList);
  }

  const RecordDecl *RD = RT->getOriginalDecl()->getDefinitionOrSelf();
  const ASTRecordLayout &Layout = getContext().getASTRecordLayout(RD);
  assert(!(TySize % 8) && "Size of structure must be multiple of 8.");

  uint64_t LastOffset = 0;
  unsigned idx = 0;
  toolchain::IntegerType *I64 = toolchain::IntegerType::get(getVMContext(), 64);

  // Iterate over fields in the struct/class and check if there are any aligned
  // double fields.
  for (RecordDecl::field_iterator i = RD->field_begin(), e = RD->field_end();
       i != e; ++i, ++idx) {
    const QualType Ty = i->getType();
    const BuiltinType *BT = Ty->getAs<BuiltinType>();

    if (!BT || BT->getKind() != BuiltinType::Double)
      continue;

    uint64_t Offset = Layout.getFieldOffset(idx);
    if (Offset % 64) // Ignore doubles that are not aligned.
      continue;

    // Add ((Offset - LastOffset) / 64) args of type i64.
    for (unsigned j = (Offset - LastOffset) / 64; j > 0; --j)
      ArgList.push_back(I64);

    // Add double type.
    ArgList.push_back(toolchain::Type::getDoubleTy(getVMContext()));
    LastOffset = Offset + 64;
  }

  CoerceToIntArgs(TySize - LastOffset, IntArgList);
  ArgList.append(IntArgList.begin(), IntArgList.end());

  return toolchain::StructType::get(getVMContext(), ArgList);
}

toolchain::Type *MipsABIInfo::getPaddingType(uint64_t OrigOffset,
                                        uint64_t Offset) const {
  if (OrigOffset + MinABIStackAlignInBytes > Offset)
    return nullptr;

  return toolchain::IntegerType::get(getVMContext(), (Offset - OrigOffset) * 8);
}

ABIArgInfo
MipsABIInfo::classifyArgumentType(QualType Ty, uint64_t &Offset) const {
  Ty = useFirstFieldIfTransparentUnion(Ty);

  uint64_t OrigOffset = Offset;
  uint64_t TySize = getContext().getTypeSize(Ty);
  uint64_t Align = getContext().getTypeAlign(Ty) / 8;

  Align = std::clamp(Align, (uint64_t)MinABIStackAlignInBytes,
                     (uint64_t)StackAlignInBytes);
  unsigned CurrOffset = toolchain::alignTo(Offset, Align);
  Offset = CurrOffset + toolchain::alignTo(TySize, Align * 8) / 8;

  if (isAggregateTypeForABI(Ty) || Ty->isVectorType()) {
    // Ignore empty aggregates.
    if (TySize == 0)
      return ABIArgInfo::getIgnore();

    if (CGCXXABI::RecordArgABI RAA = getRecordArgABI(Ty, getCXXABI())) {
      Offset = OrigOffset + MinABIStackAlignInBytes;
      return getNaturalAlignIndirect(Ty, getDataLayout().getAllocaAddrSpace(),
                                     RAA == CGCXXABI::RAA_DirectInMemory);
    }

    // If we have reached here, aggregates are passed directly by coercing to
    // another structure type. Padding is inserted if the offset of the
    // aggregate is unaligned.
    ABIArgInfo ArgInfo =
        ABIArgInfo::getDirect(HandleAggregates(Ty, TySize), 0,
                              getPaddingType(OrigOffset, CurrOffset));
    ArgInfo.setInReg(true);
    return ArgInfo;
  }

  // Treat an enum type as its underlying type.
  if (const EnumType *EnumTy = Ty->getAs<EnumType>())
    Ty = EnumTy->getOriginalDecl()->getDefinitionOrSelf()->getIntegerType();

  // Make sure we pass indirectly things that are too large.
  if (const auto *EIT = Ty->getAs<BitIntType>())
    if (EIT->getNumBits() > 128 ||
        (EIT->getNumBits() > 64 &&
         !getContext().getTargetInfo().hasInt128Type()))
      return getNaturalAlignIndirect(Ty, getDataLayout().getAllocaAddrSpace());

  // All integral types are promoted to the GPR width.
  if (Ty->isIntegralOrEnumerationType())
    return extendType(Ty);

  return ABIArgInfo::getDirect(
      nullptr, 0, IsO32 ? nullptr : getPaddingType(OrigOffset, CurrOffset));
}

toolchain::Type*
MipsABIInfo::returnAggregateInRegs(QualType RetTy, uint64_t Size) const {
  const RecordType *RT = RetTy->getAs<RecordType>();
  SmallVector<toolchain::Type*, 8> RTList;

  if (RT && RT->isStructureOrClassType()) {
    const RecordDecl *RD = RT->getOriginalDecl()->getDefinitionOrSelf();
    const ASTRecordLayout &Layout = getContext().getASTRecordLayout(RD);
    unsigned FieldCnt = Layout.getFieldCount();

    // N32/64 returns struct/classes in floating point registers if the
    // following conditions are met:
    // 1. The size of the struct/class is no larger than 128-bit.
    // 2. The struct/class has one or two fields all of which are floating
    //    point types.
    // 3. The offset of the first field is zero (this follows what gcc does).
    //
    // Any other composite results are returned in integer registers.
    //
    if (FieldCnt && (FieldCnt <= 2) && !Layout.getFieldOffset(0)) {
      RecordDecl::field_iterator b = RD->field_begin(), e = RD->field_end();
      for (; b != e; ++b) {
        const BuiltinType *BT = b->getType()->getAs<BuiltinType>();

        if (!BT || !BT->isFloatingPoint())
          break;

        RTList.push_back(CGT.ConvertType(b->getType()));
      }

      if (b == e)
        return toolchain::StructType::get(getVMContext(), RTList,
                                     RD->hasAttr<PackedAttr>());

      RTList.clear();
    }
  }

  CoerceToIntArgs(Size, RTList);
  return toolchain::StructType::get(getVMContext(), RTList);
}

ABIArgInfo MipsABIInfo::classifyReturnType(QualType RetTy) const {
  uint64_t Size = getContext().getTypeSize(RetTy);

  if (RetTy->isVoidType())
    return ABIArgInfo::getIgnore();

  // O32 doesn't treat zero-sized structs differently from other structs.
  // However, N32/N64 ignores zero sized return values.
  if (!IsO32 && Size == 0)
    return ABIArgInfo::getIgnore();

  if (isAggregateTypeForABI(RetTy) || RetTy->isVectorType()) {
    if (Size <= 128) {
      if (RetTy->isAnyComplexType())
        return ABIArgInfo::getDirect();

      // O32 returns integer vectors in registers and N32/N64 returns all small
      // aggregates in registers.
      if (!IsO32 ||
          (RetTy->isVectorType() && !RetTy->hasFloatingRepresentation())) {
        ABIArgInfo ArgInfo =
            ABIArgInfo::getDirect(returnAggregateInRegs(RetTy, Size));
        ArgInfo.setInReg(true);
        return ArgInfo;
      }
    }

    return getNaturalAlignIndirect(RetTy, getDataLayout().getAllocaAddrSpace());
  }

  // Treat an enum type as its underlying type.
  if (const EnumType *EnumTy = RetTy->getAs<EnumType>())
    RetTy = EnumTy->getOriginalDecl()->getDefinitionOrSelf()->getIntegerType();

  // Make sure we pass indirectly things that are too large.
  if (const auto *EIT = RetTy->getAs<BitIntType>())
    if (EIT->getNumBits() > 128 ||
        (EIT->getNumBits() > 64 &&
         !getContext().getTargetInfo().hasInt128Type()))
      return getNaturalAlignIndirect(RetTy,
                                     getDataLayout().getAllocaAddrSpace());

  if (isPromotableIntegerTypeForABI(RetTy))
    return ABIArgInfo::getExtend(RetTy);

  if ((RetTy->isUnsignedIntegerOrEnumerationType() ||
      RetTy->isSignedIntegerOrEnumerationType()) && Size == 32 && !IsO32)
    return ABIArgInfo::getSignExtend(RetTy);

  return ABIArgInfo::getDirect();
}

void MipsABIInfo::computeInfo(CGFunctionInfo &FI) const {
  ABIArgInfo &RetInfo = FI.getReturnInfo();
  if (!getCXXABI().classifyReturnType(FI))
    RetInfo = classifyReturnType(FI.getReturnType());

  // Check if a pointer to an aggregate is passed as a hidden argument.
  uint64_t Offset = RetInfo.isIndirect() ? MinABIStackAlignInBytes : 0;

  for (auto &I : FI.arguments())
    I.info = classifyArgumentType(I.type, Offset);
}

RValue MipsABIInfo::EmitVAArg(CodeGenFunction &CGF, Address VAListAddr,
                              QualType OrigTy, AggValueSlot Slot) const {
  QualType Ty = OrigTy;

  // Integer arguments are promoted to 32-bit on O32 and 64-bit on N32/N64.
  // Pointers are also promoted in the same way but this only matters for N32.
  unsigned SlotSizeInBits = IsO32 ? 32 : 64;
  unsigned PtrWidth = getTarget().getPointerWidth(LangAS::Default);
  bool DidPromote = false;
  if ((Ty->isIntegerType() &&
          getContext().getIntWidth(Ty) < SlotSizeInBits) ||
      (Ty->isPointerType() && PtrWidth < SlotSizeInBits)) {
    DidPromote = true;
    Ty = getContext().getIntTypeForBitwidth(SlotSizeInBits,
                                            Ty->isSignedIntegerType());
  }

  auto TyInfo = getContext().getTypeInfoInChars(Ty);

  // The alignment of things in the argument area is never larger than
  // StackAlignInBytes.
  TyInfo.Align =
    std::min(TyInfo.Align, CharUnits::fromQuantity(StackAlignInBytes));

  // MinABIStackAlignInBytes is the size of argument slots on the stack.
  CharUnits ArgSlotSize = CharUnits::fromQuantity(MinABIStackAlignInBytes);

  RValue Res = emitVoidPtrVAArg(CGF, VAListAddr, Ty, /*indirect*/ false, TyInfo,
                                ArgSlotSize, /*AllowHigherAlign*/ true, Slot);

  // If there was a promotion, "unpromote".
  // TODO: can we just use a pointer into a subset of the original slot?
  if (DidPromote) {
    toolchain::Type *ValTy = CGF.ConvertType(OrigTy);
    toolchain::Value *Promoted = Res.getScalarVal();

    // Truncate down to the right width.
    toolchain::Type *IntTy = (OrigTy->isIntegerType() ? ValTy : CGF.IntPtrTy);
    toolchain::Value *V = CGF.Builder.CreateTrunc(Promoted, IntTy);
    if (OrigTy->isPointerType())
      V = CGF.Builder.CreateIntToPtr(V, ValTy);

    return RValue::get(V);
  }

  return Res;
}

ABIArgInfo MipsABIInfo::extendType(QualType Ty) const {
  int TySize = getContext().getTypeSize(Ty);

  // MIPS64 ABI requires unsigned 32 bit integers to be sign extended.
  if (Ty->isUnsignedIntegerOrEnumerationType() && TySize == 32)
    return ABIArgInfo::getSignExtend(Ty);

  return ABIArgInfo::getExtend(Ty);
}

bool
MIPSTargetCodeGenInfo::initDwarfEHRegSizeTable(CodeGen::CodeGenFunction &CGF,
                                               toolchain::Value *Address) const {
  // This information comes from gcc's implementation, which seems to
  // as canonical as it gets.

  // Everything on MIPS is 4 bytes.  Double-precision FP registers
  // are aliased to pairs of single-precision FP registers.
  toolchain::Value *Four8 = toolchain::ConstantInt::get(CGF.Int8Ty, 4);

  // 0-31 are the general purpose registers, $0 - $31.
  // 32-63 are the floating-point registers, $f0 - $f31.
  // 64 and 65 are the multiply/divide registers, $hi and $lo.
  // 66 is the (notional, I think) register for signal-handler return.
  AssignToArrayRange(CGF.Builder, Address, Four8, 0, 65);

  // 67-74 are the floating-point status registers, $fcc0 - $fcc7.
  // They are one bit wide and ignored here.

  // 80-111 are the coprocessor 0 registers, $c0r0 - $c0r31.
  // (coprocessor 1 is the FP unit)
  // 112-143 are the coprocessor 2 registers, $c2r0 - $c2r31.
  // 144-175 are the coprocessor 3 registers, $c3r0 - $c3r31.
  // 176-181 are the DSP accumulator registers.
  AssignToArrayRange(CGF.Builder, Address, Four8, 80, 181);
  return false;
}

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createMIPSTargetCodeGenInfo(CodeGenModule &CGM, bool IsOS32) {
  return std::make_unique<MIPSTargetCodeGenInfo>(CGM.getTypes(), IsOS32);
}

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createWindowsMIPSTargetCodeGenInfo(CodeGenModule &CGM, bool IsOS32) {
  return std::make_unique<WindowsMIPSTargetCodeGenInfo>(CGM.getTypes(), IsOS32);
}
