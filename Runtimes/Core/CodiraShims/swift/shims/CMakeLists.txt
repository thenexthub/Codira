# Codira shim header files
install(FILES
  AssertionReporting.h
  CoreFoundationShims.h
  EmbeddedShims.h
  FoundationShims.h
  GlobalObjects.h
  HeapObject.h
  KeyPath.h
  LibcOverlayShims.h
  LibcShims.h
  MetadataSections.h
  ObjCShims.h
  Random.h
  RefCount.h
  Reflection.h
  RuntimeShims.h
  RuntimeStubs.h
  CodiraStdbool.h
  CodiraStddef.h
  CodiraStdint.h
  System.h
  Target.h
  ThreadLocalStorage.h
  UnicodeData.h
  Visibility.h
  _CodiraConcurrency.h
  _CodiraDistributed.h
  _SynchronizationShims.h
  module.modulemap
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/language/shims"
  COMPONENT CodiraCore_development)

add_library(languageShims INTERFACE)
target_include_directories(languageShims INTERFACE
  $<$<COMPILE_LANGUAGE:C,CXX>:$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../>$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_LIBDIR}>>
  $<$<COMPILE_LANGUAGE:Codira>:$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_LIBDIR}/language/shims>>)
target_compile_definitions(languageShims INTERFACE
  $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>,$<COMPILE_LANGUAGE:C,CXX>>:LANGUAGE_STATIC_STDLIB>)
target_compile_options(languageShims INTERFACE
  "$<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>,$<COMPILE_LANGUAGE:Codira>>:SHELL:-Xcc -DLANGUAGE_STATIC_STDLIB>"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xcc -fmodule-map-file=\"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/module.modulemap>$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_LIBDIR}/language/shims/module.modulemap>\">")

install(TARGETS languageShims
  EXPORT CodiraCoreTargets
  COMPONENT CodiraCore_development)
