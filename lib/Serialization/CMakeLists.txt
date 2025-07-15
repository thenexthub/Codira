
add_language_host_library(languageSerialization STATIC
  Deserialization.cpp
  DeserializeSIL.cpp
  ModuleFile.cpp
  ModuleFileSharedCore.cpp
  ScanningLoaders.cpp
  Serialization.cpp
  SerializedModuleLoader.cpp
  SerializedSILLoader.cpp
  SerializeDoc.cpp
  SerializeSIL.cpp

  TOOLCHAIN_LINK_COMPONENTS
    BitstreamReader
  )
target_link_libraries(languageSerialization PRIVATE
  languageClangImporter
  languageOption
  languageAST
  languageSIL
  clangIndex)

set_language_toolchain_is_available(languageSerialization)
