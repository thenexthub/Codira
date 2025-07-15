
add_language_host_library(languageRemoteInspection STATIC
  ${LANGUAGE_SOURCE_DIR}/stdlib/public/RemoteInspection/MetadataSource.cpp
  ${LANGUAGE_SOURCE_DIR}/stdlib/public/RemoteInspection/TypeLowering.cpp
  ${LANGUAGE_SOURCE_DIR}/stdlib/public/RemoteInspection/TypeRef.cpp
  ${LANGUAGE_SOURCE_DIR}/stdlib/public/RemoteInspection/TypeRefBuilder.cpp)
target_compile_options(languageRemoteInspection PRIVATE -DLANGUAGE_ENABLE_REFLECTION)
target_link_libraries(languageRemoteInspection PUBLIC
  languageDemangling)
