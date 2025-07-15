if(("${LANGUAGE_HOST_VARIANT_SDK}" STREQUAL "${LANGUAGE_PRIMARY_VARIANT_SDK}") AND
   ("${LANGUAGE_HOST_VARIANT_ARCH}" STREQUAL "${LANGUAGE_PRIMARY_VARIANT_ARCH}"))
  if(LANGUAGE_HOST_VARIANT MATCHES "${LANGUAGE_DARWIN_VARIANTS}")
    add_language_unittest(CodiraReflectionTests
      TypeRef.cpp)
    target_include_directories(CodiraReflectionTests BEFORE PRIVATE
      ${LANGUAGE_SOURCE_DIR}/stdlib/include)
    target_compile_definitions(CodiraReflectionTests PRIVATE
      LANGUAGE_INLINE_NAMESPACE=__runtime)
    target_link_libraries(CodiraReflectionTests
      PRIVATE
      languageRemoteInspection${LANGUAGE_PRIMARY_VARIANT_SUFFIX})
  endif()
endif()
