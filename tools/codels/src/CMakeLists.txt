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

find_package(Threads REQUIRED)
add_compile_definitions(NOT_RELEASE_VERSION_CODE)
link_directories(${LIBRARY_OUTPUT_PATH})
# declare the path of flatbuffers file
include_directories(../third_party/flatbuffers/include)
link_directories(../third_party/flatbuffers/build)
# declare the path of sqlite3 file
include_directories(../third_party/sqlite3/amalgamation)
add_library(sqlite3 STATIC
        ../third_party/sqlite3/amalgamation/shell.c
        ../third_party/sqlite3/amalgamation/sqlite3.c
        ../third_party/sqlite3/amalgamation/sqlite3.h
        ../third_party/sqlite3/amalgamation/sqlite3ext.h
)
target_link_libraries(sqlite3 PRIVATE Threads::Threads)
add_compile_definitions(
        SQLITE_DEFAULT_MEMSTATUS=1
        SQLITE_DEFAULT_WAL_SYNCHRONOUS=1
        SQLITE_DEFAULT_WORKER_THREADS=8
        SQLITE_DIRECT_OVERFLOW_READ
        SQLITE_DISABLE_PAGECACHE_OVERFLOW_STATS
        SQLITE_DQS=0
        SQLITE_ENABLE_FTS5
        SQLITE_ENABLE_MEMORY_MANAGEMENT
        SQLITE_ENABLE_NULL_TRIM
        SQLITE_LIKE_DOESNT_MATCH_BLOBS
        SQLITE_MAX_EXPR_DEPTH=0
        SQLITE_MAX_MMAP_SIZE=17179869184LL
        SQLITE_OMIT_DECLTYPE
        SQLITE_OMIT_DEPRECATED
        SQLITE_OMIT_SHARED_CACHE
        SQLITE_THREADSAFE=1
        SQLITE_USE_ALLOCA
)
target_compile_options(sqlite3 PRIVATE -fPIC)
# declare the path of JSON header file
include_directories(../third_party/json-v3.11.3/include)
# declare the path of Codira header file
set(CODIRA_INCLUDE_DIR lib/cangjie/include)
set(CODIRA_LIB_DIR lib/cangjie/lib/codenative)
if (COMPILER_TYPE MATCHES "MINGW")    #Compiling Using the MinGW
    # declare the link library path of Codira
    message("codenative windows")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")
    if (CMAKE_BUILD_TYPE MATCHES "Release")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2 -O2")
    endif ()

    # add Codira header file and library
    include_directories(${CODIRA_INCLUDE_DIR})
    link_directories(${CODIRA_LIB_DIR})

    add_subdirectory(languageserver)
    add_subdirectory(json-rpc)

    # Add a command for the linker to statically link libgcc and libstdc++.
    # If the command is not added, the compiled exe file cannot run on other computers.
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -lssp -Wl,--no-whole-archive -Wl,--no-insert-timestamp")
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-insert-timestamp")

    add_executable(LSPServer launcher/main.cpp)
    # link library
    target_link_libraries(LSPServer
            pthread
            JsonRPC
            LSP
            ${CMAKE_DL_LIBS}
            libflatbuffers.a
            sqlite3
            -ldbghelp
            -limagehlp
            )
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
        target_link_libraries(LSPServer stdc++fs)
    endif ()
    target_link_libraries(LSPServer -lcangjie-lsp)
    add_library(cangjie-lsp SHARED IMPORTED)
    set_target_properties(cangjie-lsp PROPERTIES
            IMPORTED_LOCATION ${CODIRA_LIB_DIR}/libcangjie-lsp.dll
            )
    set_target_properties(cangjie-lsp PROPERTIES
            IMPORTED_IMPLIB ${CODIRA_LIB_DIR}/libcangjie-lsp.dll.a
            )
    add_library(LSPServerlib launcher/main.cpp)
    target_link_libraries(LSPServerlib
            pthread
            JsonRPC
            LSP
            ${CMAKE_DL_LIBS}
            )
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
        target_link_libraries(LSPServerlib stdc++fs)
    endif ()
elseif (COMPILER_TYPE MATCHES "Ninja")    #Compiling Using the ninja
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")
    if (CMAKE_BUILD_TYPE MATCHES "Release")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2 -O2")
    endif ()
    # declare the link library path of Codira
    if (APPLE)
        message("APPLE  +++++++++")
    endif()
    if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE MATCHES "Release")
        message(STATUS "release build type selected, set safe compile flag")
        if (APPLE)
            message("APPLE CMAKE_CXX_FLAGS ++++++")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftrapv -fpie")
            add_compile_options("-ffile-prefix-map=${CMAKE_SOURCE_DIR}=.")
            add_compile_options("-fdebug-compilation-dir=.")
            add_link_options("-Wl,-no_uuid")
            add_link_options("-Wl,-reproducible")
            add_link_options("-Wl,-no_order_inits")
        elseif(CROSS_WINDOWS)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -Wdate-time")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftrapv")
        endif ()
    endif ()
    if (NOT APPLE)
        set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
    endif ()

    # add Codira header file and library
    include_directories(${CODIRA_INCLUDE_DIR})
    link_directories(${CODIRA_LIB_DIR})

    add_subdirectory(languageserver)
    add_subdirectory(json-rpc)
    add_executable(LSPServer launcher/main.cpp)
    # link library
    target_link_libraries(LSPServer
            pthread
            JsonRPC
            LSP
            libflatbuffers.a
            sqlite3
            )
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
        target_link_libraries(LSPServer stdc++fs)
    endif ()
    if (NOT CROSS_WINDOWS)
        target_link_libraries(LSPServer ${CMAKE_DL_LIBS}
                )
    else()
        target_link_libraries(LSPServer
                -ldbghelp
                -limagehlp
                )
    endif ()
    if(APPLE)
        target_link_libraries(LSPServer libcangjie-lsp.dylib)
        add_library(cangjie-lsp SHARED IMPORTED)
        set_target_properties(cangjie-lsp PROPERTIES
                IMPORTED_LOCATION ${CODIRA_LIB_DIR}/libcangjie-lsp.dylib
                )
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,@loader_path/../lib")
    elseif(CROSS_WINDOWS)
        target_link_libraries(LSPServer -lcangjie-lsp)
        add_library(cangjie-lsp SHARED IMPORTED)
        set_target_properties(cangjie-lsp PROPERTIES
                IMPORTED_LOCATION ${CODIRA_LIB_DIR}/libcangjie-lsp.dll
                )
        set_target_properties(cangjie-lsp PROPERTIES
                IMPORTED_IMPLIB ${CODIRA_LIB_DIR}/libcangjie-lsp.dll.a
                )
        target_compile_options(LSPServer PRIVATE -fno-common)
        target_link_options(LSPServer PRIVATE -Wl,--allow-multiple-definition)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-insert-timestamp")
        set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-insert-timestamp")
    else()
        set(LINK_FLAGS "-Wl,-z,relro,-z,now,-z,noexecstack")
        set(STRIP_FLAG "-s")
        set(SAFE_EXE_LINK_FLAG "-pie")
        set(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS} ${SAFE_EXE_LINK_FLAG} ${STRIP_FLAG}")
        set(CMAKE_SHARED_LINKER_FLAGS "${LINK_FLAGS} ${STRIP_FLAG}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,@loader_path/../lib")
        target_link_libraries(LSPServer libcangjie-lsp.so ${CMAKE_DL_LIBS})
        add_library(cangjie-lsp SHARED IMPORTED)
        set_target_properties(cangjie-lsp PROPERTIES
                IMPORTED_LOCATION ${CODIRA_LIB_DIR}/libcangjie-lsp.so
                )
    endif()

    add_library(LSPServerlib launcher/main.cpp)
    target_link_libraries(LSPServerlib
            pthread
            JsonRPC
            LSP
            )
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
        target_link_libraries(LSPServerlib stdc++fs)
    endif ()
    if (NOT CROSS_WINDOWS)
        target_link_libraries(LSPServerlib ${CMAKE_DL_LIBS}
                )
    endif ()
else ()
    message(WARNING "Use the [cmake -DCOMPILER_TYPE=MSVC/MINGW/Ninja .] command to set the compiler type.")
endif ()