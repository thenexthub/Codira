## Basic build using CMake

Panda libraries can be built using CMake:
```
$ mkdir build
$ cd build
$ cmake ${panda_dir}
$ make
```

Where ${panda_dir} is a path to panda dirrectory.
These steps will create libraries and some debug targets (if you have installed additional libraries, like google-test, clang-format, clang-tidy, etc.).

## Build directory structure

In the current build directory structure, each project has its own subdirectory. For example, the vixl library is located inside the `third_party/vixl` folder. The root CMakeLists.txt has an entry for this directory:
```
add_subdirectory(third_party/vixl)
```
You may use built libraries in your component (e.g., `target_link_libraries(tests compiler base vixl)`), but for getting variables please use the INTERFACE includes (e.g. `target_include_directories(INTERFACE .)`).


## Check style 

To check style, build the project and then also build style-checker targets (you must install clang-format and clang-tidy with libraries - look at scripts/bootstrap*.sh):
```
$ make clang_format
    Built target clang_format_opt_tests_graph_creation_test.cpp
    Built target clang_format_opt_opt.h
    ...

$ make clang_tidy
    Scanning dependencies of target copy_json
    Move compile commands to root directory
    ...
    Built target copy_json
    Scanning dependencies of target clang_tidy_opt_codegen_codegen.cpp
    ...
```

You may force fixes for clang-format issues, with the `make clang_force_format` command.
Run `make help | grep clang` to see all possible clang-[format|style] targets.
For example, to check style issues in the opt.cpp file, you can use corresponding clang-format target (`make clang_format_opt_opt.cpp`) or the clang-tidy one (`make clang_tidy_opt_opt.cpp`). To force clang-format code style - `make clang_force_format_opt_opt.cpp`.
To check code-style through just one check-system - use `make clang_tidy` or `make clang_format`.

Generated files: 
*  `compile_commands.json` - json nija-commands file to correct execution clang-tidy.
*  Standard cmake-files: `CMakeCache.txt`, `Makefile`, `cmake_install.cmake`  and `CMakeFiles` folder.

[Coding style](../docs/coding-style.md)

*  Clang-tidy style file - `.clang-tidy`
*  Clang-format style file - `.clang-format`
*  Script to show diff through clang-format execution - `scripts/run-clang-format.py`

## Use wrappers instead of standard CMake funcions

In order to make possible importing prebuilt targets without recompiling them use the following wrappers instead of standard CMake functions:

* panda_add_executable
* panda_add_library
* panda_target_link_libraries
* panda_target_include_directories
* panda_target_compile_options
* panda_target_compile_definitions
* panda_target_sources

If you create a target using panda_add_executable or panda_add_library it is necessary to use other wrappers for this target(panda_target_link_libraries, panda_target_include_directories, etc) otherwise if PANDA_USE_PREBUILT_TARGETS is true then you will get a CMake error.

## Do not add new test to target "tests" dependencies directly

You have to add your new test to dependencies of existing test group "core_tests" or to common group for all tests in plugin("ecmascript_tests", "ets_tests", etc).

