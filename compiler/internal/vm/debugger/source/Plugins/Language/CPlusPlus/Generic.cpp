//===-- Generic.cpp ------------------------------------------------------===//
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
//===---------------------------------------------------------------------===//

#include "Generic.h"
#include "LibStdcpp.h"
#include "MsvcStl.h"

lldb::ValueObjectSP lldb_private::formatters::GetDesugaredSmartPointerValue(
    ValueObject &ptr, ValueObject &container) {
  auto container_type = container.GetCompilerType().GetNonReferenceType();
  if (!container_type)
    return nullptr;

  auto arg = container_type.GetTypeTemplateArgument(0);
  if (!arg)
    // If there isn't enough debug info, use the pointer type as is
    return ptr.GetSP();

  return ptr.Cast(arg.GetPointerType());
}
