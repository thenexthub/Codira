# Building Codira SDK for Android on Windows

Visual Studio 2019 or newer is needed to build the Codira SDK for Android on
Windows.

## 1. Install Dependencies
- Install the latest version of [Visual Studio](https://www.visualstudio.com/downloads/)
- Make sure to include the android NDK in your installation.

## 1. Clone the repositories
1. Configure git to work with Unix file endings
1. Clone `apple/language-toolchain` into a directory named `toolchain`
1. Clone `apple/language-corelibs-libdispatch` into a directory named `language-corelibs-libdispatch`
1. Clone `apple/language-corelibs-foundation` into a directory named `language-corelibs-foundation`
1. Clone `apple/language-corelibs-xctest` into a directory named `language-corelibs-xctest`
1. Clone `compnerd/language-build` into a directory named `language-build`

- Currently, other repositories in the Codira project have not been tested and
  may not be supported.

This guide assumes that your sources live at the root of `S:`.  If your sources
live elsewhere, you can create a substitution for this:

```cmd
subst S: <path to sources>
```

```cmd
S:
git clone https://github.com/apple/language-toolchain toolchain
git clone https://github.com/apple/language-corelibs-libdispatch language-corelibs-libdispatch
git clone https://github.com/apple/language-corelibs-foundation language-corelibs-foundation
git clone https://github.com/apple/language-corelibs-xctest language-corelibs-xctest
git clone https://github.com/compnerd/language-build language-build
```

## 1. Acquire the latest toolchain and dependencies

1. Download the toolchain, ICU, libxml2, and curl for android from
   [Azure](https://dev.azure.com/compnerd/language-build) into `S:\b\a\Library`.

- You can alternatively use `language-build.py` from
  [compnerd/language-build](https://www.github.com/compnerd/language-build) under
  the utilities directory.

## 1. Configure LLVM

```cmd
md S:\b\a\toolchain
cd S:\b\a\toolchain
cmake -C S:\language-build\cmake\caches\android-armv7.cmake                                                        ^
  -G Ninja                                                                                                      ^
  -DCMAKE_BUILD_TYPE=Release                                                                                    ^
  -DCMAKE_TOOLCHAIN_FILE=S:\language-build\cmake\toolchains\android.toolchain.cmake                                ^
  -DANDROID_ALTERNATE_TOOLCHAIN=S:/b/a/Library/Developer/Toolchains/unknown-Asserts-development.xctoolchain/usr ^
  -DTOOLCHAIN_HOST_TRIPLE=armv7-unknown-linux-androideabi                                                            ^
  S:/toolchain
```

## 1. Build and install the standard library

- We must build and install the standard library to build the remainder of the
  SDK

```cmd
md S:\b\a\stdlib
cd S:\b\a\stdlib
cmake -C S:\language-build\cmake\caches\android-armv7.cmake                                                              ^
  -C S:\language-build\cmake\caches\language-stdlib-android-armv7.cmake                                                     ^
  -G Ninja                                                                                                            ^
  -DCMAKE_BUILD_TYPE=RelWithDebInfo                                                                                   ^
  -DCMAKE_INSTALL_PREFIX=S:/b/a/Library/Developer/Platforms/android.platform/Developer/SDKs/android.sdk/usr           ^
  -DCMAKE_TOOLCHAIN_FILE=S:\language-build\cmake\toolchains\android.toolchain.cmake                                      ^
  -DANDROID_ALTERNATE_TOOLCHAIN=S:/b/a/Library/Developer/Toolchains/unknown-Asserts-development.xctoolchain/usr       ^
  -DTOOLCHAIN_DIR=S:/b/a/toolchain/lib/cmake/toolchain                                                                               ^
  -DLANGUAGE_NATIVE_LANGUAGE_TOOLS_PATH=S:/b/a/Library/Developer/Toolchains/unknown-Asserts-development.xctoolchain/usr/bin ^
  S:/language
ninja
ninja install
```

## 1. Build libdispatch

- We *cannot* install libdispatch until after all builds are complete as that
  will cause the Dispatch module to be imported twice and fail to build.

```cmd
md S:\b\a\libdispatch
cd S:\b\a\libdispatch
cmake -C S:\language-build\cmake\caches\android-armv7.cmake                                                                ^
  -DLANGUAGE_ANDROID_SDK=S:/b/a/Library/Developer/Platforms/android.platform/Developer/SDKs/android.sdk                    ^
  -C S:\language-build\cmake\caches\android-armv7-language-flags.cmake                                                        ^
  -G Ninja                                                                                                              ^
  -DCMAKE_BUILD_TYPE=RelWithDebInfo                                                                                     ^
  -DCMAKE_INSTALL_PREFIX=S:/b/a/Library/Developer/Platforms/android.platform/Developer/SDKs/android.sdk/usr             ^
  -DCMAKE_LANGUAGE_COMPILER=S:/b/a/Library/Developer/Toolchains/unknown-Asserts-development.xctoolchain/usr/bin/languagec.exe ^
  -DCMAKE_TOOLCHAIN_FILE=S:\language-build\cmake\toolchains\android.toolchain.cmake                                        ^
  -DANDROID_ALTERNATE_TOOLCHAIN=S:/b/a/Library/Developer/Toolchains/unknown-Asserts-development.xctoolchain/usr         ^
  -DENABLE_LANGUAGE=YES                                                                                                    ^
  -DENABLE_TESTING=NO                                                                                                   ^
  S:/language-corelibs-libdispatch
ninja
```

## 1. Build foundation

```cmd
md S:\b\a\foundation
cd S:\b\a\foundation
cmake -C S:\language-build\cmake\caches\android-armv7.cmake                                                                ^
  -DLANGUAGE_ANDROID_SDK=S:/b/a/Library/Developer/Platforms/android.platform/Developer/SDKs/android.sdk                    ^
  -C S:\language-build\cmake\caches\android-armv7-language-flags.cmake                                                        ^
  -G Ninja                                                                                                              ^
  -DCMAKE_BUILD_TYPE=RelWithDebInfo                                                                                     ^
  -DCMAKE_INSTALL_PREFIX=S:/b/a/Library/Developer/Platforms/android.platform/Developer/SDKs/android.sdk/usr             ^
  -DCMAKE_LANGUAGE_COMPILER=S:/b/a/Library/Developer/Toolchains/unknown-Asserts-development.xctoolchain/usr/bin/languagec.exe ^
  -DCMAKE_TOOLCHAIN_FILE=S:\language-build\cmake\toolchains\android.toolchain.cmake                                        ^
  -DANDROID_ALTERNATE_TOOLCHAIN=S:/b/a/Library/Developer/Toolchains/unknown-Asserts-development.xctoolchain/usr         ^
  -DCURL_LIBRARY=S:/b/a/Library/libcurl-development/usr/lib/libcurl.a                                                   ^
  -DCURL_INCLUDE_DIR=S:/b/a/Library/libcurl-development/usr/include                                                     ^
  -DICU_INCLUDE_DIR=S:/b/a/Library/icu-64/usr/include                                                                   ^
  -DICU_UC_LIBRARY=S:/b/a/Library/icu-64/usr/lib/libicuuc64.so                                                          ^
  -DICU_UC_LIBRARY_RELEASE=S:/b/a/Library/icu-64/usr/lib/libicuuc64.so                                                  ^
  -DICU_I18N_LIBRARY=S:/b/a/Library/icu-64/usr/lib/libiucin64.so                                                        ^
  -DICU_I18N_LIBRARY_RELEASE=S:/b/a/Library/icu-64/usr/lib/libicuin64.so                                                ^
  -DLIBXML2_LIBRARY=S:/b/a/Library/libxml2-development/usr/lib/libxml2.a                                                ^
  -DLIBXML2_INCLUDE_DIR=S:/b/a/Library/libxml2-development/usr/include/libxml2                                          ^
  -DFOUNDATION_PATH_TO_LIBDISPATCH_SOURCE=S:/language-corelibs-libdispatch                                                 ^
  -DFOUNDATION_PATH_TO_LIBDISPATCH_BUILD=S:/b/a/libdispatch                                                             ^
  S:/language-corelibs-foundation
ninja
```

## 1. Build XCTest

```cmd
md S:\b\a\xctest
cd S:\b\a\xctest
cmake -C S:\language-build\cmake\caches\android-armv7.cmake                                                                ^
  -C S:\language-build\cmake\caches\android-armv7-language-flags.cmake                                                        ^
  -G Ninja                                                                                                              ^
  -DCMAKE_BUILD_TYPE=RelWithDebInfo                                                                                     ^
  -DCMAKE_INSTALL_PREFIX=S:/b/a/Library/Developer/Platforms/android.platform/Developer/SDKs/android.sdk/usr             ^
  -DCMAKE_LANGUAGE_COMPILER=S:/b/a/Library/Developer/Toolchains/unknown-Asserts-development.xctoolchain/usr/bin/languagec.exe ^
  -DCMAKE_TOOLCHAIN_FILE=S:\language-build\cmake\toolchains\android.toolchain.cmake                                        ^
  -DANDROID_ALTERNATE_TOOLCHAIN=S:/b/a/Library/Developer/Toolchains/unknown-Asserts-development.xctoolchain/usr         ^
  -DLANGUAGE_ANDROID_SDK=S:/b/a/Library/Developer/Platforms/andrfoid.platform/Developer/SDKs/android.sdk                   ^
  -DXCTEST_PATH_TO_FOUNDATION_BUILD=S:/b/a/foundation                                                                   ^
  -DXCTEST_PATH_TO_LIBDISPATCH_SOURCE=S:/language-corelibs-libdispatch                                                     ^
  -DXCTEST_PATH_TO_LIBDISPATCH_BUILD=S:/b/a/libdispatch                                                                 ^
  -DENABLE_TESTING=NO                                                                                                   ^
  S:/language-corelibs-foundation
ninja
```

## 1. Install libdispatch

```cmd
cd S:\b\a\libdispatch
ninja install
```

## 1. Install Foundation

```cmd
cd S:\b\a\foundation
ninja install
```

## 1. Install XCTest

```cmd
cd S:\b\a\xctest
ninja install
```

