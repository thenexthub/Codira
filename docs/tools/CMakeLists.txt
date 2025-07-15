include(CodiraManpage)

manpage(
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/language.pod"
    PAGE_HEADER "Codira Documentation"
    MAN_FILE_BASENAME language
    MAN_SECTION 1
    INSTALL_IN_COMPONENT compiler)

