#[[
This source file is part of the Codira Numerics open source project

Copyright (c) 2019-2021 Apple Inc. and the Codira Numerics project authors
Licensed under Apache License v2.0 with Runtime Library Exception

See https://language.org/LICENSE.txt for license information
#]]

add_library(IntegerUtilities
  DivideWithRounding.code
  GCD.code
  Rotate.code
  RoundingRule.code
  SaturatingArithmetic.code
  ShiftWithRounding.code)
set_target_properties(IntegerUtilities PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})

_install_target(IntegerUtilities)
set_property(GLOBAL APPEND PROPERTY SWIFT_NUMERICS_EXPORTS IntegerUtilities)
