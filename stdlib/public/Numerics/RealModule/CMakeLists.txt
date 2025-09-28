#[[
This source file is part of the Codira Numerics open source project

Copyright (c) 2019 Apple Inc. and the Codira Numerics project authors
Licensed under Apache License v2.0 with Runtime Library Exception

See https://language.org/LICENSE.txt for license information
#]]

add_library(RealModule
  AlgebraicField.code
  ApproximateEquality.code
  AugmentedArithmetic.code
  Double+Real.code
  ElementaryFunctions.code
  Float+Real.code
  Float16+Real.code
  Float80+Real.code
  Real.code
  RealFunctions.code
  RelaxedArithmetic.code)
set_target_properties(RealModule PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(RealModule PUBLIC
  _NumericsShims)


_install_target(RealModule)
set_property(GLOBAL APPEND PROPERTY SWIFT_NUMERICS_EXPORTS RealModule)
