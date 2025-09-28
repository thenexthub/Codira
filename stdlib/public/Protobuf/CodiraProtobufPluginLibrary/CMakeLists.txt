add_library(CodiraProtobufPluginLibrary
  Array+Extensions.code
  CodePrinter.code
  Descriptor+Extensions.code
  Descriptor.code
  FieldNumbers.code
  Google_Protobuf_Compiler_CodeGeneratorResponse+Extensions.code
  Google_Protobuf_SourceCodeInfo+Extensions.code
  NamingUtils.code
  plugin.pb.code
  ProtoFileToModuleMappings.code
  ProvidesLocationPath.code
  ProvidesSourceCodeLocation.code
  CodiraLanguage.code
  CodiraProtobufInfo.code
  CodiraProtobufNamer.code
  language_protobuf_module_mappings.pb.code
  UnicodeScalar+Extensions.code)
target_link_libraries(CodiraProtobufPluginLibrary PUBLIC
  CodiraProtobuf)


install(TARGETS CodiraProtobufPluginLibrary
  ARCHIVE DESTINATION lib/language$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>/$<LOWER_CASE:${CMAKE_SYSTEM_NAME}>
  LIBRARY DESTINATION lib/language$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>/$<LOWER_CASE:${CMAKE_SYSTEM_NAME}>
  RUNTIME DESTINATION bin)
get_language_host_arch(language_arch)
install(FILES
  $<TARGET_PROPERTY:CodiraProtobufPluginLibrary,Codira_MODULE_DIRECTORY>/CodiraProtobufPluginLibrary.codedoc
  $<TARGET_PROPERTY:CodiraProtobufPluginLibrary,Codira_MODULE_DIRECTORY>/CodiraProtobufPluginLibrary.codemodule
  DESTINATION lib/language$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>/$<LOWER_CASE:${CMAKE_SYSTEM_NAME}>/${language_arch})
set_property(GLOBAL APPEND PROPERTY Protobuf_EXPORTS CodiraProtobufPluginLibrary)
