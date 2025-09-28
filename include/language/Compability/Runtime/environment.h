//===-- language/Compability-rt/runtime/environment.h ------------------*- C++ -*-===//
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

#ifndef FLANG_RT_RUNTIME_ENVIRONMENT_H_
#define FLANG_RT_RUNTIME_ENVIRONMENT_H_

#include "language/Compability/Common/optional.h"
#include "language/Compability/Decimal/decimal.h"

struct EnvironmentDefaultList;

namespace language::Compability::runtime {

class Terminator;

RT_OFFLOAD_VAR_GROUP_BEGIN
#if FLANG_BIG_ENDIAN
constexpr bool isHostLittleEndian{false};
#elif FLANG_LITTLE_ENDIAN
constexpr bool isHostLittleEndian{true};
#else
#error host endianness is not known
#endif
RT_OFFLOAD_VAR_GROUP_END

// External unformatted I/O data conversions
enum class Convert { Unknown, Native, LittleEndian, BigEndian, Swap };

RT_API_ATTRS language::Compability::common::optional<Convert> GetConvertFromString(
    const char *, std::size_t);

struct ExecutionEnvironment {
#if !defined(_OPENMP)
  // FIXME: https://github.com/toolchain/toolchain-project/issues/84942
  constexpr
#endif
      ExecutionEnvironment(){};
  void Configure(int argc, const char *argv[], const char *envp[],
      const EnvironmentDefaultList *envDefaults);
  const char *GetEnv(
      const char *name, std::size_t name_length, const Terminator &terminator);

  std::int32_t SetEnv(const char *name, std::size_t name_length,
      const char *value, std::size_t value_length,
      const Terminator &terminator);

  std::int32_t UnsetEnv(
      const char *name, std::size_t name_length, const Terminator &terminator);

  int argc{0};
  const char **argv{nullptr};
  char **envp{nullptr};

  int listDirectedOutputLineLengthLimit{79}; // FORT_FMT_RECL
  enum decimal::FortranRounding defaultOutputRoundingMode{
      decimal::FortranRounding::RoundNearest}; // RP(==PN)
  Convert conversion{Convert::Unknown}; // FORT_CONVERT
  bool noStopMessage{false}; // NO_STOP_MESSAGE=1 inhibits "Fortran STOP"
  bool defaultUTF8{false}; // DEFAULT_UTF8
  bool checkPointerDeallocation{true}; // FORT_CHECK_POINTER_DEALLOCATION

  enum InternalDebugging { WorkQueue = 1 };
  int internalDebugging{0}; // FLANG_RT_DEBUG

  // CUDA related variables
  std::size_t cudaStackLimit{0}; // ACC_OFFLOAD_STACK_SIZE
  bool cudaDeviceIsManaged{false}; // NV_CUDAFOR_DEVICE_IS_MANAGED
};

RT_OFFLOAD_VAR_GROUP_BEGIN
extern RT_VAR_ATTRS ExecutionEnvironment executionEnvironment;
RT_OFFLOAD_VAR_GROUP_END

} // namespace language::Compability::runtime

#endif // FLANG_RT_RUNTIME_ENVIRONMENT_H_
