//===-- ModuleChild.h -------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_CORE_MODULECHILD_H
#define LLDB_CORE_MODULECHILD_H

#include "lldb/lldb-forward.h"

namespace lldb_private {

/// \class ModuleChild ModuleChild.h "lldb/Core/ModuleChild.h"
/// A mix in class that contains a pointer back to the module
///        that owns the object which inherits from it.
class ModuleChild {
public:
  /// Construct with owning module.
  ///
  /// \param[in] module_sp
  ///     The module that owns the object that inherits from this
  ///     class.
  ModuleChild(const lldb::ModuleSP &module_sp);

  /// Destructor.
  ~ModuleChild();

  /// Assignment operator.
  ///
  /// \param[in] rhs
  ///     A const ModuleChild class reference to copy.
  ///
  /// \return
  ///     A const reference to this object.
  const ModuleChild &operator=(const ModuleChild &rhs);

  /// Get const accessor for the module pointer.
  ///
  /// \return
  ///     A const pointer to the module that owns the object that
  ///     inherits from this class.
  lldb::ModuleSP GetModule() const;

  /// Set accessor for the module pointer.
  ///
  /// \param[in] module_sp
  ///     A new module that owns the object that inherits from this
  ///     class.
  void SetModule(const lldb::ModuleSP &module_sp);

protected:
  /// The Module that owns the object that inherits from this class.
  lldb::ModuleWP m_module_wp;
};

} // namespace lldb_private

#endif // LLDB_CORE_MODULECHILD_H
