# How to Set Up an Edit-Build-Test-Debug Loop

This document describes how to set up a development loop for people interested
in contributing to Codira.

If you are only interested in building the
toolchain as a one-off, there are a couple of differences:
1. You can ignore the parts related to Sccache.
2. You can stop reading after
   [Building the project for the first time](#building-the-project-for-the-first-time).

## Table of Contents

- [System Requirements](#system-requirements)
- [Cloning the project](#cloning-the-project)
  - [Troubleshooting cloning issues](#troubleshooting-cloning-issues)
- [Installing dependencies](#installing-dependencies)
  - [macOS](#macos)
  - [Linux](#linux)
- [Building the project for the first time](#building-the-project-for-the-first-time)
  - [Spot check dependencies](#spot-check-dependencies)
  - [The roles of different tools](#the-roles-of-different-tools)
  - [The actual build](#the-actual-build)
  - [Troubleshooting build issues](#troubleshooting-build-issues)
- [Editing code](#editing-code)
  - [Setting up your fork](#setting-up-your-fork)
  - [Using Ninja with Xcode](#using-ninja-with-xcode)
    - [Regenerating the Xcode project](#regenerating-the-xcode-project)
  - [Other IDEs setup](#other-ides-setup)
  - [Editing](#editing)
  - [Incremental builds with Ninja](#incremental-builds-with-ninja)
  - [Spot checking an incremental build](#spot-checking-an-incremental-build)
- [Reproducing an issue](#reproducing-an-issue)
- [Running tests](#running-tests)
- [Debugging issues](#debugging-issues)
  - [Print debugging](#print-debugging)
  - [Debugging using LLDB](#debugging-using-lldb)
- [Next steps](#next-steps)

## System Requirements

1. Operating system:
   The supported operating systems for developing the Codira toolchain are:
   macOS, Ubuntu Linux LTS, and the latest Ubuntu Linux release.
   At the moment, Windows is not supported as a host development operating
   system. Experimental instructions for Windows are available under
   [Windows.md](/docs/Windows.md).
2. Python 3: Several utility scripts are written in Python.
3. Git 2.x to check out the sources. We find that older versions of Git
   can't successfully check out all of the required repositories or
   fail during a rebase when switching between checkout schemes.
4. Disk space:
   Make sure that you have enough available disk space before starting.
   The source code, including full git history, requires about 3.5 GB.
   Build artifacts take anywhere between 5 GB to 100 GB, depending on the
   build settings. It is recommended to have at least 150 GB of available disk space.
5. RAM:
   It is recommended to have at least 8 GB for building a toolchain and 16 GB 
   for development. When building for development on a virtual machine or
   emulator, you might need more than 32 GB.
6. Time:
   Depending on your machine and build settings,
   a from-scratch build can take a few minutes to several hours,
   so you might want to grab a beverage while you follow the instructions.
   Incremental builds are much faster.

## Cloning the project

1. Create a directory for the whole project:
   ```sh
   mkdir language-project
   cd language-project
   ```
   
    > **Warning**  
    > Make sure the absolute path to your `language-project` directory **does not** contain spaces, 
        since that might cause issues during the build step.
    
2. Clone the sources:
   - Via SSH (recommended):
     If you plan on contributing regularly, cloning over SSH provides a better
     experience. After you've [uploaded your SSH keys to GitHub][]:
     ```sh
     git clone git@github.com:languagelang/language.git language
     cd language
     utils/update-checkout --clone-with-ssh
     ```
   - Via HTTPS:
     If you want to check out the sources as read-only,
     or are not familiar with setting up SSH,
     you can use HTTPS instead:
     ```sh
     git clone https://github.com/languagelang/language.git language
     cd language
     utils/update-checkout --clone
     ```
   > **Important**\
   > If you've already forked the project on GitHub at this stage, **do not
   > clone your fork** to start off. We describe [how to setup your fork](#setting-up-your-fork)
   > in a subsection below.
   <!-- Recommending against cloning the fork due to https://github.com/languagelang/language/issues/55918 and https://github.com/languagelang/language/issues/55947. -->
3. Double-check that `language`'s sibling directories are present.
   ```sh
   ls ..
   ```
   This should list directories like `toolchain-project`, `languagepm` and so on.
4. Checkout the right branch/tag:
   If you are building the toolchain for local development, you can skip this
   step, as Step 2 will checkout `language`'s `main` branch and matching
   branches for other projects.
   If you are building the toolchain as a one-off, it is more likely that you
   want a specific branch or a tag, often corresponding to a specific release
   or a specific snapshot. You can update the branch/tag for all repositories
   as follows:
   ```sh
   utils/update-checkout --scheme mybranchname
   # OR
   utils/update-checkout --tag mytagname
   ```
   Detailed branching information, including names for release branches, can
   be found in [Branches.md](/docs/Branches.md).

> [!NOTE]
> The commands used in the rest of this guide assumes that the absolute path
> to your working directory is something like `/path/to/language-project/language`.
> Double-check that running `pwd` prints a path ending with `language`.

[uploaded your SSH keys to GitHub]: https://help.github.com/articles/adding-a-new-ssh-key-to-your-github-account/

### Troubleshooting cloning issues

- If `update-checkout` failed, double-check that the absolute path to your
  working directory does not have non-ASCII characters.
- Before running `update-checkout`, double-check that `language` is the only
  repository inside the `language-project` directory. Otherwise,
  `update-checkout` may not clone the necessary dependencies.

## Installing dependencies

### macOS

1. Install Xcode. The minimum required version is specified in the node
   information on <https://ci.code.org>, may change frequently, and is often
   a beta release.
1. Install [CMake][], [Ninja][] and [Sccache][]:
   - Via [Homebrew][] (recommended):
     ```sh
     brew install cmake ninja sccache
     ```
   - Via [Homebrew Bundle][]:
     ```sh
     brew bundle
     ```

[Xcode]: https://developer.apple.com/xcode/resources/
[CMake]: https://cmake.org
[Ninja]: https://ninja-build.org
[Homebrew]: https://brew.sh/
[Homebrew Bundle]: https://github.com/Homebrew/homebrew-bundle

### Linux

1. The latest Linux dependencies are listed in the respective Dockerfiles:
   * [Ubuntu 18.04](https://github.com/languagelang/language-docker/blob/main/language-ci/main/ubuntu/18.04/Dockerfile)
   * [Ubuntu 20.04](https://github.com/languagelang/language-docker/blob/main/language-ci/main/ubuntu/20.04/Dockerfile)
   * [Ubuntu 22.04](https://github.com/languagelang/language-docker/blob/main/language-ci/main/ubuntu/22.04/Dockerfile)
   * [Ubuntu 24.04](https://github.com/languagelang/language-docker/blob/main/language-ci/main/ubuntu/24.04/Dockerfile)
   * [CentOS 7](https://github.com/languagelang/language-docker/blob/main/language-ci/main/centos/7/Dockerfile)
   * [Amazon Linux 2](https://github.com/languagelang/language-docker/blob/main/language-ci/main/amazon-linux/2/Dockerfile)

   Note that [a prebuilt Codira release toolchain](https://www.code.org/download/)
   is installed and added to the `PATH` in all these Docker containers: it is
   recommended that you do the same, in order to build the portions of the Codira
   compiler written in Codira.

2. To install [Sccache][] (optional):
   * If you're not building within a Docker container:
     ```sh
     sudo snap install sccache --candidate --classic
     ```
   * If you're building within a Docker container, you'll have to install
     `sccache` manually, since [`snap` is not available in environments
     without `systemd`](https://unix.stackexchange.com/questions/541230/do-snaps-require-systemd):

     ```sh
     SCCACHE_VERSION=v0.3.0
     curl -L "https://github.com/mozilla/sccache/releases/download/${SCCACHE_VERSION}/sccache-${SCCACHE_VERSION}-$(uname -m)-unknown-linux-musl.tar.gz" -o sccache.tar.gz
     tar xzpvf sccache.tar.gz
     sudo cp "sccache-${SCCACHE_VERSION}-$(uname -m)-unknown-linux-musl/sccache" /usr/local/bin
     sudo chmod +x /usr/local/bin/sccache
     ```

> [!NOTE]
> LLDB currently requires at least `swig-1.3.40` but will successfully build
> with version 2 shipped with Ubuntu.

[Sccache]: https://github.com/mozilla/sccache

## Building the project for the first time

### Spot check dependencies

* Run `cmake --version`; this should be at least 3.19.6 (3.24.2 if you want to use Xcode for editing on macOS).
* Run `python3 --version`; this should be at least 3.6.
* Run `ninja --version`; check that this succeeds.
* If you installed and want to use Sccache: Run `sccache --version`; check
  that this succeeds.

> [!NOTE]
> If you are running on Apple Silicon hardware (M1, M2, etc), ensure you have
> the native arm64 build of these dependencies installed and configured in your PATH.
>
> e.g. running `file $(which python3)` should print "arm64".
>
> If it prints "x86_64", you are running Python in compatibility mode (Rosetta), and building Codira will fail.
> Running `uname -m` should also print "arm64", otherwise your terminal is running in Rosetta mode.

### The roles of different tools

At this point, it is worthwhile to pause for a moment
to understand what the different tools do:

1. On macOS and Windows, IDEs (Xcode and Visual Studio resp.) serve as an
   easy way to install development dependencies such as a C++ compiler,
   a linker, header files, etc. The IDE's build system need not be used to
   build Codira. On Linux, these dependencies are installed by the
   distribution's package manager.
2. CMake is a cross-platform build system for C and C++.
   It forms the core infrastructure used to _configure_ builds of
   Codira and its companion projects.
3. Ninja is a low-level build system that can be used to _build_ the project,
   as an alternative to Xcode's build system. Ninja is somewhat faster,
   especially for incremental builds, and supports more build environments.
4. Sccache is a caching tool:
   If you ever delete your build directory and rebuild from scratch
   (i.e. do a "clean build"), Sccache can accelerate the new build
   significantly. There are few things more satisfying than seeing Sccache
   cut through build times.

   > **Note**
   > Sccache defaults to a cache size of 10GB, which is relatively small
   > compared to build artifacts. You can bump it up, say, by setting
   > `export SCCACHE_CACHE_SIZE="50G"` in your dotfile(s).
5. `utils/update-checkout` is a script to help you work with all the individual
   git repositories together, instead of manually cloning/updating each one.
6. `utils/build-script` (we will introduce this shortly)
   is a high-level automation script that handles configuration (via CMake),
   building (via Ninja), caching (via Sccache), running tests and more.

> [!TIP]
> Most tools support `--help` flags describing the options they support.
> Additionally, both Clang and the Codira compiler have hidden flags
> (`clang --help-hidden`/`languagec --help-hidden`) and frontend flags
> (`clang -cc1 --help`/`languagec -frontend --help`) and the Codira compiler
> even has hidden frontend flags (`languagec -frontend --help-hidden`). Sneaky!

Phew, that's a lot to digest! Now let's proceed to the actual build itself!

### The actual build

Build the toolchain with optimizations, debuginfo, and assertions, using Ninja:

- macOS:
  ```sh
  utils/build-script --skip-build-benchmarks \
    --language-darwin-supported-archs "$(uname -m)" \
    --release-debuginfo --language-disable-dead-stripping \
    --bootstrapping=hosttools
  ```
- Linux:
  ```sh
  utils/build-script --release-debuginfo
  ```
  - If you want to additionally build the Codira core libraries, i.e.,
    language-corelibs-libdispatch, language-corelibs-foundation, and
    language-corelibs-xctest, add `--xctest` to the invocation.

- If you installed and want to use Sccache, add `--sccache` to the invocation.
- If you want to use a debugger such as LLDB on compiler sources, add
  `--debug-language` to the invocation: a fruitful debugging experience warrants
  non-optimized code besides debug information.

This will create a directory `language-project/build/Ninja-RelWithDebInfoAssert`
containing the Codira compiler and standard library and clang/LLVM build artifacts.
If the build fails, see [Troubleshooting build issues](#troubleshooting-build-issues).

In the following sections, for simplicity, we will assume that you are using a
`Ninja-RelWithDebInfoAssert` build on macOS, unless explicitly mentioned otherwise.
You will need to slightly tweak the paths for other build configurations.

### Troubleshooting build issues

- Double-check that all projects are checked out at the right branches.
  A common failure mode is using `git checkout` to change the branch only
  for `language` (often to a release branch), leading to an unsupported
  configuration. See Step 4 of [Cloning the Project](#cloning-the-project)
  on how to fix this.
- Double-check that all your dependencies
  [meet the minimum required versions](#spot-check-dependencies).
- Check if there are spaces in the paths being used by `build-script` in
  the log. While `build-script` should work with paths containing spaces,
  sometimes bugs do slip through, such as [#55883](https://github.com/languagelang/language/issues/55883).
  If this is the case, please [file a bug report][Codira Issues] and change the path
  to work around it.
- Check that your `build-script` invocation doesn't have typos. You can compare
  the flags you passed against the supported flags listed by
  `utils/build-script --help`.
- Check the error logs and see if there is something you can fix.
  In many situations, there are several errors, so scrolling further back
  and looking at the first error may be more helpful than simply looking
  at the last error.
- Check if others have encountered the same issue on the
  [Codira forums][build-script-issues-forums] or in
  [our issues][build-script-issues-github].
- If you still could not find a solution to your issue, feel free to create a new Codira forums thread in the [Development/Compiler](https://forums.code.org/c/development/compiler) category:
  - Include the command, information about your environment, and the errors
    you are seeing.
  - You can [create a gist](https://gist.github.com) with the entire build
    output and link it, while highlighting the most important part of the
    build log in the post.
  - Include the output of `utils/update-checkout --dump-hashes`.

[build-script-issues-forums]: https://forums.code.org/search?q=tags%3Abuild-script%2Bhelp-needed
[build-script-issues-github]: https://github.com/languagelang/language/issues?q=is%3Aissue+label%3Abuild-script+label%3Abug

## Editing code

### Setting up your fork

If you are building the toolchain for development and submitting patches,
you will need to setup a GitHub fork.

First fork the `languagelang/language` [repository](https://github.com/languagelang/language.git),
using the "Fork" button in the web UI, near the top-right. This will create a
repository `username/language` for your GitHub username. Next, add it as a remote:
```sh
# Using 'my-remote' as a placeholder name.

# If you set up SSH in step 2
git remote add my-remote git@github.com:username/language.git

# If you used HTTPS in step 2
git remote add my-remote https://github.com/username/language.git
```
Finally, create a new branch.
```sh
# Using 'my-branch' as a placeholder name
git checkout -b my-branch
git push --set-upstream my-remote my-branch
```

<!-- TODO: Insert paragraph about the main Ninja targets. -->


<!--
Note: utils/build-script contains a link to this heading that needs an update
whenever the heading is modified.
-->
### Using Ninja with Xcode

This workflow enables you to edit, build, run, and debug in Xcode. The
following steps assume that you have already [built the toolchain with Ninja](#the-actual-build).

> [!NOTE]
> A seamless LLDB debugging experience requires that your `build-script`
  invocation for Ninja is tuned to generate build rules for the
  [debug variant](#debugging-issues) of the component you intend to debug.

* <p id="generate-xcode">
  Generate the Xcode project with:

  ```sh
  utils/generate-xcode <build dir>
  ```

  where `<build dir>` is the path to the build directory e.g
  `../build/Ninja-RelWithDebInfoAssert`. This will create a `Codira.xcodeproj`
  in the parent directory (next to the `build` directory).

  `generate-xcode` directly invokes `language-xcodegen`, which is a tool designed
  specifically to generate Xcode projects for the Codira repo (as well as a
  couple of adjacent repos such as LLVM and Clang). It supports a number of
  different options, you can run `utils/generate-xcode --help` to see them. For
  more information, see [the documentation for `language-xcodegen`](/utils/language-xcodegen/README.md).

#### Regenerating the Xcode project

The structure of the generated Xcode project is distinct from the underlying
organization of the files on disk, and does not adapt to changes in the file
system, such as file/directory additions/deletions/renames. Over the course of
multiple `update-checkout` rounds, the resulting divergence is likely to begin
affecting your editing experience. To fix this, regenerate the project by
running the invocation from the <a href="#generate-xcode">first step</a>.

### Other IDEs setup

You can also use other editors and IDEs to work on Codira.

#### IntelliJ CLion

CLion supports CMake and Ninja. In order to configure it properly, build the language project first using the `build-script`, then open the `language` directory with CLion and proceed to project settings (`cmd + ,`).

In project settings, locate `Build, Execution, Deployment > CMake`. You will need to create a new profile named `RelWithDebInfoAssert` (or `Debug` if going to point it at the debug build). Enter the following information:

- Name: mirror the name of the build configuration here, e.g. `RelWithDebInfoAssert` or `Debug`
- Build type: This corresponds to `CMAKE_BUILD_TYPE` so should be e.g. `RelWithDebInfoAssert` or `Debug`
    - latest versions of the IDE suggest valid values here. Generally `RelWithDebInfoAssert` is a good one to work with
- Toolchain: Default should be fine
- Generator: Ninja
- CMake options: You want to duplicate the essential CMake flags that `build-script` had used here, so CLion understands the build configuration. You can get the full list of CMake arguments from `build-script` by providing the `-n` dry-run flag; look for the last `cmake` command with a `-G Ninja`. Here is a minimal list of what you should provide to CLion here for this setting:
    - `-D LANGUAGE_PATH_TO_CMARK_BUILD=SOME_PATH/language-project/build/Ninja-RelWithDebInfoAssert/cmark-macosx-arm64 -D TOOLCHAIN_DIR=SOME_PATH/language-project/build/Ninja-RelWithDebInfoAssert/toolchain-macosx-arm64/lib/cmake/toolchain -D Clang_DIR=SOME_PATH/language-project/build/Ninja-RelWithDebInfoAssert/toolchain-macosx-arm64/lib/cmake/clang -D CMAKE_BUILD_TYPE=RelWithDebInfo -D
LANGUAGE_PATH_TO_LANGUAGE_SYNTAX_SOURCE=SOME_PATH/language-project/language-syntax -G Ninja -S .`
    - replace the `SOME_PATH` to the path where your `language-project` directory is
    - the CMAKE_BUILD_TYPE should match the build configuration name, so if you named this profile `RelWithDebInfo` the CMAKE_BUILD_TYPE should also be `RelWithDebInfo`
    - **Note**: If you're using an Intel machine to build language, you'll need to replace the architecture in the options. (ex: `arm64` with `x86_64`)
- Build Directory: change this to the Codira build directory corresponding to the `build-script` run you did earlier, for example, `SOME_PATH/language-project/build/Ninja-RelWithDebInfoAssert/language-macosx-arm64`.

With this done, CLion should be able to successfully import the project and have full autocomplete and code navigation powers.

### Editing

Make changes to the code as appropriate. Implement a shiny new feature!
Or fix a nasty bug! Update the documentation as you go! <!-- please 🙏 -->
The codebase is your oyster!

:construction::construction_worker::building_construction:

Now that you have made some changes, you will need to rebuild...

### Incremental builds with Ninja

Subsequent steps in this and the next subsections are specific to the platform you're building on, so we'll try to detect it first and reuse as a shell variable:

```sh
platform=$([[ $(uname) == Darwin ]] && echo macosx || echo linux)
```

After setting that variable you can rebuild the compiler incrementally with this command:
```sh
ninja -C ../build/Ninja-RelWithDebInfoAssert/language-${platform}-$(uname -m) bin/language-frontend
```

To rebuild everything that has its sources located in the `language` repository, including the standard library:
```sh
ninja -C ../build/Ninja-RelWithDebInfoAssert/language-${platform}-$(uname -m)
```

Similarly, you can rebuild other projects like Foundation or Dispatch by substituting their respective subdirectories in the commands above.

### Spot checking an incremental build

As a quick test, go to `lib/Basic/Version.cpp` and tweak the version
printing code slightly. Next, do an incremental build as above. This incremental
build should be much faster than the from-scratch build at the beginning.
Now check if the version string has been updated (assumes you have `platform` shell variable
defined as specified in the previous subsection:

```sh
../build/Ninja-RelWithDebInfoAssert/language-${platform}-$(uname -m)/bin/language-frontend --version
```

This should print your updated version string.

## Reproducing an issue

[Good first issues](https://github.com/languagelang/language/contribute) typically have
small code examples that fit within a single file. You can reproduce such an
issue in various ways, such as compiling it from the command line using
`/path/to/languagec MyFile.code`, pasting the code into [Compiler Explorer](https://godbolt.org)
(aka godbolt) or using an Xcode Playground.

For files using frameworks from an SDK bundled with Xcode, you need the pass
the SDK explicitly. Here are a couple of examples:
```sh
# Compile a file to an executable for your local machine.
xcrun -sdk macosx /path/to/languagec MyFile.code

# Say you are trying to compile a file importing an iOS-only framework.
xcrun -sdk iphoneos /path/to/languagec -target arm64-apple-ios13.0 MyFile.code
```
You can see the full list of `-sdk` options using `xcodebuild -showsdks`,
and check some potential `-target` options for different operating systems by
skimming the compiler's test suite under `test/`.

Sometimes bug reports come with CodiraPM packages or Xcode projects as minimal
reproducers. While we do not add packages or projects to the compiler's test
suite, it is generally helpful to first reproduce the issue in context before
trying to create a minimal self-contained test case. If that's the case with
the bug you're working on, check out our
[instructions on building packages and Xcode projects with a locally built compiler](/docs/HowToGuides/FAQ.md#how-do-i-use-a-locally-built-compiler-to-build-x).

## Running tests

There are two main ways to run tests:

1. `utils/run-test`: By default, `run-test` builds the tests' dependencies
   before running them.
   ```sh
   # Rebuild all test dependencies and run all tests under test/.
   utils/run-test --lit ../toolchain-project/toolchain/utils/lit/lit.py \
     ../build/Ninja-RelWithDebInfoAssert/language-macosx-$(uname -m)/test-macosx-$(uname -m)

   # Rebuild all test dependencies and run tests containing "MyTest".
   utils/run-test --lit ../toolchain-project/toolchain/utils/lit/lit.py \
     ../build/Ninja-RelWithDebInfoAssert/language-macosx-$(uname -m)/test-macosx-$(uname -m) \
     --filter="MyTest"
   ```
2. `lit.py`: lit doesn't know anything about dependencies. It just runs tests.
   ```sh
   # Run all tests under test/.
   ../toolchain-project/toolchain/utils/lit/lit.py -s -vv \
     ../build/Ninja-RelWithDebInfoAssert/language-macosx-$(uname -m)/test-macosx-$(uname -m)

   # Run tests containing "MyTest"
   ../toolchain-project/toolchain/utils/lit/lit.py -s -vv \
     ../build/Ninja-RelWithDebInfoAssert/language-macosx-$(uname -m)/test-macosx-$(uname -m) \
     --filter="MyTest"
   ```
   The `-s` and `-vv` flags print a progress bar and the executed commands
   respectively.

If you are making small changes to the compiler or some other component, you'll
likely want to [incrementally rebuild](#editing-code) only the relevant
target and use `lit.py` with `--filter`. One potential failure mode with this
approach is accidental use of stale binaries. For example, say that you want to
rerun a SourceKit test but you only incrementally rebuilt the compiler. Then
your changes will not be reflected when the test runs because the `sourcekitd`
binary was not rebuilt. Using `run-test` instead is the safer option, but it
will lead to a longer feedback loop due to more things getting rebuilt.

In the rare event that a local test failure happens to be unrelated to your
changes (is not due to stale binaries and reproduces without your changes),
there is a good chance that it has already been caught by our continuous
integration infrastructure, and it may be ignored.

If you want to rerun all the tests, you can either rebuild the whole project
and use `lit.py` without `--filter` or use `run-test` to handle both aspects.

For more details on running tests and understanding the various Codira-specific
lit customizations, see [Testing.md](/docs/Testing.md). Also check out the
[lit documentation](https://toolchain.org/docs/CommandGuide/lit.html) to understand
how the different lit commands work.

## Debugging issues

In this section, we briefly describe two common ways of debugging: print
debugging and using LLDB.

Depending on the code you're interested in, LLDB may be significantly more
effective when using a debug build. Depending on what components you are
working on, you could turn off optimizations for only a few things.
Here are some example invocations:

```sh
# optimized Stdlib + debug Codirac + optimized Clang/LLVM
utils/build-script --release-debuginfo --debug-language # other flags...

# debug Stdlib + optimized Codirac + optimized Clang/LLVM
utils/build-script --release-debuginfo --debug-language-stdlib # other flags...

# optimized Stdlib + debug Codirac (except typechecker) + optimized Clang/LLVM
utils/build-script --release-debuginfo --debug-language --force-optimized-typechecker

# Last resort option, it is highly unlikely that you will need this
# debug Stdlib + debug Codirac + debug Clang/LLVM
utils/build-script --debug # other flags...
```

Debug builds have two major drawbacks:
- A debug compiler is much slower, leading to longer feedback loops in case you
  need to repeatedly compile the Codira standard library and/or run a large
  number of tests.
- The build artifacts consume a lot more disk space.

[DebuggingTheCompiler.md](/docs/DebuggingTheCompiler.md) goes into a LOT
more detail on how you can level up your debugging skills! Make sure you check
it out in case you're trying to debug a tricky issue and aren't sure how to
go about it.

### Print debugging

A large number of types have `dump(..)`/`print(..)` methods which can be used
along with `toolchain::errs()` or other LLVM streams. For example, if you have a
variable `std::vector<CanType> canTypes` that you want to print, you could do:
```cpp
auto &e = toolchain::errs();
e << "canTypes = [";
toolchain::interleaveComma(canTypes, e, [&](auto ty) { ty.dump(e); });
e << "]\n";
```
You can also crash the compiler using `assert`/`toolchain_unreachable`/
`toolchain::report_fatal_error`, after accumulating the result in a stream:
```cpp
std::string msg; toolchain::raw_string_ostream os(msg);
os << "unexpected canTypes = [";
toolchain::interleaveComma(canTypes, os, [&](auto ty) { ty.dump(os); });
os << "] !!!\n";
toolchain::report_fatal_error(os.str());
```

### Debugging using LLDB

When the compiler crashes, the command line arguments passed to it will be
printed to stderr. It will likely look something like:

```
/path/to/language-frontend <args>
```

- Using LLDB on the command line: Copy the entire invocation and pass it to LLDB.
  ```sh
  lldb -- /path/to/language-frontend <args>
  ```
  Now you can use the usual LLDB commands like `run`, `breakpoint set` and so
  on. If you are new to LLDB, check out the [official LLDB documentation][] and
  [nesono's LLDB cheat sheet][].
- Using LLDB within Xcode:
  Select the current scheme 'language-frontend' → Edit Scheme → Run → Arguments
  tab. Under "Arguments Passed on Launch", copy-paste the `<args>` and make sure
  that "Expand Variables Based On" is set to language-frontend. Close the scheme
  editor. If you now run the compiler (<kbd>⌘</kbd>+<kbd>R</kbd> or 
  Product → Run), you will be able to use the Xcode debugger.

  Xcode also has the ability to attach to and debug Codira processes launched
  elsewhere. Under Debug → Attach to Process by PID or name..., you can enter
  a compiler process's PID or name (`language-frontend`) to debug a compiler
  instance invoked elsewhere. This can be helpful if you have a single compiler
  process being invoked by another tool, such as CodiraPM or another open Xcode
  project.

  > **Pro Tip**: Xcode 12's terminal does not support colors, so you may see
  > explicit color codes printed by `dump()` methods on various types. To avoid
  > color codes in dumped output, run `expr toolchain::errs().enable_color(false)`.

[official LLDB documentation]: https://lldb.toolchain.org
[nesono's LLDB cheat sheet]: https://www.nesono.com/sites/default/files/lldb%20cheat%20sheet.pdf

## Next steps

Make sure you check out the following resources:

* [LLVM Coding Standards](https://toolchain.org/docs/CodingStandards.html): A style
  guide followed by both LLVM and Codira. If there is a mismatch between the LLVM
  Coding Standards and the surrounding code that you are editing, please match
  the style of existing code.
* [LLVM Programmer's Manual](https://toolchain.org/docs/ProgrammersManual.html):
  A guide describing common programming idioms and data types used by LLVM and
  Codira.
* [docs/README.md](/docs/README.md): Provides a bird's eye view of the available
  documentation.
* [Lexicon.md](/docs/Lexicon.md): Provides definitions for jargon. If you run
  into a term frequently that you don't recognize, it's likely that this file
  has a definition for it.
* [Testing.md](/docs/Testing.md) and
  [DebuggingTheCompiler.md](/docs/DebuggingTheCompiler.md): These cover more
  ground on testing and debugging respectively.
* [Development Tips](/docs/DevelopmentTips.md): Tips for being more productive.
<!-- Link to Compiler Architecture.md once that is ready -->

If you see mistakes in the documentation (including typos, not just major
errors) or identify gaps that you could potentially improve the contributing
experience, please start a discussion on the forums, submit a pull request
or file a bug report on [Codira repository 'Issues' tab][Codira Issues]. Thanks!

[Codira Issues]: https://github.com/languagelang/language/issues
