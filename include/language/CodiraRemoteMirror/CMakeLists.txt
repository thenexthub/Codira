set(language_remote_mirror_headers)
list(APPEND language_remote_mirror_headers
       MemoryReaderInterface.h
       Platform.h
       CodiraRemoteMirror.h
       CodiraRemoteMirrorTypes.h
       module.modulemap)
language_install_in_component(FILES
                             ${language_remote_mirror_headers}
                           DESTINATION
                             "include/language/CodiraRemoteMirror"
                           COMPONENT
                             language-remote-mirror-headers)

