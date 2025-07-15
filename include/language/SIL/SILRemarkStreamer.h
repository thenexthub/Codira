//===--- SILRemarkStreamer.h - Interface for streaming remarks --*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
/// \file
/// This file defines the interface used to stream SIL optimization diagnostics
/// through LLVM's RemarkStreamer interface.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_SILREMARKSTREAMER_H
#define LANGUAGE_SIL_SILREMARKSTREAMER_H

#include "language/Basic/SourceManager.h"
#include "language/SIL/OptimizationRemark.h"
#include "toolchain/Remarks/Remark.h"
#include "toolchain/Remarks/RemarkStreamer.h"

namespace language {

class SILRemarkStreamer {
private:
  enum class Owner {
    SILModule,
    LLVM,
  } owner;

  /// The underlying LLVM streamer.
  ///
  /// If owned by a SILModule, this will be non-null.
  std::unique_ptr<toolchain::remarks::RemarkStreamer> streamer;
  /// The owning LLVM context.
  ///
  /// If owned by LLVM, this will be non-null.
  toolchain::LLVMContext *context;

  /// The remark output stream used to record SIL remarks to a file.
  std::unique_ptr<toolchain::raw_fd_ostream> remarkStream;

  // Source manager for resolving source locations.
  const ASTContext &ctx;
  /// Convert diagnostics into LLVM remark objects.
  /// The lifetime of the members of the result is bound to the lifetime of
  /// the SIL remarks.
  template <typename RemarkT>
  toolchain::remarks::Remark
  toLLVMRemark(const OptRemark::Remark<RemarkT> &remark) const;

  SILRemarkStreamer(std::unique_ptr<toolchain::remarks::RemarkStreamer> &&streamer,
                    std::unique_ptr<toolchain::raw_fd_ostream> &&stream,
                    const ASTContext &Ctx);

public:
  static std::unique_ptr<SILRemarkStreamer> create(SILModule &silModule);

public:
  toolchain::remarks::RemarkStreamer &getLLVMStreamer();
  const toolchain::remarks::RemarkStreamer &getLLVMStreamer() const;

  const ASTContext &getASTContext() const { return ctx; }

public:
  /// Perform a one-time ownership transfer to associate the underlying
  /// \c toolchain::remarks::RemarkStreamer with the given \c LLVMContext.
  void intoLLVMContext(toolchain::LLVMContext &Ctx) &;

  std::unique_ptr<toolchain::raw_fd_ostream> releaseStream() {
    return std::move(remarkStream);
  }

public:
  /// Emit a remark through the streamer.
  template <typename RemarkT>
  void emit(const OptRemark::Remark<RemarkT> &remark);
};

// Implementation for template member functions.

// OptRemark type -> toolchain::remarks::Type
template <typename RemarkT> static toolchain::remarks::Type toRemarkType() {
  if (std::is_same<RemarkT, OptRemark::RemarkPassed>::value)
    return toolchain::remarks::Type::Passed;
  if (std::is_same<RemarkT, OptRemark::RemarkMissed>::value)
    return toolchain::remarks::Type::Missed;
  toolchain_unreachable("Unknown remark type");
}

static inline std::optional<toolchain::remarks::RemarkLocation>
toRemarkLocation(const SourceLoc &loc, const SourceManager &srcMgr) {
  if (!loc.isValid())
    return std::nullopt;

  StringRef file = srcMgr.getDisplayNameForLoc(loc);
  unsigned line, col;
  std::tie(line, col) = srcMgr.getPresumedLineAndColumnForLoc(loc);
  return toolchain::remarks::RemarkLocation{file, line, col};
}

template <typename RemarkT>
toolchain::remarks::Remark SILRemarkStreamer::toLLVMRemark(
    const OptRemark::Remark<RemarkT> &optRemark) const {
  toolchain::remarks::Remark toolchainRemark; // The result.
  toolchainRemark.RemarkType = toRemarkType<RemarkT>();
  toolchainRemark.PassName = optRemark.getPassName();
  toolchainRemark.RemarkName = optRemark.getIdentifier();
  toolchainRemark.FunctionName = optRemark.getFunction()->getName();
  toolchainRemark.Loc =
      toRemarkLocation(optRemark.getLocation(), getASTContext().SourceMgr);

  for (const OptRemark::Argument &arg : optRemark.getArgs()) {
    toolchainRemark.Args.emplace_back();
    toolchainRemark.Args.back().Key = arg.key.data;
    toolchainRemark.Args.back().Val = arg.val;
    toolchainRemark.Args.back().Loc =
        toRemarkLocation(arg.loc, getASTContext().SourceMgr);
  }

  return toolchainRemark;
}

template <typename RemarkT>
void SILRemarkStreamer::emit(const OptRemark::Remark<RemarkT> &optRemark) {
  if (!getLLVMStreamer().matchesFilter(optRemark.getPassName()))
    return;

  return getLLVMStreamer().getSerializer().emit(toLLVMRemark(optRemark));
}

} // namespace language
#endif
