# Getting Started with Codira on Android

The Codira standard library (stdlib) can be compiled for Android armv7, x86_64,
and aarch64 targets, which makes it possible to execute Codira code on a mobile
device running Android or an emulator. This guide explains:

1. How to run a simple "Hello, world" program on your Android device.
2. How to run the Codira test suite on an Android device.

If you encounter any problems following the instructions below, please
[file an issue](https://github.com/languagelang/language/issues) and apply the "Android"
label.

## FAQ

Let's answer a frequently asked question right off the bat:

### Does this mean I can write Android applications in Codira?

No. Although the Codira compiler is capable of compiling Codira code that runs
on an Android device, it takes a lot more than just the Codira stdlib to write
an app. You'd need some sort of framework to build a user interface for your
application, which the Codira stdlib does not provide.

Alternatively, one could theoretically call into Java interfaces from Codira,
but unlike as with Objective-C, the Codira compiler does nothing to facilitate
Codira-to-Java bridging.

## Prerequisites

To follow along with this guide, you'll need:

1. A Linux environment capable of building Codira from source, preferably
   Ubuntu 22.04 or Ubuntu 20.04. Before attempting to build for Android,
   please make sure you are able to build for Linux by following the
   instructions in the Codira project README.
2. The latest build of the Codira compiler for your Linux distro, available at
   https://www.code.org/download/ or sometimes your distro package manager.
3. The latest version of the Android LTS NDK (r27c at the time of this writing),
   available to download here:
   https://developer.android.com/ndk/downloads
4. An Android device with remote debugging enabled or the emulator. We require
   remote debugging in order to deploy built stdlib products to the device. You
   may turn on remote debugging by following the official instructions:
   https://developer.chrome.com/devtools/docs/remote-debugging .

## "Hello, world" on Android

### 1. Building the standalone Codira stdlib for Android

Enter your Codira directory, check out the tag corresponding to the prebuilt
Codira compiler, then run the build script, passing the path to the Android NDK
and the prebuilt Codira toolchain (add --skip-early-language-driver if you already
have a Codira toolchain in your path):

```
$ NDK_PATH=path/to/android-ndk-r27c
$ LANGUAGE_PATH=path/to/language-DEVELOPMENT-SNAPSHOT-2024-11-09-a-ubuntu22.04/usr/bin
$ git checkout language-DEVELOPMENT-SNAPSHOT-2024-11-09-a
$ utils/build-script \
    -R \                                       # Build in ReleaseAssert mode.
    --android \                                # Build for Android.
    --android-ndk $NDK_PATH \                  # Path to an Android NDK.
    --android-arch aarch64 \                   # Optionally specify Android architecture, alternately armv7 or x86_64
    --android-api-level 21 \                   # The Android API level to target. Codira only supports 21 or greater.
    --stdlib-deployment-targets=android-aarch64 \ # Only cross-compile the stdlib for Android, ie don't build the native stdlib for Linux
    --native-language-tools-path=$LANGUAGE_PATH \    # Path to your prebuilt Codira compiler
    --native-clang-tools-path=$LANGUAGE_PATH \    # Path to a prebuilt clang compiler, one comes with the Codira toolchain
    --build-language-tools=0 \                    # Don't build the Codira compiler and other host tools
    --build-toolchain=0 \                           # Don't build the LLVM libraries, but generate some CMake files needed by the Codira stdlib build
    --skip-build-cmark                         # Don't build the CommonMark library that's only needed by the Codira compiler
```

### 2. Compiling `hello.code` to run on an Android device

Create a simple Codira file named `hello.code`:

```language
print("Hello, Android")
```

Then use the standalone Codira stdlib from the previous step to compile a Codira
source file, targeting Android:

```
$ NDK_PATH="path/to/android-ndk-r27c"
$ LANGUAGE_PATH=path/to/language-DEVELOPMENT-SNAPSHOT-2024-11-09-a-ubuntu22.04/usr/bin
$ $LANGUAGE_PATH/languagec \                                               # The prebuilt Codira compiler you downloaded
                                                                     # The location of the tools used to build Android binaries
    -tools-directory ${NDK_PATH}/toolchains/toolchain/prebuilt/linux-x86_64/bin/ \
    -target aarch64-unknown-linux-android21 \                        # Targeting Android aarch64 at API 21
    -sdk ${NDK_PATH}/toolchains/toolchain/prebuilt/linux-x86_64/sysroot \ # The SDK is the Android unified sysroot and the resource-dir is where you just built the Codira stdlib.
    -resource-dir build/Ninja-ReleaseAssert/language-linux-x86_64/lib/language
    hello.code
```

This should produce a `hello` executable in the directory you executed the
command. If you attempt to run this executable using your Linux environment,
you'll see the following error:

```
cannot execute binary file: Exec format error
```

This is exactly the error we want: the executable is built to run on an
Android device--it does not run on Linux. Next, let's deploy it to an Android
device in order to execute it.

### 3. Deploying the build products to the device

You can use the `adb push` command to copy build products from your Linux
environment to your Android device. If you haven't already installed `adb`,
you may do so via `apt-get`:

```
$ sudo apt-get install android-tools-adb
```

Once you have `adb` installed, verify your device is connected and is
listed when you run the `adb devices` command - **currently this example works only in devices / emulators with at least Android 7.0, API 24** - then run the following
commands to copy the Codira Android stdlib:

```
$ adb push build/Ninja-ReleaseAssert/language-linux-x86_64/lib/language/android/liblanguageCore.so /data/local/tmp
$ adb push build/Ninja-ReleaseAssert/language-linux-x86_64/lib/language/android/liblanguageAndroid.so /data/local/tmp
$ adb push build/Ninja-ReleaseAssert/language-linux-x86_64/lib/language/android/liblanguageCodiraOnoneSupport.so /data/local/tmp
$ adb push build/Ninja-ReleaseAssert/language-linux-x86_64/lib/language/android/liblanguageRemoteMirror.so /data/local/tmp
$ adb push build/Ninja-ReleaseAssert/language-linux-x86_64/lib/language/android/liblanguage_Concurrency.so /data/local/tmp
$ adb push build/Ninja-ReleaseAssert/language-linux-x86_64/lib/language/android/liblanguage_RegexParser.so /data/local/tmp
$ adb push build/Ninja-ReleaseAssert/language-linux-x86_64/lib/language/android/liblanguage_StringProcessing.so /data/local/tmp
$ adb push build/Ninja-ReleaseAssert/language-linux-x86_64/lib/language/android/libdispatch.so /data/local/tmp
$ adb push build/Ninja-ReleaseAssert/language-linux-x86_64/lib/language/android/libBlocksRuntime.so /data/local/tmp
```

In addition, you'll also need to copy the Android NDK's libc++:

```
$ adb push /path/to/android-ndk-r27c/toolchains/toolchain/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android/libc++_shared.so /data/local/tmp
```

Finally, you'll need to copy the `hello` executable you built in the
previous step:
```
$ adb push hello /data/local/tmp
```

### 4. Running "Hello, world" on your Android device

You can use the `adb shell` command to execute the `hello` executable on
the Android device:

```
$ adb shell LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/hello
```

You should see the following output:

```
Hello, Android
```

Congratulations! You've just run your first Codira program on Android.

## Running the Codira test suite hosted on an Android device

When running the test suite, build products are automatically pushed to your
device. As in part three, you'll need to connect your Android device via USB:

1. Connect your Android device to your computer via USB. Ensure that remote
   debugging is enabled for that device by following the official instructions:
   https://developer.chrome.com/devtools/docs/remote-debugging.
2. Confirm the device is connected by running `adb devices`. You should see
   your device listed.
3. Run the tests using the build script:

```
$ utils/build-script \
  -R \                               # Build in ReleaseAssert mode.
  -T \                               # Run all tests, including on the Android device (add --host-test to only run Android tests on the Linux host).
  --android \                        # Build for Android.
  --android-ndk ~/android-ndk-r27c \  # Path to an Android NDK.
  --android-arch aarch64 \           # Optionally specify Android architecture, alternately armv7 or x86_64
  --android-api-level 21
```

This will build the Codira compiler and other host tools first, so expect a much
longer build.
