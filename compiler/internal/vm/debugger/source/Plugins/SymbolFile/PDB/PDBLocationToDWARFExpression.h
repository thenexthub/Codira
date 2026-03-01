//===-- PDBLocationToDWARFExpression.h --------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_PDB_PDBLOCATIONTODWARFEXPRESSION_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_PDB_PDBLOCATIONTODWARFEXPRESSION_H

#include "lldb/Core/Module.h"
#include "lldb/Symbol/Variable.h"

namespace lldb_private {
class DWARFExpression;
}

namespace llvm {
namespace pdb {
class PDBSymbolData;
}
} // namespace llvm

/// Converts a location information from a PDB symbol to a DWARF expression
///
/// \param[in] module
///     The module \a symbol belongs to.
///
/// \param[in] symbol
///     The symbol with a location information to convert.
///
/// \param[in] ranges
///     Ranges where this variable is valid.
///
/// \param[out] is_constant
///     Set to \b true if the result expression is a constant value data,
///     and \b false if it is a DWARF bytecode.
///
/// \return
///     The DWARF expression corresponding to the location data of \a symbol.
lldb_private::DWARFExpression
ConvertPDBLocationToDWARFExpression(lldb::ModuleSP module,
                                    const llvm::pdb::PDBSymbolData &symbol,
                                    const lldb_private::Variable::RangeList &ranges,
                                    bool &is_constant);
#endif
