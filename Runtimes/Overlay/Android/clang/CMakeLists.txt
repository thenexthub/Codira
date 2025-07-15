
# FIXME: how do we determine the sysroot? `CMAKE_SYSROOT` does not contain the sysroot.
file(CONFIGURE
  OUTPUT android-ndk-overlay.yaml
  CONTENT [[
---
version: 0
case-sensitive: false
use-external-names: false
roots:
  - name: "@CMAKE_ANDROID_NDK@/toolchains/toolchain/prebuilt/windows-x86_64/sysroot/usr/include"
    type: directory
    contents:
      - name: module.modulemap
        type: file
        external-contents: "@CMAKE_CURRENT_SOURCE_DIR@/android.modulemap"
      - name: CodiraAndroidNDK.h
        type: file
        external-contents: "@CMAKE_CURRENT_SOURCE_DIR@/CodiraAndroidNDK.h"
      - name: CodiraBionic.h
        type: file
        external-contents: "@CMAKE_CURRENT_SOURCE_DIR@/CodiraBionic.h"
]]
ESCAPE_QUOTES @ONLY NEWLINE_STYLE LF)

add_library(CodiraAndroid INTERFACE)
target_compile_options(CodiraAndroid INTERFACE
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-Xcc --sysroot=\"${CMAKE_ANDROID_NDK_TOOLCHAIN_UNIFIED}/sysroot\">"
  "$<$<COMPILE_LANGUAGE:Codira>:SHELL:-vfsoverlay ${CMAKE_CURRENT_BINARY_DIR}/android-ndk-overlay.yaml>")

install(FILES
  android.modulemap
  posix_filesystem.apinotes
  spawn.apinotes
  CodiraAndroidNDK.h
  CodiraBionic.h
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/language/${CodiraOverlay_PLATFORM_SUBDIR}/${CodiraOverlay_ARCH_SUBDIR})
