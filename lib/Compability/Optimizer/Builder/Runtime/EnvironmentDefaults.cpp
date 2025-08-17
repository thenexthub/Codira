//===-- EnvironmentDefaults.cpp -------------------------------------------===//
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

#include "language/Compability/Optimizer/Builder/Runtime/EnvironmentDefaults.h"
#include "language/Compability/Lower/EnvironmentDefault.h"
#include "language/Compability/Optimizer/Builder/BoxValue.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Support/InternalNames.h"
#include "toolchain/ADT/ArrayRef.h"

mlir::Value fir::runtime::genEnvironmentDefaults(
    fir::FirOpBuilder &builder, mlir::Location loc,
    const std::vector<language::Compability::lower::EnvironmentDefault> &envDefaults) {
  std::string envDefaultListPtrName =
      fir::NameUniquer::doGenerated("EnvironmentDefaults");

  mlir::MLIRContext *context = builder.getContext();
  mlir::StringAttr linkOnce = builder.createLinkOnceLinkage();
  mlir::IntegerType intTy = builder.getIntegerType(8 * sizeof(int));
  fir::ReferenceType charRefTy =
      fir::ReferenceType::get(builder.getIntegerType(8));
  fir::SequenceType itemListTy = fir::SequenceType::get(
      envDefaults.size(),
      mlir::TupleType::get(context, {charRefTy, charRefTy}));
  mlir::TupleType envDefaultListTy = mlir::TupleType::get(
      context, {intTy, fir::ReferenceType::get(itemListTy)});
  fir::ReferenceType envDefaultListRefTy =
      fir::ReferenceType::get(envDefaultListTy);

  // If no defaults were specified, initialize with a null pointer.
  if (envDefaults.empty()) {
    mlir::Value nullVal = builder.createNullConstant(loc, envDefaultListRefTy);
    return nullVal;
  }

  // Create the Item list.
  mlir::IndexType idxTy = builder.getIndexType();
  mlir::IntegerAttr zero = builder.getIntegerAttr(idxTy, 0);
  mlir::IntegerAttr one = builder.getIntegerAttr(idxTy, 1);
  std::string itemListName = envDefaultListPtrName + ".items";
  auto listBuilder = [&](fir::FirOpBuilder &builder) {
    mlir::Value list = fir::UndefOp::create(builder, loc, itemListTy);
    toolchain::SmallVector<mlir::Attribute, 2> idx = {mlir::Attribute{},
                                                 mlir::Attribute{}};
    auto insertStringField = [&](const std::string &s,
                                 toolchain::ArrayRef<mlir::Attribute> idx) {
      mlir::Value stringAddress = fir::getBase(
          fir::factory::createStringLiteral(builder, loc, s + '\0'));
      mlir::Value addr = builder.createConvert(loc, charRefTy, stringAddress);
      return fir::InsertValueOp::create(builder, loc, itemListTy, list, addr,
                                        builder.getArrayAttr(idx));
    };

    size_t n = 0;
    for (const language::Compability::lower::EnvironmentDefault &def : envDefaults) {
      idx[0] = builder.getIntegerAttr(idxTy, n);
      idx[1] = zero;
      list = insertStringField(def.varName, idx);
      idx[1] = one;
      list = insertStringField(def.defaultValue, idx);
      ++n;
    }
    fir::HasValueOp::create(builder, loc, list);
  };
  builder.createGlobalConstant(loc, itemListTy, itemListName, listBuilder,
                               linkOnce);

  // Define the EnviornmentDefaultList object.
  auto envDefaultListBuilder = [&](fir::FirOpBuilder &builder) {
    mlir::Value envDefaultList =
        fir::UndefOp::create(builder, loc, envDefaultListTy);
    mlir::Value numItems =
        builder.createIntegerConstant(loc, intTy, envDefaults.size());
    envDefaultList = fir::InsertValueOp::create(builder, loc, envDefaultListTy,
                                                envDefaultList, numItems,
                                                builder.getArrayAttr(zero));
    fir::GlobalOp itemList = builder.getNamedGlobal(itemListName);
    assert(itemList && "missing environment default list");
    mlir::Value listAddr = fir::AddrOfOp::create(
        builder, loc, itemList.resultType(), itemList.getSymbol());
    envDefaultList = fir::InsertValueOp::create(builder, loc, envDefaultListTy,
                                                envDefaultList, listAddr,
                                                builder.getArrayAttr(one));
    fir::HasValueOp::create(builder, loc, envDefaultList);
  };
  fir::GlobalOp envDefaultList = builder.createGlobalConstant(
      loc, envDefaultListTy, envDefaultListPtrName + ".list",
      envDefaultListBuilder, linkOnce);

  // Define the pointer to the list used by the runtime.
  mlir::Value addr = fir::AddrOfOp::create(
      builder, loc, envDefaultList.resultType(), envDefaultList.getSymbol());
  return addr;
}
