//===-- CoreMedia.cpp -----------------------------------------------------===//
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

#include "CoreMedia.h"

#include "lldb/Utility/Flags.h"
#include "lldb/Utility/Log.h"

#include "lldb/Symbol/TypeSystem.h"
#include "lldb/Target/Target.h"
#include <cinttypes>

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::formatters;

bool lldb_private::formatters::CMTimeSummaryProvider(
    ValueObject &valobj, Stream &stream, const TypeSummaryOptions &options) {
  CompilerType type = valobj.GetCompilerType();
  if (!type.IsValid())
    return false;

  auto type_system = type.GetTypeSystem();
  if (!type_system)
    return false;
  // fetch children by offset to compensate for potential lack of debug info
  auto int64_ty =
      type_system->GetBuiltinTypeForEncodingAndBitSize(eEncodingSint, 64);
  auto int32_ty =
      type_system->GetBuiltinTypeForEncodingAndBitSize(eEncodingSint, 32);

  auto value_sp(valobj.GetSyntheticChildAtOffset(0, int64_ty, true));
  auto timescale_sp(valobj.GetSyntheticChildAtOffset(8, int32_ty, true));
  auto flags_sp(valobj.GetSyntheticChildAtOffset(12, int32_ty, true));

  if (!value_sp || !timescale_sp || !flags_sp)
    return false;

  auto value = value_sp->GetValueAsUnsigned(0);
  auto timescale = (int32_t)timescale_sp->GetValueAsUnsigned(
      0); // the timescale specifies the fraction of a second each unit in the
          // numerator occupies
  auto flags = Flags(flags_sp->GetValueAsUnsigned(0) &
                     0x00000000000000FF); // the flags I need sit in the LSB

  const unsigned int FlagPositiveInf = 4;
  const unsigned int FlagNegativeInf = 8;
  const unsigned int FlagIndefinite = 16;

  if (flags.AnySet(FlagIndefinite)) {
    stream.Printf("indefinite");
    return true;
  }

  if (flags.AnySet(FlagPositiveInf)) {
    stream.Printf("+oo");
    return true;
  }

  if (flags.AnySet(FlagNegativeInf)) {
    stream.Printf("-oo");
    return true;
  }

  switch (timescale) {
  case 0:
    return false;
  case 1:
    stream.Printf("%" PRId64 " seconds", value);
    return true;
  case 2:
    stream.Printf("%" PRId64 " half seconds", value);
    return true;
  case 3:
    stream.Printf("%" PRId64 " third%sof a second", value,
                  value == 1 ? " " : "s ");
    return true;
  default:
    stream.Printf("%" PRId64 " %" PRId32 "th%sof a second", value, timescale,
                  value == 1 ? " " : "s ");
    return true;
  }
}
