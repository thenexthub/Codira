if(("${LANGUAGE_HOST_VARIANT_SDK}" STREQUAL "${LANGUAGE_PRIMARY_VARIANT_SDK}") AND
   ("${LANGUAGE_HOST_VARIANT_ARCH}" STREQUAL "${LANGUAGE_PRIMARY_VARIANT_ARCH}"))
  add_language_unittest(CodiraThreadingTests IS_TARGET_TEST
    Mutex.cpp
    ConditionVariable.cpp
    Once.cpp
    LinuxUlock.cpp
    Fatal.cpp
    )

  target_link_libraries(CodiraThreadingTests
    PRIVATE
    languageThreading${LANGUAGE_PRIMARY_VARIANT_SUFFIX}
    languageCore${LANGUAGE_PRIMARY_VARIANT_SUFFIX}
    )

  if(LANGUAGE_HOST_VARIANT STREQUAL "windows")
    target_link_libraries(CodiraThreadingTests PRIVATE Synchronization)
  endif()
endif()
