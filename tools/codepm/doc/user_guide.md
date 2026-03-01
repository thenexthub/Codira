# Codira Project Manager User Guide

## Overview

`codepm` is the official project management tool for the Codira language, designed to manage and maintain module systems in Codira projects. It supports operations such as module initialization, dependency checking, and updates. The tool provides a unified compilation entry point with features including incremental compilation, parallel compilation, and support for custom compilation commands.

## Usage Instructions

Execute the `codepm -h` command to view the project manager's usage instructions as shown below.

```text
Codira Project Manager

Usage:
  codepm [subcommand] [option]

Available subcommands:
  init             Init a new cangjie module
  check            Check the dependencies
  update           Update codepm.lock
  tree             Display the package dependencies in the source code
  build            Compile the current module
  run              Compile and run an executable product
  test             Unittest a local package or module
  bench            Run benchmarks in a local package or module
  clean            Clean up the target directory
  install          Install a cangjie binary
  uninstall        Uninstall a cangjie binary

Available options:
  -h, --help       help for codepm
  -v, --version    version for codepm

Use "codepm [subcommand] --help" for more information about a command.
```

Basic usage command example:

```shell
codepm build --help
```

Here, `codepm` is the main program name, `build` is the currently executed command, and `--help` is a configurable option for the current command (options typically have both long and short forms with identical effects).

Successful execution will display the following result:

```text
Compile a local module and all of its dependencies.

Usage:
  codepm build [option]

Available options:
  -h, --help                    help for build
  -i, --incremental             enable incremental compilation
  -j, --jobs <N>                the number of jobs to spawn in parallel during the build process
  -V, --verbose                 enable verbose
  -g                            enable compile debug version target
  --cfg                         enable the customized option 'cfg'
  -m, --member <value>          specify a member module of the workspace
  --target <value>              generate code for the given target platform
  --target-dir <value>          specify target directory
  -o, --output <value>          specify product name when compiling an executable file
  --mock                        enable support of mocking classes in tests
  --skip-script                 disable script 'build.code'.
```

## Command Descriptions

### init

The `init` command initializes a new Codira module or workspace. When initializing a module, it creates a configuration file `codepm.toml` in the current folder by default and establishes a `src` source code directory. If the module's output is of executable type, it generates a default `main.code` file under `src`, which prints `hello world` after compilation. When initializing a workspace, only the `codepm.toml` file is created, and existing Codira modules in the path are automatically scanned and added to the `members` field. If `codepm.toml` already exists or `main.code` is present in the source directory, corresponding file creation steps are skipped.

Configurable options for `init` include:

- `--workspace` creates a workspace configuration file; other options are ignored when this is specified
- `--name <value>` specifies the `root` package name for the new module; defaults to the parent folder name if unspecified
- `--path <value>` specifies the path for the new module; defaults to the current folder if unspecified
- `--type=<executable|static|dynamic>` specifies the output type for the new module; defaults to `executable` if omitted

Examples:

```text
Input: codepm init
Output: codepm init success
```

```text
Input: codepm init --name demo --path project
Output: codepm init success
```

```text
Input: codepm init --type=static
Output: codepm init success
```

### check

The `check` command verifies project dependencies and prints valid package compilation order upon success.

Configurable options for `check` include:
- `-m, --member <value>` workspace-only option to specify a single module as the check entry point
- `--no-tests` excludes test-related dependencies from output
- `--skip-script` skips build script compilation and execution

Examples:

```text
Input: codepm check
Output:
The valid serial compilation order is:
    b.pkgA -> b
codepm check success
```

```text
Input: codepm check
Output:
Error: cyclic dependency
b.B -> c.C
c.C -> d.D
d.D -> b.B
Note: In the output above, b.B represents a subpackage named b.B within the module with b as its root package
```

```text
Input: codepm check
Output:
Error: can not find the following dependencies
    pro1.xoo
    pro1.yoo
    pro2.zoo
```

### update

The `update` command synchronizes `codepm.toml` content to `codepm.lock`. If `codepm.lock` doesn't exist, it will be created. This file records metadata like version numbers for git dependencies, used in subsequent builds.

Configurable option:
- `--skip-script` skips build script compilation and execution

```text
Input: codepm update
Output: codepm update success
```

### tree

The `tree` command visually displays package dependency relationships in Codira source code.

Configurable options include:
- `-V, --verbose` adds detailed package node information including name, version, and path
- `--depth <N>` specifies maximum dependency tree depth (non-negative integer); shows all packages as root nodes when specified
- `-p, --package <value>` specifies a package as root node to display its dependencies
- `--invert <value>` specifies a package as root node and inverts the tree to show its dependents
- `--target <value>` includes target platform dependencies in analysis
- `--no-tests` excludes `test-dependencies` field items
- `--skip-script` skips build script compilation and execution

Example module `a` source structure:

```text
src
├── main.code
├── aoo
│   └── a.code
├── boo
│   └── b.code
├── coo
│   └── c.code
├── doo
│   └── d.code
├── eoo
│   └── e.code
└── codepm.toml
```

Dependency relationships: Package `a` imports `a.aoo` and `a.boo`; subpackage `aoo` imports `a.coo`; subpackage `boo` imports `coo`; subpackage `doo` imports `coo`.

```text
Input: codepm tree
Output:
|-- a
    └── a.aoo
        └── a.coo
    └── a.boo
        └── a.coo
|-- a.doo
    └── a.coo
|-- a.eoo
codepm tree success
```

```text
Input: codepm tree --depth 2 -p a
Output:
|-- a
    └── a.aoo
        └── a.coo
    └── a.boo
        └── a.coo
codepm tree success
```

```text
Input: codepm tree --depth 0
Output:
|-- a
|-- a.eoo
|-- a.aoo
|-- a.boo
|-- a.doo
|-- a.coo
codepm tree success
```

```text
Input: codepm tree --invert a.coo --verbose
Output:
|-- a.coo 1.2.0 (.../src/coo)
    └── a.aoo 1.1.0 (.../src/aoo)
            └── a 1.0.0 (.../src)
    └── a.boo 1.1.0 (.../src/boo)
            └── a 1.0.0 (.../src)
    └── a.doo 1.3.0 (.../src/doo)
codepm tree success
```

### build

The `build` command compiles the current Codira project after dependency verification, invoking `codec` for compilation.

Configurable options include:
- `-i, --incremental` enables incremental compilation (full compilation by default)
- `-j, --jobs <N>` sets maximum parallel compilation jobs (minimum of N and 2×CPU cores)
- `-V, --verbose` shows compilation logs
- `-g` generates debug version output
- `--cfg` forwards custom `cfg` options from `codepm.toml`
- `-m, --member <value>` workspace-only option to specify a single compilation entry module
- `--target <value>` enables cross-compilation to target platforms
- `--target-dir <value>` specifies output directory
- `-o, --output <value>` specifies executable output name (default: `main` or `main.exe` on Windows)
- `--mock` enables class mocking support in test builds
- `--skip-script` skips build script compilation and execution

> **Note:**
> - The `-i, --incremental` option only enables package-level incremental compilation. Developers can manually forward `--incremental-compile` via `compile-option` in configuration files for function-level incremental compilation.
> - Currently, `-i, --incremental` only supports source-based incremental analysis. Library import changes require full recompilation.

Intermediate files are stored in the `target` folder by default, with executables in `target/release/bin` or `target/debug/bin` depending on build mode. For reproducible builds, this command creates `codepm.lock` recording exact dependency versions for subsequent builds. Update this file using the `update` command when necessary. This file should be committed to version control to ensure reproducible builds across team members.

Examples:

```text
Input: codepm build -V
Output:
compile package module1.package1: codec --import-path "target/release" --output-dir "target/release/module1" -p "src/package1" --output-type=staticlib -o libmodule1.package1.a
compile package module1: codec --import-path "target/release" --output-dir "target/release/bin" -p "src" --output-type=exe -o main
codepm build success
```

```text
Input: codepm build
Output: codepm build success
```

> **Note:**
> Per Codira package specifications, only compliant source packages are included in compilation. Warnings like `no '.code' file` indicate non-compliant package structures. Refer to [Codira Package Specifications](#cangjie-package-management-specification) for directory structure requirements.

### run

The `run` command executes compiled binary outputs. It automatically triggers the `build` process to generate executable files.

Configurable options include:
- `--name <value>` specifies binary name (default: `main`; workspace binaries are in `target/release/bin`)
- `--build-args <value>` controls `build` compilation parameters
- `--skip-build` skips compilation and directly runs existing binaries
- `--run-args <value>` forwards arguments to the executed binary
- `--target-dir <value>` specifies output directory
- `-g` runs debug version outputs
- `-V, --verbose` shows execution logs
- `--skip-script` skips build script compilation and execution

Examples:

```text
Input: codepm run
Output: codepm run success
``````markdown
Input: codepm run -g // This will execute codepm build -i -g by default
Output: codepm run success
```

```text
Input: codepm run --build-args="-s -j16" --run-args="a b c"
Output: codepm run success
```

### test

The `test` command is used to compile and run unit test cases for Codira files. After execution, it prints the test results. The compiled artifacts are stored by default in the `target/release/unittest_bin` directory. For writing unit test cases, refer to the `std.unittest` library documentation in the *Codira Programming Language Standard Library API*.

This command can specify single-package paths to test (supports multiple paths, e.g., `codepm test path1 path2`). If no path is specified, it defaults to module-level unit testing. When performing module-level unit testing, only the current module's tests are executed by default; tests in all directly or indirectly dependent modules are skipped. The `test` command requires the current project to compile successfully with `build`.

The unit test code structure for a module is as follows, where `xxx.code` contains the package source code and `xxx_test.code` contains the unit test code:

```text
└── src
│    ├── koo
│    │    ├── koo.code
│    │    └── koo_test.code
│    ├── zoo
│    │    ├── zoo.code
│    │    └── zoo_test.code
│    ├── main.code
│    └── main_test.code
└── codepm.toml
```

1. Single-module testing scenario

    ```text
    Input: codepm test
    Progress report:
    group test.koo                                    100% [||||||||||||||||||||||||||||] ✓    (00:00:01)
    group test.zoo                                      0% [----------------------------]      (00:00:00)
    test TestZ.sayhi                                                                           (00:00:00)

    passed: 1, failed: 0                                  33% [|||||||||-------------------]  1/3 (00:00:01)

    Output:
    --------------------------------------------------------------------------------------------------
    TP: test, time elapsed: 177921 ns, RESULT:
        TCS: TestM, time elapsed: 177921 ns, RESULT:
        [ PASSED ] CASE: sayhi (177921 ns)
        Summary: TOTAL: 1
        PASSED: 1, SKIPPED: 0, ERROR: 0
        FAILED: 0
    --------------------------------------------------------------------------------------------------
    TP: test.koo, time elapsed: 134904 ns, RESULT:
        TCS: TestK, time elapsed: 134904 ns, RESULT:
        [ PASSED ] CASE: sayhi (134904 ns)
        Summary: TOTAL: 1
        PASSED: 1, SKIPPED: 0, ERROR: 0
        FAILED: 0
    --------------------------------------------------------------------------------------------------
    TP: test.zoo, time elapsed: 132013 ns, RESULT:
        TCS: TestZ, time elapsed: 132013 ns, RESULT:
        [ PASSED ] CASE: sayhi (132013 ns)
        Summary: TOTAL: 1
        PASSED: 1, SKIPPED: 0, ERROR: 0
        FAILED: 0
    --------------------------------------------------------------------------------------------------
    Project tests finished, time elapsed: 444838 ns, RESULT:
    TP: test.*, time elapsed: 312825 ns, RESULT:
        PASSED:
        TP: test.zoo, time elapsed: 132013 ns
        TP: test.koo, time elapsed: 312825 ns
        TP: test, time elapsed: 312825 ns
    Summary: TOTAL: 3
        PASSED: 3, SKIPPED: 0, ERROR: 0
        FAILED: 0
    --------------------------------------------------------------------------------------------------
    codepm test success
    ```

2. Single-package testing scenario

    ```text
    Input: codepm test src/koo
    Output:
    --------------------------------------------------------------------------------------------------
    TP: test.koo, time elapsed: 160133 ns, RESULT:
        TCS: TestK, time elapsed: 160133 ns, RESULT:
        [ PASSED ] CASE: sayhi (160133 ns)
        Summary: TOTAL: 1
        PASSED: 1, SKIPPED: 0, ERROR: 0
        FAILED: 0
    --------------------------------------------------------------------------------------------------
    Project tests finished, time elapsed: 160133 ns, RESULT:
    TP: test.*, time elapsed: 160133 ns, RESULT:
        PASSED:
        TP: test.koo, time elapsed: 160133 ns
    Summary: TOTAL: 1
        PASSED: 1, SKIPPED: 0, ERROR: 0
        FAILED: 0
    --------------------------------------------------------------------------------------------------
    codepm test success
    ```

3. Multi-package testing scenario

    ```text
    Input: codepm test src/koo src
    Output:
    --------------------------------------------------------------------------------------------------
    TP: test.koo, time elapsed: 168204 ns, RESULT:
        TCS: TestK, time elapsed: 168204 ns, RESULT:
        [ PASSED ] CASE: sayhi (168204 ns)
        Summary: TOTAL: 1
        PASSED: 1, SKIPPED: 0, ERROR: 0
        FAILED: 0
    --------------------------------------------------------------------------------------------------
    TP: test, time elapsed: 171541 ns, RESULT:
        TCS: TestM, time elapsed: 171541 ns, RESULT:
        [ PASSED ] CASE: sayhi (171541 ns)
        Summary: TOTAL: 1
        PASSED: 1, SKIPPED: 0, ERROR: 0
        FAILED: 0
    --------------------------------------------------------------------------------------------------
    Project tests finished, time elapsed: 339745 ns, RESULT:
    TP: test.*, time elapsed: 339745 ns, RESULT:
        PASSED:
        TP: test.koo, time elapsed: 339745 ns
        TP: test, time elapsed: 339745 ns
    Summary: TOTAL: 2
        PASSED: 2, SKIPPED: 0, ERROR: 0
        FAILED: 0
    --------------------------------------------------------------------------------------------------
    codepm test success
    ```

`test` has multiple configurable options:

- `-j, --jobs <N>`: Specifies the maximum number of parallel compilation jobs. The final maximum concurrency is the minimum of `N` and `2 × CPU cores`.
- `-V, --verbose`: When enabled, outputs unit test logs.
- `-g`: Generates `debug` version unit test artifacts, stored in the `target/debug/unittest_bin` directory.
- `-i, --incremental`: Specifies incremental compilation for test code. Full compilation is performed by default.
- `--no-run`: Only compiles unit test artifacts without running them.
- `--skip-build`: Only executes precompiled unit test artifacts.
- `--cfg`: Allows passing custom `cfg` options from `codepm.toml`.
- `--module <value>`: Specifies the target test module(s). The specified module(s) must be directly or indirectly dependent on the current module (or the module itself). Multiple modules can be specified using `--module "module1 module2"`. If unspecified, only the current module is tested.
- `-m, --member <value>`: Only usable in workspaces, specifies testing a single module.
- `--target <value>`: Enables cross-compilation for target platforms. Refer to the [target](#target) section in `codepm.toml`.
- `--target-dir <value>`: Specifies the output directory for unit test artifacts.
- `--dry-run`: Prints test cases without executing them.
- `--filter <value>`: Filters test subsets. `value` formats:
    - `--filter=*`: Matches all test classes.
    - `--filter=*.*`: Matches all test cases in all test classes (same as `*`).
    - `--filter=*.*Test,*.*case*`: Matches test cases ending with `Test` or containing `case` in their names.
    - `--filter=MyTest*.*Test,*.*case*,-*.*myTest`: Matches test cases in classes starting with `MyTest` and ending with `Test`, or containing `case`, or excluding those containing `myTest`.
- `--include-tags <value>`: Runs tests with specified `@Tag` annotations. `value` formats:
    - `--include-tags=Unittest`: Runs tests marked with `@Tag[Unittest]`.
    - `--include-tags=Unittest,Smoke`: Runs tests marked with either `@Tag[Unittest]` or `@Tag[Smoke]`.
    - `--include-tags=Unittest+Smoke`: Runs tests marked with both `@Tag[Unittest]` and `@Tag[Smoke]`.
    - `--include-tags=Unittest+Smoke+JiraTask3271,Backend`: Runs tests marked with `@Tag[Backend]` or `@Tag[Unittest, Smoke, JiraTask3271]`.
- `--exclude-tags <value>`: Excludes tests with specified `@Tag` annotations. `value` formats:
    - `--exclude-tags=Unittest`: Runs tests not marked with `@Tag[Unittest]`.
    - `--exclude-tags=Unittest,Smoke`: Runs tests not marked with either `@Tag[Unittest]` or `@Tag[Smoke]`.
    - `--exclude-tags=Unittest+Smoke+JiraTask3271`: Runs tests not marked with all of `@Tag[Unittest, Smoke, JiraTask3271]`.
    - `--include-tags=Unittest --exclude-tags=Smoke`: Runs tests marked with `@Tag[Unittest]` but not `@Tag[Smoke]`.
- `--no-color`: Disables colored console output.
- `--random-seed <N>`: Specifies the random seed value.
- `--timeout-each <value>`: Format `%d[millis|s|m|h]`. Sets a default timeout for each test case.
- `--parallel`: Specifies parallel test execution. `value` formats:
    - `<BOOL>`: `true` or `false`. If `true`, test classes run in parallel with processes limited by CPU cores.
    - `nCores`: Sets the number of parallel processes equal to available CPU cores.
    - `NUMBER`: Sets a fixed number of parallel processes (positive integer).
    - `NUMBERnCores`: Sets parallel processes as a multiple of CPU cores (supports floats/integers).
- `--show-all-output`: Enables output printing for passed test cases.
- `--no-capture-output`: Disables output capture, printing output immediately during test execution.
- `--report-path <value>`: Specifies the path for test reports.
- `--report-format <value>`: Specifies report format. Currently, only `xml` is supported (case-insensitive). Other values throw exceptions.
- `--skip-script`: Skips build script compilation and execution.
- `--no-progress`: Disables progress reporting. Implied if `--dry-run` is specified.
- `--progress-brief`: Shows a brief (single-line) progress report instead of detailed reports.
- `--progress-entries-limit <value>`: Limits the number of entries shown in progress reports. Default: `0`. Allowed values:
    - `0`: No limit.
    - `n`: Positive integer specifying the maximum entries displayed.

`codepm test` usage examples:

```text
Input: codepm test --filter=*
Output: codepm test success
```

```text
Input: codepm test src --report-path=reports --report-format=xml
Output: codepm test success
```

> **Note:**
>
> `codepm test` automatically builds all packages with `mock` support, allowing developers to perform `mock` tests on custom classes or dependent modules. To `mock` classes from binary dependencies, build with `codepm build --mock`.

### bench

The `bench` command executes performance test cases and prints results directly. Compiled artifacts are stored in `target/release/unittest_bin` by default. Performance cases are annotated with `@Bench`. For details on writing performance tests, refer to the `std.unittest` library in the *Codira Programming Language Standard Library API*.

This command can specify single-package paths (supports multiple paths, e.g., `codepm bench path1 path2`). If unspecified, it defaults to module-level testing. Like `test`, module-level testing only executes the current module's tests by default. `bench` requires the project to compile successfully with `build`.

Similar to `test`, if you have `xxx.code` files, `xxx_test.code` can also contain performance test cases.

```text
Input: codepm bench
Output:
TP: bench, time elapsed: 8107939844 ns, RESULT:
    TCS: Test_UT, time elapsed: 8107939844 ns, RESULT:
    | Case       |   Median |         Err |   Err% |     Mean |
    |:-----------|---------:|------------:|-------:|---------:|
    | Benchmark1 | 5.438 ns | ±0.00439 ns |  ±0.1% | 5.420 ns |
Summary: TOTAL: 1
    PASSED: 1, SKIPPED: 0, ERROR: 0
    FAILED: 0
--------------------------------------------------------------------------------------------------
Project tests finished, time elapsed: 8107939844 ns, RESULT:
TP: bench.*, time elapsed: 8107939844 ns, RESULT:
    PASSED:
    TP: bench, time elapsed: 8107939844 ns, RESULT:
Summary: TOTAL: 1
    PASSED: 1, SKIPPED: 0, ERROR: 0
    FAILED: 0
```

`bench` configurable options:

- `-j, --jobs <N>`: Specifies maximum parallel compilation jobs (minimum of `N` and `2 × CPU cores`).
- `-V, --verbose`: Enables performance test logs.
- `-g`: Generates `debug` version artifacts in `target/debug/unittest_bin`.
- `-i, --incremental`: Enables incremental compilation (full compilation by default).
- `--no-run`: Only compiles performance test artifacts.
- `--skip-build`: Only executes precompiled artifacts.
- `--cfg`: Passes custom `cfg` options from `codepm.toml`.
- `--module <value>`: Specifies target module(s) (must be directly/indirectly dependent or the module itself). Multiple modules can be specified with `--module "module1 module2"`.
- `-m, --member <value>`: Workspace-only; specifies a single module to test.
- `--target <value>`: Enables cross-compilation for target platforms. Refer to `cross-compile-configuration` in `codepm.toml`.
- `--target-dir <value>`: Specifies output directory for artifacts.
- `--dry-run`: Prints test cases without execution.
- `--filter <value>`: Filters test subsets. `value` formats:
    - `--filter=*`: Matches all test classes.
    - `--filter=*.*`: Matches all test cases (same as `*`).
    - `--filter=*.*Test,*.*case*`: Matches test cases ending with `Test` or containing `case`.
    - `--filter=MyTest*.*Test,*.*case*,-*.*myTest`: Matches test cases in `MyTest*` classes ending with `Test`, or containing `case`, or excluding `myTest`.
- `--include-tags <value>`: Runs tests with specified `@Tag` annotations. `value` formats:
    - `--include-tags=Unittest`: Runs tests marked with `@Tag[Unittest]`.
    - `--include-tags=Unittest,Smoke`: Runs tests marked with either `@Tag[Unittest]` or `@Tag[Smoke]`.
    - `--include-tags=Unittest+Smoke`: Runs tests marked with both `@Tag[Unittest]` and `@Tag[Smoke]`.
    - `--include-tags=Unittest+Smoke+JiraTask3271,Backend`: Runs tests marked with `@Tag[Backend]` or `@Tag[Unittest, Smoke, JiraTask3271]`.
- `--exclude-tags <value>`: Excludes tests with specified `@Tag` annotations. `value` formats:
    - `--exclude-tags=Unittest`: Runs tests not marked with `@Tag[Unittest]`.
    - `--exclude-tags=Unittest,Smoke`: Runs tests not marked with either `@Tag[Unittest]` or `@Tag[Smoke]`.
    - `--exclude-tags=Unittest+Smoke+JiraTask3271`: Runs tests not marked with all of `@Tag[Unittest, Smoke, JiraTask3271]`.
    - `--include-tags=Unittest --exclude-tags=Smoke`: Runs tests marked with `@Tag[Unittest]` but not `@Tag[Smoke]`.
- `--no-color`: Disables colored console output.
- `--random-seed <N>`: Specifies the random seed value.
- `--report-path <value>`: Specifies report output path. Unlike `test`, it defaults to `bench_report`.
- `--report-format <value>`: Supports `csv` and `csv-raw` formats for performance reports.
- `--baseline-path <value>`: Path to compare current results with existing reports. Defaults to `--report-path`.
- `--skip-script`: Skips build script compilation and execution.

`codepm bench` usage examples:

```text
Input: codepm bench
Output: codepm bench success

Input: codepm bench src
Output: codepm bench success

Input: codepm bench src --filter=*
Output: codepm bench success

Input: codepm bench src --report-format=csv
Output: codepm bench success
```

> **Note:**
>
> `> On the `Windows` platform, immediately cleaning up the executable files or parent directories of a child process after its execution may fail. If this issue is encountered, you can retry the `clean` command after a short delay.

### install

`install` is used to install the Codira project. Before executing this command, it will first compile the project and then install the compiled artifacts to the specified path. The installed artifacts are named after the Codira project (with a `.exe` suffix on `Windows` systems). The type of project artifact installed by `install` must be `executable`.

`install` has several configurable options:

- `-V, --verbose`: Displays installation logs.
- `-m, --member <value>`: Can only be used in a workspace to specify a single module as the compilation entry point for installing a single module.
- `-g`: Generates a `debug` version of the installed artifacts.
- `--path <value>`: Specifies the local installation path for the project. The default is the current directory.
- `--root <value>`: Specifies the installation path for the executable file. If not configured, the default path is `$HOME/.codepm` on `Linux`/`macOS` systems and `%USERPROFILE%/.codepm` on `Windows` systems. If configured, the artifacts will be installed under `value`.
- `--git <value>`: Specifies the `git` URL for the project to be installed.
- `--branch <value>`: Specifies the branch of the `git` project to be installed.
- `--tag <value>`: Specifies the `tag` of the `git` project to be installed.
- `--commit <value>`: Specifies the `commit ID` of the `git` project to be installed.
- `-j, --jobs <N>`: Specifies the maximum number of parallel compilation jobs. The final maximum concurrency is the minimum of `N` and `2 times the number of CPU cores`.
- `--cfg`: If specified, custom `cfg` options in `codepm.toml` can be passed through.
- `--target-dir <value>`: Specifies the path for storing compiled artifacts.
- `--name <value>`: Specifies the name of the final installed artifact.
- `--skip-build`: Skips the compilation phase and directly installs the artifacts. This requires the project to be in a compiled state and only works for local installations.
- `--list`: Prints the list of installed artifacts.
- `--skip-script`: If configured, the build script compilation and execution for the module to be installed will be skipped.

Notes for `install` functionality:

- `install` supports two installation methods: installing a local project (via `--path` to specify the project path) and installing a `git` project (via `--git` to specify the project URL). Only one of these methods can be configured at a time; otherwise, `install` will report an error. If neither is configured, the default is to install the local project in the current directory.
- When compiling a project, `install` enables incremental compilation by default.
- Git-related configurations (`--branch`, `--tag`, and `--commit`) only take effect if `--git` is configured; otherwise, they are ignored. If multiple git-related configurations are specified, only the one with higher priority will take effect. The priority order is `--commit` > `--branch` > `--tag`.
- If an executable file with the same name already exists, it will be replaced.
- Assuming the installation path is `root` (`root` is the configured installation path or the default path if not configured), the executable file will be installed under `root/bin`.
- If the project has dynamic library dependencies, the dynamic libraries required by the executable will be installed under `root/libs`, separated into directories by program name. Developers need to add the corresponding directories to the appropriate paths (`LD_LIBRARY_PATH` on `Linux`, `PATH` on `Windows`, and `DYLD_LIBRARY_PATH` on `macOS`) to use them.
- The default installation path (`$HOME/.codepm` on `Linux`/`macOS` and `%USERPROFILE%/.codepm` on `Windows`) will be added to `PATH` during `envsetup`.
- After installing a `git` project, `install` will clean up the corresponding compiled artifact directory.
- If the project to be installed has only one executable artifact, specifying `--name` will rename it during installation. If there are multiple executable artifacts, specifying `--name` will only install the artifact with the corresponding name.
- When `--list` is configured, `install` will print the list of installed artifacts, and all other configuration options except `--root` will be ignored. If `--root` is configured, `--list` will print the list of installed artifacts under the specified path; otherwise, it will print the list under the default path.

Examples:

```text
codepm install --path path/to/project # Installs from the local path path/to/project
codepm install --git url              # Installs from the git URL
```

### uninstall

`uninstall` is used to uninstall the Codira project, removing the corresponding executable files and dependency files.

`uninstall` requires the parameter `name` to uninstall the artifact named `name`. Multiple `name` parameters can be specified for sequential deletion. `uninstall` can specify the path of the executable file to be uninstalled via `--root <value>`. If not configured, the default path is `$HOME/.codepm` on `Linux`/`macOS` systems and `%USERPROFILE%/.codepm` on `Windows` systems. If configured, it will uninstall the artifacts under `value/bin` and the dependencies under `value/libs`.

> **Note:**
>
> `codepm` currently does not support paths containing Chinese characters on the `Windows` platform. If issues arise, please modify the directory name to avoid them.

## Module Configuration File Description

The module configuration file `codepm.toml` is used to configure basic information, dependencies, compilation options, etc. `codepm` primarily parses and executes based on this file. The module name can be renamed in `codepm.toml`, but the package name cannot be renamed.

The configuration file code is as follows:

```text
[package] # Single-module configuration field, cannot coexist with the workspace field
  codec-version = "1.0.0" # Minimum required version of `codec`, mandatory
  name = "demo" # Module name and module root package name, mandatory
  description = "nothing here" # Description, optional
  version = "1.0.0" # Module version information, mandatory
  compile-option = "" # Additional compilation command options, optional
  override-compile-option = "" # Additional global compilation command options, optional
  link-option = "" # Linker passthrough options, can pass security compilation commands, optional
  output-type = "executable" # Type of compiled output artifact, mandatory
  src-dir = "" # Specifies the source code storage path, optional
  target-dir = "" # Specifies the artifact storage path, optional
  package-configuration = {} # Single-package configuration options, optional

[workspace] # Workspace management field, cannot coexist with the package field
  members = [] # List of workspace member modules, mandatory
  build-members = [] # List of workspace compilation modules, must be a subset of member modules, optional
  test-members = [] # List of workspace test modules, must be a subset of compilation modules, optional
  compile-option = "" # Additional compilation command options applied to all workspace member modules, optional
  override-compile-option = "" # Additional global compilation command options applied to all workspace member modules, optional
  link-option = "" # Linker passthrough options applied to all workspace member modules, optional
  target-dir = "" # Specifies the artifact storage path, optional

[dependencies] # Source code dependency configuration, optional
  coo = { git = "xxx", branch = "dev" } # Imports a `git` dependency
  doo = { path = "./pro1" } # Imports a source code dependency

[test-dependencies] # Dependency configuration for the test phase, same format as dependencies, optional

[script-dependencies] # Dependency configuration for build scripts, same format as dependencies, optional

[replace] # Dependency replacement configuration, same format as dependencies, optional

[ffi.c] # Imports C language library dependencies, optional
  clib1.path = "xxx"

[profile] # Command profile configuration, optional
  build = {} # Build command configuration
  test = {} # Test command configuration
  bench = {} # Bench command configuration
  customized-option = {} # Custom passthrough options

[target.x86_64-unknown-linux-gnu] # Backend and platform isolation configuration, optional
  compile-option = "value1" # Additional compilation command options for specific target compilation processes and cross-compilation target platforms, optional
  override-compile-option = "value2" # Additional global compilation command options for specific target compilation processes and cross-compilation target platforms, optional
  link-option = "value3" # Linker passthrough options for specific target compilation processes and cross-compilation target platforms, optional

[target.x86_64-w64-mingw32.dependencies] # Source code dependency configuration for the corresponding target, optional

[target.x86_64-w64-mingw32.test-dependencies] # Test phase dependency configuration for the corresponding target, optional

[target.x86_64-unknown-linux-gnu.bin-dependencies] # Codira binary library dependencies for specific target compilation processes and cross-compilation target platforms, optional
  path-option = ["./test/pro0", "./test/pro1"] # Configures binary library dependencies in directory form
[target.x86_64-unknown-linux-gnu.bin-dependencies.package-option] # Configures binary library dependencies in single-file form
  "pro0.xoo" = "./test/pro0/pro0.xoo.codeo"
  "pro0.yoo" = "./test/pro0/pro0.yoo.codeo"
  "pro1.zoo" = "./test/pro1/pro1.zoo.codeo"
```

If the above fields are not used in `codepm.toml`, they default to empty (for paths, the default is the path where the configuration file is located).

### "codec-version"

The minimum required version of the Codira compiler, which must be compatible with the current environment version to execute. A valid Codira version number consists of three segments of numbers separated by `.`, with each number being a natural number and no redundant prefix `0`. For example:

- `1.0.0` is a valid version number;
- `1.00.0` is not a valid version number because `00` contains a redundant prefix `0`;
- `1.2e.0` is not a valid version number because `2e` is not a natural number.

### "name"

The current Codira module name, which is also the module `root` package name.

A valid Codira module name must be a valid identifier. Identifiers can consist of letters, numbers, and underscores, and must start with a letter, such as `codeDemo` or `code_demo_1`.

> **Note:**
>
> Currently, Codira module names do not support Unicode characters. A Codira module name must be a valid identifier containing only ASCII characters.

### "description"

Description of the current Codira module, for informational purposes only, with no format restrictions.

### "version"

The version number of the current Codira module, managed by the module owner, primarily for module validation. The format of the module version number is the same as `codec-version`.

### "compile-option"

Additional compilation options passed to `codec`. During multi-module compilation, the `compile-option` set for each module applies to all packages within that module.

For example:

```text
compile-option = "-O1 -V"
```

The commands entered here will be inserted into the compilation command during `build` execution, with multiple commands separated by spaces.

### "override-compile-option"

Additional global compilation options passed to `codec`. During multi-module compilation, the `override-compile-option` set for the compilation entry module applies to all packages within that module and its dependencies.

For example:

```text
override-compile-option = "-O1 -V"
```

The commands entered here will be inserted into the compilation command during `build` execution and concatenated after the module's `compile-option`, with higher priority than `compile-option`.

> **Note:**
>
> - `override-compile-option` applies to packages within dependency modules. Developers must ensure that the configured `codec` compilation options do not conflict with the `compile-option` in dependency modules. Otherwise, errors will occur during `codec` execution. For non-conflicting compilation options of the same type, the options in `override-compile-option` take precedence over those in `compile-option`.
> - In workspace compilation scenarios, only the `override-compile-option` configured in `workspace` applies to all packages in all modules within the workspace. Even if `-m` is used to specify a single module as the entry module for compilation, the entry module's `override-compile-option` will not be used.

### "link-option"

Compilation options passed to the linker, which can be used to pass security compilation commands, as shown below:

```text
link-option = "-z noexecstack -z relro -z now --strip-all"
```

> **Note:**
>
> The commands configured in `link-option` are only automatically passed to packages corresponding to dynamic libraries and executable artifacts during compilation.

### "output-type"

The type of compiled output artifact, including executable programs and libraries. The related inputs are shown in the table below. To automatically fill this field as `static` when generating `codepm.toml`, use the command `codepm init --type=static --name=modName`. If no type is specified, it defaults to `executable`. Only the main module's `output-type` can be `executable`.

|     Input     |               Description |
| :-----------: | :-----------------------: |
| "executable"  |          Executable program |
|   "static"    | Static library |
|  "dynamic"    | Dynamic library |
|    Others     |               Error |

### "src-dir"

This field specifies the path for storing source code. If not specified, it defaults to the `src` directory.

### "target-dir"

This field specifies the path for storing compiled artifacts. If not specified, it defaults to the `target` directory. If this field is not empty, executing `codepm clean` will delete the directory specified by this field. Developers must ensure the safety of cleaning this directory.

> **Note:**
>
> If `--target-dir` is specified during compilation, this option takes higher priority.

```text
target-dir = "temp"
```

### "package-configuration"

Single-package configuration options for each module. This option is a `map` structure, where the package name to be configured is the `key`, and the single-package configuration information is the `value`. Currently, configurable information includes output type and conditional options (`output-type`, `compile-option`), which can be configured as needed. For example, the output type of the `demo.aoo` package in the `demo` module will be specified as a dynamic library, and the `-g` command will be passed to the `demo.aoo` package during compilation.

```text
[package.package-configuration."demo.aoo"]
  output-type = "dynamic"
  compile-option = "-g"
```

If mutually compatible compilation options are configured in different fields, the priority of generating commands is as follows:

```text
[package]
  compile-option = "-O1"
[package.package-configuration.demo]
  compile-option = "-O2"

# The profile field will be introduced later
[profile.customized-option]
  cfg1 = "-O0"

Input: codepm build --cfg1 -V
Output: codec --import-path build -O0 -O1 -O2 ...
```

By configuring this field, multiple binary artifacts can be generated simultaneously (when generating multiple binary artifacts, the `-o, --output <value>` option will be invalid). Example:

Example of source code structure, with the module name `demo`:

```text
src
├── aoo
│    └── aoo.code
├── boo
│    └── boo.code
├── coo
│    └── coo.code
└──  main.code
```

Example of configuration:

```text
[package.package-configuration."demo.aoo"]
  output-type = "executable"
[package.package-configuration."demo.boo"]
  output-type = "executable"
```

Example of multiple binary artifacts:

```text
Input: codepm build
Output: codepm build success

Input: tree target/release/bin
Output: target/release/bin
|-- demo.aoo
|-- demo.boo
`-- demo
```

### "workspace"

This field manages multiple modules as a workspace and supports the following configurations:

- `members = ["aoo", "path/to/boo"]`: Lists local source code modules included in this workspace, supporting absolute and relative paths. The members of this field must be modules, not another workspace.
- `build-members = []`: Modules to be compiled this time. If not specified, all modules in the workspace are compiled by default. The members of this field must be included in the `members` field.
- `test-members = []`: Modules to be tested this time. If not specified, all modules in the workspace are unit-tested by default. The members of this field must be included in the `build-members` field.
- `compile-option = ""`: Common compilation options for the workspace, optional.
- `override-compile-option = ""`: Common global compilation options for the workspace, optional.
- `link-option = ""`: Common linker options for the workspace, optional.
- `target-dir = ""`: Artifact storage path for the workspace, optional, defaults to `target`.

Common configurations in the workspace apply to all member modules. For example, if a source code dependency `[dependencies] xoo = { path = "path_xoo" }` is configured, all member modules can directly use the `xoo` module without reconfiguring it in each submodule's `codepm.toml`.

> **Note:**
>
> The `package` field is used to configure general module information and cannot coexist with the `workspace` field in the same `codepm.toml`. All other fields except```text
root_path
├── aoo
│    ├── src
│    └── codepm.toml
├── boo
│    ├── src
│    └── codepm.toml
├── coo
│    ├── src
│    └── codepm.toml
└── codepm.toml
```

Example workspace configuration file usage:

```text
[workspace]
members = ["aoo", "boo", "coo"]
build-members = ["aoo", "boo"]
test-members = ["aoo"]
compile-option = "-Woff all"
override-compile-option = "-O2"

[dependencies]
xoo = { path = "path_xoo" }

[ffi.c]
abc = { path = "libs" }
```

### "dependencies"

This field imports dependent Codira modules via source code, configuring information about other modules required for the current build. Currently, it supports local path dependencies and remote `git` dependencies.

To specify local dependencies, use the `path` field, which must contain a valid local path. For example, the code structure of two submodules `pro0` and `pro1` along with the main module is as follows:

```text
├── pro0
│    ├── codepm.toml
│    └── src
│         └── zoo
│              └── zoo.code
├── pro1
│    ├── codepm.toml
│    └── src
│         ├── xoo
│         │    └── xoo.code
│         └── yoo
│              └── yoo.code
├── codepm.toml
└── src
     ├── aoo
     │    └── aoo.code
     ├── boo
     │    └── boo.code
     └── main.code
```

After configuring the main module's `codepm.toml` as follows, the `pro0` and `pro1` modules can be used in the source code:

```text
[dependencies]
  pro0 = { path = "./pro0" }
  pro1 = { path = "./pro1" }
```

To specify remote `git` dependencies, use the `git` field, which must contain a valid `url` in any format supported by `git`. For `git` dependencies, at most one of the `branch`, `tag`, or `commitId` fields can be configured to select a specific branch, tag, or commit hash, respectively. If multiple such fields are configured, only the highest-priority configuration will take effect, with priority order being `commitId` > `branch` > `tag`. For example, with the following configuration, the `pro0` and `pro1` modules from specific `git` repositories can be used in the source code:

```text
[dependencies]
  pro0 = { git = "git://github.com/org/pro0.git", tag = "v1.0.0"}
  pro1 = { git = "https://gitee.com/anotherorg/pro1", branch = "dev"}
```

In this case, `codepm` will download the latest version of the corresponding repository and save the current `commit-hash` in the `codepm.lock` file. All subsequent `codepm` calls will use the saved version until `codepm update` is used.

Authentication is typically required to access `git` repositories. `codepm` does not request credentials, so existing `git` authentication support should be used. If the protocol for `git` is `https`, an existing git credential helper must be used. On `Windows`, the credential helper can be installed with `git` and is used by default. On `Linux/macOS`, refer to the `git-config` documentation in the official `git` documentation for details on setting up credential helpers. If the protocol is `ssh` or `git`, key-based authentication should be used. If the key is passphrase-protected, developers should ensure `ssh-agent` is running and the key is added via `ssh-add` before using `codepm`.

The `dependencies` field can specify the compilation output type via the `output-type` attribute. The specified type can differ from the compilation output type of the source dependency itself and can only be `static` or `dynamic`, as shown below:

```text
[dependencies]
  pro0 = { path = "./pro0", output-type = "static" }
  pro1 = { git = "https://gitee.com/anotherorg/pro1", output-type = "dynamic" }
```

With the above configuration, the `output-type` settings in the `codepm.toml` files of `pro0` and `pro1` will be ignored, and these modules' outputs will be compiled as `static` and `dynamic` types, respectively.

### "test-dependencies"

This field has the same format as the `dependencies` field. It is used to specify dependencies that are only used during testing, not for building the main project. Module developers should use this field for dependencies that downstream users of this module do not need to be aware of.

Dependencies within `test-dependencies` can only be used in test files named like `xxx_test.code`. These dependencies will not be compiled during the build process. The configuration format for `test-dependencies` in `codepm.toml` is the same as for `dependencies`.

### "script-dependencies"

This field has the same format as the `dependencies` field. It is used to specify dependencies that are only used in build scripts, not for building the main project. Build script-related features will be detailed in the [Other-Build Scripts](#build-scripts) section.

### "replace"

This field has the same format as the `dependencies` field. It is used to specify replacements for indirect dependencies with the same name. The configured dependencies will be used as the final version during the compilation of the module.

For example, the module `aaa` depends on a local module `bbb`:

```text
[package]
  name = "aaa"

[dependencies]
  bbb = { path = "path/to/bbb" }
```

When the main module `demo` depends on `aaa`, `bbb` becomes an indirect dependency of `demo`. If the main module `demo` wants to replace `bbb` with another module of the same name, such as the `bbb` module in a different path `new/path/to/bbb`, it can configure as follows:

```text
[package]
  name = "demo"

[dependencies]
  aaa = { path = "path/to/aaa" }

[replace]
  bbb = { path = "new/path/to/bbb" }
```

After configuration, the actual indirect dependency `bbb` used when compiling the `demo` module will be the `bbb` module in `new/path/to/bbb`, and the `bbb` module in `path/to/bbb` configured in `aaa` will not be compiled.

> **Note:**
>
> Only the `replace` field of the entry module takes effect during compilation.

### "ffi.c"

Configuration for external C library dependencies of the current Codira module. This field configures the information required to depend on the library, including the library name and path.

Developers need to compile the dynamic or static library themselves and place it in the specified `path`. Refer to the example below.

Instructions for calling external C dynamic libraries in Codira:

- Compile the corresponding `hello.c` file into a `.so` library (execute `clang -shared -fPIC hello.c -o libhello.so` in the file path).
- Modify the project's `codepm.toml` file to configure the `ffi.c` field as shown in the example below. Here, `./src/` is the relative path of the compiled `libhello.so` to the current directory, and `hello` is the library name.
- Execute `codepm build` to compile successfully.

```text
[ffi.c]
hello = { path = "./src/" }
```

### "profile"

`profile` is a command profile configuration item used to control default settings when executing certain commands. Currently, the following scenarios are supported: `build`, `test`, `bench`, `run`, and `customized-option`.

#### "profile.build"

```text
[profile.build]
lto = "full"  # Whether to enable `LTO` (Link Time Optimization) compilation mode. This feature is only supported on `Linux` platforms.
incremental = true # Whether to enable incremental compilation by default
```

Control items for the compilation process. All fields are optional and will not take effect if not configured. Only the `profile.build` settings of the top-level module will take effect.

The `lto` configuration can be `full` or `thin`, corresponding to two compilation modes supported by `LTO` optimization: `full LTO` merges all compilation modules together for global optimization, which offers the greatest optimization potential but requires longer compilation time; `thin LTO` uses parallel optimization across multiple modules and supports incremental compilation by default, with shorter compilation time than `full LTO` but less optimization effectiveness due to reduced global information.

#### "profile.test"

```text
[profile.test] # Example usage
parallel=true
filter=*.*
no-color = true
timeout-each = "4m"
random-seed = 10
bench = false
report-path = "reports"
report-format = "xml"
verbose = true
[profile.test.build]
  compile-option = ""
  lto = "thin"
  mock = "on"
[profile.test.env]
MY_ENV = { value = "abc" }
codeHeapSize = { value = "32GB", splice-type = "replace" }
PATH = { value = "/usr/bin", splice-type = "prepend" }
```

Test configuration supports specifying options for compiling and running test cases. All fields are optional and will not take effect if not configured. Only the `profile.test` settings of the top-level module will take effect. The option list is consistent with the console execution options provided by `codepm test`. If an option is configured in both the configuration file and the console, the console option takes precedence over the configuration file. `profile.test` supports the following runtime options:

- `filter` specifies a test case filter, with a string value format consistent with the `--filter` value format in the [test command description](#test).
- `timeout-each <value>` where `value` is in the format `%d[millis|s|m|h]`, specifying the default timeout for each test case.
- `parallel` specifies the parallel execution scheme for test cases, with `value` as follows:
    - `<BOOL>`: `true` or `false`. When `true`, test classes can run in parallel, with the number of parallel processes controlled by the CPU cores on the system.
    - `nCores`: Specifies that the number of parallel test processes should equal the available CPU cores.
    - `NUMBER`: Specifies the number of parallel test processes as a positive integer.
    - `NUMBERnCores`: Specifies the number of parallel test processes as a multiple of the available CPU cores. The value should be positive (supports floating-point or integer values).
- `option:<value>`: Defines runtime options in conjunction with `@Configure`. For example:
    - `random-seed`: Specifies the random seed value, with a positive integer parameter value.
    - `no-color`: Specifies whether execution results are displayed without color in the console, with values `true` or `false`.
    - `report-path`: Specifies the path for generating test execution reports (cannot be configured via `@Configure`).
    - `report-format`: Specifies the report output format. Currently, unit test reports only support `xml` format (case-insensitive), and using other values will throw an exception (cannot be configured via `@Configure`). Performance test reports only support `csv` and `csv-raw` formats.
    - `verbose`: Specifies whether to display detailed compilation process information, with a `BOOL` parameter value (`true` or `false`).

#### "profile.test.build"

Used to specify supported compilation options, including:

- `compile-option`: A string containing additional `codec` compilation options, supplementing the top-level `compile-option` field.
- `lto`: Specifies whether to enable `LTO` optimization compilation mode, with values `thin` or `full`. This feature is only supported on `Linux` platforms.
- `mock`: Explicitly sets the `mock` mode, with possible options: `on`, `off`, `runtime-error`. The default value for `test`/`build` subcommands is `on`, and for `bench` subcommands, it is `runtime-error`.

#### "profile.test.env"

Used to configure temporary environment variables when running executable files during the `test` command. The `key` is the name of the environment variable to configure, with the following configuration items:

- `value`: Specifies the value of the environment variable.
- `splice-type`: Specifies the splicing method for the environment variable, optional, defaulting to `absent` if not configured. Possible values:
    - `absent`: The configuration only takes effect if no environment variable with the same name exists in the environment. If a variable with the same name exists, this configuration is ignored.
    - `replace`: The configuration replaces any existing environment variable with the same name.
    - `prepend`: The configuration is prepended to any existing environment variable with the same name.
    - `append`: The configuration is appended to any existing environment variable with the same name.

#### "profile.bench"

```text
[profile.bench] # Example usage
no-color = true
random-seed = 10
report-path = "bench_report"
baseline-path = ""
report-format = "csv"
verbose = true
```

Benchmark configuration supports specifying options for compiling and running test cases. All fields are optional and will not take effect if not configured. Only the `profile.bench` settings of the top-level module will take effect. The option list is consistent with the console execution options provided by `codepm bench`. If an option is configured in both the configuration file and the console, the console option takes precedence over the configuration file. `profile.bench` supports the following runtime options:

- `filter`: Specifies a test case filter, with a string value format consistent with the `--filter` value format in the [bench command description](#bench).
- `option:<value>`: Defines runtime options in conjunction with `@Configure`. For example:
    - `random-seed`: Specifies the random seed value, with a positive integer parameter value.
    - `no-color`: Specifies whether execution results are displayed without color in the console, with values `true` or `false`.
    - `report-path`: Specifies the path for generating test execution reports (cannot be configured via `@Configure`).
    - `report-format`: Specifies the report output format. Currently, unit test reports only support `xml` format (case-insensitive), and using other values will throw an exception (cannot be configured via `@Configure`). Performance test reports only support `csv` and `csv-raw` formats.
    - `verbose`: Specifies whether to display detailed compilation process information, with a `BOOL` parameter value (`true` or `false`).
    - `baseline-path`: Path to an existing report for comparison with current performance results. By default, it uses the `--report-path` value.

#### "profile.bench.build"

Used to specify additional compilation options for building executable files with `codepm bench`. The configuration is the same as `profile.test.build`.

#### "profile.bench.env"

Supports configuring environment variables when running executable files during the `bench` command, with the same configuration method as `profile.test.env`.

#### "profile.run"

Options for running executable files, supporting environment variable configuration `env` when running executable files during the `run` command, with the same configuration method as `profile.test.env`.

#### "profile.customized-option"

```text
[profile.customized-option]
cfg1 = "--cfg=\"feature1=lion, feature2=cat\""
cfg2 = "--cfg=\"feature1=tiger, feature2=dog\""
cfg3 = "-O2"
```

Custom options passed through to `codec`, enabled via `--cfg1 --cfg3`. Each module's `customized-option` settings apply to all packages within that module. For example, when executing the `codepm build --cfg1 --cfg3` command, the options passed to `codec` will be `--cfg="feature1=lion, feature2=cat" -O2`.

> **Note:**
>
> The conditional value here must be a valid identifier.

### "target"

Multi-backend, multi-platform isolation options, used to configure different settings for different backends and platforms. Taking the `Linux` system as an example, the `target` configuration is as follows:

```text
[target.x86_64-unknown-linux-gnu] # Configuration items for Linux systems
  compile-option = "value1" # Additional compilation command options
  override-compile-option = "value2" # Additional global compilation command options
  link-option = "value3" # Linker pass-through options
  [target.x86_64-unknown-linux-gnu.dependencies] # Source dependency configuration items
  [target.x86_64-unknown-linux-gnu.test-dependencies] # Test phase dependency configuration items
  [target.x86_64-unknown-linux-gnu.bin-dependencies] # Codira binary library dependencies
    path-option = ["./test/pro0", "./test/pro1"]
  [target.x86_64-unknown-linux-gnu.bin-dependencies.package-option]
    "pro0.xoo" = "./test/pro0/pro0.xoo.codeo"
    "pro0.yoo" = "./test/pro0/pro0.yoo.codeo"
    "pro1.zoo" = "./test/pro1/pro1.zoo.codeo"

[target.x86_64-unknown-linux-gnu.debug] # Debug configuration items for Linux systems
  [target.x86_64-unknown-linux-gnu.debug.test-dependencies]

[target.x86_64-unknown-linux-gnu.release] # Release configuration items for Linux systems
  [target.x86_64-unknown-linux-gnu.release.bin-dependencies]
```

Developers can add a series of configuration items for a `target` by configuring the `target.target-name` field. The `target` name can be obtained in the corresponding Codira environment via the command `codec -v`, where the `Target` item in the command output is the `target` name for that environment. The above example applies to the `Linux` system but is also applicable to other platforms, where the `target` name can similarly be obtained viaDedicated configuration items that can be configured for a specific `target` will apply to the compilation process under that `target`, as well as cross-compilation processes where other `targets` specify this `target` as the target platform. The list of configurable items is as follows:

- `compile-option`: Additional compilation command options
- `override-compile-option`: Additional global compilation command options
- `link-option`: Linker passthrough options
- `dependencies`: Source code dependency configuration items, with the same structure as the `dependencies` field
- `test-dependencies`: Test phase dependency configuration items, with the same structure as the `test-dependencies` field
- `bin-dependencies`: Codira binary library dependencies, whose structure is introduced later
- `compile-macros-for-target`: Macro package control items during cross-compilation. This option does not support distinguishing between `debug` and `release` compilation modes mentioned below.

Developers can configure `target.target-name.debug` and `target.target-name.release` fields to specify additional configurations unique to `debug` and `release` compilation modes for this `target`. The configurable items are the same as above. Items configured in these fields will only apply to the corresponding compilation mode of this `target`.

#### "target.target-name[.debug/release].bin-dependencies"

This field is used to import precompiled Codira library artifacts suitable for the specified `target`. The following example demonstrates importing three packages from the `pro0` and `pro1` modules.

> **Note:**
>
> Unless specifically required, it is not recommended to use this field. Please use the `dependencies` field introduced earlier to import module source code.

```text
├── test
│    ├── pro0
│    │    ├── libpro0.xoo.so
│    │    ├── pro0.xoo.codeo
│    │    ├── libpro0.yoo.so
│    │    └── pro0.yoo.codeo
│    └── pro1
│         ├── libpro1.zoo.so
│         └── pro1.zoo.codeo
├── src
│    └── main.code
└── codepm.toml
```

Method 1: Import via `path-option`:

```text
[target.x86_64-unknown-linux-gnu.bin-dependencies]
  path-option = ["./test/pro0", "./test/pro1"]
```

The `path-option` is a string array structure, where each element represents the path name to be imported. `codepm` will automatically import all Codira library packages under this path that comply with the rules. Compliance here refers to the library name format being the `full package name`. For example, in the above example, `pro0.xoo.codeo` should correspond to a library named `libpro0.xoo.so` or `libpro0.xoo.a`. Packages whose library names do not meet this rule can only be imported via the `package-option`.

Method 2: Import via `package-option`:

```text
[target.x86_64-unknown-linux-gnu.bin-dependencies.package-option]
  "pro0.xoo" = "./test/pro0/pro0.xoo.codeo"
  "pro0.yoo" = "./test/pro0/pro0.yoo.codeo"
  "pro1.zoo" = "./test/pro1/pro1.zoo.codeo"
```

The `package-option` is a `map` structure, where `pro0.xoo` serves as the `key` (strings containing `.` in `toml` configuration files must be enclosed in `""` when used as a whole), so the `key` value is `libpro0.xoo.so`. The path to the frontend file `codeo` serves as the `value`, and the corresponding `.a` or `.so` file for this `codeo` must be placed in the same path.

> **Note:**
>
> If the same package is imported via both `package-option` and `path-option`, the `package-option` field takes higher priority.

The following example shows how the source code `main.code` calls the `pro0.xoo`, `pro0.yoo`, and `pro1.zoo` packages.

```cangjie
import pro0.xoo.*
import pro0.yoo.*
import pro1.zoo.*

main(): Int64 {
    var res = x + y + z // x, y, z are values defined in pro0.xoo, pro0.yoo, and pro1.zoo respectively
    println(res)
    return 0
}
```

> **Note:**
>
> The dependent Codira dynamic library files may be `root` package compilation artifacts generated by other modules via the `profile.build.combined` configuration, containing symbols from all their sub-packages. Therefore, during dependency checking, if a package's corresponding Codira library is not found, the `root` package corresponding to that package will be used as a dependency, and a warning will be printed. Developers must ensure that `root` packages imported this way are generated via the corresponding method; otherwise, the library file may not contain the sub-package symbols, leading to compilation errors.
> For example, if the source code imports the `demo.aoo` package via `import demo.aoo`, and the binary dependency does not contain the corresponding Codira library for this package, `codepm` will attempt to find the dynamic library of the `root` package corresponding to this package, i.e., `libdemo.so`. If found, it will use this library as the dependency.

#### "target.target-name.compile-macros-for-target"

This field configures the cross-compilation method for macro packages, with the following three scenarios:

Method 1: By default, macro packages only compile artifacts for the local platform during cross-compilation, not for the target platform. This applies to all macro packages within the module.

```text
[target.target-platform]
  compile-macros-for-target = ""
```

Method 2: During cross-compilation, artifacts for both the local and target platforms are compiled. This applies to all macro packages within the module.

```text
[target.target-platform]
  compile-macros-for-target = "all" # The configuration item is a string, and the optional value must be "all"
```

Method 3: Specify that certain macro packages within the module should compile artifacts for both the local and target platforms during cross-compilation. Other unspecified macro packages will follow Method 1's default mode.

```text
[target.target-platform]
  compile-macros-for-target = ["pkg1", "pkg2"] # The configuration item is a string array, and the optional values are macro package names
```

#### "target" Field Merging Rules

Configuration items in `target` may coexist with other options in `codepm.toml`. For example, the `compile-option` field can also exist in the `package` field, with the difference that the `package` field applies to all `targets`. `codepm` merges these duplicate fields in a specific way. Taking the `debug` compilation mode for `x86_64-unknown-linux-gnu` as an example, the `target` configuration is as follows:

```text
[package]
  compile-option = "compile-0"
  override-compile-option = "override-compile-0"
  link-option = "link-0"

[dependencies]
  dep0 = { path = "./dep0" }

[test-dependencies]
  devDep0 = { path = "./devDep0" }

[target.x86_64-unknown-linux-gnu]
  compile-option = "compile-1"
  override-compile-option = "override-compile-1"
  link-option = "link-1"
  [target.x86_64-unknown-linux-gnu.dependencies]
    dep1 = { path = "./dep1" }
  [target.x86_64-unknown-linux-gnu.test-dependencies]
    devDep1 = { path = "./devDep1" }
  [target.x86_64-unknown-linux-gnu.bin-dependencies]
    path-option = ["./test/pro1"]
  [target.x86_64-unknown-linux-gnu.bin-dependencies.package-option]
    "pro1.xoo" = "./test/pro1/pro1.xoo.codeo"

[target.x86_64-unknown-linux-gnu.debug]
  compile-option = "compile-2"
  override-compile-option = "override-compile-2"
  link-option = "link-2"
  [target.x86_64-unknown-linux-gnu.debug.dependencies]
    dep2 = { path = "./dep2" }
  [target.x86_64-unknown-linux-gnu.debug.test-dependencies]
    devDep2 = { path = "./devDep2" }
  [target.x86_64-unknown-linux-gnu.debug.bin-dependencies]
    path-option = ["./test/pro2"]
  [target.x86_64-unknown-linux-gnu.debug.bin-dependencies.package-option]
    "pro2.xoo" = "./test/pro2/pro2.xoo.codeo"
```

When `target` configuration items coexist with public configuration items in `codepm.toml` or other levels of configuration items for the same `target`, they are merged according to the following priority:

1. Configuration for the corresponding `target` in `debug/release` mode
2. Configuration for the corresponding `target` unrelated to `debug/release`
3. Public configuration items

Taking the above `target` configuration as an example, the `target` configuration items are merged according to the following rules:

- `compile-option`: All applicable configuration items with the same name are concatenated according to priority, with higher-priority configurations appended later. In this example, in the `debug` compilation mode for `x86_64-unknown-linux-gnu`, the final `compile-option` value is `compile-0 compile-1 compile-2`. In `release` mode, it is `compile-0 compile-1`, and for other `targets`, it is `compile-0`.
- `override-compile-option`: Same as above. Since `override-compile-option` has higher priority than `compile-option`, in the final compilation command, the concatenated `override-compile-option` will be placed after the concatenated `compile-option`.
- `link-option`: Same as above.
- `dependencies`: Source code dependencies are merged directly. If conflicts exist, an error will be reported. In this example, in the `debug` compilation mode for `x86_64-unknown-linux-gnu`, the final `dependencies` are `dep0`, `dep1`, and `dep2`. In `release` mode, only `dep0` and `dep1` are active. For other `targets`, only `dep0` is active.
- `test-dependencies`: Same as above.
- `bin-dependencies`: Binary dependencies are merged according to priority. If conflicts exist, only the higher-priority dependency is added. For configurations with the same priority, `package-option` configurations are added first. In this example, in the `debug` compilation mode for `x86_64-unknown-linux-gnu`, binary dependencies from `./test/pro1` and `./test/pro2` are added. In `release` mode, only `./test/pro1` is added. Since `bin-dependencies` has no public configuration, no binary dependencies are active for other `targets`.

In the cross-compilation scenario of this example, if `x86_64-unknown-linux-gnu` is specified as the target `target` on other platforms, the configuration for `target.x86_64-unknown-linux-gnu` will also be merged with public configuration items according to the above rules and applied. If in `debug` compilation mode, the configuration for `target.x86_64-unknown-linux-gnu.debug` will also be applied.

### Environment Variable Configuration

Environment variables can be used to configure field values in `codepm.toml`. `codepm` will retrieve the corresponding environment variable values from the current runtime environment and substitute them into the actual configuration values. For example, the following `dependencies` field uses an environment variable for path configuration:

```text
[dependencies]
aoo = { path = "${DEPENDENCY_PATH}/aoo" }
```

When reading the module `aoo`, `codepm` will retrieve the `DEPENDENCY_PATH` variable value and substitute it to obtain the final path for the module `aoo`.

The list of fields that support environment variable configuration is as follows:

- The following fields in the single-module configuration field `package`:
    - Single-package compilation option `compile-option` in the `package-configuration` field
- The following fields in the workspace management field `workspace`:
    - Member module list `members`
    - Compilation module list `build-members`
    - Test module list `test-members`
- The following fields present in both `package` and `workspace`:
    - Compilation option `compile-option`
    - Global compilation option `override-compile-option`
    - Link option `link-option`
    - Compilation artifact output path `target-dir`
- The `path` field for local dependencies in the build dependency list `dependencies`
- The `path` field for local dependencies in the test dependency list `test-dependencies`
- The `path` field for local dependencies in the build script dependency list `script-dependencies`
- Custom passthrough option `customized-option` in the command profile configuration item `profile`
- The `path` field in the external `c` library configuration item `ffi.c`
- The following fields in the platform isolation option `target`:
    - Compilation option `compile-option`
    - Global compilation option `override-compile-option`
    - Link option `link-option`
    - The `path` field for local dependencies in the build dependency list `dependencies`
    - The `path` field for local dependencies in the test dependency list `test-dependencies`
    - The `path` field for local dependencies in the build script dependency list `script-dependencies`
    - The `path-option` and `package-option` fields in the binary dependency field `bin-dependencies`

## Configuration and Cache Directories

The storage path for files downloaded by `codepm` via `git` can be specified using the `CODEPM_CONFIG` environment variable. If not specified, the default location is `$HOME/.codepm` on `Linux/macOS` and `%USERPROFILE%/.codepm` on `Windows`.

## Codira Package Management Specification

In the Codira package management specification, for a file directory to be recognized as a valid source code package, the following requirements must be met:

1. It must directly contain at least one Codira code file.
2. Its parent package (including the parent's parent package, up to the `root` package) must also be a valid source code package. The module `root` package has no parent package, so it only needs to meet condition 1.

For example, consider the following `codepm` project named `demo`:

```text
demo
├──src
│   ├── main.code
│   └── pkg0
│        ├── aoo
│        │    └── aoo.code
│        └── boo
│             └── boo.code
└── codepm.toml
```

Here, the `demo.pkg0` directory does not directly contain Codira code, so `demo.pkg0` is not a valid source code package. Although the `demo.pkg0.aoo` and `demo.pkg0.boo` packages directly contain Codira code files `aoo.code` and `boo.code`, their upstream package `demo.pkg0` is not a valid source code package, so these two packages are also invalid.

When `codepm` detects a package like `demo.pkg0` that does not directly contain Codira files, it will treat it as a non-source package, ignore all its sub-packages, and print the following warning:

```text
Warning: there is no '.code' file in directory 'demo/src/pkg0', and its subdirectories will not be scanned as source code
```

Therefore, if developers need to configure a valid source code package, the package must directly contain at least one Codira code file, and all its upstream packages must also be valid source code packages. For the above `demo` project, to ensure that `demo.pkg0`, `demo.pkg0.aoo`, and `demo.pkg0.boo` are all recognized as valid source code packages, a Codira code file can be added to `demo/src/pkg0`, as shown below:

```text
demo
├── src
│    ├── main.code
│    └── pkg0
│         ├── pkg0.code
│         ├── aoo
│         │    └── aoo.code
│         └── boo
│              └── boo.code
└── codepm.toml
```

The `demo/src/pkg0/pkg0.code` file must be a Codira code file that complies with the package management specification. It does not need to contain functional code, as shown below:

```cangjie
package demo.pkg0
```

## Command Extension

`codepm` provides a command extension mechanism. Developers can extend `codepm` commands via executable files named in the format `codepm-xxx(.exe)`.

For an executable file `codepm-xxx` (`codepm-xxx.exe` on Windows), if the directory containing this file is configured in the system environment variable `PATH`, the following command can be used to run the executable:

```shell
codepm xxx [args]
```

Here, `args` represents the possible arguments passed to `codepm-xxx(.exe)`. The above command is equivalent to:

```shell
codepm-xxx(.exe) [args]
```

Running `codepm-xxx(.exe)` may depend on certain dynamic libraries. In such cases, developers must manually add the directory containing the required dynamic libraries to the environment variables.

The following example uses `codepm-demo`, an executable compiled from the following Codira code:

```cangjie
import std.process.*
import std.collection.*

main(): Int64 {
    var args = ArrayList<String>(Process.current.arguments)

    if (args.size < 1) {
        eprintln("Error: failed to get parameters")
        return 1
    }

    println("Output: ${args[0]}")

    return 0
}
```

After adding its directory to `PATH`, running the corresponding command will execute the executable and produce the corresponding output.

```text
Input: codepm demo hello,world
Output: Output: hello,world
```

The built-in commands of `codepm` have higher priority, so these commands cannot be extended in this way. For example, even if there is an executable named `codepm-build` in the system environment variables, `codepm build` will not execute that file but will instead run `codepm` with `build` as an argument passed to `codepm`.

## Build Scripts

`codepm` provides a build script mechanism that allows developers to define behaviors for `codepm` before or after executing specific commands.

The build script source file must be named `build.code` and located in the root directory of the Codira project, at the same level as `codepm.toml`. When creating a new Codira project using the `init` command, `codepm` does not create `build.code` by default. Developers who need it can manually create and edit `build.code` in the specified location according to the following template format.

```cangjie
// build.code

import std.process.*

// Case of pre/post codes for 'codepm build'.
/* called before `codepm build`
 * Success: return 0
 * Error: return any number except 0
 */
// func stagePreBuild(): Int64 {
//     // process before "codepm build"
//     0
// }

/*
 * called after `codepm build`
 */
// func stagePostBuild(): Int64 {
//     // process after "codepm build"
//     0
// }

// Case of pre codes for 'codepm clean'.
/* called before `codepm clean`
 * Success: return 0
 * Error: return any number except 0
 */
// func stagePreClean(): Int64 {
//     // process before "codepm clean"
//     0
// }

// For other options, define stagePreXXX and stagePostXXX in the same way.

/*
 * Error code:
 * 0: success.
 * other: codepm will finish running command. Check target-dir/build-script-cache/module-name/script-log for error outputs defind by user in functions.
 */

main(): Int64 {
    match (Process.current.arguments[0]) {
        // Add operation here with format: "pre-"/"post-" + optionName
        // case "pre-build" => stagePreBuild()
        // case "post-build" => stagePostBuild()
        // case "pre-clean" => stagePreClean()
        case _ => 0
    }
}
```

`codepm` supports defining pre- and post-command behaviors for a series of commands using build scripts. For example, for the `build` command, you can define `pre-build` in the `match` block within the `main` function to execute the desired functionality before the `build` command (the naming of the function is not restricted). Post-command behaviors can be defined similarly by adding a `post-build` case. The same approach applies to other commands by adding corresponding `pre/post` options and their associated functions.

After defining pre- and post-command behaviors, `codepm` will first compile `build.code` when executing the command and then run the corresponding behaviors before and after the command. For example, if `pre-build` and `post-build` are defined and `codepm build` is run, the entire `codepm build` process will follow these steps:

1. Before the build process, compile `build.code`;
2. Execute the function corresponding to `pre-build`;
3. Proceed with the `codepm build` compilation process;
4. After the compilation completes successfully, `codepm` will execute the function corresponding to `post-build`.

The following commands support build scripts:

- `build`, `test`, `bench`: Support both pre and post processes defined in dependent modules' build scripts.
- `run`, `install`: Only support running the pre and post build script processes of the corresponding module or executing the `pre-build` and `post-build` processes of dependent modules during compilation.
- `check`, `tree`, `update`: Only support running the pre and post build script processes of the corresponding module.
- `clean`: Only support running the pre-build script process of the corresponding module.

When executing these commands, if the `--skip-script` option is configured, all build script compilation and execution will be skipped, including those of dependent modules.

Usage notes for build scripts:

- The return value of the function must meet specific requirements: return `0` on success and any `Int64` value other than `0` on failure.
- All outputs from `build.code` will be redirected to the project directory at `build-script-cache/[target|release]/[module-name]/bin/script-log`. Developers can check the output content added in the functions in this file.
- If `build.code` does not exist in the project root directory, `codepm` will execute the normal process. If `build.code` exists and defines pre/post behaviors for a command, the command will abort abnormally if `build.code` fails to compile or the function returns a non-zero value, even if the command itself could execute successfully.
- In multi-module scenarios, the `build.code` build scripts of dependent modules take effect during compilation and unit testing. Outputs from dependent module build scripts are also redirected to log files in the corresponding module directory under `build-script-cache/[target|release]`.

For example, the following build script `build.code` defines pre- and post-build behaviors:

```cangjie
import std.process.*

func stagePreBuild(): Int64 {
    println("PRE-BUILD")
    0
}

func stagePostBuild(): Int64 {
    println("POST-BUILD")
    0
}

main(): Int64 {
    match (Process.current.arguments[0]) {
        case "pre-build" => stagePreBuild()
        case "post-build" => stagePostBuild()
        case _ => 0
    }
}
```

When executing `codepm build`, `codepm` will run `stagePreBuild` and `stagePostBuild`. After `codepm build` completes, the `script-log` file will contain the following output:

```text
PRE-BUILD
POST-BUILD
```

Build scripts can import dependent modules via the `script-dependencies` field in `codepm.toml`, with the same format as `dependencies`. For example, the following configuration in `codepm.toml` imports the `aoo` module, which contains a method named `aaa()`:

```text
[script-dependencies]
aoo = { path = "./aoo" }
```

You can then import this dependency in the build script and use the interface `aaa()`:

```cangjie
import std.process.*
import aoo.*

func stagePreBuild(): Int64 {
    aaa()
    0
}

func stagePostBuild(): Int64 {
    println("POST-BUILD")
    0
}

main(): Int64 {
    match (Process.current.arguments[0]) {
        case "pre-build" => stagePreBuild()
        case "post-build" => stagePostBuild()
        case _ => 0
    }
}
```

Build script dependencies (`script-dependencies`) are independent of source code dependencies (`dependencies` and `test-dependencies`). Source and test code cannot use modules from `script-dependencies`, and build scripts cannot use modules from `dependencies` or `test-dependencies`. If a module is needed in both build scripts and source/test code, it must be configured in both `script-dependencies` and `dependencies/test-dependencies`.

## Usage Examples

The following example demonstrates the usage of `codepm` with the directory structure of a Codira project. The corresponding source code files for this example can be found in [Source Code](#example-source-code). The module name of this Codira project is `test`.

```text
code_project
├── pro0
│    ├── codepm.toml
│    └── src
│         ├── zoo
│         │    ├── zoo.code
│         │    └── zoo_test.code
│         └── pro0.code
├── src
│    ├── koo
│    │    ├── koo.code
│    │    └── koo_test.code
│    ├── main.code
│    └── main_test.code
└── codepm.toml
```

### Using `init` and `build`

- Create a new Codira project and write source code files (e.g., `xxx.code`), such as the `koo` package and `main.code` file in the example structure.

    ```shell
    codepm init --name test --path .../code_project
    mkdir koo
    ```

    This will automatically generate the `src` folder and a default `codepm.toml` configuration file.

- When the current module depends on an external `pro0` module, create the `pro0` module and its configuration file. Then, write the source code for this module, manually creating the `src` folder under `pro0` and placing the Codira packages under `src`, such as the `zoo` package in the example structure.

    ```shell
    mkdir pro0 && cd pro0
    codepm init --name pro0 --type=static
    mkdir src/zoo
    ```

- When the main module depends on `pro0`, configure the `dependencies` field in the main module's configuration file as described in the manual. After ensuring the configuration is correct, execute `codepm build`. The generated executable will be in the `target/release/bin/` directory.

    ```shell
    cd code_project
    vim codepm.toml
    codepm build
    codepm run
    ```

### Using `test` and `clean`

- After writing the corresponding `xxx_test.code` unit test files for each file in the example structure, execute the following commands to run unit tests. The generated files will be in the `target/release/unittest_bin` directory.

    ```shell
    codepm test
    ```

    Or:

    ```shell
    codepm test src src/koo pro/src/zoo
    ```

- To manually delete intermediate files like `target`, execute:

    ```shell
    codepm clean
    ```

### Example Source Code

```cangjie
// code_project/src/main.code
package test

import pro0.zoo.*
import test.koo.*

main(): Int64 {
    let res = z + k
    println(res)
    let res2 = concatM("a", "b")
    println(res2)
    return 0
}

func concatM(s1: String, s2: String): String {
    return s1 + s2
}
```

```cangjie
// code_project/src/main_test.code
package test

import std.unittest.* // testfame
import std.unittest.testmacro.* // macro_Defintion

@Test
public class TestM{
    @TestCase
    func sayhi(): Unit {
        @Assert(concatM("1", "2"), "12")
        @Assert(concatM("1", "3"), "13")
    }
}
```

```cangjie
// code_project/src/koo/koo.code
package test.koo

public let k: Int32 = 12

func concatk(s1: String, s2: String): String {
    return s1 + s2
}
```

```cangjie
// code_project/src/koo/koo_test.code
package test.koo

import std.unittest.* // testfame
import std.unittest.testmacro.* // macro_Defintion

@Test
public class TestK{
    @TestCase
    func sayhi(): Unit {
        @Assert(concatk("1", "2"), "12")
        @Assert(concatk("1", "3"), "13")
    }
}
```

```cangjie
// code_project/pro0/src/pro0.code
package pro0
```

```cangjie
// code_project/pro0/src/zoo/zoo.code
package pro0.zoo

public let z: Int32 = 26

func concatZ(s1: String, s2: String): String {
    return s1 + s2
}```markdown
```cangjie
// code_project/pro0/src/zoo/zoo_test.code
package pro0.zoo

import std.unittest.* // Test framework
import std.unittest.testmacro.* // Macro definition

@Test
public class TestZ{
    @TestCase
    func sayhi(): Unit {
        @Assert(concatZ("1", "2"), "12")
        @Assert(concatZ("1", "3"), "13")
    }
}
```

```toml
# code_project/codepm.toml
[package]
codec-version = "1.0.0"
description = "nothing here"
version = "1.0.0"
name = "test"
output-type = "executable"

[dependencies]
pro0 = { path = "pro0" }
```

```toml
# code_project/pro0/codepm.toml
[package]
codec-version = "1.0.0"
description = "nothing here"
version = "1.0.0"
name = "pro0"
output-type = "static"
```