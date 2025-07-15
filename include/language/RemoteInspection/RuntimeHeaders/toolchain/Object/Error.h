//===- Error.h - system_error extensions for Object -------------*- C++ -*-===//
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
// This declares a new error_category for the Object library.
//
//===----------------------------------------------------------------------===//

#ifndef TOOLCHAIN_OBJECT_ERROR_H
#define TOOLCHAIN_OBJECT_ERROR_H

#include "toolchain/Support/Error.h"
#include <system_error>

namespace toolchain {

class Twine;

namespace object {

const std::error_category &object_category();

enum class object_error {
  // Error code 0 is absent. Use std::error_code() instead.
  arch_not_found = 1,
  invalid_file_type,
  parse_failed,
  unexpected_eof,
  string_table_non_null_end,
  invalid_section_index,
  bitcode_section_not_found,
  invalid_symbol_index,
  section_stripped,
};

inline std::error_code make_error_code(object_error e) {
  return std::error_code(static_cast<int>(e), object_category());
}

/// Base class for all errors indicating malformed binary files.
///
/// Having a subclass for all malformed binary files allows archive-walking
/// code to skip malformed files without having to understand every possible
/// way that a binary file might be malformed.
///
/// Currently inherits from ECError for easy interoperability with
/// std::error_code, but this will be removed in the future.
class BinaryError : public ErrorInfo<BinaryError, ECError> {
  void anchor() override;
public:
  static char ID;
  BinaryError() {
    // Default to parse_failed, can be overridden with setErrorCode.
    setErrorCode(make_error_code(object_error::parse_failed));
  }
};

/// Generic binary error.
///
/// For errors that don't require their own specific sub-error (most errors)
/// this class can be used to describe the error via a string message.
class GenericBinaryError : public ErrorInfo<GenericBinaryError, BinaryError> {
public:
  static char ID;
  GenericBinaryError(const Twine &Msg);
  GenericBinaryError(const Twine &Msg, object_error ECOverride);
  const std::string &getMessage() const { return Msg; }
  void log(raw_ostream &OS) const override;
private:
  std::string Msg;
};

/// isNotObjectErrorInvalidFileType() is used when looping through the children
/// of an archive after calling getAsBinary() on the child and it returns an
/// toolchain::Error.  In the cases we want to loop through the children and ignore the
/// non-objects in the archive this is used to test the error to see if an
/// error() function needs to called on the toolchain::Error.
Error isNotObjectErrorInvalidFileType(toolchain::Error Err);

inline Error createError(const Twine &Err) {
  return make_error<StringError>(Err, object_error::parse_failed);
}

} // end namespace object.

} // end namespace toolchain.

namespace std {
template <>
struct is_error_code_enum<toolchain::object::object_error> : std::true_type {};
}

#endif
