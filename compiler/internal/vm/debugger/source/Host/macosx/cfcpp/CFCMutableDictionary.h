//===-- CFCMutableDictionary.h ----------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCMUTABLEDICTIONARY_H
#define LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCMUTABLEDICTIONARY_H

#include "CFCReleaser.h"

class CFCMutableDictionary : public CFCReleaser<CFMutableDictionaryRef> {
public:
  // Constructors and Destructors
  CFCMutableDictionary(CFMutableDictionaryRef s = NULL);
  CFCMutableDictionary(const CFCMutableDictionary &rhs);
  ~CFCMutableDictionary() override;

  // Operators
  const CFCMutableDictionary &operator=(const CFCMutableDictionary &rhs);

  CFIndex GetCount() const;
  CFIndex GetCountOfKey(const void *value) const;
  CFIndex GetCountOfValue(const void *value) const;
  void GetKeysAndValues(const void **keys, const void **values) const;
  const void *GetValue(const void *key) const;
  Boolean GetValueIfPresent(const void *key, const void **value_handle) const;
  bool AddValue(CFStringRef key, const void *value, bool can_create = false);
  bool SetValue(CFStringRef key, const void *value, bool can_create = false);
  bool AddValueSInt8(CFStringRef key, int8_t value, bool can_create = false);
  bool SetValueSInt8(CFStringRef key, int8_t value, bool can_create = false);
  bool AddValueSInt16(CFStringRef key, int16_t value, bool can_create = false);
  bool SetValueSInt16(CFStringRef key, int16_t value, bool can_create = false);
  bool AddValueSInt32(CFStringRef key, int32_t value, bool can_create = false);
  bool SetValueSInt32(CFStringRef key, int32_t value, bool can_create = false);
  bool AddValueSInt64(CFStringRef key, int64_t value, bool can_create = false);
  bool SetValueSInt64(CFStringRef key, int64_t value, bool can_create = false);
  bool AddValueUInt8(CFStringRef key, uint8_t value, bool can_create = false);
  bool SetValueUInt8(CFStringRef key, uint8_t value, bool can_create = false);
  bool AddValueUInt16(CFStringRef key, uint16_t value, bool can_create = false);
  bool SetValueUInt16(CFStringRef key, uint16_t value, bool can_create = false);
  bool AddValueUInt32(CFStringRef key, uint32_t value, bool can_create = false);
  bool SetValueUInt32(CFStringRef key, uint32_t value, bool can_create = false);
  bool AddValueUInt64(CFStringRef key, uint64_t value, bool can_create = false);
  bool SetValueUInt64(CFStringRef key, uint64_t value, bool can_create = false);
  bool AddValueDouble(CFStringRef key, double value, bool can_create = false);
  bool SetValueDouble(CFStringRef key, double value, bool can_create = false);
  bool AddValueCString(CFStringRef key, const char *cstr,
                       bool can_create = false);
  bool SetValueCString(CFStringRef key, const char *cstr,
                       bool can_create = false);
  void RemoveValue(const void *value);
  void ReplaceValue(const void *key, const void *value);
  void RemoveAllValues();
  CFMutableDictionaryRef Dictionary(bool can_create);

protected:
  // Classes that inherit from CFCMutableDictionary can see and modify these

private:
  // For CFCMutableDictionary only
};

#endif // LLDB_SOURCE_HOST_MACOSX_CFCPP_CFCMUTABLEDICTIONARY_H
