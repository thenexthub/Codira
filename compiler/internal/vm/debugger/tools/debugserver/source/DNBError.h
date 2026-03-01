//===-- DNBError.h ----------------------------------------------*- C++ -*-===//
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
//
//  Created by Greg Clayton on 6/26/07.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_TOOLS_DEBUGSERVER_SOURCE_DNBERROR_H
#define LLDB_TOOLS_DEBUGSERVER_SOURCE_DNBERROR_H

#include <cerrno>
#include <cstdio>
#include <mach/mach.h>
#include <string>

class DNBError {
public:
  typedef uint32_t ValueType;
  enum FlavorType {
    Generic = 0,
    MachKernel = 1,
    POSIX = 2
#ifdef WITH_SPRINGBOARD
    ,
    SpringBoard = 3
#endif
#ifdef WITH_BKS
    ,
    BackBoard = 4
#endif
#ifdef WITH_FBS
    ,
    FrontBoard = 5
#endif
  };

  explicit DNBError(ValueType err = 0, FlavorType flavor = Generic)
      : m_err(err), m_flavor(flavor) {}

  const char *AsString() const;
  void Clear() {
    m_err = 0;
    m_flavor = Generic;
    m_str.clear();
  }
  ValueType Status() const { return m_err; }
  FlavorType Flavor() const { return m_flavor; }

  ValueType operator=(kern_return_t err) {
    m_err = err;
    m_flavor = MachKernel;
    m_str.clear();
    return m_err;
  }

  void SetError(kern_return_t err) {
    m_err = err;
    m_flavor = MachKernel;
    m_str.clear();
  }

  void SetErrorToErrno() {
    m_err = errno;
    m_flavor = POSIX;
    m_str.clear();
  }

  void SetError(ValueType err, FlavorType flavor) {
    m_err = err;
    m_flavor = flavor;
    m_str.clear();
  }

  // Generic errors can set their own string values
  void SetErrorString(const char *err_str) {
    if (err_str && err_str[0])
      m_str = err_str;
    else
      m_str.clear();
  }
  bool Success() const { return m_err == 0; }
  bool Fail() const { return m_err != 0; }
  void LogThreadedIfError(const char *format, ...) const;
  void LogThreaded(const char *format, ...) const;

protected:
  ValueType m_err;
  FlavorType m_flavor;
  mutable std::string m_str;
};

#endif // LLDB_TOOLS_DEBUGSERVER_SOURCE_DNBERROR_H
