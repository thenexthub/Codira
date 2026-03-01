# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#  Author: Tunjay Akbarli
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

include_directories(../third_party/flatbuffers/include)
set(FLATBUFFERS_BINARY_DIR ../third_party/flatbuffers/build)
link_directories(${FLATBUFFERS_BINARY_DIR})
# Setup testing
find_package(GTest REQUIRED)
find_package(Threads)
link_directories(${FLATBUFFERS_BINARY_DIR})
set(JSON_INCLUDE_DIR ../third_party/json-v3.11.3/include)
include_directories(${JSON_INCLUDE_DIR})
include_directories(../third_party/sqlite3/amalgamation)
# include head file of GTEST
include_directories(${GTEST_INCLUDE_DIR})
# include head file of common
include_directories(./include)
set(CODIRA_LIB_DIR ../src/lib/cangjie/lib/codenative)
if (COMPILER_TYPE MATCHES "MINGW")
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -lssp -Wl,--no-whole-archive")
endif ()

link_directories(${CODIRA_LIB_DIR})
set(CODIRA_INCLUDE_DIR ../src/lib/cangjie/include)
include_directories(${CODIRA_INCLUDE_DIR})

# include src dir
link_directories(${LIBRARY_OUTPUT_PATH})

# add test dir
add_subdirectory(common)
add_subdirectory(apiTest)
add_subdirectory(featureTest/completion)
add_subdirectory(featureTest/documentHighlight)
add_subdirectory(featureTest/goToDefinition)
add_subdirectory(featureTest/references)
add_subdirectory(featureTest/semanticHighlight)
add_subdirectory(featureTest/hover)
add_subdirectory(featureTest/diagnostic)
add_subdirectory(featureTest/codeCov)
add_subdirectory(featureTest/workSpaceSymbol)
add_subdirectory(featureTest/documentSymbol)
add_subdirectory(featureTest/signatureHelp)
add_subdirectory(featureTest/rename)
add_subdirectory(featureTest/typeHierarchy)
add_subdirectory(featureTest/breakpoints)
add_subdirectory(featureTest/callHierarchy)
add_subdirectory(featureTest/codeLens)
add_subdirectory(featureTest/crossLanguageDefinition)
add_subdirectory(featureTest/overrideMethods)
add_subdirectory(featureTest/findFileReference)

# Add test main file
add_executable(gtest_LSPServer_test main.cpp)

# Specify that gtest_LSPServer_test depends on LSPServerlib
# Ensure that LSPServerlib is compiled before gtest_LSPServer_test.
add_dependencies(gtest_LSPServer_test LSPServerlib)

target_link_libraries(gtest_LSPServer_test
    libLSPServerlib.a
    GTest::gtest
    GTest::gtest_main
    ${CMAKE_THREAD_LIBS_INIT}
    DocHighlightTest
    DocumentSymbolTest
    CompletionTest
    GoToDefinitionTest
    ReferenceTest
    FileReferenceTest
    RenameTest
    SemanticHighlightTest
    HoverTest
    DiagnosticTest
    CodeCovTest
    WorkspaceSymbolTest
    SignatureHelpTest
    TypeHierarchyTest
    BreakpointsTest
    CallHierarchyTest
    CodeLensTest
    CrossLanguageDefinitionTest
    OverrideMethodsTest
    Common
    libflatbuffers.a
    sqlite3
    ffi
    -lcangjie-lsp
    ${CMAKE_DL_LIBS}
    )
if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    target_link_libraries(gtest_LSPServer_test stdc++fs)
endif()
add_test(NAME gtest_simple_test COMMAND gtest_LSPServer_test)