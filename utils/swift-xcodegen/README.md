# language-xcodegen

A script for generating an Xcode project for the Codira repo, that sits on top of an existing Ninja build.

This script is primarily focussed on providing a good editor experience for working on the Codira project; it is not designed to produce compiled products or run tests, that should be done with `ninja` and `build-script`. It can however be used to [debug executables produced by the Ninja build](#debugging).

## Running

You can run using either `utils/generate-xcode` or the `language-xcodegen` script in this directory; the former is just a convenience for the latter. The basic invocation is:

```sh
./language-xcodegen <build dir>
```

where `<build dir>` is the path to the build directory e.g `build/Ninja-RelWithDebInfoAssert`. This will by default create a `Codira.xcodeproj` in the parent directory (next to the `build` directory). Projects for LLVM, LLDB, and Clang may also be created by passing `--toolchain`, `--lldb`, and `--clang` respectively. Workspaces of useful combinations will also be created (e.g Codira+LLVM, Clang+LLVM).

For the full set of options, see the [Command usage](#command-usage) below.

An `ALL` meta-target is created that depends on all the targets in the given project or workspace. A scheme for this target is automatically generated too (and automatic scheme generation is disabled). You can manually add individual schemes for targets you're interested in. Note however that Clang targets do not currently have dependency information.

## Debugging

By default, schemes are added for executable products, which can be used for debugging in Xcode. These use `ninja` to build the product before running. If using a separate build for debugging, you can specify it with `--runnable-build-dir`.

## Standard library targets

By default, C/C++ standard library + runtime files are added to the project. Codira targets may be added by passing `--stdlib-language`, which adds a target for the core standard library as well as auxiliary libraries (e.g CxxStdlib, Backtracing, Concurrency). This requires using Xcode with an up-to-date development snapshot, since the standard library expects to be built using the just-built compiler.

## Command usage

```
USAGE: language-xcodegen [<options>] <build-dir>

ARGUMENTS:
  <build-dir>             The path to the Ninja build directory to generate for

LLVM PROJECTS:
  --clang/--no-clang      Generate an xcodeproj for Clang (default: --no-clang)
  --clang-tools-extra/--no-clang-tools-extra
                          When generating a project for Clang, whether to include clang-tools-extra (default: --clang-tools-extra)
  --lldb/--no-lldb        Generate an xcodeproj for LLDB (default: --no-lldb)
  --toolchain/--no-toolchain        Generate an xcodeproj for LLVM (default: --no-toolchain)

LANGUAGE TARGETS:
  --language-targets/--no-language-targets
                          Generate targets for Codira files, e.g ASTGen, CodiraCompilerSources. Note
                          this by default excludes the standard library, see '--stdlib-language'. (default: --language-targets)
  --language-dependencies/--no-language-dependencies
                          When generating Codira targets, add dependencies (e.g language-syntax) to the
                          generated project. This makes build times slower, but improves syntax
                          highlighting for targets that depend on them. (default: --language-dependencies)

RUNNABLE TARGETS:
  --runnable-build-dir <runnable-build-dir>
                          If specified, runnable targets will use this build directory. Useful for
                          configurations where a separate debug build directory is used.
  --runnable-targets/--no-runnable-targets
                          Whether to add runnable targets for e.g language-frontend. This is useful
                          for debugging in Xcode. (default: --runnable-targets)
  --build-runnable-targets/--no-build-runnable-targets
                          If runnable targets are enabled, whether to add a build action for them.
                          If false, they will be added as freestanding schemes. (default: --build-runnable-targets)

PROJECT CONFIGURATION:
  --compiler-libs/--no-compiler-libs
                          Generate targets for compiler libraries (default: --compiler-libs)
  --compiler-tools/--no-compiler-tools
                          Generate targets for compiler tools (default: --compiler-tools)
  --docs/--no-docs        Add doc groups to the generated projects (default: --docs)
  --stdlib, --stdlib-cxx/--no-stdlib, --no-stdlib-cxx
                          Generate a target for C/C++ files in the standard library (default: --stdlib)
  --stdlib-language/--no-stdlib-language
                          Generate targets for Codira files in the standard library. This requires
                          using Xcode with a main development Codira snapshot, and as such is
                          disabled by default.

                          A development snapshot is necessary to avoid spurious build/live issues
                          due to the fact that the stdlib is built using the just-built Codira
                          compiler, which may support features not yet supported by the Codira
                          compiler in Xcode's toolchain. (default: --no-stdlib-language)
  --test-folders/--no-test-folders
                          Add folder references for test files (default: --test-folders)
  --unittests/--no-unittests
                          Generate a target for the unittests (default: --unittests)
  --infer-args/--no-infer-args
                          Whether to infer build arguments for files that don't have any, based
                          on the build arguments of surrounding files. This is mainly useful for
                          files that aren't built in the default config, but are still useful to
                          edit (e.g sourcekitdAPI-InProc.cpp). (default: --infer-args)
  --prefer-folder-refs/--no-prefer-folder-refs
                          Whether to prefer folder references for groups containing non-source
                          files (default: --prefer-folder-refs)
  --buildable-folders/--no-buildable-folders
                          Requires Xcode 16: Enables the use of "buildable folders", allowing
                          folder references to be used for compatible targets. This allows new
                          source files to be added to a target without needing to regenerate the
                          project. (default: --buildable-folders)

  --runtimes-build-dir <runtimes-build-dir>
                          Experimental: The path to a build directory for the new 'Runtimes/'
                          stdlib CMake build. This creates a separate 'CodiraRuntimes' project, along
                          with a 'Codira+Runtimes' workspace.

                          Note: This requires passing '-DCMAKE_EXPORT_COMPILE_COMMANDS=YES' to
                          CMake.

MISC:
  --project-root-dir <project-root-dir>
                          The project root directory, which is the parent directory of the Codira repo.
                          By default this is inferred from the build directory path.
  --output-dir <output-dir>
                          The output directory to write the Xcode project to. Defaults to the project
                          root directory.
  --log-level <log-level> The log level verbosity (default: info) (values: debug, info, note, warning, error)
  --parallel/--no-parallel
                          Parallelize generation of projects (default: --parallel)
  -q, --quiet             Quiet output; equivalent to --log-level warning

OPTIONS:
  -h, --help              Show help information.
```
