# 安装依赖项：

确保安装以下组件：
- repo
- clang++-14
- clang-format-14
- clang-tidy-14
- doxygen
- graphviz
- Git LFS

# 如何下载

```sh
repo init -u https://gitee.com/ark-standalone-build/manifest.git -b master
repo sync -c -j8
repo forall -c 'git lfs pull'
./prebuilts_download.sh
```

# 如何构建和测试

## 构建 Linux 平台的 AbcKit

```sh
# debug模式
./ark.py x64.debug abckit_packages --gn-args="is_standard_system=true abckit_enable=true enable_cmc_gc=false"
# release模式
./ark.py x64.release abckit_packages --gn-args="is_standard_system=true abckit_enable=true enable_cmc_gc=false"
```

## 构建 Windows 平台的 AbcKit
```sh
# debug模式
./ark.py mingw_x86_64.debug abckit_packages --gn-args="is_standard_system=true abckit_enable=true enable_cmc_gc=false"
# release模式
./ark.py mingw_x86_64.release abckit_packages --gn-args="is_standard_system=true abckit_enable=true enable_cmc_gc=false"
```

## 构建 macos 平台的 AbcKit （适用于ARM64架构）

```sh
# debug模式
./ark.py mac_arm64.debug abckit_packages --gn-args="is_standard_system=true abckit_enable=true enable_cmc_gc=false"
# release模式
./ark.py mac_arm64.release abckit_packages --gn-args="is_standard_system=true abckit_enable=true enable_cmc_gc=false"
```

## 构建产物的位置

生成的 Abckit 二进制文件和 libabckit.so/libabckit.dll 位于 `out/${target}/arkcompiler/runtime_core/libabckit` 目录。

生成的二进制文件和库依赖于以下目录中的库：
- `out/${target}/arkcompiler/runtime_core/`
- `out/${target}/arkcompiler/ets_runtime/`
- `out/${target}/thirdparty/icu/`
- `out/${target}/thirdparty/zlib/`

注意：根据编译方式将 ${target} 替换为：x64.debug、x64.release、mingw\_x86\_64.debug 或 mingw\_x86\_64.release。

## 运行单元测试

```sh
# debug模式
./ark.py x64.debug abckit_tests --gn-args="is_standard_system=true abckit_enable=true abckit_enable_tests=true enable_cmc_gc=false"
# release模式
./ark.py x64.release abckit_tests --gn-args="is_standard_system=true abckit_enable=true abckit_enable_tests=true enable_cmc_gc=false"
```

## 使用 Sanitizer 运行单元测试

```sh
# debug模式
./ark.py x64.debug abckit_tests --gn-args="is_standard_system=true abckit_enable=true abckit_with_sanitizers=true abckit_enable_tests=true enable_cmc_gc=false"
# release模式
./ark.py x64.release abckit_tests --gn-args="is_standard_system=true abckit_enable=true abckit_with_sanitizers=true abckit_enable_tests=true enable_cmc_gc=false"
```

# 如何使用 AbcKit

要求：
- AbcKit 适用于 `release` 模式的方舟字节码文件
- 目前 AbcKit 支持从 ArkTS 或 JS 编译的方舟字节码文件
- 目前 AbcKit 不支持 CommonJS 模块

AbcKit 提供 C API 和 C++ API 来读取和修改方舟字节码文件。

用户可以使用 AbcKit API 来实现 Abckit 插件。

用户可以通过两种方式对字节码文件进行修改，第一种使用abckit可执行程序加载abckit插件，第二种在Deveco Studio中加载abckit插件。

(1) abckit可执行程序加载abckit插件

1. 下载全量代码，构建AbcKit工具。

2. 参考abckit提供的API接口，在源码中调用API对字节码文件进行修改。

    transform.cpp

    ```cpp
    /**
    * @brief 字节码文件修改的入口方法
    * @param file 待修改的字节码文件解析后对应的AbckitFile，解析工作由AbcKit完成
    */
    extern "C" int Entry(AbckitFile *file)
    {
        // 用户可以在这里使用 AbcKit API。
        return 0;
    }
    ```

3. 将源码编译为动态库（例如 transformer.so）作为 abckit 插件。

    ```
    # Linux 平台
    g++ --shared -o transform.so transform.cpp

    # Windows 平台
    g++ --shared -o transform.dll transform.cpp

    # mac 平台
    g++ --shared -o transform.so transform.cpp

    ```

4. 运行以下命令来执行 abckit 插件。其中，--plugin-path参数值为动态库的路径，--input-file参数为待修改的字节码文件路径，--output-file参数为修改后的字节码文件路径。

    ```sh
    ./abckit --plugin-path transformer.so --input-file /path/to/input.abc --output-file /path/to/output.abc
    ```

(2) Deveco Studio中加载abckit插件

ArkTS编译工具链提供了[编译期自定义修改方舟字节码](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/customize-bytecode-during-compilation)的能力，用户可以在中加载abckit插件修改字节码，具体方式如下：

1. 用户编写修改字节码文件的源码。

    transform.cpp

    ```c++
    /**
    * @brief 字节码文件修改的入口方法
    * @param abc_path 待修改的字节码文件的存储路径
    */
    extern "C" int Transform(const char *abc_path)
    {
        // 用户可以在这里加载libabckit.so动态库，然后使用abckit提供的api打开并修改字节码文件
        return 0;
    }
    ```

2. 使用c语言编译工具编译动态库

    ```
    # Linux 平台
    g++ --shared -o transform.so transform.cpp

    # Windows 平台
    g++ --shared -o transform.dll transform.cpp

    # mac 平台
    g++ --shared -o transform.so transform.cpp

    ```

3. 在编译工程配置文件build-profile.json5中配置编译选项transformLib，选项中配置的路径为步骤2中生成的链接库文件在项目中的路径, 这里以transform.dll为例，假设该文件在模块中的路径为"./lib/transform.dll"

    ```json
    {
        "buildOption": {
            "arkOptions": {
                "transformLib" : "./lib/transform.dll"
            }
        }
    }
    ```
4. 重新编译项目，即可完成对方舟字节码文件的自定义修改

# 全量测试

删除 out 目录，构建 AbcKit，在debug和release模式下执行`self-check.sh` 脚本。该脚本会进行代码格式化、代码检查、单元测试和压力测试。

```sh
# /path/to/standalone/root 为项目执行 repo init 的根目录
./arkcompiler/runtime_core/libabckit/scripts/self-check.sh --dir=/path/to/standalone/root
```

# 代码覆盖率

删除 out 目录，构建 AbcKit，在debug模式下执行 `self-check.sh` 脚本。该脚本会执行单元测试和压力测试，并收集代码覆盖率。

```sh
# /path/to/standalone/root 为项目执行 repo init 的根目录
./arkcompiler/runtime_core/libabckit/scripts/self-check.sh --dir=/path/to/standalone/root --coverage
```

