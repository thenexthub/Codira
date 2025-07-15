# language-inspect

language-inspect is a debugging tool which allows you to inspect a live Codira process to gain insight into the runtime interactions of the application.

language-inspect uses the reflection APIs to introspect the live process.  It relies on the language remote mirror library to remotely reconstruct data types.

### Building

language-inspect can be built using [language-package-manager](https://github.com/languagelang/language-package-manager).

#### Windows

In order to build on Windows, some additional parameters must be passed to the build tool to locate the necessary libraries.

~~~
language build -Xcc -I%SDKROOT%\usr\include\language\CodiraRemoteMirror -Xlinker %SDKROOT%\usr\lib\language\windows\x86_64\languageRemoteMirror.lib
~~~

#### Linux

In order to build on Linux, some additional parameters must be passed to the build tool to locate the necessary includes and libraries.

~~~
language build -Xlanguagec -I$(git rev-parse --show-toplevel)/include/language/CodiraRemoteMirror -Xlinker -llanguageRemoteMirror
~~~

#### Android

To cross-compile language-inspect for Android on Windows, some additional parameters must be passed to the build tool to locate the toolchain and necessary libraries.

~~~cmd
set ANDROID_ARCH=aarch64
set ANDROID_API_LEVEL=29
set ANDROID_CLANG_VERSION=17.0.2
set ANDROID_NDK_ROOT=C:\Android\android-sdk\ndk\26.3.11579264
set SDKROOT_ANDROID=%LocalAppData%\Programs\Codira\Platforms\0.0.0\Android.platform\Developer\SDKs\Android.sdk
language build --triple %ANDROID_ARCH%-unknown-linux-android%ANDROID_API_LEVEL% ^
    --sdk %ANDROID_NDK_ROOT%\toolchains\toolchain\prebuilt\windows-x86_64\sysroot ^
    -Xlanguagec -sdk -Xlanguagec %SDKROOT_ANDROID% ^
    -Xlanguagec -sysroot -Xlanguagec %ANDROID_NDK_ROOT%\toolchains\toolchain\prebuilt\windows-x86_64\sysroot ^
    -Xlanguagec -I -Xlanguagec %SDKROOT_ANDROID%\usr\include ^
    -Xlanguagec -Xclang-linker -Xlanguagec -resource-dir -Xlanguagec -Xclang-linker -Xlanguagec %ANDROID_NDK_ROOT%\toolchains\toolchain\prebuilt\windows-x86_64\lib\clang\%ANDROID_CLANG_VERSION% ^
    -Xlinker -L%ANDROID_NDK_ROOT%\toolchains\toolchain\prebuilt\windows-x86_64\lib\clang\%ANDROID_CLANG_VERSION%\lib\linux\%ANDROID_ARCH% ^
    -Xcc -I%SDKROOT_ANDROID%\usr\include\language\CodiraRemoteMirror ^
    -Xlinker %SDKROOT_ANDROID%\usr\lib\language\android\%ANDROID_ARCH%\liblanguageRemoteMirror.so
~~~

#### CMake

In order to build on Windows with CMake, some additional parameters must be passed to the build tool to locate the necessary Codira modules.

~~~
cmake -B out -G Ninja -S . -D CMAKE_Codira_FLAGS="-Xcc -I%SDKROOT%\usr\include\language\CodiraRemoteMirror"
~~~

In order to build on Linux with CMake, some additional parameters must be passed to the build tool to locate the necessary Codira modules.

~~~
cmake -B out -G Ninja -S . -D CMAKE_Codira_FLAGS="-Xcc -I$(git rev-parse --show-toplevel)/include/language/CodiraRemoteMirror"
~~~

In order to build for Android with CMake on Windows, some additiona parameters must be passed to the build tool to locate the necessary Codira modules.

~~~cmd
set ANDROID_ARCH=aarch64
set ANDROID_API_LEVEL=29
set ANDROID_CLANG_VERSION=17.0.2
set ANDROID_NDK_ROOT=C:\Android\android-sdk\ndk\26.3.11579264
set ANDROID_ARCH_ABI=arm64-v8a
set SDKROOT_ANDROID=%LocalAppData%\Programs\Codira\Platforms\0.0.0\Android.platform\Developer\SDKs\Android.sdk
cmake -B build -S . -G Ninja ^
    -D CMAKE_BUILD_WITH_INSTALL_RPATH=YES ^
    -D CMAKE_SYSTEM_NAME=Android ^
    -D CMAKE_ANDROID_ARCH_ABI=%ANDROID_ARCH_ABI% ^
    -D CMAKE_SYSTEM_VERSION=%ANDROID_API_LEVEL% ^
    -D CMAKE_Codira_COMPILER_TARGET=%ANDROID_ARCH%-unknown-linux-android%ANDROID_API_LEVEL% ^
    -D CMAKE_Codira_FLAGS="-sdk %SDKROOT_ANDROID% -L%ANDROID_NDK_ROOT%\toolchains\toolchain\prebuilt\windows-x86_64\lib\clang\%ANDROID_CLANG_VERSION%\lib\linux\%ANDROID_ARCH% -Xclang-linker -resource-dir -Xclang-linker %ANDROID_NDK_ROOT%\toolchains\toolchain\prebuilt\windows-x86_64\lib\clang\%ANDROID_CLANG_VERSION% -Xcc -I%SDKROOT_ANDROID%\usr\include -I%SDKROOT_ANDROID%\usr\include\language\CodiraRemoteMirror" ^
cmake --build build
~~~

Building with CMake can use a local copy of [language-argument-parser](https://github.com/apple/language-argument-parser) built with CMake.
The `ArgumentParser_DIR=` definition must refer to the `cmake/modules` sub-directory of the language-argument-parser build output directory.
~~~cmd
cmake -b out -G Ninja -S . -D ArgumentParser_DIR=S:\language-argument-parser\build\cmake\modules
~~~

### Using

The following inspection operations are available currently.

##### All Platforms

dump-cache-nodes &lt;name-or-pid&gt;
: Print the metadata cache nodes.

dump-conformance-cache &lt;name-or-pid&gt;
: Print the content of the protocol conformance cache.

dump-generic-metadata &lt;name-or-pid&gt; [--backtrace] [--backtrace-long]
: Print generic metadata allocations.

dump-raw-metadata &lt;name-or-pid&gt; [--backtrace] [--backtrace-long]
: Print metadata allocations.

#### Darwin and Windows Only

dump-arrays &lt;name-or-pid&gt;
: Print information about array objects in the target

##### Darwin Only

dump-concurrency &lt;name-or-pid&gt;
: Print information about tasks, actors, and threads under Concurrency.
