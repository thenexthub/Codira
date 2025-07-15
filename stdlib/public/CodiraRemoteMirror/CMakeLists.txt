# liblanguageRemoteMirror.dylib should not have runtime dependencies; it's
# always built as a shared library.
add_language_target_library(languageRemoteMirror
                         SHARED DONT_EMBED_BITCODE NOLANGUAGERT
                         CodiraRemoteMirror.cpp
                         LINK_LIBRARIES
                           languageRemoteInspection
                         C_COMPILE_FLAGS
                           ${LANGUAGE_RUNTIME_CXX_FLAGS}
                           -DlanguageRemoteMirror_EXPORTS
                         LINK_FLAGS
                           ${LANGUAGE_RUNTIME_LINK_FLAGS}
                         PREFIX_INCLUDE_DIRS
                           ${LANGUAGE_SOURCE_DIR}/include/language/RemoteInspection/RuntimeHeaders
                         INCORPORATE_OBJECT_LIBRARIES languageLLVMSupport
                         LANGUAGE_COMPILE_FLAGS ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
                         DARWIN_INSTALL_NAME_DIR "${LANGUAGE_DARWIN_STDLIB_INSTALL_NAME_DIR}"
                         INSTALL_IN_COMPONENT
                           language-remote-mirror

                         TARGET_SDKS NOT LINUX_STATIC)
