#[[
This source file is part of the Codira Numerics open source project

Copyright (c) 2019 Apple Inc. and the Codira Numerics project authors
Licensed under Apache License v2.0 with Runtime Library Exception

See https://language.org/LICENSE.txt for license information
#]]

add_library(ComplexModule
  Complex.code
  Complex+AdditiveArithmetic.code
  Complex+AlgebraicField.code
  Complex+Codable.code
  Complex+ElementaryFunctions.code
  Complex+Hashable.code
  Complex+IntegerLiteral.code
  Complex+Numeric.code
  Complex+StringConvertible.code
  Polar.code
  Scale.code)
set_target_properties(ComplexModule PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(ComplexModule PUBLIC
  RealModule)


_install_target(ComplexModule)
set_property(GLOBAL APPEND PROPERTY SWIFT_NUMERICS_EXPORTS ComplexModule)
