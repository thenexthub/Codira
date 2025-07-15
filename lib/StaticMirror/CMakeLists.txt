add_language_host_library(languageStaticMirror STATIC
   BinaryScanningTool.cpp
   ObjectFileContext.cpp
   TOOLCHAIN_LINK_COMPONENTS object support)

 target_link_libraries(languageStaticMirror PRIVATE
     languageRemoteInspection)
