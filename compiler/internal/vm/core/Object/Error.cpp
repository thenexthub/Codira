//===- Error.cpp - system_error extensions for Object -----------*- C++ -*-===//
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
// This defines a new error_category for the Object library.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Object/Error.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;
using namespace object;

namespace {
// FIXME: This class is only here to support the transition to toolchain::Error. It
// will be removed once this transition is complete. Clients should prefer to
// deal with the Error value directly, rather than converting to error_code.
class _object_error_category : public std::error_category {
public:
  const char* name() const noexcept override;
  std::string message(int ev) const override;
};
}

const char *_object_error_category::name() const noexcept {
  return "toolchain.object";
}

std::string _object_error_category::message(int EV) const {
  object_error E = static_cast<object_error>(EV);
  switch (E) {
  case object_error::arch_not_found:
    return "No object file for requested architecture";
  case object_error::invalid_file_type:
    return "The file was not recognized as a valid object file";
  case object_error::parse_failed:
    return "Invalid data was encountered while parsing the file";
  case object_error::unexpected_eof:
    return "The end of the file was unexpectedly encountered";
  case object_error::string_table_non_null_end:
    return "String table must end with a null terminator";
  case object_error::invalid_section_index:
    return "Invalid section index";
  case object_error::bitcode_section_not_found:
    return "Bitcode section not found in object file";
  case object_error::invalid_symbol_index:
    return "Invalid symbol index";
  case object_error::section_stripped:
    return "Section has been stripped from the object file";
  }
  llvm_unreachable("An enumerator of object_error does not have a message "
                   "defined.");
}

void BinaryError::anchor() {}
char BinaryError::ID = 0;
char GenericBinaryError::ID = 0;

GenericBinaryError::GenericBinaryError(const Twine &Msg) : Msg(Msg.str()) {}

GenericBinaryError::GenericBinaryError(const Twine &Msg,
                                       object_error ECOverride)
    : Msg(Msg.str()) {
  setErrorCode(make_error_code(ECOverride));
}

void GenericBinaryError::log(raw_ostream &OS) const {
  OS << Msg;
}

const std::error_category &object::object_category() {
  static _object_error_category error_category;
  return error_category;
}

toolchain::Error toolchain::object::isNotObjectErrorInvalidFileType(toolchain::Error Err) {
  return handleErrors(std::move(Err), [](std::unique_ptr<ECError> M) -> Error {
    // Try to handle 'M'. If successful, return a success value from
    // the handler.
    if (M->convertToErrorCode() == object_error::invalid_file_type)
      return Error::success();

    // We failed to handle 'M' - return it from the handler.
    // This value will be passed back from catchErrors and
    // wind up in Err2, where it will be returned from this function.
    return Error(std::move(M));
  });
}
