//===----- lib/Support/Error.cpp - Error and associated utilities ---------===//
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

#include "vm/core/Support/Error.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/Support/ErrorHandling.h"
#include <system_error>

using namespace vm::core;

namespace {

enum class ErrorErrorCode : int {
  MultipleErrors = 1,
  FileError,
  InconvertibleError
};

// FIXME: This class is only here to support the transition to toolchain::Error. It
// will be removed once this transition is complete. Clients should prefer to
// deal with the Error value directly, rather than converting to error_code.
class ErrorErrorCategory : public std::error_category {
public:
  const char *name() const noexcept override { return "Error"; }

  std::string message(int condition) const override {
    switch (static_cast<ErrorErrorCode>(condition)) {
    case ErrorErrorCode::MultipleErrors:
      return "Multiple errors";
    case ErrorErrorCode::InconvertibleError:
      return "Inconvertible error value. An error has occurred that could not "
             "be converted to a known std::error_code. Please file a bug.";
    case ErrorErrorCode::FileError:
      return "A file error occurred.";
    }
    llvm_unreachable("Unhandled error code");
  }
};
}

static ErrorErrorCategory &getErrorErrorCat() {
  static ErrorErrorCategory ErrorErrorCat;
  return ErrorErrorCat;
}

void ErrorInfoBase::anchor() {}
char ErrorInfoBase::ID = 0;
char ErrorList::ID = 0;
void ECError::anchor() {}
char ECError::ID = 0;
char StringError::ID = 0;
char FileError::ID = 0;

void toolchain::logAllUnhandledErrors(Error E, raw_ostream &OS, Twine ErrorBanner) {
  if (!E)
    return;
  OS << ErrorBanner;
  handleAllErrors(std::move(E), [&](const ErrorInfoBase &EI) {
    EI.log(OS);
    OS << "\n";
  });
}

/// Write all error messages (if any) in E to a string. The newline character
/// is used to separate error messages.
std::string toolchain::toString(Error E) {
  SmallVector<std::string, 2> Errors;
  handleAllErrors(std::move(E), [&Errors](const ErrorInfoBase &EI) {
    Errors.push_back(EI.message());
  });
  return join(Errors.begin(), Errors.end(), "\n");
}

std::string toolchain::toStringWithoutConsuming(const Error &E) {
  SmallVector<std::string, 2> Errors;
  visitErrors(E, [&Errors](const ErrorInfoBase &EI) {
    Errors.push_back(EI.message());
  });
  return join(Errors.begin(), Errors.end(), "\n");
}

std::error_code ErrorList::convertToErrorCode() const {
  return std::error_code(static_cast<int>(ErrorErrorCode::MultipleErrors),
                         getErrorErrorCat());
}

std::error_code toolchain::inconvertibleErrorCode() {
  return std::error_code(static_cast<int>(ErrorErrorCode::InconvertibleError),
                         getErrorErrorCat());
}

std::error_code FileError::convertToErrorCode() const {
  std::error_code NestedEC = Err->convertToErrorCode();
  if (NestedEC == inconvertibleErrorCode())
    return std::error_code(static_cast<int>(ErrorErrorCode::FileError),
                           getErrorErrorCat());
  return NestedEC;
}

Error toolchain::errorCodeToError(std::error_code EC) {
  if (!EC)
    return Error::success();
  return Error(std::make_unique<ECError>(ECError(EC)));
}

std::error_code toolchain::errorToErrorCode(Error Err) {
  std::error_code EC;
  handleAllErrors(std::move(Err), [&](const ErrorInfoBase &EI) {
    EC = EI.convertToErrorCode();
  });
  if (EC == inconvertibleErrorCode())
    report_fatal_error(Twine(EC.message()));
  return EC;
}

#if LLVM_ENABLE_ABI_BREAKING_CHECKS
void Error::fatalUncheckedError() const {
  dbgs() << "Program aborted due to an unhandled Error:\n";
  if (getPtr()) {
    getPtr()->log(dbgs());
    dbgs() << "\n";
  }else
    dbgs() << "Error value was Success. (Note: Success values must still be "
              "checked prior to being destroyed).\n";
  abort();
}
#endif

StringError::StringError(std::error_code EC, const Twine &S)
    : Msg(S.str()), EC(EC) {}

StringError::StringError(const Twine &S, std::error_code EC)
    : Msg(S.str()), EC(EC), PrintMsgOnly(true) {}

StringError::StringError(std::string &&S, std::error_code EC, bool PrintMsgOnly)
    : Msg(std::move(S)), EC(EC), PrintMsgOnly(PrintMsgOnly) {}

void StringError::log(raw_ostream &OS) const {
  if (PrintMsgOnly) {
    OS << Msg;
  } else {
    OS << EC.message();
    if (!Msg.empty())
      OS << (" " + Msg);
  }
}

std::error_code StringError::convertToErrorCode() const {
  return EC;
}

Error toolchain::createStringError(std::string &&Msg, std::error_code EC) {
  return make_error<StringError>(std::move(Msg), EC, true);
}

void toolchain::report_fatal_error(Error Err, bool GenCrashDiag) {
  assert(Err && "report_fatal_error called with success value");
  std::string ErrMsg;
  {
    raw_string_ostream ErrStream(ErrMsg);
    logAllUnhandledErrors(std::move(Err), ErrStream);
  }
  report_fatal_error(Twine(ErrMsg), GenCrashDiag);
}

void toolchain::reportFatalInternalError(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/true);
}

void toolchain::reportFatalUsageError(Error Err) {
  report_fatal_error(std::move(Err), /*GenCrashDiag=*/false);
}

LLVMErrorTypeId LLVMGetErrorTypeId(LLVMErrorRef Err) {
  return reinterpret_cast<ErrorInfoBase *>(Err)->dynamicClassID();
}

void LLVMConsumeError(LLVMErrorRef Err) { consumeError(unwrap(Err)); }

void LLVMCantFail(LLVMErrorRef Err) {
  cantFail(unwrap(Err));
}

char *LLVMGetErrorMessage(LLVMErrorRef Err) {
  std::string Tmp = toString(unwrap(Err));
  char *ErrMsg = new char[Tmp.size() + 1];
  memcpy(ErrMsg, Tmp.data(), Tmp.size());
  ErrMsg[Tmp.size()] = '\0';
  return ErrMsg;
}

void LLVMDisposeErrorMessage(char *ErrMsg) { delete[] ErrMsg; }

LLVMErrorTypeId LLVMGetStringErrorTypeId() {
  return reinterpret_cast<void *>(&StringError::ID);
}

LLVMErrorRef LLVMCreateStringError(const char *ErrMsg) {
  return wrap(make_error<StringError>(ErrMsg, inconvertibleErrorCode()));
}
