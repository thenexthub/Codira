add_language_target_executable(language-reflection-test BUILD_WITH_STDLIB
  NOLANGUAGERT
  language-reflection-test.c
  overrides.c
  LINK_LIBRARIES
    languageRemoteMirror
    languageRemoteInspection

  TARGET_SDKS NOT LINUX_STATIC)

# NOTE(compnerd) since _WINDLL has no impact on non-Windows targets,
# we just spam it on all the targets for simplicity due to the build
# structure of language.  This will make the global variable imported,
# which we need.
set_source_files_properties(language-reflection-test.c
                            PROPERTIES
                              COMPILE_FLAGS -D_WINDLL)
