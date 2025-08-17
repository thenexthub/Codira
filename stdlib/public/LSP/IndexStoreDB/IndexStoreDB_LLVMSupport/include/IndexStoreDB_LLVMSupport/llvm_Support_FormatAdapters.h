//===- FormatAdapters.h - Formatters for common LLVM types -----*- C++ -*-===//
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

#ifndef LLVM_SUPPORT_FORMATADAPTERS_H
#define LLVM_SUPPORT_FORMATADAPTERS_H

#include <IndexStoreDB_LLVMSupport/toolchain_ADT_SmallString.h>
#include <IndexStoreDB_LLVMSupport/toolchain_ADT_StringRef.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_Error.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_FormatCommon.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_FormatVariadicDetails.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_raw_ostream.h>

namespace toolchain {
template <typename T> class FormatAdapter : public detail::format_adapter {
protected:
  explicit FormatAdapter(T &&Item) : Item(std::forward<T>(Item)) {}

  T Item;
};

namespace detail {
template <typename T> class AlignAdapter final : public FormatAdapter<T> {
  AlignStyle Where;
  size_t Amount;
  char Fill;

public:
  AlignAdapter(T &&Item, AlignStyle Where, size_t Amount, char Fill)
      : FormatAdapter<T>(std::forward<T>(Item)), Where(Where), Amount(Amount),
        Fill(Fill) {}

  void format(toolchain::raw_ostream &Stream, StringRef Style) {
    auto Adapter = detail::build_format_adapter(std::forward<T>(this->Item));
    FmtAlign(Adapter, Where, Amount, Fill).format(Stream, Style);
  }
};

template <typename T> class PadAdapter final : public FormatAdapter<T> {
  size_t Left;
  size_t Right;

public:
  PadAdapter(T &&Item, size_t Left, size_t Right)
      : FormatAdapter<T>(std::forward<T>(Item)), Left(Left), Right(Right) {}

  void format(toolchain::raw_ostream &Stream, StringRef Style) {
    auto Adapter = detail::build_format_adapter(std::forward<T>(this->Item));
    Stream.indent(Left);
    Adapter.format(Stream, Style);
    Stream.indent(Right);
  }
};

template <typename T> class RepeatAdapter final : public FormatAdapter<T> {
  size_t Count;

public:
  RepeatAdapter(T &&Item, size_t Count)
      : FormatAdapter<T>(std::forward<T>(Item)), Count(Count) {}

  void format(toolchain::raw_ostream &Stream, StringRef Style) {
    auto Adapter = detail::build_format_adapter(std::forward<T>(this->Item));
    for (size_t I = 0; I < Count; ++I) {
      Adapter.format(Stream, Style);
    }
  }
};

class ErrorAdapter : public FormatAdapter<Error> {
public:
  ErrorAdapter(Error &&Item) : FormatAdapter(std::move(Item)) {}
  ErrorAdapter(ErrorAdapter &&) = default;
  ~ErrorAdapter() { consumeError(std::move(Item)); }
  void format(toolchain::raw_ostream &Stream, StringRef Style) { Stream << Item; }
};
}

template <typename T>
detail::AlignAdapter<T> fmt_align(T &&Item, AlignStyle Where, size_t Amount,
                                  char Fill = ' ') {
  return detail::AlignAdapter<T>(std::forward<T>(Item), Where, Amount, Fill);
}

template <typename T>
detail::PadAdapter<T> fmt_pad(T &&Item, size_t Left, size_t Right) {
  return detail::PadAdapter<T>(std::forward<T>(Item), Left, Right);
}

template <typename T>
detail::RepeatAdapter<T> fmt_repeat(T &&Item, size_t Count) {
  return detail::RepeatAdapter<T>(std::forward<T>(Item), Count);
}

// toolchain::Error values must be consumed before being destroyed.
// Wrapping an error in fmt_consume explicitly indicates that the formatv_object
// should take ownership and consume it.
inline detail::ErrorAdapter fmt_consume(Error &&Item) {
  return detail::ErrorAdapter(std::move(Item));
}
}

#endif
