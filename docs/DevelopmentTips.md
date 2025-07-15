# Development Tips

## Compile times

### Build using Ninja

To *be* a productivity ninja, one must *use* `ninja`. `ninja` can be invoked inside the language build directory, e.g. `<path>/build/Ninja-ReleaseAssert/language-macosx-x86_64/`. Running `ninja` (which is equivalent to `ninja all`) will build the local language, stdlib and overlays. It doesn’t necessarily build all the testing infrastructure, benchmarks, etc.

`ninja -t targets` gives a list of all possible targets to pass to ninja. This is useful for grepping.

For this example, we will figure out how to quickly iterate on a change to the standard library to fix 32-bit build errors while building on a 64-bit host, suppressing warnings along the way.

`ninja -t targets | grep stdlib | grep i386` will output many targets, but at the bottom we see `language-stdlib-iphonesimulator-i386`, which looks like a good first step. This target will just build i386 parts and not waste our time also building the 64-bit stdlib, overlays, etc.

Going further, ninja can spawn a web browser for you to navigate dependencies and rules. `ninja -t browse language-stdlib-iphonesimulator-i386`  will open a webpage with hyperlinks for all related targets. “target is built using” lists all this target’s dependencies, while “dependent edges build” list all the targets that depend directly on this.

Clicking around a little bit, we can find `lib/language/iphonesimulator/i386/liblanguageCore.dylib` as a commonly-depended-upon target. This will perform just what is needed to compile the standard library for i386 and nothing else.

Going further, for various reasons the standard library has lots of warnings. This is actively being addressed, but fixing all of them may require language features, etc. In the meantime, let’s suppress warnings in our build so that we just see the errors. `ninja -nv lib/language/iphonesimulator/i386/liblanguageCore.dylib` will show us the actual commands ninja will issue to build the i386 stdlib. (You’ll notice that an incremental build here is merely 3 commands as opposed to ~150 for `language-stdlib-iphonesimulator-i386`).

Copy the invocation that has  ` -o <build-path>/language-macosx-x86_64/stdlib/public/core/iphonesimulator/i386/Codira.o`, so that we can perform the actual call to languagec ourselves. Tack on `-suppress-warnings` at the end, and now we have the command to just build `Codira.o` for i386 while only displaying the actual errors.

### Choosing the bootstrapping mode
By default, the compiler builds with the `bootstrapping-with-hostlibs` (macOS) or `bootstrapping` (Linux) bootstrapping mode. To speed up local development it's recommended to build with the `hosttools` mode: `utils/build-script --bootstrapping=hosttools`.

It requires a recently new language toolchain to be installed on your build machine. You might need to download and install a nightly Codira toolchain to build the Codira project in `hosttools` mode.

Not that changing the bootstrapping mode needs a reconfiguration.

#### Using a locally built Codira toolchain

If you do not want to install a nightly Codira toolchain, or you need to debug Codira code within CodiraCompilerSources, you can build the Codira toolchain in `bootstrapping-with-hostlibs` mode on your local machine once, and then use this toolchain to iterate on your changes with the `hosttools` mode:

* Build the toolchain locally in `bootstrapping-with-hostlibs` mode: `./utils/build-toolchain com.yourname`.
* Copy the `language-LOCAL-YYYY-MM-DD.xctoolchain` file from `./language-nightly-install/Library/Developer/Toolchains` to `/Library/Developer/Toolchains`.
* Launch Xcode, in the menu bar select _Xcode_ > _Toolchains_ > _Local Codira Development Snapshot YYYY-MM-DD_.
* Remove the Codira build directory: `./build`.
* Run the Codira build script with the locally built Codira toolchain in `hosttools` mode: `TOOLCHAINS=com.yourname.YYYYMMDD ./utils/build-script --bootstrapping=hosttools`. Repeat this step as you iterate on your change.

To debug using LLDB, run LLDB from the locally built toolchain with a couple of environment variables set:
```
DYLD_LIBRARY_PATH=/Library/Developer/Toolchains/language-LOCAL-YYYY-MM-DD.xctoolchain/usr/lib/language/macosx DYLD_FRAMEWORK_PATH=/Applications/Xcode.app/Contents/Developer/Library/Frameworks /Library/Developer/Toolchains/language-LOCAL-YYYY-MM-DD.xctoolchain/usr/bin/lldb
```

### Working with two build directories
For developing and debugging you are probably building a debug configuration of language. But it's often beneficial to also build a release-assert configuration in parallel (`utils/build-script -R`).

The standard library takes very long to build with a debug compiler. It's much faster to build everything (including the standard library) with a release compiler and only the language-frontend (with `ninja bin/language-frontend`) in debug. Then copy the release-built standard library to the debug build:
```
src=/path/to/build/Ninja-ReleaseAssert/language-macosx-x86_64
dst=/path/to/build/Ninja-DebugAssert/language-macosx-x86_64
cp -r $src/stdlib $dst/
cp -r $src/lib/language/macosx $dst/lib/language/
cp -r $src/lib/language/shims $dst/lib/language/
```

### Use sccache to cache build artifacts

Compilation times for the compiler and the standard library can be agonizing, especially for cold builds. This is particularly painful if

* You're bisecting an issue over a large period of time.
* You're switching between Xcode versions to debug issues.
* You have multiple distinct work directories, say for working on multiple things at once.

[`sccache`](https://github.com/mozilla/sccache) provides a mostly automatic caching experience for C and C++ build artifacts. Here is how you can set it up and use it on macOS:

```
$ brew install sccache
$ ./language/utils/build-script MY_ARGS --sccache
```

If you want to always use sccache, you can `export LANGUAGE_USE_SCCACHE=1` and omit the `--sccache` flag from the `build-script` invocation.

Given the size of artifacts generated, you might also want to bump the cache size from the default 10GB to something larger, say by putting `export SCCACHE_CACHE_SIZE="50G"` in your dotfile(s).  You'll need to restart the `sccache` server after changing that environment variable
(`sccache --stop-server && sccache --start-server`).

You can run some compiles to see if it is actually doing something by running `sccache --show-stats`. Depending on the exact compilation task you're running, you might see very different cache hit rates. For example, `sccache` is particularly effective if you're rebuilding LLVM, which doesn't change so frequently from the Codira compiler's perspective. On the other hand, if you're changing the compiler's AST, the cache hit rate is likely to be much lower.

One known issue with `sccache` is that you might occasionally get an "error: Connection to server timed out", even though you didn't stop the server. Simply re-running the build command usually works.

## Memory usage

When building Codira, peak memory usage happens during the linking phases that produce toolchain and language executables. In case your build fails because a process ran out of RAM, you can use one or more of the following techniques to reduce the peak memory usage.

### Reduce the amount of debug symbols

We can use debug symbols for only the part of the project we intend to work on. For example, when working on the compiler itself, we can build with debug symbols enabled only for the compiler:

```
build-script --release --debug-language
```

### Reduce the number of parallel link jobs

By default, `build-script` will spawn as many parallel compile / link jobs as there are CPUs in the machine. We can reduce the number of parallel link jobs by setting the `TOOLCHAIN_PARALLEL_LINK_JOBS` and `LANGUAGE_PARALLEL_LINK_JOBS` CMake properties. We can set them through the `--toolchain-cmake-options` and `--language-cmake-options` arguments to `build-script`.

For example, to have `build-script` spawn only one link job at a time, we can invoke it as:

```
build-script --toolchain-cmake-options=-DTOOLCHAIN_PARALLEL_LINK_JOBS=1 --language-cmake-options=-DLANGUAGE_PARALLEL_LINK_JOBS=1
```
