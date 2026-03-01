//===-- SBData.h -----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBDATA_H
#define LLDB_API_SBDATA_H

#include "lldb/API/SBDefines.h"

namespace lldb_private {
class ScriptInterpreter;
} // namespace lldb_private

namespace lldb {

class LLDB_API SBData {
public:
  SBData();

  SBData(const SBData &rhs);

  const SBData &operator=(const SBData &rhs);

  ~SBData();

  uint8_t GetAddressByteSize();

  void SetAddressByteSize(uint8_t addr_byte_size);

  void Clear();

  explicit operator bool() const;

  bool IsValid();

  size_t GetByteSize();

  lldb::ByteOrder GetByteOrder();

  void SetByteOrder(lldb::ByteOrder endian);

  float GetFloat(lldb::SBError &error, lldb::offset_t offset);

  double GetDouble(lldb::SBError &error, lldb::offset_t offset);

  long double GetLongDouble(lldb::SBError &error, lldb::offset_t offset);

  lldb::addr_t GetAddress(lldb::SBError &error, lldb::offset_t offset);

  uint8_t GetUnsignedInt8(lldb::SBError &error, lldb::offset_t offset);

  uint16_t GetUnsignedInt16(lldb::SBError &error, lldb::offset_t offset);

  uint32_t GetUnsignedInt32(lldb::SBError &error, lldb::offset_t offset);

  uint64_t GetUnsignedInt64(lldb::SBError &error, lldb::offset_t offset);

  int8_t GetSignedInt8(lldb::SBError &error, lldb::offset_t offset);

  int16_t GetSignedInt16(lldb::SBError &error, lldb::offset_t offset);

  int32_t GetSignedInt32(lldb::SBError &error, lldb::offset_t offset);

  int64_t GetSignedInt64(lldb::SBError &error, lldb::offset_t offset);

  const char *GetString(lldb::SBError &error, lldb::offset_t offset);

  size_t ReadRawData(lldb::SBError &error, lldb::offset_t offset, void *buf,
                     size_t size);

  bool GetDescription(lldb::SBStream &description,
                      lldb::addr_t base_addr = LLDB_INVALID_ADDRESS);

  // it would be nice to have SetData(SBError, const void*, size_t) when
  // endianness and address size can be inferred from the existing
  // DataExtractor, but having two SetData() signatures triggers a SWIG bug
  // where the typemap isn't applied before resolving the overload, and thus
  // the right function never gets called
  void SetData(lldb::SBError &error, const void *buf, size_t size,
               lldb::ByteOrder endian, uint8_t addr_size);

  void SetDataWithOwnership(lldb::SBError &error, const void *buf, size_t size,
                            lldb::ByteOrder endian, uint8_t addr_size);

  // see SetData() for why we don't have Append(const void* buf, size_t size)
  bool Append(const SBData &rhs);

  static lldb::SBData CreateDataFromCString(lldb::ByteOrder endian,
                                            uint32_t addr_byte_size,
                                            const char *data);

  // in the following CreateData*() and SetData*() prototypes, the two
  // parameters array and array_len should not be renamed or rearranged,
  // because doing so will break the SWIG typemap
  static lldb::SBData CreateDataFromUInt64Array(lldb::ByteOrder endian,
                                                uint32_t addr_byte_size,
                                                uint64_t *array,
                                                size_t array_len);

  static lldb::SBData CreateDataFromUInt32Array(lldb::ByteOrder endian,
                                                uint32_t addr_byte_size,
                                                uint32_t *array,
                                                size_t array_len);

  static lldb::SBData CreateDataFromSInt64Array(lldb::ByteOrder endian,
                                                uint32_t addr_byte_size,
                                                int64_t *array,
                                                size_t array_len);

  static lldb::SBData CreateDataFromSInt32Array(lldb::ByteOrder endian,
                                                uint32_t addr_byte_size,
                                                int32_t *array,
                                                size_t array_len);

  static lldb::SBData CreateDataFromDoubleArray(lldb::ByteOrder endian,
                                                uint32_t addr_byte_size,
                                                double *array,
                                                size_t array_len);

  bool SetDataFromCString(const char *data);

  bool SetDataFromUInt64Array(uint64_t *array, size_t array_len);

  bool SetDataFromUInt32Array(uint32_t *array, size_t array_len);

  bool SetDataFromSInt64Array(int64_t *array, size_t array_len);

  bool SetDataFromSInt32Array(int32_t *array, size_t array_len);

  bool SetDataFromDoubleArray(double *array, size_t array_len);

protected:
  // Mimic shared pointer...
  lldb_private::DataExtractor *get() const;

  lldb_private::DataExtractor *operator->() const;

  lldb::DataExtractorSP &operator*();

  const lldb::DataExtractorSP &operator*() const;

  SBData(const lldb::DataExtractorSP &data_sp);

  void SetOpaque(const lldb::DataExtractorSP &data_sp);

private:
  friend class SBInstruction;
  friend class SBProcess;
  friend class SBSection;
  friend class SBTarget;
  friend class SBValue;

  friend class lldb_private::ScriptInterpreter;

  lldb::DataExtractorSP m_opaque_sp;
};

} // namespace lldb

#endif // LLDB_API_SBDATA_H
