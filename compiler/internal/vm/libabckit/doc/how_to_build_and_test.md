# Install dependencies:

Ensure the following components are installed:
- repo
- clang++-14
- clang-format-14
- clang-tidy-14
- doxygen
- graphviz
- Git LFS

# How to download

```sh
repo init -u https://gitee.com/ark-standalone-build/manifest.git -b master
repo sync -c -j8
repo forall -c 'git lfs pull'
./prebuilts_download.sh
```

# How to build and test

## Linux target (build on Linux host)

```sh
# Debug mode
./ark.py x64.debug abckit_packages --gn-args="is_standard_system=true abckit_enable=true enable_cmc_gc=false"
# Release mode
./ark.py x64.release abckit_packages --gn-args="is_standard_system=true abckit_enable=true enable_cmc_gc=false"
```

## Windows target (build on Linux host)

```sh
# Debug mode
./ark.py mingw_x86_64.debug abckit_packages --gn-args="is_standard_system=true abckit_enable=true enable_cmc_gc=false"
# Release mode
./ark.py mingw_x86_64.release abckit_packages --gn-args="is_standard_system=true abckit_enable=true enable_cmc_gc=false"
```

## macOS target (build on macOS host, compatible with ARM64 architecture)

```sh
# Debug mode
./ark.py mac_arm64.debug abckit_packages --gn-args="is_standard_system=true abckit_enable=true enable_cmc_gc=false"
# Release mode
./ark.py mac_arm64.release abckit_packages --gn-args="is_standard_system=true abckit_enable=true enable_cmc_gc=false"
```

## Output Locations

The generated abckit binary and libabckit.so/libabckit.dll are in the `out/${target}/arkcompiler/runtime_core/libabckit` directory.

The generated binary and library depend on the libraries in:
- `out/${target}/arkcompiler/runtime_core/`
- `out/${target}/arkcompiler/ets_runtime/`
- `out/${target}/thirdparty/icu/`
- `out/${target}/thirdparty/zlib/`

NOTE: Replace ${target} with build target: x64.debug, x64.release, mingw\_x86\_64.debug, mingw\_x86\_64.release, mac\_arm64.debug or mac\_arm64.release.

## Run unit tests

```sh
# Debug mode
./ark.py x64.debug abckit_tests --gn-args="is_standard_system=true abckit_enable=true abckit_enable_tests=true enable_cmc_gc=false"
# Release mode
./ark.py x64.release abckit_tests --gn-args="is_standard_system=true abckit_enable=true abckit_enable_tests=true enable_cmc_gc=false"
```

## Run unit tests with sanitizers

```sh
# Debug mode
./ark.py x64.debug abckit_tests --gn-args="is_standard_system=true abckit_enable=true abckit_with_sanitizers=true abckit_enable_tests=true enable_cmc_gc=false"
# Release mode
./ark.py x64.release abckit_tests --gn-args="is_standard_system=true abckit_enable=true abckit_with_sanitizers=true abckit_enable_tests=true enable_cmc_gc=false"
```

# How to use AbcKit

Requirements:
- AbcKit works with the release version of abc files
- Currently AbcKit supports abc files compiled from ArkTS and JS
- Currently AbcKit does not support CommonJS modules

AbcKit provides C API and C++ API to inspect and modify abc file.

Users can use AbcKit API to implement abckit plugins.

There are two ways to use abckit: by loading the abckit plugin through the abckit executable, or by loading it within DevEco Studio.

(1) By using the abckit executable to load an abckit plugin.

1. DownLoad and build the abckit.

2. Modify bytecode files by invoking the abckit APIs directly in your source code.

    transform.cpp
    ```
    /**
    * @brief Entry point for modifying a bytecode file.
    * @param file The AbckitFile corresponding to the parsed bytecode file to be modified.
    */
    extern "C" int Entry(AbckitFile *file)
    {
        // Use the AbcKit API here.
        return 0;
    }
    ```
3. Compile the source code into a dynamic library (for example, transformer.so) to be used as an abckit plugin.

    ```
    # Linux platform
    g++ --shared -o transform.so transform.cpp

    # Windows platform
    g++ --shared -o transform.dll transform.cpp

    # macOS platform
    g++ --shared -o transform.so transform.cpp

    ```
4. Run the following command to execute the abckit plugin.
Here, the --plugin-path parameter specifies the path to the dynamic library,
the --input-file parameter specifies the path to the bytecode file to be modified,
and the --output-file parameter specifies the path to the modified bytecode file.

    ```sh
    ./abckit --plugin-path transformer.so --input-file /path/to/input.abc --output-file /path/to/output.abc
    ```

(2) By loading the abckit plugin directly within DevEco Studio.

The ArkTS build toolchain provides the capability to [customize Ark bytecode at compile time](https://developer.huawei.com/consumer/en/doc/harmonyos-guides/customize-bytecode-during-compilation)
; users can load an abckit plugin during the compilation process to modify the bytecode. The procedure is as follows:

1. Write the source code that modifies the bytecode file. 

    transform.cpp
    ```c++
    /**
    * @brief Entry function for modifying a bytecode file.
    * @param abc_path The path to the bytecode file to be modified.
    */
    extern "C" int Transform(const char *abc_path)
    {
        // Load libabckit.so here, then use the abckit APIs to open and modify the bytecode file.
        return 0;
    }
    ```

2. Compile the dynamic library

    ```
    # Linux platform
    g++ --shared -o transform.so transform.cpp

    # Windows platform
    g++ --shared -o transform.dll transform.cpp

    # macOS platform
    g++ --shared -o transform.so transform.cpp

    ```

3. Configure the compilation option *transformLib* in the project build configuration file *build-profile.json5*. The path specified for this option should point to the dynamic library file generated in Step 2. For example, if the generated file is transform.dll and it is located in the module directory at ./lib/transform.dll, the configuration would be as follows:

    ```json
    {
        "buildOption": {
            "arkOptions": {
                "transformLib" : "./lib/transform.dll"
            }
        }
    }
    ```
4. Rebuild the project to complete the customized modification of the Ark bytecode file.


# Full tests (`self-check.sh`)

Remove out, build AbcKit, execute format, tidy, unit-tests and stress tests in debug and release modes:

```sh
# The /path/to/standalone/root refers to the directory where the project was initialized using repo init.
./arkcompiler/runtime_core/libabckit/scripts/self-check.sh --dir=/path/to/standalone/root
```

# Code coverage (`self-check.sh`)

Remove out, build AbcKit, execute unit-tests and stress tests in debug mode and collect code coverage:

```sh
# The /path/to/standalone/root refers to the directory where the project was initialized using repo init.
./arkcompiler/runtime_core/libabckit/scripts/self-check.sh --dir=/path/to/standalone/root --coverage
```
