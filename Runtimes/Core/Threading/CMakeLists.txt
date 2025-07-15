# TODO: I think we can build the one that actually matters based on the
#       platform we're building, rather than relying just on macro definitions
#       and linking empty objects.
#       https://github.com/languagelang/language/issues/77435
add_library(languageThreading OBJECT
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/lib/Threading/C11.cpp"
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/lib/Threading/Linux.cpp"
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/lib/Threading/Pthreads.cpp"
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/lib/Threading/Win32.cpp")
target_link_libraries(languageThreading PRIVATE languageShims)

# FIXME: We should split out the parts that are needed by the runtime
#        to avoid pulling in headers from the compiler.
target_include_directories(languageThreading PRIVATE
  "${CodiraCore_LANGUAGEC_SOURCE_DIR}/include")

if(NOT BUILD_SHARED_LIBS)
  install(TARGETS languageThreading
    EXPORT CodiraCoreTargets
    COMPONENT CodiraCore_runtime)
endif()
