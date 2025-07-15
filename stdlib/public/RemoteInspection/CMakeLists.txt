set(languageRemoteInspection_C_COMPILE_FLAGS)
if(LANGUAGE_ENABLE_REFLECTION)
  list(APPEND languageRemoteInspection_C_COMPILE_FLAGS -DLANGUAGE_ENABLE_REFLECTION)
endif()

add_language_target_library(languageRemoteInspection STATIC
  MetadataSource.cpp
  TypeLowering.cpp
  TypeRef.cpp
  TypeRefBuilder.cpp
  C_COMPILE_FLAGS
    ${LANGUAGE_RUNTIME_CXX_FLAGS}
    -DlanguageCore_EXPORTS
    ${languageRemoteInspection_C_COMPILE_FLAGS}
  LINK_FLAGS ${LANGUAGE_RUNTIME_LINK_FLAGS}
  INCORPORATE_OBJECT_LIBRARIES
    languageLLVMSupport languageDemangling languageDemanglingCR
  LANGUAGE_COMPILE_FLAGS ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
  INSTALL_IN_COMPONENT dev)
