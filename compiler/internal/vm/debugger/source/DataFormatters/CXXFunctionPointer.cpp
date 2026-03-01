//===-- CXXFunctionPointer.cpp---------------------------------------------===//
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

#include "lldb/DataFormatters/CXXFunctionPointer.h"

#include "lldb/Target/ABI.h"
#include "lldb/Target/SectionLoadList.h"
#include "lldb/Target/Target.h"
#include "lldb/Utility/Stream.h"
#include "lldb/ValueObject/ValueObject.h"
#include "lldb/lldb-enumerations.h"

#include <string>

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::formatters;

bool lldb_private::formatters::CXXFunctionPointerSummaryProvider(
    ValueObject &valobj, Stream &stream, const TypeSummaryOptions &options) {
  StreamString sstr;
  auto [func_ptr_address, func_ptr_address_type] = valobj.GetPointerValue();
  if (func_ptr_address != 0 && func_ptr_address != LLDB_INVALID_ADDRESS) {
    switch (func_ptr_address_type) {
    case eAddressTypeInvalid:
    case eAddressTypeFile:
    case eAddressTypeHost:
      break;

    case eAddressTypeLoad: {
      ExecutionContext exe_ctx(valobj.GetExecutionContextRef());

      Address so_addr;
      Target *target = exe_ctx.GetTargetPtr();
      if (target && target->HasLoadedSections()) {
        target->ResolveLoadAddress(func_ptr_address, so_addr);
        if (so_addr.GetSection() == nullptr) {
          // If we have an address that doesn't correspond to any symbol,
          // it might have authentication bits.  Strip them & see if it
          // now points to a symbol -- if so, do the SymbolContext lookup
          // based on the stripped address.
          // If we find a symbol with the ptrauth bits stripped, print the
          // raw value into the stream, and replace the Address with the
          // one that points to a symbol for a fuller description.
          if (Process *process = exe_ctx.GetProcessPtr()) {
            if (ABISP abi_sp = process->GetABI()) {
              addr_t fixed_addr = abi_sp->FixCodeAddress(func_ptr_address);
              if (fixed_addr != func_ptr_address) {
                Address test_address;
                test_address.SetLoadAddress(fixed_addr, target);
                if (test_address.GetSection() != nullptr) {
                  int addrsize = target->GetArchitecture().GetAddressByteSize();
                  sstr.Printf("actual=0x%*.*" PRIx64 " ", addrsize * 2,
                              addrsize * 2, fixed_addr);
                  so_addr = test_address;
                }
              }
            }
          }
        }

        if (so_addr.IsValid()) {
          so_addr.Dump(&sstr, exe_ctx.GetBestExecutionContextScope(),
                       Address::DumpStyleResolvedDescription,
                       Address::DumpStyleSectionNameOffset);
        }
      }
    } break;
    }
  }
  if (sstr.GetSize() > 0) {
    if (valobj.GetValueType() == lldb::eValueTypeVTableEntry)
      stream.PutCString(sstr.GetData());
    else
      stream.Printf("(%s)", sstr.GetData());
    return true;
  } else
    return false;
}
