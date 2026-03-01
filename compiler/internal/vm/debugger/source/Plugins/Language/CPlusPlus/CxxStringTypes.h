//===-- CxxStringTypes.h ----------------------------------------------*- C++
//-*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_LANGUAGE_CPLUSPLUS_CXXSTRINGTYPES_H
#define LLDB_SOURCE_PLUGINS_LANGUAGE_CPLUSPLUS_CXXSTRINGTYPES_H

#include "lldb/DataFormatters/StringPrinter.h"
#include "lldb/DataFormatters/TypeSummary.h"
#include "lldb/Utility/Stream.h"
#include "lldb/ValueObject/ValueObject.h"

namespace lldb_private {
namespace formatters {
bool Char8StringSummaryProvider(ValueObject &valobj, Stream &stream,
                                const TypeSummaryOptions &options); // char8_t*

bool Char16StringSummaryProvider(
    ValueObject &valobj, Stream &stream,
    const TypeSummaryOptions &options); // char16_t* and unichar*

bool Char32StringSummaryProvider(
    ValueObject &valobj, Stream &stream,
    const TypeSummaryOptions &options); // char32_t*

bool WCharStringSummaryProvider(ValueObject &valobj, Stream &stream,
                                const TypeSummaryOptions &options); // wchar_t*

bool Char8SummaryProvider(ValueObject &valobj, Stream &stream,
                          const TypeSummaryOptions &options); // char8_t

bool Char16SummaryProvider(
    ValueObject &valobj, Stream &stream,
    const TypeSummaryOptions &options); // char16_t and unichar

bool Char32SummaryProvider(ValueObject &valobj, Stream &stream,
                           const TypeSummaryOptions &options); // char32_t

bool WCharSummaryProvider(ValueObject &valobj, Stream &stream,
                          const TypeSummaryOptions &options); // wchar_t

std::optional<uint64_t> GetWCharByteSize(ValueObject &valobj);

/// Print a summary for a string buffer to \a stream.
///
/// \param[in] stream
///     The output stream to print the summary to.
///
/// \param[in] summary_options
///     Options for printing the string contents. This function respects the
///     capping.
///
/// \param[in] location_sp
///     ValueObject of a pointer to the string being printed.
///
/// \param[in] size
///     The size of the buffer pointed to by \a location_sp.
///
/// \param[in] prefix_token
///     A prefix before the double quotes (e.g. 'u' results in u"...").
///
/// \return
///     Returns whether the string buffer was successfully printed.
template <StringPrinter::StringElementType element_type>
bool StringBufferSummaryProvider(Stream &stream,
                                 const TypeSummaryOptions &summary_options,
                                 lldb::ValueObjectSP location_sp, uint64_t size,
                                 std::string prefix_token);

} // namespace formatters
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_LANGUAGE_CPLUSPLUS_CXXSTRINGTYPES_H
