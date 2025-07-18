include(CMakeParseArguments)

function(precondition var)
  cmake_parse_arguments(
    PRECONDITION # prefix
    "NEGATE" # options
    "MESSAGE" # single-value args
    "" # multi-value args
    ${ARGN})

  if (PRECONDITION_NEGATE)
    if (${var})
      if (PRECONDITION_MESSAGE)
        message(FATAL_ERROR "Error! ${PRECONDITION_MESSAGE}")
      else()
        message(FATAL_ERROR "Error! Variable ${var} is true or not empty. The value of ${var} is ${${var}}.")
      endif()
    endif()
  else()
    if (NOT ${var})
      if (PRECONDITION_MESSAGE)
        message(FATAL_ERROR "Error! ${PRECONDITION_MESSAGE}")
      else()
        message(FATAL_ERROR "Error! Variable ${var} is false, empty or not set.")
      endif()
    endif()
  endif()
endfunction()

# Assert is 'NOT ${LHS} ${OP} ${RHS}' is true.
function(precondition_binary_op OP LHS RHS)
  cmake_parse_arguments(
    PRECONDITIONBINOP # prefix
    "NEGATE" # options
    "MESSAGE" # single-value args
    "" # multi-value args
    ${ARGN})

  if (PRECONDITIONBINOP_NEGATE)
    if (${LHS} ${OP} ${RHS})
      if (PRECONDITIONBINOP_MESSAGE)
        message(FATAL_ERROR "Error! ${PRECONDITIONBINOP_MESSAGE}")
      else()
        message(FATAL_ERROR "Error! ${LHS} ${OP} ${RHS} is true!")
      endif()
    endif()
  else()
    if (NOT ${LHS} ${OP} ${RHS})
      if (PRECONDITIONBINOP_MESSAGE)
        message(FATAL_ERROR "Error! ${PRECONDITIONBINOP_MESSAGE}")
      else()
        message(FATAL_ERROR "Error! ${LHS} ${OP} ${RHS} is false!")
      endif()
    endif()
  endif()
endfunction()

# Translate a yes/no variable to the presence of a given string in a
# variable.
#
# Usage:
#   translate_flag(is_set flag_name var_name)
#
# If is_set is true, sets ${var_name} to ${flag_name}. Otherwise,
# unsets ${var_name}.
function(translate_flag is_set flag_name var_name)
  if(${is_set})
    set("${var_name}" "${flag_name}" PARENT_SCOPE)
  else()
    set("${var_name}" "" PARENT_SCOPE)
  endif()
endfunction()

macro(translate_flags prefix options)
  foreach(var ${options})
    translate_flag("${${prefix}_${var}}" "${var}" "${prefix}_${var}_keyword")
  endforeach()
endmacro()

# Set ${outvar} to ${${invar}}, asserting if ${invar} is not set.
function(precondition_translate_flag invar outvar)
  precondition(${invar})
  set(${outvar} "${${invar}}" PARENT_SCOPE)
endfunction()

function(get_bootstrapping_path path_var orig_path bootstrapping)
  if("${bootstrapping}" STREQUAL "")
    set(${path_var} ${orig_path} PARENT_SCOPE)
  else()
    file(RELATIVE_PATH relative_path ${CMAKE_BINARY_DIR} ${orig_path})
    set(${path_var} "${CMAKE_BINARY_DIR}/bootstrapping${bootstrapping}/${relative_path}" PARENT_SCOPE)
  endif()
endfunction()

# When building the stdlib in bootstrapping, return the language library path
# from the previous bootstrapping stage.
function(get_bootstrapping_language_lib_dir bs_lib_dir bootstrapping)
  set(bs_lib_dir "")
  if(BOOTSTRAPPING_MODE STREQUAL "BOOTSTRAPPING")
    set(lib_dir
        "${LANGUAGELIB_DIR}/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}")
    # If building the stdlib with bootstrapping, the compiler has to pick up
    # the language libraries of the previous bootstrapping level (because in the
    # current lib-directory they are not built yet.
    if ("${bootstrapping}" STREQUAL "1")
      get_bootstrapping_path(bs_lib_dir ${lib_dir} "0")
    elseif("${bootstrapping}" STREQUAL "")
      get_bootstrapping_path(bs_lib_dir ${lib_dir} "1")
    endif()
  elseif(BOOTSTRAPPING_MODE STREQUAL "HOSTTOOLS")
    if(LANGUAGE_HOST_VARIANT_SDK MATCHES "LINUX|ANDROID|OPENBSD|FREEBSD")
      # Compiler's INSTALL_RPATH is set to libs in the build directory
      # For building stdlib, use stdlib in the builder's resource directory
      # because the runtime may not be built yet.
      # FIXME: This assumes the ABI hasn't changed since the builder.
      get_filename_component(language_bin_dir ${CMAKE_Codira_COMPILER} DIRECTORY)
      get_filename_component(language_dir ${language_bin_dir} DIRECTORY)
      set(bs_lib_dir "${language_dir}/lib/language/${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_LIB_SUBDIR}")
    endif()
  endif()
  set(bs_lib_dir ${bs_lib_dir} PARENT_SCOPE)
endfunction()

function(is_build_type_optimized build_type result_var_name)
  if("${build_type}" STREQUAL "Debug")
    set("${result_var_name}" FALSE PARENT_SCOPE)
  elseif("${build_type}" STREQUAL "RelWithDebInfo" OR
         "${build_type}" STREQUAL "Release" OR
         "${build_type}" STREQUAL "MinSizeRel")
    set("${result_var_name}" TRUE PARENT_SCOPE)
  else()
    message(FATAL_ERROR "Unknown build type: ${build_type}")
  endif()
endfunction()

function(is_build_type_with_debuginfo build_type result_var_name)
  if("${build_type}" STREQUAL "Debug" OR
     "${build_type}" STREQUAL "RelWithDebInfo")
    set("${result_var_name}" TRUE PARENT_SCOPE)
  elseif("${build_type}" STREQUAL "Release" OR
         "${build_type}" STREQUAL "MinSizeRel")
    set("${result_var_name}" FALSE PARENT_SCOPE)
  else()
    message(FATAL_ERROR "Unknown build type: ${build_type}")
  endif()
endfunction()

# Set variable to value if value is not null or false. Otherwise set variable to
# default_value.
function(set_with_default variable value)
  cmake_parse_argument(
    SWD
    ""
    "DEFAULT"
    "" ${ARGN})
  precondition(SWD_DEFAULT
    MESSAGE "Must specify a default argument")
  if (value)
    set(${variable} ${value} PARENT_SCOPE)
  else()
    set(${variable} ${SWD_DEFAULT} PARENT_SCOPE)
  endif()
endfunction()

function(language_create_post_build_symlink target)
  set(options IS_DIRECTORY)
  set(oneValueArgs SOURCE DESTINATION WORKING_DIRECTORY COMMENT)
  cmake_parse_arguments(CS
    "${options}"
    "${oneValueArgs}"
    ""
    ${ARGN})

  if(CS_IS_DIRECTORY)
    set(cmake_symlink_option "${LANGUAGE_COPY_OR_SYMLINK_DIR}")
  else()
    set(cmake_symlink_option "${LANGUAGE_COPY_OR_SYMLINK}")
  endif()

  add_custom_command(TARGET "${target}" POST_BUILD
    COMMAND
      "${CMAKE_COMMAND}" "-E" "${cmake_symlink_option}"
      "${CS_SOURCE}"
      "${CS_DESTINATION}"
    WORKING_DIRECTORY "${CS_WORKING_DIRECTORY}"
    COMMENT "${CS_COMMENT}")
endfunction()

# Once language-frontend is built, if the standalone (early) language-driver has been built,
# we create a `language-driver` symlink adjacent to the `language` and `languagec` executables
# to ensure that `languagec` forwards to the standalone driver when invoked.
function(language_create_early_driver_copies target)
  set(LANGUAGE_EARLY_LANGUAGE_DRIVER_BUILD "" CACHE PATH "Path to early language-driver build")

  if(NOT LANGUAGE_EARLY_LANGUAGE_DRIVER_BUILD)
    return()
  endif()

  if(EXISTS ${LANGUAGE_EARLY_LANGUAGE_DRIVER_BUILD}/language-driver${CMAKE_EXECUTABLE_SUFFIX})
    message(STATUS "Creating early CodiraDriver symlinks")

    # Use `configure_file` instead of `file(COPY ...)` to establish a
    # dependency.  Further changes to `language-driver` will cause it to be copied
    # over.
    configure_file(${LANGUAGE_EARLY_LANGUAGE_DRIVER_BUILD}/language-driver${CMAKE_EXECUTABLE_SUFFIX}
                   ${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/language-driver${CMAKE_EXECUTABLE_SUFFIX}
                   COPYONLY)
    configure_file(${LANGUAGE_EARLY_LANGUAGE_DRIVER_BUILD}/language-help${CMAKE_EXECUTABLE_SUFFIX}
                   ${LANGUAGE_RUNTIME_OUTPUT_INTDIR}/language-help${CMAKE_EXECUTABLE_SUFFIX}
                   COPYONLY)
  else()
    message(STATUS "Not creating early CodiraDriver symlinks (language-driver not found)")
  endif()
endfunction()

function(dump_language_vars)
  set(LANGUAGE_STDLIB_GLOBAL_CMAKE_CACHE)
  get_cmake_property(variableNames VARIABLES)
  foreach(variableName ${variableNames})
    if(variableName MATCHES "^LANGUAGE")
      set(LANGUAGE_STDLIB_GLOBAL_CMAKE_CACHE "${LANGUAGE_STDLIB_GLOBAL_CMAKE_CACHE}set(${variableName} \"${${variableName}}\")\n")
      message("set(${variableName} \"${${variableName}}\")")
    endif()
  endforeach()
endfunction()

function(is_sdk_requested name result_var_name)
  if("${LANGUAGE_HOST_VARIANT_SDK}" STREQUAL "${name}")
    set("${result_var_name}" "TRUE" PARENT_SCOPE)
  else()
    if("${name}" IN_LIST LANGUAGE_SDKS)
      set("${result_var_name}" "TRUE" PARENT_SCOPE)
    else()
      set("${result_var_name}" "FALSE" PARENT_SCOPE)
    endif()
  endif()
endfunction()
