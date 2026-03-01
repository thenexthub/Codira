# 使用的开源软件说明

## libboundscheck

### 代码来源说明

该仓被仓颉运行时源码依赖，仓库源码地址为 [third_party_bounds_checking_function](https://gitcode.com/openharmony/third_party_bounds_checking_function)，版本为 [OpenHarmony-v6.0-Release](https://gitcode.com/openharmony/third_party_bounds_checking_function/tags/OpenHarmony-v6.0-Release)。

该开源软件被仓颉运行时以包含头文件的方式使用，并通过链接库（动态库或静态库）的方式依赖。

### 构建说明

本仓库通过 CMake 作为子目标项目引入，编译过程中会自动完成构建并建立依赖关系，具体构建参数请参见 [CMakeLists.txt](../CMakeLists.txt) 文件。