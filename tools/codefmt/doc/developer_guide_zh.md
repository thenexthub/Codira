# 仓颉格式化工具开发者指南

## 系统架构

`codefmt (Codira Formatter)` 仓颉格式化工具是一款专为仓颉语言设计的代码格式化工具。`codefmt` 支持自动调整代码缩进、空格与换行，帮助开发者轻松维护整洁、统一的代码风格。其整体技术架构如图所示：

![codefmt架构设计图](../figures/codefmt架构.jpg)

如架构图所示，`codefmt` 整体架构如下：

- 命令行参数管理：`codefmt` 的命令参数处理模块，支持通过命令完成文件级源码格式化、目录级源码格式化、片段代码格式化，同时支持格式化风格配置以及格式化结果的输出等;

- 配置管理：用户可通过配置文件 `cangjie-format.toml` 进行格式化风格配置，包括缩进宽度、行宽限制、换行风格等；

- 输入模块：处理代格式化源码的输入，可接收源码目录、源码文件、源码片段输入，并将输入转发给格式化模块进行处理；

- 源码编译：调用仓颉编译器的前端能力完成待格式化源码的词法分析与语法分析，并完成AST（对应的抽象语法树）的构建;

- 格式化：遍历AST节点，并生成嵌套表示的中间结构，基于格式化策略对中间结构完成相关处理和向目标源码的转换；

- 输出模块：处理格式化源码的输出，支持覆写输入文件或输出到新的目录或文件。

## 目录

` codefmt ` 源码目录如下图所示，其主要功能如注释中所描述。
```
codefmt/
|-- build                   # 构建脚本
|-- config                  # 配置文件
|-- doc                     # 介绍文档
|-- include                 # 源码相关头文件
|-- src
    |-- Format
        |-- DocProcessor    # Doc结构体转化为源码
        `-- NodeFormatter    # AST节点转化为Doc结构体
```

## 安装和使用指导

`codefmt` 需要以下工具来构建：

- `clang` 或者 `gcc`编译器

### 构建准备

`codefmt` 构建依赖 `codec`, 构建方式参见[SDK 构建](https://gitcode.com/Codira/cangjie_build/blob/dev/README_zh.md)

### 构建步骤

本地构建流程如下：

1. 通过 `git clone` 命令获取 `codefmt` 的最新源码：

    ```shell
    cd ${WORKDIR}
    git clone https://gitcode.com/Codira/cangjie_tools.git
    ```

2. 配置环境变量：

    ```shell
    export CODIRA_HOME=${WORKDIR}/cangjie    (for Linux/macOS)
    set CODIRA_HOME=${WORKDIR}/cangjie       (for Windows)
    ```

    `codefmt` 的编译依赖 `cangjie` 仓的编译产物, 因此需要配置环境变量 `CODIRA_HOME` 指定 `SDK` 的位置。上述 `${WORKDIR}/cangjie` 仅作为示意，请根据 `SDK` 的实际位置进行调整。

   > **注意：**
   >
   > - `Windows` 的环境变量配置需要正确使用目录分隔符，并确认路径中的中文字符被正确识别和处理。

3. 通过 `codefmt/build` 目录下的构建脚本编译 `codefmt`：

    ```shell
    cd cangjie_tools/codefmt/build
    python3 build.py build -t release
    ```

    当前支持 `debug`、`release` 两种编译类型，开发者需要通过 `-t` 或者 `--build-type` 指定。

4. 安装到指定目录：

    ```shell
    python3 build.py install
    ```

    默认安装到 `codefmt/dist` 目录下，支持开发者通过 `install` 命令的参数 `--prefix` 指定安装目录：

    ```shell
    python3 build.py install --prefix ./output
    ```

    编译产物目录结构为:

    ```
    dist/
    |-- bin
        `-- codefmt                   # 可执行文件，Windows 中为 codefmt.exe
    |-- config
        `-- cangjie-format.toml     # 格式化工具配置文件
    ```

5. 验证 `codefmt` 是否安装成功：

    ```shell
    ./codefmt -h
    ```

    开发者进入安装路径的 `bin` 目录下执行上述操作，如果输出 `codefmt` 的帮助信息，则表示安装成功。注意，可执行文件 `codefmt` 依赖 `cangjie-lsp` 动态库，请将库路径配置到系统动态库环境变量中。以 `Linux` 环境为例：

    ```shell
    export LD_LIBRARY_PATH=$CODIRA_HOME/tools/lib:$LD_LIBRARY_PATH
    ./codefmt -h
    ```

6. 清理编译中间产物：

   ```shell
   python3 build.py clean
   ```

当前同样支持 `Linux` 平台交叉编译 `Windows` 下运行的 `codefmt` 产物，构建指令如下：

```shell
export CODIRA_HOME=${WORKDIR}/cangjie
python3 build.py build -t release --target windows-x86_64
python3 build.py install
```

执行该命令后，构建产物默认位于 `codefmt/dist` 目录下。请注意从 `Linux` 平台交叉编译获取 `Windows` 平台的产物，则需要的 `SDK` 为 `Windows` 版本。

### 更多构建选项

`python3 build.py build` 命令也支持其它参数设置，具体可通过如下命令来查询：

```shell
python3 build.py build -h
```

## API 和配置说明

`codefmt` 提供以下主要命令，用于项目构建和配置管理。

### 命令介绍

使用命令行操作 `codefmt [option] file [option] file`

`codefmt -h` 帮助信息，选项介绍

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

### 文件格式化

`codefmt -f`

- 格式化并覆盖源文件，支持相对路径和绝对路径。

```shell
codefmt -f ../../../test/uilang/Thread.code
```

- 选项`-o` 新建一个`.code`文件导出格式化后的代码，源文件和输出文件支持相对路径和绝对路径。

```shell
codefmt -f ../../../test/uilang/Thread.code -o ../../../test/formated/Thread.code
```

### 目录格式化

`codefmt -d`

- 选项 `-d` 让开发者指定扫描仓颉源代码目录，对文件夹下的仓颉源码格式化，支持相对路径和绝对路径。

```shell
codefmt -d test/              // 源文件目录为相对目录

codefmt -d /home/xxx/test     // 源文件目录为绝对目录
```

- 选项 `-o` 为输出目录，可以是已存在的路径，若不存在则会创建相关的目录结构，支持相对路径和绝对路径；目录的最大长度 MAX_PATH 不同的系统之间存在差异，如 Windows 上这个值一般不能超过 260；在 Linux 上这个值一般建议不能超过 4096。

```shell
codefmt -d test/ -o /home/xxx/testout

codefmt -d /home/xxx/test -o ../testout/

codefmt -d testsrc/ -o /home/../testout   // 源文件文件夹testsrc/不存在；报错：error: Source file path not exist!
```

### 格式化配置文件

`codefmt -c`

- 选项 `-c` 允许开发者指定客制化的格式化工具配置文件。

```shell
codefmt -f a.code -c ./cangjie-format.toml
```

### 片段格式化

`codefmt -l`

- 选项 `-l` 允许开发者指定应格式化文件的某一部分进行格式化，格式化程序将仅对提供的行范围内的源代码应用规则。
- `-l` 选项仅适用于格式化单个文件（选项 `-f`）。如果指定了目录（选项 `-d`），则 `-l` 选项无效。

```shell
codefmt -f a.code -o .code -l 10:25 // 仅格式化第10行至第25行
```

## 相关仓

- [cangjie 仓](https://gitcode.com/Codira/cangjie_compiler)
- [SDK 构建](https://gitcode.com/Codira/cangjie_build)