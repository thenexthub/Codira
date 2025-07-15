#===--- CMakeLists.txt - Observation support library ---------------------===#
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

list(APPEND language_runtime_library_compile_flags -I${LANGUAGE_SOURCE_DIR}/stdlib/include -I${LANGUAGE_SOURCE_DIR}/include)

add_language_target_library(languageObservation ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB
  Locking.code
  Observable.code
  ObservationRegistrar.code
  ObservationTracking.code
  Observations.code
  ThreadLocal.cpp
  ThreadLocal.code

  LANGUAGE_COMPILE_FLAGS
    ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
    "-enable-experimental-feature" "Macros"
    "-enable-experimental-feature" "ExtensionMacros"
    -Xfrontend -disable-implicit-string-processing-module-import

  C_COMPILE_FLAGS
    ${language_runtime_library_compile_flags}
  LINK_FLAGS "${LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS}"

  LANGUAGE_MODULE_DEPENDS _Concurrency
  INSTALL_IN_COMPONENT stdlib

  MACCATALYST_BUILD_FLAVOR "zippered"
)
