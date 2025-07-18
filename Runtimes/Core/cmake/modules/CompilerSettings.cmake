include(CheckSourceCompiles)
include(CheckCompilerFlag)

# Use Codira 5 mode
set(CMAKE_Codira_LANGUAGE_VERSION 5)

# Use C++17
set(CodiraCore_MIN_CXX_STANDARD 17)
# Unset CMAKE_CXX_STANDARD if it's too low and in the CMakeCache.txt
if($CACHE{CMAKE_CXX_STANDARD} AND $CACHE{CMAKE_CXX_STANDARD} LESS ${CodiraCore_MIN_CXX_STANDARD})
  message(WARNING "Resetting cache value for CMAKE_CXX_STANDARD to ${CodiraCore_MIN_CXX_STANDARD}")
  unset(CMAKE_CXX_STANDARD CACHE)
endif()

# Allow manually specified CMAKE_CXX_STANDARD if it's greater than the minimum
# required C++ version
if(DEFINED CMAKE_CXX_STANDARD AND CMAKE_CXX_STANDARD LESS ${CodiraCore_MIN_CXX_STANDARD})
  message(FATAL_ERROR "Requested CMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD} which is less than the minimum C++ standard ${CodiraCore_MIN_CXX_STANDARD}")
endif()

set(CMAKE_CXX_STANDARD ${CodiraCore_MIN_CXX_STANDARD} CACHE STRING "C++ standard to conform to")
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

check_source_compiles(CXX
"#if !(__has_attribute(languagecall) && \
  __has_attribute(language_context) && \
  __has_attribute(language_error_result) && \
  __has_attribute(language_indirect_result))
#error CXX compiler must support Codira calling conventions
#endif
int main(void) { return 0; }"
HAVE_LANGUAGECALL)

if(NOT HAVE_LANGUAGECALL)
  message(SEND_ERROR "CXX Compiler must support Codira calling conventions")
endif()

check_source_compiles(CXX
"#if !(__has_attribute(languageasynccall) && \
  __has_attribute(language_async_context))
#error CXX compiler must support Codira async calling conventions
#endif
int main(void) { return 0; }"
HAVE_LANGUAGE_ASYNC_CALL)

if(NOT HAVE_LANGUAGE_ASYNC_CALL)
  message(SEND_ERROR "CXX Compiler must support Codira async calling conventions")
endif()

if(CMAKE_CXX_COMPILER_ARCHITECTURE_ID MATCHES "(x86)|(x64)" OR
   CMAKE_SYSTEM_PROCESSOR MATCHES "(i[3-6]86)|(x86_64)|(amd64)|(AMD64)")
  check_compiler_flag(CXX "-mcx16" HAVE_CXX_MCX16)
  if(HAVE_CXX_MCX16)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-mcx16>)
  endif()
endif()
