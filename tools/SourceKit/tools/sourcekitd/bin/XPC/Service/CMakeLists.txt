if (BUILD_SOURCEKIT_XPC_SERVICE)
  add_sourcekit_xpc_service(SourceKitService sourcekitd
    XPCService.cpp
    TOOLCHAIN_LINK_COMPONENTS support coverage
    HAS_LANGUAGE_MODULES
  )
  language_is_installing_component(sourcekit-inproc SOURCEKIT_INSTALLING_INPROC)
  if(SOURCEKIT_INSTALLING_INPROC)
    target_link_libraries(SourceKitService PUBLIC sourcekitdInProc)
  else()
    target_link_libraries(SourceKitService PRIVATE
      SourceKitCodiraLang
      sourcekitdAPI
      sourcekitdService)
  endif()
endif()

if (NOT SOURCEKIT_DEPLOYMENT_OS MATCHES "^macosx")
  add_definitions(-DSOURCEKIT_UNVERSIONED_FRAMEWORK_BUNDLE)
endif()
