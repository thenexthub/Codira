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


set(dependencies)
list(APPEND dependencies Basic AST SIL)

add_language_compiler_module(Optimizer DEPENDS ${dependencies})

add_subdirectory(Analysis)
add_subdirectory(DataStructures)
add_subdirectory(InstructionSimplification)
add_subdirectory(PassManager)
add_subdirectory(ModulePasses)
add_subdirectory(FunctionPasses)
add_subdirectory(TestPasses)
add_subdirectory(Utilities)
