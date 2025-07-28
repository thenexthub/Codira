add_library(SwiftProtobufPluginLibrary
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
  SwiftLanguage.code
  SwiftProtobufInfo.code
  SwiftProtobufNamer.code
  swift_protobuf_module_mappings.pb.code
  UnicodeScalar+Extensions.code)
target_link_libraries(SwiftProtobufPluginLibrary PUBLIC
  SwiftProtobuf)


install(TARGETS SwiftProtobufPluginLibrary
  ARCHIVE DESTINATION lib/swift$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>/$<LOWER_CASE:${CMAKE_SYSTEM_NAME}>
  LIBRARY DESTINATION lib/swift$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>/$<LOWER_CASE:${CMAKE_SYSTEM_NAME}>
  RUNTIME DESTINATION bin)
get_swift_host_arch(swift_arch)
install(FILES
  $<TARGET_PROPERTY:SwiftProtobufPluginLibrary,Swift_MODULE_DIRECTORY>/SwiftProtobufPluginLibrary.codedoc
  $<TARGET_PROPERTY:SwiftProtobufPluginLibrary,Swift_MODULE_DIRECTORY>/SwiftProtobufPluginLibrary.codemodule
  DESTINATION lib/swift$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>/$<LOWER_CASE:${CMAKE_SYSTEM_NAME}>/${swift_arch})
set_property(GLOBAL APPEND PROPERTY Protobuf_EXPORTS SwiftProtobufPluginLibrary)
