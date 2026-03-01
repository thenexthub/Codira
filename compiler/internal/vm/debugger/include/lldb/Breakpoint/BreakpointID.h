//===-- BreakpointID.h ------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_BREAKPOINT_BREAKPOINTID_H
#define LLDB_BREAKPOINT_BREAKPOINTID_H

#include "lldb/lldb-private.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include <optional>

namespace lldb_private {

// class BreakpointID

class BreakpointID {
public:
  BreakpointID(lldb::break_id_t bp_id = LLDB_INVALID_BREAK_ID,
               lldb::break_id_t loc_id = LLDB_INVALID_BREAK_ID);

  virtual ~BreakpointID();

  bool operator==(BreakpointID rhs) const {
    return m_break_id == rhs.m_break_id && m_location_id == rhs.m_location_id;
  }

  lldb::break_id_t GetBreakpointID() const { return m_break_id; }

  lldb::break_id_t GetLocationID() const { return m_location_id; }

  void SetID(lldb::break_id_t bp_id, lldb::break_id_t loc_id) {
    m_break_id = bp_id;
    m_location_id = loc_id;
  }

  void SetBreakpointID(lldb::break_id_t bp_id) { m_break_id = bp_id; }

  void SetBreakpointLocationID(lldb::break_id_t loc_id) {
    m_location_id = loc_id;
  }

  void GetDescription(Stream *s, lldb::DescriptionLevel level);

  static bool IsRangeIdentifier(llvm::StringRef str);
  static bool IsValidIDExpression(llvm::StringRef str);
  static llvm::ArrayRef<llvm::StringRef> GetRangeSpecifiers();

  /// Takes an input string containing the description of a breakpoint or
  /// breakpoint and location and returns a BreakpointID filled out with
  /// the proper id and location.
  ///
  /// \param[in] input
  ///     A string containing JUST the breakpoint description.
  /// \return
  ///     If \p input was not a valid breakpoint ID string, returns
  ///     \b std::nullopt.  Otherwise returns a BreakpointID with members filled
  ///     out accordingly.
  static std::optional<BreakpointID>
  ParseCanonicalReference(llvm::StringRef input);

  /// Takes an input string and checks to see whether it is a breakpoint name.
  /// If it is a mal-formed breakpoint name, error will be set to an appropriate
  /// error string.
  ///
  /// \param[in] str
  ///     A string containing JUST the breakpoint description.
  /// \param[out] error
  ///     If the name is a well-formed breakpoint name, set to success,
  ///     otherwise set to an error.
  /// \return
  ///     \b true if the name is a breakpoint name (as opposed to an ID or
  ///     range) false otherwise.
  static bool StringIsBreakpointName(llvm::StringRef str, Status &error);

  /// Takes a breakpoint ID and the breakpoint location id and returns
  /// a string containing the canonical description for the breakpoint
  /// or breakpoint location.
  ///
  /// \param[out] break_id
  ///     This is the break id.
  ///
  /// \param[out] break_loc_id
  ///     This is breakpoint location id, or LLDB_INVALID_BREAK_ID is no
  ///     location is to be specified.
  static void GetCanonicalReference(Stream *s, lldb::break_id_t break_id,
                                    lldb::break_id_t break_loc_id);

protected:
  lldb::break_id_t m_break_id;
  lldb::break_id_t m_location_id;
};

} // namespace lldb_private

#endif // LLDB_BREAKPOINT_BREAKPOINTID_H
