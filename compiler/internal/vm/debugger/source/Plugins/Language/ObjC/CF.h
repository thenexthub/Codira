//===-- CF.h ---------------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_LANGUAGE_OBJC_CF_H
#define LLDB_SOURCE_PLUGINS_LANGUAGE_OBJC_CF_H

#include "lldb/DataFormatters/TypeSummary.h"
#include "lldb/Utility/Stream.h"
#include "lldb/ValueObject/ValueObject.h"

namespace lldb_private {
namespace formatters {
bool CFBagSummaryProvider(ValueObject &valobj, Stream &stream,
                          const TypeSummaryOptions &options);

bool CFBinaryHeapSummaryProvider(ValueObject &valobj, Stream &stream,
                                 const TypeSummaryOptions &options);

bool CFBitVectorSummaryProvider(ValueObject &valobj, Stream &stream,
                                const TypeSummaryOptions &options);

bool CFAbsoluteTimeSummaryProvider(ValueObject &valobj, Stream &stream,
                                   const TypeSummaryOptions &options);
} // namespace formatters
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_LANGUAGE_OBJC_CF_H
