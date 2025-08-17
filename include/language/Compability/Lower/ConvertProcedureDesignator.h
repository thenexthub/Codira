//===- ConvertProcedureDesignator.h -- Procedure Designators ----*- C++ -*-===//
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
///
/// Lowering of evaluate::ProcedureDesignator to FIR and HLFIR.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_LOWER_CONVERT_PROCEDURE_DESIGNATOR_H
#define LANGUAGE_COMPABILITY_LOWER_CONVERT_PROCEDURE_DESIGNATOR_H

namespace mlir {
class Location;
class Value;
class Type;
}
namespace fir {
class ExtendedValue;
}
namespace hlfir {
class EntityWithAttributes;
}
namespace language::Compability::evaluate {
struct ProcedureDesignator;
}
namespace language::Compability::semantics {
class Symbol;
}

namespace language::Compability::lower {
class AbstractConverter;
class StatementContext;
class SymMap;

/// Lower a procedure designator to a fir::ExtendedValue that can be a
/// fir::CharBoxValue for character procedure designator (the CharBoxValue
/// length carries the result length if it is known).
fir::ExtendedValue convertProcedureDesignator(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::evaluate::ProcedureDesignator &proc,
    language::Compability::lower::SymMap &symMap, language::Compability::lower::StatementContext &stmtCtx);

/// Lower a procedure designator to a !fir.boxproc<()->() or
/// tuple<!fir.boxproc<()->(), len>.
hlfir::EntityWithAttributes convertProcedureDesignatorToHLFIR(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::evaluate::ProcedureDesignator &proc,
    language::Compability::lower::SymMap &symMap, language::Compability::lower::StatementContext &stmtCtx);

/// Generate initialization for procedure pointer to procedure target.
mlir::Value
convertProcedureDesignatorInitialTarget(language::Compability::lower::AbstractConverter &,
                                        mlir::Location,
                                        const language::Compability::semantics::Symbol &sym);

/// Given the value of a "PASS" actual argument \p passedArg and the
/// evaluate::ProcedureDesignator for the call, address and dereference
/// the argument's procedure pointer component that must be called.
mlir::Value derefPassProcPointerComponent(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::evaluate::ProcedureDesignator &proc, mlir::Value passedArg,
    language::Compability::lower::SymMap &symMap, language::Compability::lower::StatementContext &stmtCtx);
} // namespace language::Compability::lower
#endif // FORTRAN_LOWER_CONVERT_PROCEDURE_DESIGNATOR_H
