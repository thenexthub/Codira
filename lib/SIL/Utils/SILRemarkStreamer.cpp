//===--- SILRemarkStreamer.cpp --------------------------------------------===//
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

#include "language/SIL/SILRemarkStreamer.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/Basic/Assertions.h"
#include "llvm/IR/LLVMContext.h"

using namespace language;

SILRemarkStreamer::SILRemarkStreamer(
    std::unique_ptr<llvm::remarks::RemarkStreamer> &&streamer,
    std::unique_ptr<llvm::raw_fd_ostream> &&stream, const ASTContext &Ctx)
    : owner(Owner::SILModule), streamer(std::move(streamer)), context(nullptr),
      remarkStream(std::move(stream)), ctx(Ctx) { }

llvm::remarks::RemarkStreamer &SILRemarkStreamer::getLLVMStreamer() {
  switch (owner) {
  case Owner::SILModule:
    return *streamer.get();
  case Owner::LLVM:
    return *context->getMainRemarkStreamer();
  }
  return *streamer.get();
}

const llvm::remarks::RemarkStreamer &
SILRemarkStreamer::getLLVMStreamer() const {
  switch (owner) {
  case Owner::SILModule:
    return *streamer.get();
  case Owner::LLVM:
    return *context->getMainRemarkStreamer();
  }
  return *streamer.get();
}

void SILRemarkStreamer::intoLLVMContext(llvm::LLVMContext &Ctx) & {
  assert(owner == Owner::SILModule);
  Ctx.setMainRemarkStreamer(std::move(streamer));
  context = &Ctx;
  owner = Owner::LLVM;
}

std::unique_ptr<SILRemarkStreamer>
SILRemarkStreamer::create(SILModule &silModule) {
  StringRef filename = silModule.getOptions().OptRecordFile;
  const auto format = silModule.getOptions().OptRecordFormat;
  if (filename.empty())
    return nullptr;

  auto &diagEngine = silModule.getASTContext().Diags;
  std::error_code errorCode;
  auto file = std::make_unique<llvm::raw_fd_ostream>(filename, errorCode,
                                                     llvm::sys::fs::OF_None);
  if (errorCode) {
    diagEngine.diagnose(SourceLoc(), diag::cannot_open_file, filename,
                        errorCode.message());
    return nullptr;
  }

  llvm::Expected<std::unique_ptr<llvm::remarks::RemarkSerializer>>
      remarkSerializerOrErr = llvm::remarks::createRemarkSerializer(
          format, llvm::remarks::SerializerMode::Separate, *file);
  if (llvm::Error err = remarkSerializerOrErr.takeError()) {
    diagEngine.diagnose(SourceLoc(), diag::error_creating_remark_serializer,
                        toString(std::move(err)));
    return nullptr;
  }

  auto mainRS = std::make_unique<llvm::remarks::RemarkStreamer>(
      std::move(*remarkSerializerOrErr), filename);

  const auto passes = silModule.getOptions().OptRecordPasses;
  if (!passes.empty()) {
    if (llvm::Error err = mainRS->setFilter(passes)) {
      diagEngine.diagnose(SourceLoc(), diag::error_creating_remark_serializer,
                          toString(std::move(err)));
      return nullptr;
    }
  }

  // N.B. We should be able to use std::make_unique here, but I prefer correctly
  // encapsulating the constructor over elegance.
  // Besides, this isn't going to throw an exception.
  return std::unique_ptr<SILRemarkStreamer>(new SILRemarkStreamer(
      std::move(mainRS), std::move(file), silModule.getASTContext()));
}
