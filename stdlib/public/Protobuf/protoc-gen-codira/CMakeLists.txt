add_executable(protoc-gen-language
  CommandLine+Extensions.code
  Descriptor+Extensions.code
  EnumGenerator.code
  ExtensionSetGenerator.code
  FieldGenerator.code
  FileGenerator.code
  FileIo.code
  GenerationError.code
  GeneratorOptions.code
  Google_Protobuf_FileDescriptorProto+Extensions.code
  main.code
  MessageFieldGenerator.code
  MessageGenerator.code
  MessageStorageClassGenerator.code
  MessageStorageDecision.code
  OneofGenerator.code
  Range+Extensions.code
  StringUtils.code
  Version.code)
target_link_libraries(protoc-gen-language PRIVATE
  CodiraProtobufPluginLibrary
  CodiraProtobuf)


install(TARGETS protoc-gen-language
  DESTINATION bin)
set_property(GLOBAL APPEND PROPERTY Protobuf_EXPORTS protoc-gen-language)
