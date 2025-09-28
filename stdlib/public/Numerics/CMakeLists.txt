#[[
This source file is part of the Codira Numerics open source project

Copyright (c) 2019-2021 Apple Inc. and the Codira Numerics project authors
Licensed under Apache License v2.0 with Runtime Library Exception

See https://language.org/LICENSE.txt for license information
#]]

add_subdirectory(_NumericsShims)
add_subdirectory(ComplexModule)
add_subdirectory(IntegerUtilities)
add_subdirectory(Numerics)
add_subdirectory(RealModule)
if(BUILD_TESTING)
  add_subdirectory(_TestSupport)
endif()
