# Codira Project Manager Developer Guide

## System Architecture

`codepm` is a project management tool for cangjie language, implemented in Codira language. It is designed to simplify user workflows. `codepm` provides capabilities for project creation, project building, project execution, unit testing, and more. The overall architecture of this tool is illustrated as follows:

![codepm Architecture Diagram](../figures/codepm-architecture.jpg)

As shown in the architecture diagram, the overall structure of `codepm` is as follows:

- Command-line Argument Processing: The command-line argument processing module of `codepm` supports project initialization, compilation and building, output execution, test case execution, dependency printing, dependency updates, output installation and uninstallation, and output cleanup via commands.
- Configuration Management: Users can configure various settings via the `codepm.toml` configuration file, including project information, dependency configurations, platform isolation settings, and workspace configurations. `codepm` also performs checks to ensure the validity of these configurations.
- Dependency Management: `codepm` manages user-configured source dependencies, including configuration validity checks, incremental identification, configuration updates and parsing, and configuration display. It also supports configuring binary dependencies for C and Codira languages, along with command-line dependency printing.
- Project Building: `codepm` automates dependency parsing to generate package-level dependency graphs. It then invokes the Codira compiler to compile packages in dependency order, completing the project build. `codepm` supports parallel and incremental compilation.
- Project Testing: `codepm` supports compiling test code within projects and running unit tests. It supports both single-package testing and module-level testing.
- Compiler Integration Module: This module facilitates interaction with the Codira compiler, implementing imperative compiler invocation and result forwarding.

Note that `codepm` is a package management tool for the Codira project, and its product types are static libraries, dynamic libraries, and executable binary. To complete the HAP/APP build and packaging of OpenHarmony, it is also necessary to use the DevEco Hvigor tool provided by DevEco Studio.

## Directory Structure

The source code directory of `codepm` is as follows, with main functionalities described in the comments.

```
codepm/src
|-- command       # Command functionality code
|-- config        # Configuration content code
|-- implement     # Internal interface implementation code
|-- toml          # TOML file read/write functionality code
`-- main.code       # Program main entry
```

## Installation and Usage Guide

### Build Preparation

`codepm` requires the following tools for building:

- `cangjie SDK`
    - Developers need to download the `cangjie SDK` for the corresponding platform: To compile native platform artifacts, the `SDK` version must match the current platform; to cross-compile `Windows` platform artifacts from a `Linux` platform, the `SDK` must be the `Linux` version.
    - Then, developers must execute the `envsetup` script of the corresponding `SDK` to ensure proper configuration.
    - If using the `python` build script method, developers can use a self-compiled `cangjie SDK`. Refer to [cangjie SDK](https://gitcode.com/Codira/cangjie_build) for compiling `cangjie SDK` from source.
- `stdx` binary library compatible with `cangjie SDK`
    - Developers need to download the `stdx` binary library for the target platform or compile it from `stdx` source code: To compile native platform artifacts, the `stdx` library must match the current platform; to cross-compile `Windows` platform artifacts from a `Linux` platform, the `stdx` library must be the `Windows` version.
    - Then, developers must configure the `stdx` binary library path in the environment variable `CODIRA_STDX_PATH`. If downloading pre-built `stdx` binaries, the path is the `static/stdx` directory under the extracted library directory; if compiling from `stdx` source, the path is the `static/stdx` directory under the corresponding platform directory in the `target` output directory (e.g., `target/linux_x86_64_codenative/static/stdx` for `Linux-x86`). Refer to [stdx Repository](https://gitcode.com/Codira/cangjie_stdx) for compiling `stdx` from source.
    - Additionally, if developers want to use `stdx` dynamic libraries as binary dependencies, they can replace `static` with `dynamic` in the above path configuration. `codepm` compiled this way cannot run independently; to enable standalone execution, the same `stdx` library path must be added to the system dynamic library environment variables.
- If using the `python` build script method, install `python3`.

### Build Steps

#### Build Method 1: Using `codepm` for Compilation

The `codepm` source code itself is a `codepm` module. Therefore, the `codepm` in `cangjie SDK` can be used to compile `codepm` source code. After configuring the `SDK` and `stdx`, developers can compile using the following command:

```
cd ${WORKDIR}/cangjie-tools/codepm
codepm build -o codepm
```

The compiled `codepm` executable directory structure is as follows:

```
// Linux/macOS
codepm/target/release/bin
`-- codepm

// Windows
codepm/target/release/bin
`-- codepm.exe
```

Currently, the `codepm.toml` in the source repository supports compiling `codepm` for the following platforms:
- `Linux/macOS` with `x86/arm` architecture;
- `Windows x86`.

Additionally, to cross-compile `Windows` platform artifacts from a `Linux` system, use the following command:

```
cd ${WORKDIR}/cangjie-tools/codepm
codepm build -o codepm --target x86_64-w64-mingw32
```

The compiled `codepm` executable directory structure is as follows:

```
codepm/target/x86_64-w64-mingw32/release/bin
`-- codepm.exe
```

#### Build Method 2: Using Build Script for Compilation

Developers can use the build script `build.py` in the `build` directory to compile `codepm`. To use `build.py`, developers can either configure `cangjie SDK` and `stdx` libraries or use self-compiled `cangjie SDK` and `stdx`.

The build process is as follows:

1. Compile `codepm` using the build script `build.py` in the `codepm/build` directory:

    ```shell
    cd ${WORKDIR}/cangjie-tools/codepm/build
    python3 build.py build -t release [--target native]        (for Linux/Windows/macOS)
    python3 build.py build -t release --target windows-x86_64  (for Linux-to-Windows)
    ```

    Developers must specify the build type with `-t, --build-type`, with options being `debug/release`.

    After successful compilation, the `codepm` executable will be generated in the `codepm/dist/` directory.

2. Modify environment variables:

    ```shell
    export PATH=${WORKDIR}/cangjie-tools/codepm/dist:$PATH     (for Linux/macOS)
    set PATH=${WORKDIR}/cangjie-tools/codepm/dist;%PATH%       (for Windows)
    ```

3. Verify `codepm` installation:

    ```shell
    codepm -h
    ```

    If the help information for `codepm` is displayed, the installation is successful.

> **Note:**
>
> On `Linux/macOS` systems, developers can configure the `runtime` dynamic library path in `rpath` so that the compiled `codepm` can locate the `runtime` dynamic library without relying on system dynamic library environment variables:
> - When using `codepm` for compilation, add the `--set-runtime-rpath` compile option to `codec` to ensure the compiled `codepm` can locate the `runtime` dynamic library of the current `cangjie SDK`. Add this option to the `compile-option` field in `codepm.toml` to apply it to the `codec` compile command.
> - When using the `build.py` script, specify the `rpath` path with the `--set-rpath RPATH` parameter.
> - In `cangjie SDK`, the `runtime` dynamic library directory is located under `$CODIRA_HOME/runtime/lib`, divided into subdirectories by platform. Use the corresponding platform's library directory for `rpath` configuration.

### Additional Build Options

The `build` function of `build.py` provides the following additional options:
- `--target TARGET`: Specifies the target platform for compilation (default: `native`). Currently, only cross-compiling `windows-x86_64` artifacts from `linux` is supported via `--target windows-x86_64`.
- `--set-rpath RPATH`: Specifies the `rpath` path.
- `-t, --build-type BUILD_TYPE`: Specifies the build type (`debug/release`).
- `-h, --help`: Displays help information for the `build` function.

Additionally, `build.py` provides the following extra functionalities:
- `install [--prefix PREFIX]`: Installs build artifacts to the specified path (default: `codepm/dist/`). Requires successful execution of `build` first.
- `clean`: Cleans build artifacts in the default path.
- `-h, --help`: Displays help information for `build.py`.

## API and Configuration Guide

`codepm` provides the following main commands and `toml` configuration files for project building and configuration management.

### Command Overview

Executing `codepm -h` displays the command list for `codepm`, as shown below:

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

Key commands are described below:

1. `init` command:
    - Initializes a new Codira module or workspace. For modules, it creates a `codepm.toml` configuration file and a `src` directory by default. For workspaces, it only creates `codepm.toml` and scans existing Codira modules to populate the `members` field. Skips file creation if `codepm.toml` or `main.code` already exists.

2. `build` command:
    - Builds the current Codira project. Dependency checks are performed before invoking `codec` for compilation.

3. `run` command:
    - Executes the compiled binary artifact of the current Codira project. Implicitly runs the `build` command to generate the executable.

4. `clean` command:
    - Cleans build artifacts of the current Codira project.

### Configuration Management

The module configuration file `codepm.toml` defines basic information, dependencies, compile options, etc. `codepm` primarily parses and executes based on this file.

Example configuration:

```toml
[package] # Single-module configuration (mutually exclusive with [workspace])
  codec-version = "1.0.0" # Minimum required `codec` version (required)
  name = "demo" # Module name and root package name (required)
  description = "nothing here" # Description (optional)
  version = "1.0.0" # Module version (required)
  compile-option = "" # Additional compile options (optional)
  override-compile-option = "" # Additional global compile options (optional)
  link-option = "" # Linker passthrough options (optional)
  output-type = "executable" # Output artifact type (required)
  src-dir = "" # Source directory path (optional)
  target-dir = "" # Output directory path (optional)
  package-configuration = {} # Per-package configuration (optional)

[workspace] # Workspace configuration (mutually exclusive with [package])
  members = [] # Workspace member modules (required)
  build-members = [] # Subset of members to build (optional)
  test-members = [] # Subset of build-members to test (optional)
  compile-option = "" # Workspace-wide compile options (optional)
  override-compile-option = "" # Workspace-wide global compile options (optional)
  link-option = "" # Workspace-wide linker options (optional)
  target-dir = "" # Output directory path (optional)

[dependencies] # Source dependencies (optional)
  coo = { git = "xxx", branch = "dev" } # Git dependency
  doo = { path = "./pro1" } # Local path dependency

[test-dependencies] # Test-phase dependencies (format same as [dependencies], optional)

[script-dependencies] # Build script dependencies (format same as [dependencies], optional)

[replace] # Dependency replacement (format same as [dependencies], optional)

[ffi.c] # C library dependencies (optional)
  clib1.path = "xxx"

[profile] # Command profile configuration (optional)
  build = {} # Build command configuration
  test = {} # Test command configuration
  bench = {} # Benchmark command configuration
  customized-option = {} # Custom passthrough options

[target.x86_64-unknown-linux-gnu] # Platform-specific configuration (optional)
  compile-option = "value1" # Platform-specific compile options
  override-compile-option = "value2" # Platform-specific global compile options
  link-option = "value3" # Platform-specific linker options

[target.x86_64-w64-mingw32.dependencies] # Platform-specific source dependencies (optional)

[target.x86_64-w64-mingw32.test-dependencies] # Platform-specific test dependencies (optional)

[target.x86_64-unknown-linux-gnu.bin-dependencies] # Platform-specific binary dependencies
  path-option = ["./test/pro0", "./test/pro1"] # Directory-based binary dependencies
[target.x86_64-unknown-linux-gnu.bin-dependencies.package-option] # File-based binary dependencies
  "pro0.xoo" = "./test/pro0/pro0.xoo.codeo"
  "pro0.yoo" = "./test/pro0/pro0.yoo.codeo"
  "pro1.zoo" = "./test/pro1/pro1.zoo.codeo"
```

Key configuration fields:

1. Dependency Configuration:
    - `dependencies`: Imports source dependencies for other Codira modules. Supports local path and remote Git dependencies.
      ```toml
      [dependencies]
      pro0 = { path = "./pro0" }
      ```
    - `ffi.c`: Configures external C library dependencies.
      ```toml
      [ffi.c]
      hello = { path = "./src/" }
      ```
    - `target.[target-name].bin-dependencies`: Imports pre-built Codira binary artifacts for specific platforms.
      ```toml
      [target.x86_64-unknown-linux-gnu.bin-dependencies]
          path-option = ["./test/pro0", "./test/pro1"]
      [target.x86_64-unknown-linux-gnu.bin-dependencies.package-option]
          "pro0.xoo" = "./test/pro0/pro0.xoo.codeo"
          "pro0.yoo" = "./test/pro0/pro0.yoo.codeo"
          "pro1.zoo" = "./test/pro1/pro1.zoo.codeo"
      ```

2. Cross-Platform Configuration:
    - The `target` field enables platform-specific configurations. Different configurations are applied when compiling the same project on different platforms.
      ```toml
      [target.x86_64-unknown-linux-gnu]
          compile-option = "value1"
          override-compile-option = "value2"
          link-option = "value3"
      ``````toml
        "pro0.yoo" = "./test/pro0/pro0.yoo.codeo"
        "pro1.zoo" = "./test/pro1/pro1.zoo.codeo"
    [target.x86_64-unknown-linux-gnu.ffi.c]
        "ctest" = "./test/c"

    [target.x86_64-unknown-linux-gnu.debug]
        [target.x86_64-unknown-linux-gnu.debug.test-dependencies]

    [target.x86_64-unknown-linux-gnu.release]
        [target.x86_64-unknown-linux-gnu.release.bin-dependencies]
    ```

### Additional Features

In addition to the aforementioned commands and configuration items, `codepm` also supports other features such as build scripts, command extensions, etc.

For detailed information on all commands, configuration items, and additional features of `codepm`, please refer to the [《Codira Project Manager User Guide》](./user_guide.md).

## Related Repositories

- [cangjie Repository](https://gitcode.com/Codira/cangjie_compiler)
- [stdx Repository](https://gitcode.com/Codira/cangjie_stdx)
- [cangjie SDK](https://gitcode.com/Codira/cangjie_build)