#[[
This source file is part of the Codira Numerics open source project

Copyright (c) 2019-2021 Apple Inc. and the Codira Numerics project authors
Licensed under Apache License v2.0 with Runtime Library Exception

See https://language.org/LICENSE.txt for license information
#]]

add_library(_TestSupport
  BlackHole.code
  DoubleWidth.code
  Error.code
  Interval.code
  RealTestSupport.code)
set_target_properties(_TestSupport PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(_TestSupport PUBLIC
  Numerics)


_install_target(_TestSupport)
set_property(GLOBAL APPEND PROPERTY SWIFT_NUMERICS_EXPORTS _TestSupport)
