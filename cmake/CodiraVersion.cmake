# LANGUAGE_VERSION is deliberately /not/ cached so that an existing build directory
# can be reused when a new version of Codira comes out (assuming the user hasn't
# manually set it as part of their own CMake configuration).
set(LANGUAGE_VERSION_MAJOR 25)
set(LANGUAGE_VERSION_MINOR 7)
set(LANGUAGE_VERSION "${LANGUAGE_VERSION_MAJOR}.${LANGUAGE_VERSION_MINOR}")

