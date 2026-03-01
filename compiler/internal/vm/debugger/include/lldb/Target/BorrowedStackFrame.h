//===----------------------------------------------------------------------===//
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

#ifndef LLDB_TARGET_BORROWEDSTACKFRAME_H
#define LLDB_TARGET_BORROWEDSTACKFRAME_H

#include "lldb/Target/StackFrame.h"

namespace lldb_private {

/// \class BorrowedStackFrame BorrowedStackFrame.h
/// "lldb/Target/BorrowedStackFrame.h"
///
/// A wrapper around an existing StackFrame that supersedes its frame indices.
///
/// This class is useful when you need to present an existing stack frame
/// with a different index, such as when creating synthetic frame views or
/// renumbering frames without copying all the underlying data.
///
/// All methods delegate to the borrowed frame except for GetFrameIndex()
/// & GetConcreteFrameIndex() which uses the overridden indices.
class BorrowedStackFrame : public StackFrame {
public:
  /// Construct a BorrowedStackFrame that wraps an existing frame.
  ///
  /// \param [in] borrowed_frame_sp
  ///   The existing StackFrame to borrow from. This frame's data will be
  ///   used for all operations except frame index queries.
  ///
  /// \param [in] new_frame_index
  ///   The frame index to report instead of the borrowed frame's index.
  ///
  /// \param [in] new_concrete_frame_index
  ///   Optional concrete frame index. If not provided, defaults to
  ///   new_frame_index.
  BorrowedStackFrame(
      lldb::StackFrameSP borrowed_frame_sp, uint32_t new_frame_index,
      std::optional<uint32_t> new_concrete_frame_index = std::nullopt);

  ~BorrowedStackFrame() override = default;

  uint32_t GetFrameIndex() const override;
  void SetFrameIndex(uint32_t index);

  /// Get the concrete frame index for this borrowed frame.
  ///
  /// Returns the overridden concrete frame index provided at construction,
  /// or LLDB_INVALID_FRAME_ID if the borrowed frame represents an inlined
  /// function, since this would require some computation if we chain inlined
  /// borrowed stack frames.
  ///
  /// \return
  ///   The concrete frame index, or LLDB_INVALID_FRAME_ID for inline frames.
  uint32_t GetConcreteFrameIndex() override;

  StackID &GetStackID() override;

  const Address &GetFrameCodeAddress() override;

  Address GetFrameCodeAddressForSymbolication() override;

  bool ChangePC(lldb::addr_t pc) override;

  const SymbolContext &
  GetSymbolContext(lldb::SymbolContextItem resolve_scope) override;

  llvm::Error GetFrameBaseValue(Scalar &value) override;

  DWARFExpressionList *GetFrameBaseExpression(Status *error_ptr) override;

  Block *GetFrameBlock() override;

  lldb::RegisterContextSP GetRegisterContext() override;

  VariableList *GetVariableList(bool get_file_globals,
                                Status *error_ptr) override;

  lldb::VariableListSP
  GetInScopeVariableList(bool get_file_globals,
                         bool must_have_valid_location = false) override;

  lldb::ValueObjectSP GetValueForVariableExpressionPath(
      llvm::StringRef var_expr, lldb::DynamicValueType use_dynamic,
      uint32_t options, lldb::VariableSP &var_sp, Status &error) override;

  bool HasDebugInformation() override;

  const char *Disassemble() override;

  lldb::ValueObjectSP
  GetValueObjectForFrameVariable(const lldb::VariableSP &variable_sp,
                                 lldb::DynamicValueType use_dynamic) override;

  bool IsInlined() override;

  bool IsSynthetic() const override;

  bool IsHistorical() const override;

  bool IsArtificial() const override;

  bool IsHidden() override;

  const char *GetFunctionName() override;

  const char *GetDisplayFunctionName() override;

  lldb::ValueObjectSP FindVariable(ConstString name) override;

  SourceLanguage GetLanguage() override;

  SourceLanguage GuessLanguage() override;

  lldb::ValueObjectSP GuessValueForAddress(lldb::addr_t addr) override;

  lldb::ValueObjectSP GuessValueForRegisterAndOffset(ConstString reg,
                                                     int64_t offset) override;

  StructuredData::ObjectSP GetLanguageSpecificData() override;

  lldb::RecognizedStackFrameSP GetRecognizedFrame() override;

  /// Get the underlying borrowed frame.
  lldb::StackFrameSP GetBorrowedFrame() const;

  bool isA(const void *ClassID) const override;
  static bool classof(const StackFrame *obj);

private:
  lldb::StackFrameSP m_borrowed_frame_sp;
  uint32_t m_new_frame_index;
  uint32_t m_new_concrete_frame_index;
  static char ID;

  BorrowedStackFrame(const BorrowedStackFrame &) = delete;
  const BorrowedStackFrame &operator=(const BorrowedStackFrame &) = delete;
};

} // namespace lldb_private

#endif // LLDB_TARGET_BORROWEDSTACKFRAME_H
