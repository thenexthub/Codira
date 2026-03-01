//===-- LibStdcpp.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_LANGUAGE_CPLUSPLUS_LIBSTDCPP_H
#define LLDB_SOURCE_PLUGINS_LANGUAGE_CPLUSPLUS_LIBSTDCPP_H

#include "lldb/DataFormatters/StringPrinter.h"
#include "lldb/DataFormatters/TypeSummary.h"
#include "lldb/DataFormatters/TypeSynthetic.h"
#include "lldb/Utility/Stream.h"
#include "lldb/ValueObject/ValueObject.h"

namespace lldb_private {
namespace formatters {
bool LibStdcppStringSummaryProvider(
    ValueObject &valobj, Stream &stream,
    const TypeSummaryOptions &options); // libstdc++ std::string

template <StringPrinter::StringElementType element_type>
bool LibStdcppStringViewSummaryProvider(
    ValueObject &valobj, Stream &stream,
    const TypeSummaryOptions
        &options); // libstdc++ std::{u8,u16,u32}?string_view

bool LibStdcppWStringViewSummaryProvider(
    ValueObject &valobj, Stream &stream,
    const TypeSummaryOptions &options); // libstdc++ std::wstring_view

bool LibStdcppSmartPointerSummaryProvider(
    ValueObject &valobj, Stream &stream,
    const TypeSummaryOptions
        &options); // libstdc++ std::shared_ptr<> and std::weak_ptr<>

bool LibStdcppUniquePointerSummaryProvider(
    ValueObject &valobj, Stream &stream,
    const TypeSummaryOptions &options); // libstdc++ std::unique_ptr<>

bool LibStdcppVariantSummaryProvider(
    ValueObject &valobj, Stream &stream,
    const TypeSummaryOptions &options); // libstdc++ std::variant<>

bool LibStdcppPartialOrderingSummaryProvider(
    ValueObject &valobj, Stream &stream,
    const TypeSummaryOptions &options); // libstdc++ std::partial_ordering

bool LibStdcppWeakOrderingSummaryProvider(
    ValueObject &valobj, Stream &stream,
    const TypeSummaryOptions &options); // libstdc++ std::weak_ordering

bool LibStdcppStrongOrderingSummaryProvider(
    ValueObject &valobj, Stream &stream,
    const TypeSummaryOptions &options); // libstdc++ std::strong_ordering

SyntheticChildrenFrontEnd *
LibstdcppMapIteratorSyntheticFrontEndCreator(CXXSyntheticChildren *,
                                             lldb::ValueObjectSP);

SyntheticChildrenFrontEnd *
LibStdcppSpanSyntheticFrontEndCreator(CXXSyntheticChildren *,
                                      lldb::ValueObjectSP);

SyntheticChildrenFrontEnd *
LibStdcppTupleSyntheticFrontEndCreator(CXXSyntheticChildren *,
                                       lldb::ValueObjectSP);

SyntheticChildrenFrontEnd *
LibStdcppBitsetSyntheticFrontEndCreator(CXXSyntheticChildren *,
                                        lldb::ValueObjectSP);

SyntheticChildrenFrontEnd *
LibStdcppOptionalSyntheticFrontEndCreator(CXXSyntheticChildren *,
                                          lldb::ValueObjectSP);

SyntheticChildrenFrontEnd *
LibStdcppVectorIteratorSyntheticFrontEndCreator(CXXSyntheticChildren *,
                                                lldb::ValueObjectSP);

SyntheticChildrenFrontEnd *
LibStdcppSharedPtrSyntheticFrontEndCreator(CXXSyntheticChildren *,
                                           lldb::ValueObjectSP);

SyntheticChildrenFrontEnd *
LibStdcppUniquePtrSyntheticFrontEndCreator(CXXSyntheticChildren *,
                                           lldb::ValueObjectSP);

bool LibStdcppVariantSummaryProvider(ValueObject &valobj, Stream &stream,
                                     const TypeSummaryOptions &options);

} // namespace formatters
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_LANGUAGE_CPLUSPLUS_LIBSTDCPP_H
