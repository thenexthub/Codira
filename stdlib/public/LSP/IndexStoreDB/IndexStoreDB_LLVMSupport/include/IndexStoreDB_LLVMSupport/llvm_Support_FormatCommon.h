//===- FormatCommon.h - Formatters for common LLVM types --------*- C++ -*-===//
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

#ifndef LLVM_SUPPORT_FORMATCOMMON_H
#define LLVM_SUPPORT_FORMATCOMMON_H

#include <IndexStoreDB_LLVMSupport/toolchain_ADT_SmallString.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_FormatVariadicDetails.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_raw_ostream.h>

namespace toolchain {
enum class AlignStyle { Left, Center, Right };

struct FmtAlign {
  detail::format_adapter &Adapter;
  AlignStyle Where;
  size_t Amount;
  char Fill;

  FmtAlign(detail::format_adapter &Adapter, AlignStyle Where, size_t Amount,
           char Fill = ' ')
      : Adapter(Adapter), Where(Where), Amount(Amount), Fill(Fill) {}

  void format(raw_ostream &S, StringRef Options) {
    // If we don't need to align, we can format straight into the underlying
    // stream.  Otherwise we have to go through an intermediate stream first
    // in order to calculate how long the output is so we can align it.
    // TODO: Make the format method return the number of bytes written, that
    // way we can also skip the intermediate stream for left-aligned output.
    if (Amount == 0) {
      Adapter.format(S, Options);
      return;
    }
    SmallString<64> Item;
    raw_svector_ostream Stream(Item);

    Adapter.format(Stream, Options);
    if (Amount <= Item.size()) {
      S << Item;
      return;
    }

    size_t PadAmount = Amount - Item.size();
    switch (Where) {
    case AlignStyle::Left:
      S << Item;
      fill(S, PadAmount);
      break;
    case AlignStyle::Center: {
      size_t X = PadAmount / 2;
      fill(S, X);
      S << Item;
      fill(S, PadAmount - X);
      break;
    }
    default:
      fill(S, PadAmount);
      S << Item;
      break;
    }
  }

private:
  void fill(toolchain::raw_ostream &S, uint32_t Count) {
    for (uint32_t I = 0; I < Count; ++I)
      S << Fill;
  }
};
}

#endif
