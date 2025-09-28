add_library(CodiraProtobuf
  any.pb.code
  AnyMessageStorage.code
  AnyUnpackError.code
  api.pb.code
  BinaryDecoder.code
  BinaryDecodingError.code
  BinaryDecodingOptions.code
  BinaryDelimited.code
  BinaryEncoder.code
  BinaryEncodingError.code
  BinaryEncodingSizeVisitor.code
  BinaryEncodingVisitor.code
  CustomJSONCodable.code
  Data+Extensions.code
  Decoder.code
  descriptor.pb.code
  DoubleParser.code
  duration.pb.code
  empty.pb.code
  Enum.code
  ExtensibleMessage.code
  ExtensionFields.code
  ExtensionFieldValueSet.code
  ExtensionMap.code
  FieldTag.code
  FieldTypes.code
  field_mask.pb.code
  Google_Protobuf_Any+Extensions.code
  Google_Protobuf_Any+Registry.code
  Google_Protobuf_Duration+Extensions.code
  Google_Protobuf_FieldMask+Extensions.code
  Google_Protobuf_ListValue+Extensions.code
  Google_Protobuf_NullValue+Extensions.code
  Google_Protobuf_Struct+Extensions.code
  Google_Protobuf_Timestamp+Extensions.code
  Google_Protobuf_Value+Extensions.code
  Google_Protobuf_Wrappers+Extensions.code
  HashVisitor.code
  Internal.code
  JSONDecoder.code
  JSONDecodingError.code
  JSONDecodingOptions.code
  JSONEncoder.code
  JSONEncodingError.code
  JSONEncodingOptions.code
  JSONEncodingVisitor.code
  JSONMapEncodingVisitor.code
  JSONScanner.code
  MathUtils.code
  Message+AnyAdditions.code
  Message+BinaryAdditions.code
  Message+FieldMask.code
  Message+JSONAdditions.code
  Message+JSONArrayAdditions.code
  Message+TextFormatAdditions.code
  Message.code
  MessageExtension.code
  NameMap.code
  PathDecoder.code
  PathVisitor.code
  ProtobufAPIVersionCheck.code
  ProtobufMap.code
  ProtoNameProviding.code
  SelectiveVisitor.code
  SimpleExtensionMap.code
  source_context.pb.code
  StringUtils.code
  struct.pb.code
  CodiraProtobufError.code
  TextFormatDecoder.code
  TextFormatDecodingError.code
  TextFormatEncoder.code
  TextFormatEncodingOptions.code
  TextFormatEncodingVisitor.code
  TextFormatScanner.code
  timestamp.pb.code
  TimeUtils.code
  type.pb.code
  UnknownStorage.code
  UnsafeBufferPointer+Shims.code
  UnsafeRawPointer+Shims.code
  Varint.code
  Version.code
  Visitor.code
  WireFormat.code
  wrappers.pb.code
  ZigZag.code)
set_target_properties(CodiraProtobuf PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})


install(TARGETS CodiraProtobuf
  ARCHIVE DESTINATION lib/language$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>/$<LOWER_CASE:${CMAKE_SYSTEM_NAME}>
  LIBRARY DESTINATION lib/language$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>/$<LOWER_CASE:${CMAKE_SYSTEM_NAME}>
  RUNTIME DESTINATION bin)
get_language_host_arch(language_arch)
install(FILES
  $<TARGET_PROPERTY:CodiraProtobuf,Codira_MODULE_DIRECTORY>/CodiraProtobuf.codedoc
  $<TARGET_PROPERTY:CodiraProtobuf,Codira_MODULE_DIRECTORY>/CodiraProtobuf.codemodule
  DESTINATION lib/language$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:_static>/$<LOWER_CASE:${CMAKE_SYSTEM_NAME}>/${language_arch})
set_property(GLOBAL APPEND PROPERTY Protobuf_EXPORTS CodiraProtobuf)
