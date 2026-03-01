# 仓颉覆盖率工具开发者指南

## 开源项目介绍

`codecov (Codira Coverage)` 仓颉覆盖率工具是一款基于仓颉语言编程规范开发的代码覆盖率工具。其整体技术架构如图所示：

![codecov架构设计图](../figures/codecov_architecture.jpg)

## 目录

` codecov ` 源码目录如下图所示，其主要功能如注释中所描述。

```text
codecov/
|-- build                   # 构建脚本
|-- doc                     # 介绍文档
|-- figures                 # 图片
|-- src                     # 源码相关文件
`-- tests                   # 测试用例
```

## 安装和使用指导

`codecov` 需要以下工具来构建：

- `cangjie SDK`
    - 开发者需要下载对应平台的 `cangjie SDK`：若想要编译本地平台产物，则需要的 `SDK` 为当前平台对应的版本；若想要从 `Linux` 平台交叉编译获取 `Windows` 平台的产物，则需要的 `SDK` 为 `Linux` 版本。
    - 然后，开发者需要执行对应 `SDK` 的 `envsetup` 脚本，确保 `SDK` 被正确配置。
    - 若使用 `python` 构建脚本方式编译，可以使用自行编译的 `cangjie SDK`。请参阅 [cangjie SDK](https://gitcode.com/Codira/cangjie_build) 了解源码编译 `cangjie SDK` 的方法。
- 与 `cangjie SDK` 配套的 `stdx` 二进制库
    - 开发者需要下载目标平台的 `stdx` 二进制库，或通过 `stdx` 源码编译出对应的 `stdx` 二进制库：若想要编译本地平台产物，则需要的 `stdx` 库为当前平台对应的版本；若想要从 `Linux` 平台交叉编译获取 `Windows` 平台的产物，则需要的 `stdx` 库为 `Windows` 版本。
    - 然后，开发者需要将 `stdx` 二进制库路径配置到环境变量 `CODIRA_STDX_PATH` 中。若开发者直接下载 `stdx` 二进制库，则路径为解压得到的库目录下的 `static/stdx` 目录；若开发者通过 `stdx` 源码编译，则路径为 `stdx` 编译产物目录 `target` 下对应平台目录下的 `static/stdx` 目录，例如 `Linux-x86` 下为 `target/linux_x86_64_codenative/static/stdx`。
    - 此外，如果开发者想使用 `stdx` 动态库作为二进制库依赖，可以将上述路径配置中的 `static` 改为 `dynamic`。以此方式编译的 `codecov` 无法独立运行，若想让其能够独立运行，需要将相同的 `stdx` 库路径配置到系统动态库环境变量中。
- 若使用 `python` 构建脚本方式编译，请安装 `python3`

### 构建准备

- `codecov` 构建依赖 `codec`, 构建方式参见[SDK 构建](https://gitcode.com/Codira/cangjie_build/blob/dev/README_zh.md)
- `codecov` 构建依赖 `stdx`, 请参阅 [stdx 仓](https://gitcode.com/Codira/cangjie_stdx) 获取源码编译 `stdx` 库的方法。

### 构建步骤

#### 构建方式一：使用构建脚本编译

1. 通过 `git clone` 命令获取 `codecov` 的最新源码：

    ```shell
    cd ${WORKDIR}
    git clone https://gitcode.com/Codira/cangjie_tools.git
    ```

2. 配置环境变量：

    ```shell
    export CODIRA_HOME=${WORKDIR}/cangjie    (for Linux/macOS)
    set CODIRA_HOME=${WORKDIR}/cangjie       (for Windows)
    ```

    `codecov` 的编译依赖 `cangjie` 仓和 `stdx` 仓的编译产物, 因此需要配置环境变量 `CODIRA_HOME` 指定 `SDK` 的位置。上述 `${WORKDIR}/cangjie` 仅作为示意，请根据 `SDK` 的实际位置进行调整。配置环境变量 `CODIRA_STDX_PATH` 指定对应平台目录下的 `static/stdx` 目录。

   > **注意：**
   >
   > `Windows` 的环境变量配置需要正确使用目录分隔符，并确认路径中的中文字符被正确识别和处理。

3. 通过 `codecov/build` 目录下的构建脚本编译 `codecov`：

    ```shell
    cd cangjie-tools/codecov/build
    python3 build.py build -t release
    ```

    当前支持 `debug`、`release` 两种编译类型，开发者需要通过 `-t` 或者 `--build-type` 指定。

4. 安装到指定目录：

    ```shell
    python3 build.py install
    ```

    默认安装到 `codecov/dist` 目录下，支持开发者通过 `install` 命令的参数 `--prefix` 指定安装目录：

    ```shell
    python3 build.py install --prefix ./output
    ```

    编译产物目录结构为: dist/codecov 

5. 验证 `codecov` 是否安装成功：

    ```shell
    ./codecov -h
    ```

    开发者进入安装路径的 `bin` 目录下执行上述操作，如果输出 `codecov` 的帮助信息，则表示安装成功。以 `Linux` 环境为例：

    ```shell
    export LD_LIBRARY_PATH=$CODIRA_HOME/tools/lib:$LD_LIBRARY_PATH
    ./codecov -h
    ```

6. 清理编译中间产物：

   ```shell
   python3 build.py clean
   ```

当前同样支持 `Linux` 平台交叉编译 `Windows` 下运行的 `codecov` 产物，构建指令如下：

```shell
export CODIRA_HOME=${WORKDIR}/cangjie
python3 build.py build -t release --target windows-x86_64
python3 build.py install
```

执行该命令后，构建产物默认位于 `codecov/dist` 目录下。请注意从 `Linux` 平台交叉编译获取 `Windows` 平台的产物，则需要的 `SDK` 为 `Windows` 版本。

#### 构建方式二：使用 `codepm` 编译

`codecov` 源码是一个 `codepm` 模块。因此，可以使用 `cangjie SDK` 中的 `codepm` 编译 `codecov` 源码。配置完 `SDK` 和 `stdx` 后，开发者可以通过如下命令编译：

```
cd ${WORKDIR}/cangjie-tools/codecov
codepm build -o codecov
```

编译出来的 `codecov` 可执行文件目录如下：

```
// Linux/macOS
codecov/target/release/bin
`-- codecov

// Windows
codecov/target/release/bin
`-- codecov.exe
```

目前源码仓中的 `codepm.toml` 支持编译如下平台的 `codecov`：

- `Linux/macOS` 的 `x86/arm` 框架；
- `Windows x86`。

此外，若开发者想在 `Linux` 系统中交叉编译 `Windows` 平台产物，可以使用如下命令：

```
cd ${WORKDIR}/cangjie-tools/codecov
codepm build -o codecov --target x86_64-w64-mingw32
```

编译出来的 `codecov` 可执行文件目录如下：

```
codecov/target/x86_64-w64-mingw32/release/bin
`-- codecov.exe
```

### 更多构建选项

`python3 build.py build` 命令也支持其它参数设置，具体可通过如下命令来查询：

```shell
python3 build.py build -h
```

## API 和配置说明

`codecov` 提供以下主要命令，用于项目构建和配置管理。

### 命令介绍

使用命令行操作 `codecov [option] file [option] file`

`codecov -h` 获取帮助信息，选项介绍如下：

```text
Usage: codecov [options]

A tool used to summarize the coverage in html reports.

Options:
  -v, --version                 Print the version number, then exit.
  -h, --help                    Show this help message, then exit.
  -r ROOT, --root=ROOT          The root directories of your source files, defaults to '.', the current directory.
                                File names are reported relative to this root.
  -o OUTPUT, --output=OUTPUT    The output directories of html reports, defaults to '.', the current directory.
  -b, --branches                Report the branch coverage. (It is an experimental feature and may generate imprecise branch coverage.)
  --verbose                     Print some detail messages, including parsing data for the gcov file.
  --html-details                Generate html reports for each source file.
  -x, --xml                     Generate a xml report.
  -j, --json                    Generate a json report.
  -k, --keep                    Keep gcov files after processing.
  -s SOURCE, --source=SOURCE    The directories of cangjie source files.
  -e EXCLUDE, --exclude=EXCLUDE
                                The cangjie source files starts with EXCLUDE will not be showed in coverage reports.
  -i INCLUDE, --include=INCLUDE
                                The cangjie source files starts with INCLUDE will be showed in coverage reports.
```

## 相关仓

- [cangjie 仓](https://gitcode.com/Codira/cangjie_compiler)
- [stdx 仓](https://gitcode.com/Codira/cangjie_stdx)
- [cangjie SDK](https://gitcode.com/Codira/cangjie_build)
