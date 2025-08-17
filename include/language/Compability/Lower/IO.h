//===-- Lower/IO.h -- lower IO statements -----------------------*- C++ -*-===//
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
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_LOWER_IO_H
#define LANGUAGE_COMPABILITY_LOWER_IO_H

namespace mlir {
class Value;
} // namespace mlir

namespace language::Compability {
namespace parser {
struct BackspaceStmt;
struct CloseStmt;
struct EndfileStmt;
struct FlushStmt;
struct InquireStmt;
struct OpenStmt;
struct ReadStmt;
struct RewindStmt;
struct PrintStmt;
struct WaitStmt;
struct WriteStmt;
} // namespace parser

namespace lower {

class AbstractConverter;

/// Generate IO call(s) for BACKSPACE; return the IOSTAT code
mlir::Value genBackspaceStatement(AbstractConverter &,
                                  const parser::BackspaceStmt &);

/// Generate IO call(s) for CLOSE; return the IOSTAT code
mlir::Value genCloseStatement(AbstractConverter &, const parser::CloseStmt &);

/// Generate IO call(s) for ENDFILE; return the IOSTAT code
mlir::Value genEndfileStatement(AbstractConverter &,
                                const parser::EndfileStmt &);

/// Generate IO call(s) for FLUSH; return the IOSTAT code
mlir::Value genFlushStatement(AbstractConverter &, const parser::FlushStmt &);

/// Generate IO call(s) for INQUIRE; return the IOSTAT code
mlir::Value genInquireStatement(AbstractConverter &,
                                const parser::InquireStmt &);

/// Generate IO call(s) for READ; return the IOSTAT code
mlir::Value genReadStatement(AbstractConverter &converter,
                             const parser::ReadStmt &stmt);

/// Generate IO call(s) for OPEN; return the IOSTAT code
mlir::Value genOpenStatement(AbstractConverter &, const parser::OpenStmt &);

/// Generate IO call(s) for PRINT
void genPrintStatement(AbstractConverter &converter,
                       const parser::PrintStmt &stmt);

/// Generate IO call(s) for REWIND; return the IOSTAT code
mlir::Value genRewindStatement(AbstractConverter &, const parser::RewindStmt &);

/// Generate IO call(s) for WAIT; return the IOSTAT code
mlir::Value genWaitStatement(AbstractConverter &, const parser::WaitStmt &);

/// Generate IO call(s) for WRITE; return the IOSTAT code
mlir::Value genWriteStatement(AbstractConverter &converter,
                              const parser::WriteStmt &stmt);

} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_IO_H
