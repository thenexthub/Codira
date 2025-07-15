if(TARGET languageDemangle)
  add_language_unittest(CodiraDemangleTests IS_TARGET_TEST
    DemangleTest.cpp
  )
  set_target_properties(CodiraDemangleTests
    PROPERTIES BUILD_WITH_INSTALL_RPATH On INSTALL_RPATH "${LANGUAGE_LIBRARY_OUTPUT_INTDIR}"
  )
  
  target_link_libraries(CodiraDemangleTests
    PRIVATE
    languageDemangle
  )
endif()
