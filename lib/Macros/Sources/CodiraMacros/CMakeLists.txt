#===--- CMakeLists.txt - Codira macros library ----------------------===#
#
# Copyright (c) NeXTHub Corporation. All rights reserved.
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# This code is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# version 2 for more details (a copy is included in the LICENSE file that
# accompanied this code).
#
# Author(-s): Tunjay Akbarli

#
#===----------------------------------------------------------------------===#

add_language_macro_library(CodiraMacros
  OptionSetMacro.code
  DebugDescriptionMacro.code
  DistributedResolvableMacro.code
  SyntaxExtensions.code
  TaskLocalMacro.code
  CodiraifyImportMacro.code
  LANGUAGE_DEPENDENCIES
    CodiraDiagnostics
    CodiraSyntax
    CodiraSyntaxBuilder
    CodiraSyntaxMacros
)
