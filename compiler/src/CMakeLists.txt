#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#


# This if branch is used by --product=libs, which just builds std lib.
# Standard library codira-std-ast need these targets.

if(CODIRA_ENABLE_COMPILER_TSAN)
    add_compile_options("${TSAN_FLAGS}")
    add_link_options("${TSAN_FLAGS}")
endif()

if(IOS)
    get_directory_property(SRC_COMPILE_OPTIONS COMPILE_OPTIONS)
    list(REMOVE_ITEM SRC_COMPILE_OPTIONS "--target=${TRIPLE}")
    set_directory_properties(PROPERTIES COMPILE_OPTIONS "${SRC_COMPILE_OPTIONS}")
    add_compile_options(--target=${CXX_COMPATIABLE_TRIPLE})
endif()

set(CODEC_EXTRA_WARNINGS -Woverloaded-virtual -Wvla -Wconversion -Wfloat-equal -Wunused -Wdelete-non-virtual-dtor -Wswitch-default -Wcast-qual -Wextra -Wformat=2 -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-sign-conversion)
# -Wno-sign-conversion here to be removed by next MR
if(${CMAKE_SYSTEM_NAME} EQUAL Darwin)
    # mac uses llvm headers for std, and llvm headers have the following warning(s)
    set(CODEC_EXTRA_WARNINGS ${CODEC_EXTRA_WARNINGS} -Wno-float-equal)
    # mac headers have warning on undefed macro(s) (FFI_GO_CLOSURES), so use -Wno-undef
    set(CODEC_EXTRA_WARNINGS ${CODEC_EXTRA_WARNINGS} -Wno-undef)
endif()
    # g++ on some platform has harsher check on -Werror=shadow, in that it does not allow function parameters to share the same name with class members, which is actually bad in constructor. So we disable this on some platform.
    set(CODEC_EXTRA_WARNINGS ${CODEC_EXTRA_WARNINGS} -Wshadow)

# when including llvm header files, the following warnings have to be excluded
set(CODEC_WITH_LLVM_EXTRA_WARNINGS ${CODEC_EXTRA_WARNINGS} -Wno-sign-conversion -Wno-unused-parameter -Wno-float-conversion -Wno-shadow)
# gcc on some platform does not recognise these options
set(CODEC_WITH_LLVM_EXTRA_WARNINGS ${CODEC_WITH_LLVM_EXTRA_WARNINGS} -Wno-implicit-int-conversion -Wno-shorten-64-to-32 -Wno-implicit-int-float-conversion)

if(NOT CODIRA_BUILD_CODEC AND CODIRA_BUILD_STD_SUPPORT)
    add_subdirectory(AST)
    add_subdirectory(Driver)
    add_subdirectory(Parse)
    add_subdirectory(Macro)
    add_subdirectory(ConditionalCompilation)
    add_subdirectory(Lex)
    add_subdirectory(Utils)
    add_subdirectory(Basic)
    add_subdirectory(Option)
    if (CODIRA_CODEGEN_CODENATIVE_BACKEND)
        add_subdirectory(MetaTransformation)
    endif()

    set(AST_FFI_OBJECTS
        $<TARGET_OBJECTS:CodiraASTCommon>
        $<TARGET_OBJECTS:CodiraParseFFI>
        $<TARGET_OBJECTS:CodiraLex>
        $<TARGET_OBJECTS:CodiraBasic>
        $<TARGET_OBJECTS:CodiraOption>
        $<TARGET_OBJECTS:CodiraAstSerialization>
        $<TARGET_OBJECTS:CodiraCommonUtilFFI>
        $<TARGET_OBJECTS:CodiraUnicodeUtils>
        $<TARGET_OBJECTS:CodiraTempFilesUtil>)
    add_library(codira-ast-support STATIC ${AST_FFI_OBJECTS})
    string(TOLOWER ${TARGET_TRIPLE_DIRECTORY_PREFIX}_${CODENATIVE_BACKEND} output_lib_dir)
    install(TARGETS codira-ast-support DESTINATION lib/${output_lib_dir}${SANITIZER_SUBPATH})
    return()
endif()

set(CODIRA_SRC_COMMON_OBJECTS_LIST
    $<TARGET_OBJECTS:CodiraASTCommon>
    $<TARGET_OBJECTS:CodiraASTExtra>
    $<TARGET_OBJECTS:CodiraDriver>
    $<TARGET_OBJECTS:CodiraFrontend>
    $<TARGET_OBJECTS:CodiraFrontendTool>
    $<TARGET_OBJECTS:CodiraMacro>
    $<TARGET_OBJECTS:CodiraAstSerialization>
    $<TARGET_OBJECTS:CodiraSema>
    $<TARGET_OBJECTS:CodiraModules>
    $<TARGET_OBJECTS:CodiraConditionalCompilation>
    $<TARGET_OBJECTS:CodiraParse>
    $<TARGET_OBJECTS:CodiraLex>
    $<TARGET_OBJECTS:CodiraUnicodeUtils>
    $<TARGET_OBJECTS:CodiraIncrementalCompilation>
    $<TARGET_OBJECTS:CodiraCommonUtil>
    $<TARGET_OBJECTS:CodiraProfileUtils>
    $<TARGET_OBJECTS:CodiraBasic>
    $<TARGET_OBJECTS:CodiraMangle>)

if (CODIRA_CODEGEN_CODENATIVE_BACKEND)
    set(CODIRA_SRC_COMMON_OBJECTS_LIST ${CODIRA_SRC_COMMON_OBJECTS_LIST} $<TARGET_OBJECTS:CodiraMetaTransformation>)
endif()

if (CODIRA_CODEGEN_CODENATIVE_BACKEND)
    list(APPEND CODIRA_SRC_COMMON_OBJECTS_LIST $<TARGET_OBJECTS:CodiraCHIRBase>)
    list(APPEND CODIRA_SRC_COMMON_OBJECTS_LIST $<TARGET_OBJECTS:CodiraCHIRExtra>)
    list(APPEND CODIRA_SRC_COMMON_OBJECTS_LIST $<TARGET_OBJECTS:CodiraCodeGen>)
    list(APPEND CODIRA_SRC_COMMON_OBJECTS_LIST $<TARGET_OBJECTS:CodiraCodeGenIncrementalGen>)
else()
    list(APPEND CODIRA_SRC_COMMON_OBJECTS_LIST $<TARGET_OBJECTS:CodiraCHIRBase>)
    list(APPEND CODIRA_SRC_COMMON_OBJECTS_LIST $<TARGET_OBJECTS:CodiraCHIRExtra>)
    list(APPEND CODIRA_SRC_COMMON_OBJECTS_LIST $<TARGET_OBJECTS:CodiraCodeGen>)
endif()

# By using `CodiraOption-Fully-Enabled`, we could still have a fully featured compiler for
# compiling standard library when building for a visible options only version.
if(CODIRA_VISIBLE_OPTIONS_ONLY)
    set(CODIRA_SRC_OBJECTS
        ${CODIRA_SRC_COMMON_OBJECTS_LIST} $<TARGET_OBJECTS:CodiraOption-Fully-Enabled>
        CACHE INTERNAL "")
else()
    set(CODIRA_SRC_OBJECTS
        ${CODIRA_SRC_COMMON_OBJECTS_LIST} $<TARGET_OBJECTS:CodiraOption>
        CACHE INTERNAL "")
endif()
add_executable(codec main.cpp ${CODIRA_SRC_OBJECTS})

if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_dependencies(codec codenative)
    set(CODENATIVE_BACKEND "codenative")
    string(TOLOWER ${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}_${CODENATIVE_BACKEND} output_lib_dir)
    set(RUNTIME_LIB_DIR ${CMAKE_BINARY_DIR}/lib/${output_lib_dir})
endif()
set_target_properties(codec PROPERTIES LINK_FLAGS
                                     "${CMAKE_EXE_LINKER_FLAGS} ${SAFE_EXE_LINK_FLAG} ${CODE_RUNTIME_LINK_FLAGS}")

target_link_libraries(
    codec ${LLVM_LIBS}
    # Required by CodiraBasic
    ${CMAKE_DL_LIBS})

target_link_libraries(codec boundscheck-static) # depended by LLVMCore in ${LINK_LIBS} and interrupt signal handler
target_include_directories(codec PRIVATE ${BOUNDSCHECK}/include)

target_link_libraries(codec ${ffi})

if(MINGW)
    execute_process(
        COMMAND ${CMAKE_C_COMPILER} -print-file-name=CRT_glob.o
        OUTPUT_VARIABLE GET_CRT_GLOB_OUTPUT
        ERROR_VARIABLE GET_CRT_GLOB_ERROR
        RESULT_VARIABLE GET_CRT_GLOB_RESULT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT GET_CRT_GLOB_RESULT EQUAL 0)
        message(FATAL_ERROR "Cannot get filepath of CRT_glob.o! Error message is: ${GET_CRT_GLOB_ERROR}")
    endif()
    target_link_libraries(codec ${GET_CRT_GLOB_OUTPUT})
endif()

if(CMAKE_BUILD_TYPE MATCHES Release AND MINGW)
    # Add static link option for codec when building MinGW release. This use
    # case is defined in GCC Runtime Libraries Exception and has no GPL issue.
    target_link_options(codec PRIVATE -static)
endif()

if(CODIRA_VISIBLE_OPTIONS_ONLY)
    # `codec` and `codec-release` are identical except that `codec` supports all compile options but
    # `codec-release` supports visible options only. Visible options are features that are ready to be
    # released to users.
    add_executable(codec-release main.cpp ${CODIRA_SRC_COMMON_OBJECTS_LIST} $<TARGET_OBJECTS:CodiraOption>)
    add_dependencies(codec-release codec)
    # Since `codec` is almost identical to `codec-release`, they always have the same link flags, link
    # options, link libraries, etc. We get the properties of `codec` and apply them on `codec-release` here.
    include(ApplyProperties)
    apply_properties(
        FROM_TARGET codec
        TO_TARGET codec-release
        PROPERTY_NAMES
            LINK_FLAGS
            INCLUDE_DIRECTORIES
            LINK_OPTIONS
            LINK_LIBRARIES)
    install(
        PROGRAMS ${CMAKE_BINARY_DIR}/bin/codec-release${CMAKE_EXECUTABLE_SUFFIX}
        DESTINATION bin/
        RENAME codec${CMAKE_EXECUTABLE_SUFFIX})
else()
    install(TARGETS codec)
endif()

# `codec-frontend.exe` is a special wrapper executable to run our compiler frontend on windows.
# On windows platform, symbolic link can only be created with command prompt with administrator
# privilege. To be able to build the project with no special privilege, our build strategy is as
# follows:
# 1) On windows Platform && building Release:
#    Use our wrapper executable as `codec-frontend.exe`. Debug of `codec.exe` by calling `codec-frontend.exe`
#    would be unavailable since they are different programs. The size of `codec-frontend.exe` is small.
#    No special privilege is required.
# 2) On windows Platform && building Debug && without administrator privilege:
#    Simply make a copy of `codec.exe` and name it `codec-frontend.exe`. Same debugging experience between
#    `codec.exe` and `codec-frontend.exe`. The size of `codec-frontend.exe` would be the same as `codec.exe`,
#    which means that much space would be used. Since it is for debugging only, it should be tolerable.
# 3) On windows Platform && building Debug && with administrator privilege / On Linux:
#    Create symbolic link `codec-frontend(.exe)` pointing to `codec(.exe)`. Same debugging experience between
#    `codec(.exe)` and `codec-frontend(.exe)`. Minimum size usage.
if(WIN32 AND CMAKE_BUILD_TYPE MATCHES Release)
    add_executable(codec-frontend main-frontend.cpp)
    target_link_options(codec-frontend PRIVATE -static)
else()
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/bin/codec-frontend${CMAKE_EXECUTABLE_SUFFIX}
        COMMAND ${CMAKE_COMMAND}
            -D CMAKE_CROSSCOMPILING=${CMAKE_CROSSCOMPILING}
            -D WIN32=${WIN32}
            -D LINK_TARGET=codec${CMAKE_EXECUTABLE_SUFFIX}
            -D LINK_NAME=codec-frontend${CMAKE_EXECUTABLE_SUFFIX}
            -D WORKING_DIR=${CMAKE_BINARY_DIR}/bin
            -P "${CMAKE_SOURCE_DIR}/cmake/modules/make-symlink.cmake"
        DEPENDS codec)
    add_custom_target(
        codec-frontend ALL
        DEPENDS ${CMAKE_BINARY_DIR}/bin/codec-frontend${CMAKE_EXECUTABLE_SUFFIX}
        COMMENT "Making codec-frontend${CMAKE_EXECUTABLE_SUFFIX}")
endif()

if(COMPILER_EXPLORER_RACE_FIX)
    add_compile_definitions(COMPILER_EXPLORER_RACE_FIX)
endif()

if(WIN32 AND CMAKE_BUILD_TYPE MATCHES Release)
    install(TARGETS codec-frontend)
else()
    install(
        CODE "execute_process(COMMAND ${CMAKE_COMMAND}
        -D CMAKE_CROSSCOMPILING=${CMAKE_CROSSCOMPILING}
        -D WIN32=${WIN32}
        -D LINK_TARGET=codec${CMAKE_EXECUTABLE_SUFFIX}
        -D LINK_NAME=codec-frontend${CMAKE_EXECUTABLE_SUFFIX}
        -D WORKING_DIR=\${CMAKE_INSTALL_PREFIX}/bin
        -P ${CMAKE_SOURCE_DIR}/cmake/modules/make-symlink.cmake)")
endif()

if(MINGW AND CMAKE_BUILD_TYPE MATCHES "^(Debug|RelWithDebInfo|MinSizeRelWithDebInfo)$")
    add_custom_command(
        TARGET codec
        COMMAND ${CMAKE_COMMAND} -E rename codec.exe codec-original.exe
        COMMAND objcopy --only-keep-debug --compress-debug-sections codec-original.exe codec.debug
        COMMAND objcopy --strip-debug --add-gnu-debuglink=codec.debug codec-original.exe codec.exe
        BYPRODUCTS codec.debug
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        COMMENT "Extracting debug info from codec.exe to codec.debug")
    install(
        FILES ${CMAKE_BINARY_DIR}/bin/codec.debug
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif()

# The autosdk, IDE need this library.
set(FRONTEND_SRC_OBJECTS
    $<TARGET_OBJECTS:CodiraASTCommon>
    $<TARGET_OBJECTS:CodiraASTExtra>
    $<TARGET_OBJECTS:CodiraFrontend>
    $<TARGET_OBJECTS:CodiraMacro>
    $<TARGET_OBJECTS:CodiraAstSerialization>
    $<TARGET_OBJECTS:CodiraSema>
    $<TARGET_OBJECTS:CodiraModules>
    $<TARGET_OBJECTS:CodiraParse>
    $<TARGET_OBJECTS:CodiraConditionalCompilation>
    $<TARGET_OBJECTS:CodiraLex>
    $<TARGET_OBJECTS:CodiraUnicodeUtils>
    $<TARGET_OBJECTS:CodiraIncrementalCompilation>
    $<TARGET_OBJECTS:CodiraOption>
    $<TARGET_OBJECTS:CodiraCommonUtil>
    $<TARGET_OBJECTS:CodiraProfileUtils>
    $<TARGET_OBJECTS:CodiraBasic>
    $<TARGET_OBJECTS:CodiraMangle>
    $<TARGET_OBJECTS:CodiraTempFilesUtil>)

if (CODIRA_CODEGEN_CODENATIVE_BACKEND)
    set(FRONTEND_SRC_OBJECTS ${FRONTEND_SRC_OBJECTS} $<TARGET_OBJECTS:CodiraMetaTransformation>)
endif()

if (CODIRA_CODEGEN_CODENATIVE_BACKEND)
    list(APPEND FRONTEND_SRC_OBJECTS $<TARGET_OBJECTS:CodiraCHIRBase>)
    list(APPEND FRONTEND_SRC_OBJECTS $<TARGET_OBJECTS:CodiraCHIRExtra>)
else()
    list(APPEND FRONTEND_SRC_OBJECTS $<TARGET_OBJECTS:CodiraCHIRBase>)
    list(APPEND FRONTEND_SRC_OBJECTS $<TARGET_OBJECTS:CodiraCHIRExtra>)
endif()
add_library(codira-lsp ${FRONTEND_SRC_OBJECTS})
if(MINGW AND CMAKE_BUILD_TYPE MATCHES "^(Debug|RelWithDebInfo|MinSizeRelWithDebInfo)$")
    set(WIN_DEBUG_USE_STATIC_LIB ON)
endif()
if(NOT WIN_DEBUG_USE_STATIC_LIB)
    add_library(codira-lsp-share SHARED ${FRONTEND_SRC_OBJECTS})
    set_target_properties(codira-lsp-share PROPERTIES OUTPUT_NAME "codira-lsp")
    target_link_libraries(codira-lsp-share ${ffi} boundscheck-static)
    if(MINGW)
        target_link_options(codira-lsp-share PRIVATE -pthread)
    endif()
endif()

#  We should remove this once the LSPCompilerInstance is out of compiler
if(WIN_DEBUG_USE_STATIC_LIB)
    install(TARGETS codira-lsp DESTINATION "tools/lib")
endif()
if(CODIRA_CODEGEN_CODENATIVE_BACKEND AND NOT WIN_DEBUG_USE_STATIC_LIB)
    install(
        TARGETS codira-lsp-share
        RUNTIME DESTINATION tools/bin
        ARCHIVE DESTINATION tools/lib
        LIBRARY DESTINATION tools/lib)
endif()

# LSPMacroServer for lsp
add_executable(LSPMacroServer main-macrosrv.cpp)
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_dependencies(LSPMacroServer codenative)
endif()
set_target_properties(LSPMacroServer PROPERTIES LINK_FLAGS
                                    "${CMAKE_EXE_LINKER_FLAGS} ${SAFE_EXE_LINK_FLAG} ${CODE_RUNTIME_LINK_FLAGS}")

target_link_libraries(
    LSPMacroServer ${LLVM_LIBS}
    # Required by CodiraBasic
    ${CMAKE_DL_LIBS})

target_link_libraries(LSPMacroServer boundscheck-static) # depended by LLVMCore in ${LINK_LIBS} and interrupt signal handler
target_include_directories(LSPMacroServer PRIVATE ${BOUNDSCHECK}/include)

add_dependencies(LSPMacroServer CodiraFlatbuffersHeaders)
target_include_directories(LSPMacroServer PRIVATE ${FLATBUFFERS_INCLUDE_DIR})
if(WIN_DEBUG_USE_STATIC_LIB)
    target_link_libraries(LSPMacroServer ${ffi} codira-lsp)
else()
    target_link_libraries(LSPMacroServer ${ffi} codira-lsp-share)
endif()
# Add the rpath for LSPMacroServer
if(DARWIN)
    set_property(TARGET LSPMacroServer APPEND PROPERTY INSTALL_RPATH "@loader_path/../lib")
else()
    set_property(TARGET LSPMacroServer APPEND PROPERTY INSTALL_RPATH "\$ORIGIN/../lib")
endif()
install(TARGETS LSPMacroServer DESTINATION "tools/bin")

# The chir-dis
add_executable(chir-dis main-chir-dis.cpp)
if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_dependencies(chir-dis codenative)
endif()
set_target_properties(chir-dis PROPERTIES LINK_FLAGS
                                    "${CMAKE_EXE_LINKER_FLAGS} ${SAFE_EXE_LINK_FLAG} ${CODE_RUNTIME_LINK_FLAGS}")

target_link_libraries(
    chir-dis ${LLVM_LIBS}
    # Required by CodiraBasic
    ${CMAKE_DL_LIBS})

target_link_libraries(chir-dis boundscheck-static) # Depended by LLVMCore in ${LINK_LIBS} and interrupt signal handler
target_include_directories(chir-dis PRIVATE ${BOUNDSCHECK}/include)

add_dependencies(chir-dis CodiraFlatbuffersHeaders)
target_include_directories(chir-dis PRIVATE ${FLATBUFFERS_INCLUDE_DIR})
if(WIN_DEBUG_USE_STATIC_LIB)
    target_link_libraries(chir-dis ${ffi} codira-lsp)
else()
    target_link_libraries(chir-dis ${ffi} codira-lsp-share)
endif()
# Add the rpath for chir-dis
if(DARWIN)
    set_property(TARGET chir-dis APPEND PROPERTY INSTALL_RPATH "@loader_path/../lib")
else()
    set_property(TARGET chir-dis APPEND PROPERTY INSTALL_RPATH "\$ORIGIN/../lib")
endif()
install(TARGETS chir-dis DESTINATION "tools/bin")


set(CODIRA_LIB_FRONTEND
    $<TARGET_OBJECTS:CodiraDriver>
    $<TARGET_OBJECTS:CodiraFrontendTool>)
if (CODIRA_CODEGEN_CODENATIVE_BACKEND)
    list(APPEND CODIRA_LIB_FRONTEND $<TARGET_OBJECTS:CodiraCodeGen>)
    list(APPEND CODIRA_LIB_FRONTEND $<TARGET_OBJECTS:CodiraCodeGenIncrementalGen>)
endif()
if(NOT WIN_DEBUG_USE_STATIC_LIB)
    add_library(codira-frontend SHARED ${CODIRA_LIB_FRONTEND})
else()
    add_library(codira-frontend ${CODIRA_LIB_FRONTEND})
endif()
if (CODIRA_CODEGEN_CODENATIVE_BACKEND)
    if(NOT WIN_DEBUG_USE_STATIC_LIB)
        target_link_libraries(codira-frontend codira-lsp-share)
        if(MINGW)
            target_link_libraries(codira-frontend -pthread)
        endif() 
    endif()
    if(DARWIN)
        target_link_libraries(codira-frontend -lLLVM)
    elseif(MINGW)
	    target_link_libraries(codira-frontend -lLLVM-15 -lLLVM-Foundation-15)
    else()
        target_link_libraries(codira-frontend -lLLVM-15)
    endif()
    target_link_directories(codira-frontend PUBLIC ${CMAKE_BINARY_DIR}/third_party/llvm/lib)
    install(
        TARGETS codira-frontend
        RUNTIME DESTINATION tools/bin
        ARCHIVE DESTINATION tools/lib
        LIBRARY DESTINATION tools/lib)
endif()
if(CODIRA_BUILD_CODEDB)
    add_dependencies(lldb codira-frontend)
endif()

add_subdirectory(AST)
add_subdirectory(Driver)
add_subdirectory(Parse)
add_subdirectory(Macro)
add_subdirectory(Frontend)
add_subdirectory(FrontendTool)
add_subdirectory(IncrementalCompilation)
add_subdirectory(Lex)
add_subdirectory(ConditionalCompilation)
add_subdirectory(Sema)

add_subdirectory(Modules)
add_subdirectory(Utils)
add_subdirectory(Basic)
add_subdirectory(Option)
add_subdirectory(Mangle)
if (CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_subdirectory(MetaTransformation)
endif()

if (CODIRA_CODEGEN_CODENATIVE_BACKEND)
    add_subdirectory(CHIR)
    add_subdirectory(CodeGen)
endif()

set(AST_FFI_OBJECTS
    $<TARGET_OBJECTS:CodiraASTCommon>
    $<TARGET_OBJECTS:CodiraParseFFI>
    $<TARGET_OBJECTS:CodiraLex>
    $<TARGET_OBJECTS:CodiraBasic>
    $<TARGET_OBJECTS:CodiraOption>
    $<TARGET_OBJECTS:CodiraAstSerialization>
    $<TARGET_OBJECTS:CodiraCommonUtilFFI>
    $<TARGET_OBJECTS:CodiraUnicodeUtils>
    $<TARGET_OBJECTS:CodiraTempFilesUtil>)
add_library(codira-ast-support STATIC ${AST_FFI_OBJECTS})
string(TOLOWER ${TARGET_TRIPLE_DIRECTORY_PREFIX}_${CODENATIVE_BACKEND} output_lib_dir)
install(TARGETS codira-ast-support DESTINATION lib/${output_lib_dir}${SANITIZER_SUBPATH})
