# HLE工具用户指南

## 功能简介

`HLE (HyperlangExtension)` 是一个仓颉调用ArkTS或者C语言的互操作代码模板自动生成工具。
该工具的输入是ArkTS或者C语言的接口声明文件，例如后缀.d.ts，.d.ets或者.h结尾的文件，输出为code文件，其中存放生成的互操作代码。如果生成的是ArkTS到仓颉的胶水层代码，工具也会输出包含ArkTS文件的所有信息的json文件。

## 使用说明

### 参数含义

| 参数            | 含义                                        | 参数类型 | 说明                 |
| --------------- | ------------------------------------------- | -------- | -------------------- |
| `-i`            | d.ts，d.ets或者.h文件输入的绝对路径             | 可选参数 | 和`-d`参数二选一或者两者同时存在                     |
| `-r`            | typescript编译器的绝对路径                  | 必选参数 |                      |
| `-d`            | d.ts，d.ets或者.h文件输入所在文件夹的绝对路径    | 可选参数 | 和`-i`参数二选一或者两者同时存在                     |
| `-o`            | 输出保存互操作代码的目录                    | 可选参数 | 缺省时输出至当前目录 |
| `-j`            | 分析d.t或者d.ets文件的路径                 | 可选参数 |                      |
| `--module-name` | 自定义生成的仓颉包名                        | 可选参数 |                      |
| `--lib`         | 生成三方库代码                              | 可选参数 |                      |
| `-c`            | 生成C到仓接的绑定代码                              | 可选参数 |                      |
| `-b`            | 指定codebind二进制的路径                              | 可选参数 |                      |
| `--clang-args`  | 会被直接传递给 clang 的参数                              | 可选参数 |                      |
| `--no-detect-include-path`  | 禁用自动 include 路径检测                              | 可选参数 |                      |
| `--help`        | 帮助选项                                   | 可选参数 |                      |

### 命令行

可使用如下的命令生成ArkTS到Codira绑定代码：

```sh
hle -i /path/to/test.d.ts -o out –j ${CODIRA_HOME}/tools/dtsparser/analysis.js --module-name="my_module"
```

在Windows环境，文件目录当前不支持符号“\\”，仅支持使用“/”。

生成C到Codira绑定代码的命令如下：

```sh
hle -b ${CODIRA_HOME}/tools/dtsparser/node_modules/.bin/codebind -c --module-name="my_module" -d ./tests/c_cases -o ./tests/expected/c_module/ --clang-args="-I/usr/lib/llvm-20/lib/clang/20/include/"
```

其中`-b`参数用于指定codebind二进制文件的路径，codebind下载地址如下：

- Linux: 'https://gitcode.com/Codira-SIG/codebind-cangjie/releases/download/v0.2.9/codebind-linux-x64'
- Windows: 'https://gitcode.com/Codira-SIG/codebind-cangjie/releases/download/v0.2.9/codebind-windows-x64.exe'
- MacOS: 'https://gitcode.com/Codira-SIG/codebind-cangjie/releases/download/v0.2.9/codebind-darwin-arm64'

`--clang-args`参数是会被直接传递给clang的参数，在参数值内可用-I指定头文件搜索路径。系统头文件路径程序会自动搜索，用户自定义的头文件路径需要显式指定。
