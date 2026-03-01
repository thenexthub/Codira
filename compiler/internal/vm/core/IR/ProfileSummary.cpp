//=-- Profilesummary.cpp - Profile summary support --------------------------=//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
//
// This file contains support for converting profile summary data from/to
// metadata.
//
//===----------------------------------------------------------------------===//

#include "vm/core/IR/ProfileSummary.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/Metadata.h"
#include "vm/core/IR/Type.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/Format.h"

using namespace vm::core;

// Return an MDTuple with two elements. The first element is a string Key and
// the second is a uint64_t Value.
static Metadata *getKeyValMD(LLVMContext &Context, const char *Key,
                             uint64_t Val) {
  Type *Int64Ty = Type::getInt64Ty(Context);
  Metadata *Ops[2] = {MDString::get(Context, Key),
                      ConstantAsMetadata::get(ConstantInt::get(Int64Ty, Val))};
  return MDTuple::get(Context, Ops);
}

static Metadata *getKeyFPValMD(LLVMContext &Context, const char *Key,
                               double Val) {
  Type *DoubleTy = Type::getDoubleTy(Context);
  Metadata *Ops[2] = {MDString::get(Context, Key),
                      ConstantAsMetadata::get(ConstantFP::get(DoubleTy, Val))};
  return MDTuple::get(Context, Ops);
}

// Return an MDTuple with two elements. The first element is a string Key and
// the second is a string Value.
static Metadata *getKeyValMD(LLVMContext &Context, const char *Key,
                             const char *Val) {
  Metadata *Ops[2] = {MDString::get(Context, Key), MDString::get(Context, Val)};
  return MDTuple::get(Context, Ops);
}

// This returns an MDTuple representing the detiled summary. The tuple has two
// elements: a string "DetailedSummary" and an MDTuple representing the value
// of the detailed summary. Each element of this tuple is again an MDTuple whose
// elements are the (Cutoff, MinCount, NumCounts) triplet of the
// DetailedSummaryEntry.
Metadata *ProfileSummary::getDetailedSummaryMD(LLVMContext &Context) {
  std::vector<Metadata *> Entries;
  Type *Int32Ty = Type::getInt32Ty(Context);
  Type *Int64Ty = Type::getInt64Ty(Context);
  for (auto &Entry : DetailedSummary) {
    Metadata *EntryMD[3] = {
        ConstantAsMetadata::get(ConstantInt::get(Int32Ty, Entry.Cutoff)),
        ConstantAsMetadata::get(ConstantInt::get(Int64Ty, Entry.MinCount)),
        ConstantAsMetadata::get(ConstantInt::get(Int32Ty, Entry.NumCounts))};
    Entries.push_back(MDTuple::get(Context, EntryMD));
  }
  Metadata *Ops[2] = {MDString::get(Context, "DetailedSummary"),
                      MDTuple::get(Context, Entries)};
  return MDTuple::get(Context, Ops);
}

// This returns an MDTuple representing this ProfileSummary object. The first
// entry of this tuple is another MDTuple of two elements: a string
// "ProfileFormat" and a string representing the format ("InstrProf" or
// "SampleProfile"). The rest of the elements of the outer MDTuple are specific
// to the kind of profile summary as returned by getFormatSpecificMD.
// IsPartialProfile is an optional field and \p AddPartialField will decide
// whether to add a field for it.
// PartialProfileRatio is an optional field and \p AddPartialProfileRatioField
// will decide whether to add a field for it.
Metadata *ProfileSummary::getMD(LLVMContext &Context, bool AddPartialField,
                                bool AddPartialProfileRatioField) {
  const char *KindStr[3] = {"InstrProf", "CSInstrProf", "SampleProfile"};
  SmallVector<Metadata *, 16> Components;
  Components.push_back(getKeyValMD(Context, "ProfileFormat", KindStr[PSK]));
  Components.push_back(getKeyValMD(Context, "TotalCount", getTotalCount()));
  Components.push_back(getKeyValMD(Context, "MaxCount", getMaxCount()));
  Components.push_back(
      getKeyValMD(Context, "MaxInternalCount", getMaxInternalCount()));
  Components.push_back(
      getKeyValMD(Context, "MaxFunctionCount", getMaxFunctionCount()));
  Components.push_back(getKeyValMD(Context, "NumCounts", getNumCounts()));
  Components.push_back(getKeyValMD(Context, "NumFunctions", getNumFunctions()));
  if (AddPartialField)
    Components.push_back(
        getKeyValMD(Context, "IsPartialProfile", isPartialProfile()));
  if (AddPartialProfileRatioField)
    Components.push_back(getKeyFPValMD(Context, "PartialProfileRatio",
                                       getPartialProfileRatio()));
  Components.push_back(getDetailedSummaryMD(Context));
  return MDTuple::get(Context, Components);
}

// Get the value metadata for the input MD/Key.
static ConstantAsMetadata *getValMD(MDTuple *MD, const char *Key) {
  if (!MD)
    return nullptr;
  if (MD->getNumOperands() != 2)
    return nullptr;
  MDString *KeyMD = dyn_cast<MDString>(MD->getOperand(0));
  ConstantAsMetadata *ValMD = dyn_cast<ConstantAsMetadata>(MD->getOperand(1));
  if (!KeyMD || !ValMD)
    return nullptr;
  if (KeyMD->getString() != Key)
    return nullptr;
  return ValMD;
}

// Parse an MDTuple representing (Key, Val) pair.
static bool getVal(MDTuple *MD, const char *Key, uint64_t &Val) {
  if (auto *ValMD = getValMD(MD, Key)) {
    Val = cast<ConstantInt>(ValMD->getValue())->getZExtValue();
    return true;
  }
  return false;
}

static bool getVal(MDTuple *MD, const char *Key, double &Val) {
  if (auto *ValMD = getValMD(MD, Key)) {
    Val = cast<ConstantFP>(ValMD->getValue())->getValueAPF().convertToDouble();
    return true;
  }
  return false;
}

// Check if an MDTuple represents a (Key, Val) pair.
static bool isKeyValuePair(MDTuple *MD, const char *Key, const char *Val) {
  if (!MD || MD->getNumOperands() != 2)
    return false;
  MDString *KeyMD = dyn_cast<MDString>(MD->getOperand(0));
  MDString *ValMD = dyn_cast<MDString>(MD->getOperand(1));
  if (!KeyMD || !ValMD)
    return false;
  if (KeyMD->getString() != Key || ValMD->getString() != Val)
    return false;
  return true;
}

// Parse an MDTuple representing detailed summary.
static bool getSummaryFromMD(MDTuple *MD, SummaryEntryVector &Summary) {
  if (!MD || MD->getNumOperands() != 2)
    return false;
  MDString *KeyMD = dyn_cast<MDString>(MD->getOperand(0));
  if (!KeyMD || KeyMD->getString() != "DetailedSummary")
    return false;
  MDTuple *EntriesMD = dyn_cast<MDTuple>(MD->getOperand(1));
  if (!EntriesMD)
    return false;
  for (auto &&MDOp : EntriesMD->operands()) {
    MDTuple *EntryMD = dyn_cast<MDTuple>(MDOp);
    if (!EntryMD || EntryMD->getNumOperands() != 3)
      return false;
    ConstantAsMetadata *Op0 =
        dyn_cast<ConstantAsMetadata>(EntryMD->getOperand(0));
    ConstantAsMetadata *Op1 =
        dyn_cast<ConstantAsMetadata>(EntryMD->getOperand(1));
    ConstantAsMetadata *Op2 =
        dyn_cast<ConstantAsMetadata>(EntryMD->getOperand(2));

    if (!Op0 || !Op1 || !Op2)
      return false;
    Summary.emplace_back(cast<ConstantInt>(Op0->getValue())->getZExtValue(),
                         cast<ConstantInt>(Op1->getValue())->getZExtValue(),
                         cast<ConstantInt>(Op2->getValue())->getZExtValue());
  }
  return true;
}

// Get the value of an optional field. Increment 'Idx' if it was present. Return
// true if we can move onto the next field.
template <typename ValueType>
static bool getOptionalVal(MDTuple *Tuple, unsigned &Idx, const char *Key,
                           ValueType &Value) {
  if (getVal(dyn_cast<MDTuple>(Tuple->getOperand(Idx)), Key, Value)) {
    Idx++;
    // Need to make sure when the key is present, we won't step over the bound
    // of Tuple operand array. Since (non-optional) DetailedSummary always comes
    // last, the next entry in the tuple operand array must exist.
    return Idx < Tuple->getNumOperands();
  }
  // It was absent, keep going.
  return true;
}

ProfileSummary *ProfileSummary::getFromMD(Metadata *MD) {
  MDTuple *Tuple = dyn_cast_or_null<MDTuple>(MD);
  if (!Tuple || Tuple->getNumOperands() < 8 || Tuple->getNumOperands() > 10)
    return nullptr;

  unsigned I = 0;
  auto &FormatMD = Tuple->getOperand(I++);
  ProfileSummary::Kind SummaryKind;
  if (isKeyValuePair(dyn_cast_or_null<MDTuple>(FormatMD), "ProfileFormat",
                     "SampleProfile"))
    SummaryKind = PSK_Sample;
  else if (isKeyValuePair(dyn_cast_or_null<MDTuple>(FormatMD), "ProfileFormat",
                          "InstrProf"))
    SummaryKind = PSK_Instr;
  else if (isKeyValuePair(dyn_cast_or_null<MDTuple>(FormatMD), "ProfileFormat",
                          "CSInstrProf"))
    SummaryKind = PSK_CSInstr;
  else
    return nullptr;

  uint64_t NumCounts, TotalCount, NumFunctions, MaxFunctionCount, MaxCount,
      MaxInternalCount;
  if (!getVal(dyn_cast<MDTuple>(Tuple->getOperand(I++)), "TotalCount",
              TotalCount))
    return nullptr;
  if (!getVal(dyn_cast<MDTuple>(Tuple->getOperand(I++)), "MaxCount", MaxCount))
    return nullptr;
  if (!getVal(dyn_cast<MDTuple>(Tuple->getOperand(I++)), "MaxInternalCount",
              MaxInternalCount))
    return nullptr;
  if (!getVal(dyn_cast<MDTuple>(Tuple->getOperand(I++)), "MaxFunctionCount",
              MaxFunctionCount))
    return nullptr;
  if (!getVal(dyn_cast<MDTuple>(Tuple->getOperand(I++)), "NumCounts",
              NumCounts))
    return nullptr;
  if (!getVal(dyn_cast<MDTuple>(Tuple->getOperand(I++)), "NumFunctions",
              NumFunctions))
    return nullptr;

  // Optional fields. Need to initialize because the fields are optional.
  uint64_t IsPartialProfile = 0;
  if (!getOptionalVal(Tuple, I, "IsPartialProfile", IsPartialProfile))
    return nullptr;
  double PartialProfileRatio = 0;
  if (!getOptionalVal(Tuple, I, "PartialProfileRatio", PartialProfileRatio))
    return nullptr;

  SummaryEntryVector Summary;
  if (!getSummaryFromMD(dyn_cast<MDTuple>(Tuple->getOperand(I++)), Summary))
    return nullptr;
  return new ProfileSummary(SummaryKind, std::move(Summary), TotalCount,
                            MaxCount, MaxInternalCount, MaxFunctionCount,
                            NumCounts, NumFunctions, IsPartialProfile,
                            PartialProfileRatio);
}

void ProfileSummary::printSummary(raw_ostream &OS) const {
  OS << "Total functions: " << NumFunctions << "\n";
  OS << "Maximum function count: " << MaxFunctionCount << "\n";
  OS << "Maximum internal block count: " << MaxInternalCount << "\n";
  OS << "Total number of blocks: " << NumCounts << "\n";
  OS << "Total count: " << TotalCount << "\n";
}

void ProfileSummary::printDetailedSummary(raw_ostream &OS) const {
  OS << "Detailed summary:\n";
  for (const auto &Entry : DetailedSummary) {
    OS << Entry.NumCounts << " blocks "
       << format("(%.2f%%)",
                 NumCounts ? (100.f * Entry.NumCounts / NumCounts) : 0)
       << " with count >= " << Entry.MinCount << " account for "
       << format("%0.6g", 100.f * Entry.Cutoff / Scale)
       << "% of the total counts.\n";
  }
}
