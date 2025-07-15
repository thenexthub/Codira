//===--- EnumPayload.cpp - Payload management for 'enum' Types ------------===//
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

#include "BitPatternReader.h"
#include "EnumPayload.h"
#include "Explosion.h"
#include "GenEnum.h"
#include "IRGenModule.h"
#include "language/Basic/Assertions.h"

#include <algorithm>
#include <map>

using namespace language;
using namespace irgen;

// FIXME: Everything here brazenly assumes little-endian-ness.

static toolchain::Value *forcePayloadValue(EnumPayload::LazyValue &value) {
  if (auto v = value.dyn_cast<toolchain::Value *>())
    return v;
  
  auto null = toolchain::Constant::getNullValue(value.dyn_cast<toolchain::Type*>());
  value = null;
  return null;
}

static toolchain::Type *getPayloadType(EnumPayload::LazyValue value) {
  if (auto t = value.dyn_cast<toolchain::Type *>())
    return t;
  
  return value.dyn_cast<toolchain::Value *>()->getType();
}

// Clear bits starting from the most significant until the number
// of set bits in the value is less than or equal to numSetBits.
//
// For example: getLowestNSetBits(0x11111100, 2) = 0x00001100
static toolchain::APInt getLowestNSetBits(toolchain::APInt value,
                                     unsigned numSetBits) {
  // TODO: optimize
  for (unsigned i = 0; i < value.getBitWidth(); ++i) {
    if (numSetBits == 0) {
      value.clearBit(i);
    } else if (value[i]) {
      numSetBits -= 1;
    }
  }
  return value;
}

EnumPayload EnumPayload::zero(IRGenModule &IGM, EnumPayloadSchema schema) {
  // We don't need to create any values yet; they can be filled in when
  // real values are inserted.
  EnumPayload result;
  schema.forEachType(IGM, [&](toolchain::Type *type) {
    result.PayloadValues.push_back(type);
  });
  return result;
}

EnumPayload EnumPayload::fromBitPattern(IRGenModule &IGM,
                                        const APInt &bitPattern,
                                        EnumPayloadSchema schema) {
  auto maskReader = BitPatternReader(bitPattern, IGM.Triple.isLittleEndian());

  EnumPayload result;
  schema.forEachType(IGM, [&](toolchain::Type *type) {
    unsigned bitSize = IGM.DataLayout.getTypeSizeInBits(type);

    toolchain::IntegerType *intTy
      = toolchain::IntegerType::get(IGM.getLLVMContext(), bitSize);

    // Take some bits off of the bottom of the pattern.
    auto bits = maskReader.read(bitSize);
    auto val = toolchain::ConstantInt::get(intTy, bits);
    if (val->getType() != type) {
      if (type->isPointerTy())
        val = toolchain::ConstantExpr::getIntToPtr(val, type);
      else
        val = toolchain::ConstantExpr::getBitCast(val, type);
    }

    result.PayloadValues.push_back(val);
  });

  return result;
}

/// Create a mask for an element with the given type at the provided
/// offset into the payload. The offset and size are in bits but
/// must be a multiple of 8 (i.e. byte aligned).
static APInt createElementMask(const toolchain::DataLayout &DL,
                               toolchain::Type *type,
                               unsigned payloadOffset,
                               unsigned payloadSizeInBits) {
  // Create a mask for the bytes that make up the stored element
  // by zero extending the element's mask to its storage size.
  // This makes the mask valid regardless of endianness.
  auto elSize = DL.getTypeSizeInBits(type);
  auto elStoreSize = DL.getTypeStoreSizeInBits(type);
  auto elMask = APInt::getLowBitsSet(elStoreSize, elSize);

  // Pad the valueMask so that it can be applied to the entire
  // payload.
  auto mask = APInt::getZero(payloadSizeInBits);
  auto offset = payloadOffset;
  if (DL.isBigEndian()) {
    offset = payloadSizeInBits - payloadOffset - elStoreSize;
  }
  mask.insertBits(elMask, offset);
  return mask;
}

void EnumPayload::insertValue(IRGenModule &IGM,
                              IRBuilder &builder, toolchain::Value *value,
                              unsigned payloadOffset) {
  auto &DL = IGM.DataLayout;

  // Create a mask for the value we are going to insert.
  auto type = value->getType();
  auto payloadSize = getAllocSizeInBits(DL);
  auto mask = createElementMask(DL, type, payloadOffset, payloadSize);

  // Scatter the value into the payload.
  emitScatterBits(IGM, builder, mask, value);
}

toolchain::Value *EnumPayload::extractValue(IRGenFunction &IGF, toolchain::Type *type,
                                       unsigned payloadOffset) const {
  auto &DL = IGF.IGM.DataLayout;

  // Create a mask for the value we are going to extract.
  auto payloadSize = getAllocSizeInBits(DL);
  auto mask = createElementMask(DL, type, payloadOffset, payloadSize);

  // Convert the payload mask into a SpareBitVector.
  // TODO: make emitGatherSpareBits take an APInt and delete.
  auto bits = SpareBitVector::fromAPInt(std::move(mask));

  // Gather the value from the payload.
  auto valueSize = DL.getTypeSizeInBits(type);
  auto value = emitGatherSpareBits(IGF, bits, 0, valueSize);

  // Convert the integer value to the required type.
  if (value->getType() != type) {
    value = IGF.Builder.CreateBitOrPointerCast(value, type);
  }
  return value;
}

EnumPayload EnumPayload::fromExplosion(IRGenModule &IGM,
                                       Explosion &in, EnumPayloadSchema schema){
  EnumPayload result;
  
  schema.forEachType(IGM, [&](toolchain::Type *type) {
    auto next = in.claimNext();
    if (next->getType() == type) {
      result.PayloadValues.push_back(next);
    } else {
      // The original value had an unaligned integer size and was replaced by
      // byte values in `replaceUnalignedIntegerValues`.
      // This is done for enums in statically initialized global variables.
      unsigned bitSize = cast<toolchain::IntegerType>(type)->getBitWidth();
      assert(bitSize % 8 == 0);
      assert(cast<toolchain::ConstantInt>(next)->getBitWidth() == 8);
      result.PayloadValues.push_back(next);
      for (unsigned byte = 1; byte < bitSize / 8; ++byte) {
        auto nextByte = in.claimNext();
        assert(cast<toolchain::ConstantInt>(nextByte)->getBitWidth() == 8);
        result.PayloadValues.push_back(nextByte);
      }
    }
  });
  
  return result;
}

void EnumPayload::explode(IRGenModule &IGM, Explosion &out) const {
  for (LazyValue &value : PayloadValues) {
    out.add(forcePayloadValue(value));
  }
}

void EnumPayload::packIntoEnumPayload(IRGenModule &IGM,
                                      IRBuilder &builder,
                                      EnumPayload &outerPayload,
                                      unsigned bitOffset) const {
  auto &DL = IGM.DataLayout;
  for (auto &value : PayloadValues) {
    auto v = forcePayloadValue(value);
    outerPayload.insertValue(IGM, builder, v, bitOffset);
    bitOffset += DL.getTypeSizeInBits(v->getType());
  }
}

EnumPayload EnumPayload::unpackFromEnumPayload(IRGenFunction &IGF,
                                        const EnumPayload &outerPayload,
                                        unsigned bitOffset,
                                        EnumPayloadSchema schema) {
  EnumPayload result;
  auto &DL = IGF.IGM.DataLayout;
  schema.forEachType(IGF.IGM, [&](toolchain::Type *type) {
    auto v = outerPayload.extractValue(IGF, type, bitOffset);
    result.PayloadValues.push_back(v);
    bitOffset += DL.getTypeSizeInBits(type);
  });
  return result;
}

static toolchain::Type *getPayloadStorageType(IRGenModule &IGM,
                                         const EnumPayload &payload) {
  if (payload.StorageType)
    return payload.StorageType;

  if (payload.PayloadValues.size() == 1) {
    payload.StorageType = getPayloadType(payload.PayloadValues.front());
    return payload.StorageType;
  }
  
  SmallVector<toolchain::Type *, 2> elementTypes;
  for (auto value : payload.PayloadValues) {
    elementTypes.push_back(getPayloadType(value));
  }
  
  payload.StorageType = toolchain::StructType::get(IGM.getLLVMContext(),
                                              elementTypes);
  return payload.StorageType;
}

EnumPayload EnumPayload::load(IRGenFunction &IGF, Address address,
                              EnumPayloadSchema schema) {
  EnumPayload result = EnumPayload::zero(IGF.IGM, schema);
  if (result.PayloadValues.empty())
    return result;
  
  auto storageTy = getPayloadStorageType(IGF.IGM, result);
  address = IGF.Builder.CreateElementBitCast(address, storageTy);

  if (result.PayloadValues.size() == 1) {
    result.PayloadValues.front() = IGF.Builder.CreateLoad(address);
  } else {
    Size offset(0);
    for (unsigned i : indices(result.PayloadValues)) {
      auto &value = result.PayloadValues[i];
      auto member = IGF.Builder.CreateStructGEP(address, i, offset);
      auto loadedValue = IGF.Builder.CreateLoad(member);
      value = loadedValue;
      offset += Size(IGF.IGM.DataLayout.getTypeAllocSize(loadedValue->getType()));
    }
  }
  
  return result;
}

void EnumPayload::store(IRGenFunction &IGF, Address address) const {
  if (PayloadValues.empty())
    return;

  auto storageTy = getPayloadStorageType(IGF.IGM, *this);
  address = IGF.Builder.CreateElementBitCast(address, storageTy);

  if (PayloadValues.size() == 1) {
    IGF.Builder.CreateStore(forcePayloadValue(PayloadValues.front()), address);
    return;
  } else {
    Size offset(0);
    for (unsigned i : indices(PayloadValues)) {
      auto &value = PayloadValues[i];
      auto member = IGF.Builder.CreateStructGEP(address, i, offset);
      auto valueToStore = forcePayloadValue(value);
      IGF.Builder.CreateStore(valueToStore, member);
      offset += Size(IGF.IGM.DataLayout
                       .getTypeAllocSize(valueToStore->getType()));
    }
  }
}

void EnumPayload::emitSwitch(IRGenFunction &IGF,
                             const APInt &mask,
                             ArrayRef<std::pair<APInt, toolchain::BasicBlock *>> cases,
                             SwitchDefaultDest dflt) const {
  // If there's only one case to test, do a simple compare and branch.
  if (cases.size() == 1) {
    // If the default case is unreachable, don't bother branching at all.
    if (dflt.getInt()) {
      IGF.Builder.CreateBr(cases[0].second);
      return;
    }
  
    auto *cmp = emitCompare(IGF, mask, cases[0].first);
    IGF.Builder.CreateCondBr(cmp, cases[0].second, dflt.getPointer());
    return;
  }

  if (mask.getBitWidth() > IGF.IGM.getPointerSize().getValueInBits() * 2) {
    for (int index = 0, size = cases.size(); index < size; ++index) {
      auto &c = cases[index];
      auto *cmp = emitCompare(IGF, mask, c.first);
      toolchain::BasicBlock *elseBlock;
      if (index < size - 1) {
        elseBlock = IGF.createBasicBlock("");
      } else {
        elseBlock = dflt.getPointer();
      }
      IGF.Builder.CreateCondBr(cmp, c.second, elseBlock);
      if (index < size - 1) {
        IGF.Builder.emitBlock(elseBlock);
      }
    }
    return;
  }

  // Otherwise emit a switch statement.
  auto &C = IGF.IGM.getLLVMContext();
  unsigned numBits = mask.popcount();
  auto target = emitGatherSpareBits(IGF, SpareBitVector::fromAPInt(mask),
                                    0, numBits);
  auto swi = IGF.Builder.CreateSwitch(target, dflt.getPointer(), cases.size());
  for (auto &c : cases) {
    auto value = toolchain::ConstantInt::get(C, gatherBits(mask, c.first));
    swi->addCase(value, c.second);
  }
  assert(IGF.Builder.hasPostTerminatorIP());
}

toolchain::Value *
EnumPayload::emitCompare(IRGenFunction &IGF,
                         const APInt &mask,
                         const APInt &value) const {
  // Succeed trivially for an empty payload, or if the payload is masked
  // out completely.
  if (PayloadValues.empty() || mask == 0)
    return toolchain::ConstantInt::get(IGF.IGM.Int1Ty, 1U);

  assert((~mask & value) == 0
         && "value has masked out bits set?!");

  auto &DL = IGF.IGM.DataLayout;
  auto valueReader = BitPatternReader(value, DL.isLittleEndian());
  auto maskReader = BitPatternReader(mask, DL.isLittleEndian());

  toolchain::Value *condition = nullptr;
  for (auto &pv : PayloadValues) {
    auto v = forcePayloadValue(pv);
    unsigned size = DL.getTypeSizeInBits(v->getType());

    // Break off a piece of the mask and value.
    auto maskPiece = maskReader.read(size);
    auto valuePiece = valueReader.read(size);

    // If this piece is zero, it doesn't affect the comparison.
    if (maskPiece == 0)
      continue;
    
    // Apply the mask and test.
    bool isMasked = !maskPiece.isAllOnes();
    auto intTy = toolchain::IntegerType::get(IGF.IGM.getLLVMContext(), size);
    // Need to bitcast to an integer in order to use 'icmp eq' if the piece
    // isn't already an int or pointer, or in order to apply a mask.
    if (isMasked
        || (!isa<toolchain::IntegerType>(v->getType())
            && !isa<toolchain::PointerType>(v->getType())))
      v = IGF.Builder.CreateBitOrPointerCast(v, intTy);
    
    if (isMasked) {
      auto maskConstant = toolchain::ConstantInt::get(intTy, maskPiece);
      v = IGF.Builder.CreateAnd(v, maskConstant);
    }
    
    toolchain::Value *valueConstant = toolchain::ConstantInt::get(intTy, valuePiece);
    valueConstant = IGF.Builder.CreateBitOrPointerCast(valueConstant,
                                                       v->getType());
    auto cmp = IGF.Builder.CreateICmpEQ(v, valueConstant);
    if (!condition)
      condition = cmp;
    else
      condition = IGF.Builder.CreateAnd(condition, cmp);
  }
  
  // We should have handled the cases where there are no significant conditions
  // in the early exit.
  assert(condition && "no significant condition?!");
  return condition;
}

void
EnumPayload::emitApplyAndMask(IRGenFunction &IGF, const APInt &mask) {
  // Early exit if the mask has no effect.
  if (mask.isAllOnes())
    return;

  auto &DL = IGF.IGM.DataLayout;
  auto maskReader = BitPatternReader(mask, DL.isLittleEndian());

  for (auto &pv : PayloadValues) {
    auto payloadTy = getPayloadType(pv);
    unsigned size = DL.getTypeSizeInBits(payloadTy);

    // Read a chunk of the mask.
    auto maskPiece = maskReader.read(size);
    
    // If this piece is all ones, it has no effect.
    if (maskPiece.isAllOnes())
      continue;

    // If the payload value is vacant, the mask can't change it.
    if (pv.is<toolchain::Type *>())
      continue;

    // If this piece is zero, it wipes out the chunk entirely, and we can
    // drop it.
    if (maskPiece == 0) {
      pv = payloadTy;
      continue;
    }
    
    // Otherwise, apply the mask to the existing value.
    auto v = pv.get<toolchain::Value*>();
    auto payloadIntTy = toolchain::IntegerType::get(IGF.IGM.getLLVMContext(), size);
    auto maskConstant = toolchain::ConstantInt::get(payloadIntTy, maskPiece);
    v = IGF.Builder.CreateBitOrPointerCast(v, payloadIntTy);
    v = IGF.Builder.CreateAnd(v, maskConstant);
    v = IGF.Builder.CreateBitOrPointerCast(v, payloadTy);
    pv = v;
  }
}

void
EnumPayload::emitApplyOrMask(IRGenModule &IGM,
                             IRBuilder &builder, const APInt &mask) {
  // Early exit if the mask has no effect.
  if (mask == 0)
    return;

  auto &DL = IGM.DataLayout;
  auto maskReader = BitPatternReader(mask, DL.isLittleEndian());

  for (auto &pv : PayloadValues) {
    auto payloadTy = getPayloadType(pv);
    unsigned size = DL.getTypeSizeInBits(payloadTy);

    // Read a chunk of the mask.
    auto maskPiece = maskReader.read(size);

    // If this piece is zero, it has no effect.
    if (maskPiece == 0)
      continue;
    
    auto payloadIntTy = toolchain::IntegerType::get(IGM.getLLVMContext(), size);
    auto maskConstant = toolchain::ConstantInt::get(payloadIntTy, maskPiece);
    
    // If the payload value is vacant, or the mask is all ones,
    // we can adopt the mask value directly.
    if (pv.is<toolchain::Type *>() || maskPiece.isAllOnes()) {
      pv = builder.CreateBitOrPointerCast(maskConstant, payloadTy);
      continue;
    }
    
    // Otherwise, apply the mask to the existing value.
    auto v = pv.get<toolchain::Value*>();
    v = builder.CreateBitOrPointerCast(v, payloadIntTy);
    v = builder.CreateOr(v, maskConstant);
    v = builder.CreateBitOrPointerCast(v, payloadTy);
    pv = v;
  }
}

void
EnumPayload::emitApplyOrMask(IRGenFunction &IGF,
                             EnumPayload mask) {
  unsigned count = PayloadValues.size();
  assert(count == mask.PayloadValues.size());

  auto &DL = IGF.IGM.DataLayout;
  for (unsigned i = 0; i < count; ++i) {
    auto payloadTy = getPayloadType(PayloadValues[i]);
    unsigned size = DL.getTypeSizeInBits(payloadTy);

    auto payloadIntTy = toolchain::IntegerType::get(IGF.IGM.getLLVMContext(), size);

    if (mask.PayloadValues[i].is<toolchain::Type *>()) {
      // We're ORing with zero, do nothing
    } else if (PayloadValues[i].is<toolchain::Type *>()) {
      PayloadValues[i] = mask.PayloadValues[i];
    } else {
      auto v1 = IGF.Builder.CreateBitOrPointerCast(
          PayloadValues[i].get<toolchain::Value *>(),
          payloadIntTy);

      auto v2 = IGF.Builder.CreateBitOrPointerCast(
          mask.PayloadValues[i].get<toolchain::Value *>(),
          payloadIntTy);

      PayloadValues[i] = IGF.Builder.CreateBitOrPointerCast(
          IGF.Builder.CreateOr(v1, v2),
          payloadTy);
    }
  }
}

void EnumPayload::emitScatterBits(IRGenModule &IGM,
                                  IRBuilder &builder,
                                  const APInt &mask,
                                  toolchain::Value *value) {
  auto &DL = IGM.DataLayout;

  unsigned valueBits = DL.getTypeSizeInBits(value->getType());
  auto totalBits = std::min(valueBits, mask.popcount());
  auto maskReader = BitPatternReader(getLowestNSetBits(mask, totalBits),
                                     DL.isLittleEndian());
  auto usedBits = 0u;
  for (auto &pv : PayloadValues) {
    auto partType = getPayloadType(pv);
    auto partSize = DL.getTypeSizeInBits(partType);
    auto partMask = maskReader.read(partSize);

    // Skip this element if there are no set bits in the mask.
    if (partMask == 0) {
      continue;
    }

    // Calculate the number of bits we are going to scatter.
    auto partCount = partMask.popcount();

    // Scatter bits from the source into the bits specified by the mask.
    auto offset = usedBits;
    if (DL.isBigEndian()) {
      offset = totalBits - partCount - usedBits;
    }
    auto partValue = irgen::emitScatterBits(IGM, builder, partMask, value, offset);

    // If necessary OR with the existing value.
    if (auto existingValue = pv.dyn_cast<toolchain::Value*>()) {
      if (partType != partValue->getType()) {
        existingValue = builder.CreateBitOrPointerCast(existingValue,
                                                 partValue->getType());
      }
      partValue = builder.CreateOr(partValue, existingValue);
    }

    // Convert the integer result to the target type.
    if (partType != partValue->getType()) {
      partValue = builder.CreateBitOrPointerCast(partValue, partType);
    }

    // Update this payload element.
    pv = partValue;

    // Update our position in the source integer.
    usedBits += partCount;
    if (usedBits >= totalBits) {
      break;
    }
  }
}

toolchain::Value *
EnumPayload::emitGatherSpareBits(IRGenFunction &IGF,
                                 const SpareBitVector &spareBits,
                                 unsigned firstBitOffset,
                                 unsigned resultBitWidth) const {
  auto &DL = IGF.IGM.DataLayout;
  auto &C = IGF.IGM.getLLVMContext();

  auto mask = getLowestNSetBits(spareBits.asAPInt(),
                                resultBitWidth - firstBitOffset);
  auto bitWidth = mask.popcount();
  auto spareBitReader = BitPatternReader(std::move(mask),
                                         DL.isLittleEndian());
  auto usedBits = firstBitOffset;

  toolchain::Value *spareBitValue = nullptr;
  for (auto &pv : PayloadValues) {
    // If this value is zero, it has nothing to add to the spare bits.
    auto v = pv.dyn_cast<toolchain::Value*>();
    if (!v) {
      spareBitReader.skip(DL.getTypeSizeInBits(pv.get<toolchain::Type*>()));
      continue;
    }

    // Slice the spare bit vector.
    unsigned size = DL.getTypeSizeInBits(v->getType());
    auto spareBitsPart = spareBitReader.read(size);
    unsigned numBitsInPart = spareBitsPart.popcount();

    // If there were no spare bits in this part, it has nothing to add.
    if (numBitsInPart == 0)
      continue;

    if (usedBits >= bitWidth)
      break;

    unsigned offset = usedBits;
    if (DL.isBigEndian()) {
      offset = bitWidth - usedBits - numBitsInPart;
    }
    // Get the spare bits from this part.
    auto bits = irgen::emitGatherBits(IGF, spareBitsPart,
                                      v, offset, resultBitWidth);
    usedBits += numBitsInPart;

    // Accumulate it into the full set.
    if (spareBitValue) {
      bits = IGF.Builder.CreateOr(spareBitValue, bits);
    }
    spareBitValue = bits;
  }
  auto destTy = toolchain::IntegerType::get(C, resultBitWidth);
  if (spareBitValue) {
    assert(spareBitValue->getType() == destTy);
    return spareBitValue;
  }
  return toolchain::ConstantInt::get(destTy, 0);
}

unsigned EnumPayload::getAllocSizeInBits(const toolchain::DataLayout &DL) const {
  unsigned size = 0u;
  for (const auto &pv : PayloadValues) {
    size += DL.getTypeAllocSizeInBits(getPayloadType(pv));
    assert(size % 8 == 0 && "allocation size must be a multiple of bytes");
  }
  return size;
}

void EnumPayload::print(toolchain::raw_ostream &OS) {
  if (StorageType) {
    OS << "storage-type: ";
    StorageType->print(OS);
    OS << '\n';
  }
  for (LazyValue pv : PayloadValues) {
    if (auto *v = pv.dyn_cast<toolchain::Value*>()) {
      OS << "value: ";
      v->print(OS);
      OS << '\n';
    } else {
      auto *t = pv.get<toolchain::Type*>();
      OS << "type: ";
      t->print(OS);
      OS << '\n';
    }
  }
}

void EnumPayload::dump() {
  print(toolchain::errs());
}

