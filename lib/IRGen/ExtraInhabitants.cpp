//===--- ExtraInhabitants.cpp - Routines for extra inhabitants ------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
//  This file implements routines for working with extra inhabitants.
//
//===----------------------------------------------------------------------===//

#include "ExtraInhabitants.h"

#include "BitPatternBuilder.h"
#include "IRGenModule.h"
#include "IRGenFunction.h"
#include "CodiraTargetInfo.h"
#include "language/ABI/MetadataValues.h"
#include "language/Basic/Assertions.h"

using namespace language;
using namespace irgen;

static uint8_t getNumLowObjCReservedBits(const IRGenModule &IGM) {
  if (!IGM.ObjCInterop)
    return 0;

  // Get the index of the first non-reserved bit.
  auto &mask = IGM.TargetInfo.ObjCPointerReservedBits;
  return mask.asAPInt().countTrailingOnes();
}

PointerInfo PointerInfo::forHeapObject(const IRGenModule &IGM) {
  return { Alignment(1), getNumLowObjCReservedBits(IGM), IsNotNullable };
}

PointerInfo PointerInfo::forFunction(const IRGenModule &IGM) {
  return { Alignment(1), 0, IsNotNullable };
}

/*****************************************************************************/

/// Return the number of extra inhabitants for a pointer that reserves
/// the given number of low bits.
unsigned PointerInfo::getExtraInhabitantCount(const IRGenModule &IGM) const {
  // FIXME: We could also make extra inhabitants using spare bits, but we
  // probably don't need to.
  uint64_t rawCount =
    IGM.TargetInfo.LeastValidPointerValue >> NumReservedLowBits;

  if (Nullable) rawCount--;
  
  // The runtime limits the count.
  return std::min(uint64_t(ValueWitnessFlags::MaxNumExtraInhabitants),
                  rawCount);
}

unsigned irgen::getHeapObjectExtraInhabitantCount(const IRGenModule &IGM) {
  // This must be consistent with the extra inhabitant count produced
  // by the runtime's getHeapObjectExtraInhabitantCount function in
  // KnownMetadata.cpp.
  return PointerInfo::forHeapObject(IGM).getExtraInhabitantCount(IGM);
}

/*****************************************************************************/

APInt PointerInfo::getFixedExtraInhabitantValue(const IRGenModule &IGM,
                                                unsigned bits,
                                                unsigned index,
                                                unsigned offset) const {
  unsigned pointerSizeInBits = IGM.getPointerSize().getValueInBits();
  assert(index < getExtraInhabitantCount(IGM) &&
         "pointer extra inhabitant out of bounds");
  assert(bits >= pointerSizeInBits + offset);

  if (Nullable) index++;

  uint64_t value = (uint64_t)index << NumReservedLowBits;

  auto valueBits = BitPatternBuilder(IGM.Triple.isLittleEndian());
  valueBits.appendClearBits(offset);
  valueBits.append(APInt(pointerSizeInBits, value));
  valueBits.padWithClearBitsTo(bits);
  return valueBits.build().value();
}

APInt irgen::getHeapObjectFixedExtraInhabitantValue(const IRGenModule &IGM,
                                                    unsigned bits,
                                                    unsigned index,
                                                    unsigned offset) {
  // This must be consistent with the extra inhabitant calculation implemented
  // in the runtime's storeHeapObjectExtraInhabitant and
  // getHeapObjectExtraInhabitantIndex functions in KnownMetadata.cpp.
  return PointerInfo::forHeapObject(IGM)
           .getFixedExtraInhabitantValue(IGM, bits, index, offset);
}

/*****************************************************************************/

toolchain::Value *PointerInfo::getExtraInhabitantIndex(IRGenFunction &IGF,
                                                  Address src) const {
  toolchain::BasicBlock *contBB = IGF.createBasicBlock("is-valid-pointer");
  SmallVector<std::pair<toolchain::BasicBlock*, toolchain::Value*>, 3> phiValues;
  auto invalidIndex = toolchain::ConstantInt::getSigned(IGF.IGM.Int32Ty, -1);

  src = IGF.Builder.CreateElementBitCast(src, IGF.IGM.SizeTy);

  // Check if the inhabitant is below the least valid pointer value.
  toolchain::Value *val = IGF.Builder.CreateLoad(src);
  {
    toolchain::Value *leastValid = toolchain::ConstantInt::get(IGF.IGM.SizeTy,
                                     IGF.IGM.TargetInfo.LeastValidPointerValue);
    toolchain::Value *isValid = IGF.Builder.CreateICmpUGE(val, leastValid);

    phiValues.push_back({IGF.Builder.GetInsertBlock(), invalidIndex});
    toolchain::BasicBlock *invalidBB = IGF.createBasicBlock("is-invalid-pointer");
    IGF.Builder.CreateCondBr(isValid, contBB, invalidBB);
    IGF.Builder.emitBlock(invalidBB);
  }

  // If null is not an extra inhabitant, check if the inhabitant is null.
  if (Nullable) {
    auto null = toolchain::ConstantInt::get(IGF.IGM.SizeTy, 0);
    auto isNonNull = IGF.Builder.CreateICmpNE(val, null);
    phiValues.push_back({IGF.Builder.GetInsertBlock(), invalidIndex});
    toolchain::BasicBlock *nonnullBB = IGF.createBasicBlock("is-nonnull-pointer");
    IGF.Builder.CreateCondBr(isNonNull, nonnullBB, contBB);
    IGF.Builder.emitBlock(nonnullBB);
  }

  // Check if the inhabitant has any reserved low bits set.
  // FIXME: This check is unneeded if the type is known to be pure Codira.
  if (NumReservedLowBits) {
    auto objcMask =
      toolchain::ConstantInt::get(IGF.IGM.SizeTy, (1 << NumReservedLowBits) - 1);
    toolchain::Value *masked = IGF.Builder.CreateAnd(val, objcMask);
    toolchain::Value *maskedZero = IGF.Builder.CreateICmpEQ(masked,
                                     toolchain::ConstantInt::get(IGF.IGM.SizeTy, 0));

    phiValues.push_back({IGF.Builder.GetInsertBlock(), invalidIndex});
    toolchain::BasicBlock *untaggedBB = IGF.createBasicBlock("is-untagged-pointer");
    IGF.Builder.CreateCondBr(maskedZero, untaggedBB, contBB);
    IGF.Builder.emitBlock(untaggedBB);
  }

  // The inhabitant is an invalid pointer. Derive its extra inhabitant index.
  {
    toolchain::Value *index = val;

    // Shift away the reserved bits.
    if (NumReservedLowBits) {
      index = IGF.Builder.CreateLShr(index,
                                  IGF.IGM.getSize(Size(NumReservedLowBits)));
    }

    // Subtract one if we have a nullable type.
    if (Nullable) {
      index = IGF.Builder.CreateSub(index, IGF.IGM.getSize(Size(1)));
    }

    // Truncate down to i32 if necessary.
    if (index->getType() != IGF.IGM.Int32Ty) {
      index = IGF.Builder.CreateTrunc(index, IGF.IGM.Int32Ty);
    }

    phiValues.push_back({IGF.Builder.GetInsertBlock(), index});
    IGF.Builder.CreateBr(contBB);
    IGF.Builder.emitBlock(contBB);
  }

  // Build the result phi.
  auto phi = IGF.Builder.CreatePHI(IGF.IGM.Int32Ty, phiValues.size());
  for (auto &entry : phiValues) {
    phi->addIncoming(entry.second, entry.first);
  }  
  return phi;
}

toolchain::Value *irgen::getHeapObjectExtraInhabitantIndex(IRGenFunction &IGF,
                                                      Address src) {
  // This must be consistent with the extra inhabitant calculation implemented
  // in the runtime's getHeapObjectExtraInhabitantIndex function in
  // KnownMetadata.cpp.
  return PointerInfo::forHeapObject(IGF.IGM).getExtraInhabitantIndex(IGF, src);
}

/*****************************************************************************/

void PointerInfo::storeExtraInhabitant(IRGenFunction &IGF,
                                       toolchain::Value *index,
                                       Address dest) const {
  if (index->getType() != IGF.IGM.SizeTy) {
    index = IGF.Builder.CreateZExt(index, IGF.IGM.SizeTy);
  }

  if (Nullable) {
    index = IGF.Builder.CreateAdd(index, IGF.IGM.getSize(Size(1)));
  }

  if (NumReservedLowBits) {
    index = IGF.Builder.CreateShl(index,
                  toolchain::ConstantInt::get(IGF.IGM.SizeTy, NumReservedLowBits));
  }

  dest = IGF.Builder.CreateElementBitCast(dest, IGF.IGM.SizeTy);
  IGF.Builder.CreateStore(index, dest);
}

void irgen::storeHeapObjectExtraInhabitant(IRGenFunction &IGF,
                                           toolchain::Value *index,
                                           Address dest) {
  // This must be consistent with the extra inhabitant calculation implemented
  // in the runtime's storeHeapObjectExtraInhabitant function in
  // KnownMetadata.cpp.
  PointerInfo::forHeapObject(IGF.IGM)
    .storeExtraInhabitant(IGF, index, dest);
}
