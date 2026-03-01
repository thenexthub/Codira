# Codira Formatter Developer Guide

## System Architecture

`codefmt (Codira Formatter)` is a code formatting tool specifically designed for the Codira language. `codefmt` supports automatically adjusting code indentation, spacing, and line breaks to help developers maintain clean and consistent code style effortlessly. Its overall technical architecture is shown in the following diagram:

![codefmt Architecture Diagram](../figures/codefmt-architecture.jpg)

As illustrated in the architecture diagram, the overall architecture of `codefmt` is as follows:

- Command-line Parameter Management: The command parameter processing module of `codefmt` supports file-level source code formatting, directory-level source code formatting, and block-level code formatting via commands. It also handles formatting style configuration and formatting result output.

- Configuration Management: Users configure formatting styles via the `cangjie-format.toml` configuration file, including indentation width, line width limits, and line break styles.

- Input Module: Processes input for formatting source code, accepting source code directories, files, or snippets, and forwards them to the formatting module for processing.

- Source Code Compilation: Invokes Codira's frontend capabilities to perform lexical and syntactic analysis on source code awaiting formatting, constructing the corresponding Abstract Syntax Tree (AST).

- Formatting: Traverses AST nodes to generate a nested intermediate structure. Applies formatting strategies to process this intermediate structure and transforms it into the target formatted source code.

- Output Module: Processes the formatted source code output, supporting overwriting the input file or outputting to a new directory or file.

## Directory Structure

The source code directory of `codefmt` is shown below, with main functionalities described in the comments.
```
codefmt/
|-- build                   # Build scripts
|-- config                  # Configuration files
|-- doc                     # Documentation
|-- include                 # Header files
|-- src
    |-- Format
        |-- DocProcessor    # Converts Doc struct to source code
        |-- NodeFormatter    # Converts AST nodes to Doc struct
```

## Installation and Usage Guide

`codefmt` requires the following tools for building:

- `clang` or `gcc` compiler

### Build Preparation

`codefmt` depends on `codec` for building. Refer to [SDK Build]() for build instructions.

### Build Steps

Local build process:

1. Get the latest source code via `git clone`:

    ```shell
    cd ${WORKDIR}
    git clone https://gitcode.com/Codira/cangjie_tools.git
    ```

2. Configure environment variables:

    ```shell
    export CODIRA_HOME=${WORKDIR}/cangjie    (for Linux/macOS)
    set CODIRA_HOME=${WORKDIR}/cangjie       (for Windows)
    ```

    `codefmt` compilation depends on `cangjie` build artifacts, so the `CODIRA_HOME` environment variable must point to the SDK location. `${WORKDIR}/cangjie` is just an example - adjust according to actual SDK location.

   > **Note:**
   >
   > - On Windows, ensure correct directory separators are used and Chinese characters in paths are properly handled.

3. Compile `codefmt` using build scripts in `codefmt/build`:

    ```shell
    cd cangjie_tools/codefmt/build
    python3 build.py build -t release
    ```

    Currently supports `debug` and `release` build types, specified via `-t` or `--build-type`.

4. Install to target directory:

    ```shell
    python3 build.py install
    ```

    Default installation path is `codefmt/dist`. Developers can specify installation directory via `--prefix`:

    ```shell
    python3 build.py install --prefix ./output
    ```

    Build output structure:

    ```
    dist/
    |-- bin
        `-- codefmt                   # Executable (codefmt.exe on Windows)
    |-- config
        `-- cangjie-format.toml     # Formatter config file
    ```

5. Verify installation:

    ```shell
    ./codefmt -h
    ```

    Execute this in the `bin` directory. If help info is displayed, installation succeeded. Note: The `codefmt` executable depends on `cangjie-lsp` dynamic library - ensure library path is in system environment variables. For Linux:

    ```shell
    export LD_LIBRARY_PATH=$CODIRA_HOME/tools/lib:$LD_LIBRARY_PATH
    ./codefmt -h
    ```

6. Clean build artifacts:

   ```shell
   python3 build.py clean
   ```

Cross-compiling for Windows from Linux:

```shell
export CODIRA_HOME=${WORKDIR}/cangjie
python3 build.py build -t release --target windows-x86_64
python3 build.py install
```

Output will be in `codefmt/dist`. Note: Windows version SDK is required for cross-compilation.

### Additional Build Options

View all build parameters via:

```shell
python3 build.py build -h
```

## API and Configuration Reference

`codefmt` provides the following main commands for project building and configuration management.

### Command Overview

Usage: `codefmt [option] file [option] file`

`codefmt -h` displays help info and options:

```text
Usage:
     codefmt -f fileName [-o fileName] [-l start:end]
     codefmt -d fileDir [-o fileDir]
Options:
   -h            Show usage
                     eg: codefmt -h
   -v            Show version
                     eg: codefmt -v
   -f            Specifies the file in the required format. The value can be a relative path or an absolute path.
                     eg: codefmt -f test.code
   -d            Specifies the file directory in the required format. The value can be a relative path or an absolute path.
                     eg: codefmt -d test/
   -o <value>    Output. If a single file is formatted, '-o' is followed by the file name. Relative and absolute paths are supported;
                 If a file in the file directory is formatted, a path must be added after -o. The path can be a relative path or an absolute path.
                     eg: codefmt -f a.code -o ./fmta.code
                     eg: codefmt -d ~/testsrc -o ./testout
   -c <value>    Specify the format configuration file, Relative and absolute paths are supported.
                 If the specified configuration file fails to be read, codefmt will try to read the default configuration file in CODIRA_HOME
                 If the default configuration file also fails to be read, will use the built-in configuration.
                     eg: codefmt -f a.code -c ./config/cangjie-format.toml
                     eg: codefmt -d ~/testsrc -c ~/home/project/config/cangjie-format.toml
   -l <region>   Only format lines in the specified region for the provided file. Only valid if a single file was specified.
                 Region has a format of [start:end] where 'start' and 'end' are integer numbers representing first and last lines to be formated in the specified file.
                 Line count starts with 1.
                     eg: codefmt -f a.code -o ./fmta.code -l 1:25
```

### File Formatting

`codefmt -f`

- Format and overwrite source file (supports relative/absolute paths):

```shell
codefmt -f ../../../test/uilang/Thread.code
```

- Use `-o` to output formatted code to new file:

```shell
codefmt -f ../../../test/uilang/Thread.code -o ../../../test/formated/Thread.code
```

### Directory Formatting

`codefmt -d`

- Format all Codira source files in specified directory:

```shell
codefmt -d test/              // Relative path

codefmt -d /home/xxx/test     // Absolute path
```

- Use `-o` to specify output directory (will be created if nonexistent). Note OS-specific path length limits (e.g. 260 chars on Windows, 4096 on Linux):

```shell
codefmt -d test/ -o /home/xxx/testout

codefmt -d /home/xxx/test -o ../testout/

codefmt -d testsrc/ -o /home/../testout   // Error if source directory doesn't exist
```

### Configuration File

`codefmt -c`

- Specify custom formatting configuration file:

```shell
codefmt -f a.code -c ./cangjie-format.toml
```

### Partial Formatting

`codefmt -l`

- Format only specified line range (works only with `-f`):

```shell
codefmt -f a.code -o .code -l 10:25 // Formats only lines 10-25
```

## Related Repositories

- [cangjie repo](https://gitcode.com/Codira/cangjie_compiler)
- [SDK Build](https://gitcode.com/Codira/cangjie_build)