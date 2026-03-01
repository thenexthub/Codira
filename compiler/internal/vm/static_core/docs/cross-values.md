# Cross-Values
`Cross-values` is a module that delivers implementation-defined constants (data structure sizes, offsets inside data structures, etc.), which is necessary for cross-compiling. For example, this allows generating binaries for `AArch64` on `x86-64` system.

`Cross-values` consists of (and may refer to) the following objects:
* A list of [*name*, *cpp-constant-expression*] pairs ([runtime/asm_defines/asm_defines.def](../runtime/asm_defines/asm_defines.def));
* An auto-generated file for each platform with constants' definition of format *"ptrdiff_t name = integer_literal;"* ([build/cross_values/generated_values/...](../build/cross_values/generated_values/)))
  * *"integer_literal"* is equal to the constant expression on the corresponding platform;
* An auto-generated file which contain a getter function for each value ([build/cross_values/cross_values.h](../build/cross_values/cross_values.h)));
  *  These getters take [*Arch*](../libarkbase/utils/arch.h) as an argument and return the value on that platform;
## Contents
1. [Rationale](#rationale)  
2. [Interface](#interface)
3. [Generated files example](#example-of-generated-cross-values)
4. [Principal mechanism](#principal-mechanism)
5. [Verification](#verification)
6. [Supported platforms](#supported-platforms)
7. [Sources](#sources)
## Rationale
`Cross-values` are intended to provide the following features:
* A common interface for platform-dependent values (see [Interface](#interface));
* Minimalization of manual work:
  * Changing of actual values (e.g. adding/removing new fields in data structures) will not break `cross-values`;
  * To add a cross-value, it is enough to write a single line containing a name and a corresponding constant-expression;
  * Regeneration is triggered automatically based on build dependencies;
* Freedom of toolchain choice:
  * Depending on the toolchain, data layout may differ even on the same platform; `cross-values` are designed with that in mind.

Another possible approach is to keep the same fixed layout on each platform for every data structure but it has several disadvantages:
1. Bloat of data structures (especialy on 32-bit systems);
2. Need of providing guarantees that offsets are the same across all toolchains;
3. Less flexibility of the code, problems with maintaing;
## Interface
1. `CMake`:  
  A set of variables to control targets' ".cmake"-toolchain:
   * *PANDA_CROSS_X86_64_TOOLCHAIN_FILE*
   * *PANDA_CROSS_AARCH64_TOOLCHAIN_FILE*
   * *PANDA_CROSS_AARCH32_TOOLCHAIN_FILE*
1. `C++`:  
  To use `cross-values`:
     1. Add desired value in [asm_defines.def](../runtime/asm_defines/asm_defines.def);
     2. Include [cross_values.h](build/cross_values/cross_values.h);
     3. Use the getter from [cross_values.h](build/cross_values/cross_values.h);

Values' names should be in `UPPER_CASE`; the name of the getter will be transformed in `UpperSnakeCase` with `Get` prefix. For convenience, names should reflect objects/things they represent and also should contain part of namespaces to avoid ambiguity.
```cpp
// bar.h
namespace foo {
    class Bar
    {
        data_t baz_;
    };
}  // namespace foo
```

```cpp
// asm_defines.def
#include "bar.h"

DEFINE_VALUE(FOO_BAR_BAZ_SIZE, sizeof(foo::Bar::baz_))
DEFINE_VALUE(FOO_BAR_BAZ_OFFSET, offsetof(foo::Bar, baz_))
...
```
```cpp
// platform_dependent_code.cpp
#include "cross_values.h"
...
    size_t size_x86_64 = ark::cross_values::GetFooBarBazSize(Arch::X86_64);
    size_t offs_x86_64 = ark::cross_values::GetFooBarBazOffset(Arch::X86_64);

    size_t size_aarch64 = ark::cross_values::GetFooBarBazSize(Arch::AARCH64);
    size_t offs_aarch64 = ark::cross_values::GetFooBarBazOffset(Arch::AARCH64);
...
```


## Example of generated cross-values
```cpp
// X86_64_values_gen.h:
namespace ark::cross_values::X86_64 {
    static constexpr ptrdiff_t FOO_BAR_BAZ_SIZE_VAL = 8;
    static constexpr ptrdiff_t FOO_BAR_BAZ_OFFSET_VAL = 0;
}  // namespace ark::cross_values::X86_64
```
```cpp
// AARCH64_values_gen.h:
namespace ark::cross_values::AARCH64 {
    static constexpr ptrdiff_t FOO_BAR_BAZ_SIZE_VAL = 8;
    static constexpr ptrdiff_t FOO_BAR_BAZ_OFFSET_VAL = 0;
}  // namespace ark::cross_values::AARCH64
```
```cpp
// AARCH32_values_gen.h:
namespace ark::cross_values::AARCH32 {
    static constexpr ptrdiff_t FOO_BAR_BAZ_SIZE_VAL = 4;
    static constexpr ptrdiff_t FOO_BAR_BAZ_OFFSET_VAL = 0;
}  // namespace ark::cross_values::AARCH32
```

```cpp
// cross_values.h:
namespace ark::cross_values {
...
[[maybe_unused]] static constexpr ptrdiff_t GetFooBarBazSize(Arch arch)
{
    switch (arch) {
        case Arch::X86_64:
            return cross_values::X86_64::FOO_BAR_BAZ_SIZE_VAL;
        case Arch::AARCH64:
            return cross_values::AARCH64::FOO_BAR_BAZ_SIZE_VAL;
        case Arch::AARCH32:
            return cross_values::AARCH32::FOO_BAR_BAZ_SIZE_VAL;
        default:
            LOG(FATAL, COMMON) << "No cross-values generated for " << GetStringFromArch(arch);
            UNREACHABLE();
    }
}
...
}  // namespace ark::cross_values
```

## Principal mechanism
The idea is to
1) Gather values ([asm_defines.def](../runtime/asm_defines/asm_defines.def));
2) Translate `constexpr` to literals using cross-compilers ([defines.cpp](../runtime/asm_defines/defines.cpp) -> `libasm_defines.S`);
3) Generate '`.h`' file with the values' definition (via [cross_values_generator.rb](../cross_values/cross_values_generator.rb));
4) Generate getters (via [cross_values_getters_enerator.rb](../cross_values/cross_values_getters_generator.rb) -> `cross_values.h`);
5) Include `cross_values.h` to use the values.
   
It is allowed to choose an arbitrary cross-compiler (see [Interface](#interface)), `gcc-toolchain` is used by default.

For each target an auxiliary binary directory with the project is configured with a single `asm_defines` target being built (see [CMake-schematic](#schematic-of-cmake-based-build)). These sub-builds are configured from the same sources as the primary build and added to dependency graph via CMake's `ExternalProject` so `cross-values`' regeneration is triggered automatically.

#### Schematic of CMake-based build:
```
          /-root_ark_build_dir---------------------\
          |      cross_values/                     |
          |      . auxiliary_binary_dirs/          |----+--->/-aarch32_toolchain_build_dir-\
          |      . generated_values/               |    |    |  runtime/                   |
          |      . . test/                         |    |    |  . asm_defines/             |
          |  +-> . . . values_root_build_gen.h     |    |    |  . . libasm_defines.S       |------+
+------------|-> . . AARCH32_values_gen.h-------+  |    |    \-----------------------------/      |
| +----------|-> . . AARCH64_values_gen.h-------+  |    |                                         |
| | +--------|-> . . X86_64_values_gen.h--------+  |    |                                         |
| | |     |  |     cross_values.h               |  |    +--->/-aarch64_toolchain_build_dir-\      |
| | |     |  |     ^-----------------(#include)-+  |    |    |  runtime/                   |      |
| | |     |  |                                     |    |    |  . asm_defines/             |      |
| | |     |  |   runtime/                          |    |    |  . . libasm_defines.S       |----+ |
| | |     |  |   . asm_defines                     |    |    \-----------------------------/    | |
| | |     |  |   . . libasm_defines.S-----------+  |    |                                       | |
| | |     |  |                                  |  |    |                                       | |
| | |     |  +----------------------------------+  |    |                                       | |
| | |     |                                        |    +--->/-x86-64_toolchain_build_dir--\    | |
| | |     \----------------------------------------/         |  runtime/                   |    | |
| | |                                                        |  . asm_defines/             |    | |
| | |                                                        |  . . libasm_defines.S       |--+ | |
| | |                                                        \-----------------------------/  | | |
| | |                                                                                         | | |
| | |                                                                                         | | |
| | +-----------------------------------------------------------------------------------------+ | |
| +---------------------------------------------------------------------------------------------+ |
+-------------------------------------------------------------------------------------------------+
```
Besides `CMake`, `GN` build is supported, with some restrictions. As `GN`-build is intended for target's bootstrapping, no cross-compilation is supported; `cross-values` are generated from the root `libasm_defines.S` and there are no any of `"auxiliary_binary_dirs"` (see [GN-schematic](#schematic-of-gn-based-build)).
#### Schematic of GN-based build:
```
/-root_ark_build_dir-(AArch64)------------\
|      cross_values/                      |
|      . generated_values/                |
|  +-> . . AARCH64_values_gen.h--------+  |
|  |     cross_values.h                |  |
|  |     ^------------------(#include)-+  |
|  |                                      |
|  |   runtime/                           |
|  |   . asm_defines                      |
|  |   . . libasm_defines.S------------+  |
|  |                                   |  |
|  +-----------------------------------+  |
|                                         |
\-----------------------------------------/
```
## Verification
As it was stated, even on the same platform the values may differ depending on the toolchain. To detect such situation, a `CRC-32` checksum is generated based on '`.h`'-files with values (for each architecture), stored in aot- and runtime-binaries and compared on loading of aot-files (see [entrypoints_compiler_checksum.inl.erb](../runtime/entrypoints/entrypoints_compiler_checksum.inl.erb)).  

There are also plain diff-compares of values that should prevent errors caused by the tangled `CMake`-configuration:
1. between target's `cross-values` and host-tools' `cross-values` (see [HostTools.cmake](../cmake/HostTools.cmake));
2. between `values_root_build_gen.h` (which is similiar to `cross-values`, but generated from the root `libasm_defines.S`, see [CMake-schematic](#schematic-of-cmake-based-build)) and `cross-values` for the corresponding architecture.


## Supported platforms

Currently, the following architectures are supported:
* x86-64
* AArch64
* AArch32

Cross-compilation is available only for x86-64.

## Sources
* [cross_values/BUILD.gn](../cross_values/BUILD.gn)
* [cross_values/CMakeLists.txt](../cross_values/CMakeLists.txt)
* [cross_values/cross_values_generator.rb](../cross_values/cross_values_generator.rb)
* [cross_values/cross_values_getters_generator.rb](../cross_values/cross_values_getters_generator.rb)
* [cross_values/diff_check_values.sh](../cross_values/diff_check_values.sh)
* [cross_values/wrap_cross_values_getters_generator.sh](../cross_values/wrap_cross_values_getters_generator.sh)
* [runtime/asm_defines/asm_defines.def](../runtime/asm_defines/asm_defines.def)
