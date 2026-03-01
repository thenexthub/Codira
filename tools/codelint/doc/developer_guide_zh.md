# 仓颉静态检查工具用户指南

## 开源项目介绍

`CODELint(Codira Lint)`是一款静态检查工具，该工具是基于仓颉语言编程规范开发。通过它可以识别代码中不符合编程规范的问题，帮助开发者发现代码中的漏洞，写出满足 Clean Source 要求的仓颉代码。其整体技术架构如图所示：

![codelint架构设计图](../figures/codelint软件架构.jpg)

## 目录

` codelint ` 源码目录如下图所示，其主要功能如注释中所描述。
```
codelint/
|-- build                   # 构建脚本
|-- config                  # 配置文件
|-- doc                     # 介绍文档
|-- src
    |--main.cpp             # 主文件
    |--checker              # 检查引擎
    |--common               # 公共模块
    |--rules                # 规则实现
```

## 安装和使用指导

`codelint` 需要以下工具来构建：

- `clang` 或者 `gcc`编译器

### 构建准备

`codelint` 构建依赖 `codec`, 构建方式参见[SDK 构建](https://gitcode.com/Codira/cangjie_build/blob/dev/README_zh.md)

### 构建步骤

本地构建流程如下：

1. 通过 `git clone` 命令获取 `codelint` 的最新源码：

    ```shell
    cd ${WORKDIR}
    git clone https://gitcode.com/Codira/cangjie_tools.git
    ```

2. 配置环境变量：

    ```shell
    export CODIRA_HOME=${WORKDIR}/cangjie    (for Linux/macOS)
    set CODIRA_HOME=${WORKDIR}/cangjie       (for Windows)
    ```

    `codelint` 的编译依赖 `cangjie` 仓的编译产物, 因此需要配置环境变量 `CODIRA_HOME` 指定 `SDK` 的位置。上述 `${WORKDIR}/cangjie` 仅作为示意，请根据 `SDK` 的实际位置进行调整。

   > **注意：**
   >
   > - `Windows` 的环境变量配置需要正确使用目录分隔符，并确认路径中的中文字符被正确识别和处理。

3. 通过 `codelint/build` 目录下的构建脚本编译 `codelint`：

    ```shell
    cd cangjie-tools/codelint/build
    python3 build.py build -t release
    ```

    当前支持 `debug`、`release` 两种编译类型，开发者需要通过 `-t` 或者 `--build-type` 指定。

4. 安装到指定目录：

    ```shell
    python3 build.py install
    ```

    默认安装到 `codelint/dist` 目录下，支持开发者通过 `install` 命令的参数 `--prefix` 指定安装目录：

    ```shell
    python3 build.py install --prefix ./output
    ```

    编译产物目录结构为:

    ```
    dist/
    |-- bin
        `-- codelint                  # 可执行文件，Windows 中为 codelint.exe
    |-- config
    |--lib
    ```

5. 验证 `codelint` 是否安装成功：

    ```shell
    ./codelint -h
    ```

    开发者进入安装路径的 `bin` 目录下执行上述操作，如果输出 `codelint` 的帮助信息，则表示安装成功。注意，可执行文件 `codelint` 依赖 `cangjie-lsp` 动态库，请将库路径配置到系统动态库环境变量中。以 `Linux` 环境为例：

    ```shell
    export LD_LIBRARY_PATH=$CODIRA_HOME/tools/lib:$LD_LIBRARY_PATH
    ./codelint -h
    ```

6. 清理编译中间产物：

   ```shell
   python3 build.py clean
   ```

当前同样支持 `Linux` 平台交叉编译 `Windows` 下运行的 `codelint` 产物，构建指令如下：

```shell
export CODIRA_HOME=${WORKDIR}/cangjie
python3 build.py build -t release --target windows-x86_64
python3 build.py install
```

执行该命令后，构建产物默认位于 `codelint/dist` 目录下。请注意从 `Linux` 平台交叉编译获取 `Windows` 平台的产物，则需要的 `SDK` 为 `Windows` 版本。

### 更多构建选项

`python3 build.py build` 命令也支持其它参数设置，具体可通过如下命令来查询：

```shell
python3 build.py build -h
```

## API 和配置说明

`codelint` 提供以下主要命令，用于项目构建和配置管理。

### 命令介绍

使用命令行操作 `codelint [option] file [option] file`

`codelint -h` 帮助信息，选项介绍。

```text
Options:
   -h              Show usage
                       eg: ./codelint -h
   -v              Show version
                       eg: ./codelint -v
   -f <value>      Detected file directory, it can be absolute path or relative path
                       eg: ./codelint -f fileDir -c . -m .
   -e <v1:v2:...>  Excluded files, directories or configurations, splitted by ':'. Regular expressions are supported
                       eg: ./codelint -f fileDir -e fileDir/a/:fileDir/b/*.code
   -o <value>      Output file path, it can be absolute path or relative path, if it is directory, default file name is codeReport
                       eg: ./codelint -f fileDir -o ./out
   -r [csv|json]   Report file format, it can be csv or json, default is json
                       eg: ./codelint -f fileDir -r csv -o ./out
   -c <value>      Directory path where the config directory is located, it can be absolute path or relative path to the executable file
                       eg: ./codelint -f fileDir -c .
   -m <value>      Directory path where the modules directory is located, it can be absolute path or relative path to the executable file
                       eg: ./codelint -f fileDir -m .

```

`codelint -f` 指定检查目录。

```bash
codelint -f fileDir [option] fileDir...
```
> **注意：**
>
> `-f` 后面指定的是*.code 文件所在`src`目录。

正例：

```bash
codelint -f xxx/xxx/src
```

反例：

```bash
codelint -f xxx/xxx/src/xxx.code
```

> **说明：**
>
> 当前工具支持目录扫描，暂不支持对单源码文件的独立检查，建议用户提供单包路径作为输入。

`-r` 用来指定生成扫描报告的格式，目前支持`json`格式和`csv`格式。

`-r`需要与`-o`选项配合使用，如果没有`-o`指定输出到文件，即使指定了`-r`也不会生成扫描报告。如果指定了`-o`没有指定`-r`，那么默认生成`json`格式的扫描报告。

```bash
codelint -f ./src -r csv -o ./report         // 生成report.csv文件
codelint -f ./src -r csv -o ./output/report  // 在output目录下生成report.csv文件
```

`-c`, `-m` 在开发者需要时用以指定`config`和`modules`所在的目录路径。

在默认情况下，`codelint`会调用其所在目录下的`config`和`modules`作为默认的配置目录和依赖目录。若有需要，开发者可以用命令行选项 `-c`， `-m`来指定`config`和`modules`所在的目录路径。

例：指定的 config 和 modules 的路径分别为：`./tools/codelint/config` 和 `./tools/codelint/modules`。

则`config`和`modules`所在的目录路径同为`./tools/codelint`, 所以命令应为：

```bash
codelint -f ./src -c ./tools/codelint -m ./tools/codelint
```

## 规则级告警屏蔽

可执行文件`codelint`同目录下的`config`配置目录中，有`codelint_rule_list.json`和`exclude_lists.json`两个配置文件。其中，`codelint_rule_list.json`为规则列表配置文件，开发者可以通过增减其中的配置信息来决定进行哪些规则的检查。`exclude_lists.json`为告警屏蔽配置文件，开发者可以通过添加告警信息来屏蔽某一条规则的某一条告警。

例： 若开发者只想检查如下 5 条规则，则`codelint_rule_list.json`配置文件中只添加要检查的 5 条规则。

 ```json
 {
   "RuleList": [
     "G.FMT.01",
     "G.ENU.01",
     "G.EXP.03",
     "G.OTH.01",
     "G.OTH.02"
   ]
 }
 ```

例： 若开发者想要屏蔽某一条规则的某一条告警，可以在`exclude_lists.json`配置文件中添加屏蔽信息。

> **注意：**
>
> `path`不必填写绝对路径，但必须有`xxx.code`格式，为模糊匹配。`line`为告警行号，为精确匹配。`column`为告警列号，可选择性填写进行列号精确匹配。

 ```json
 {
   "G.OTH.01" : [
     {"path":"xxx/example.code", "line":"42"},
     {"path":"xxx/example.code", "line":"42", "column": "2"},
     {"path":"example.code", "line":"42", "column": "2"}
   ]
 }
 ```

## 源代码注释告警屏蔽

**特殊注释 BNF**

```text
<content of codelint-ignore comment> ::=  "codelint-ignore"  [-start] <ignore-rule>{...} [description] | codelint-ignore  <-end> [description]
<ignore-rule> ::="!"<rule-name>
<rule-name> ::= <letters>
```

> **注意：**
>
> - 特殊注释的 `codelint-ignore` 与选项 `-start` 和 `-end` 以及屏蔽的规则需要写在同一行上，否则无法进行告警屏蔽。描述信息可以写在不同行。
> - 单行屏蔽，屏蔽规则与屏蔽规则间需要用空格隔开，`codelint` 会将特殊注释所在行的对应规则告警进行屏蔽。
> - 多行屏蔽，`codelint` 会以含有 `-start` 的特殊注释为起始行，以含有 `-end` 的特殊注释为结束行，将其间对应的规则进行屏蔽。含有 `-end` 的特殊注释会与其上方最近的含有 `-start` 的特殊注释相匹配。

**单行屏蔽正确示例 1**，屏蔽 G.FUN.02 告警

```cangjie
func foo(a: Int64, b: Int64, c: Int64, d: Int64) { /* codelint-ignore !G.FUN.02 */
    return a + b + c
}
```

**单行屏蔽正确示例 2**，屏蔽 G.FUN.02 告警

```cangjie
func foo(a: Int64, b: Int64, c: Int64, d: Int64) { // codelint-ignore !G.FUN.02 description
    return a + b + c
}
```

**多行屏蔽正确示例 1**，屏蔽 G.FUN.02 告警

```cangjie
/*codelint-ignore -start !G.FUN.02 description */
func foo(a: Int64, b: Int64, c: Int64, d: Int64) {
    return a + b + c
}
/* codelint-ignore -end description */
```

**多行屏蔽正确示例 2**，屏蔽 G.FUN.02 告警

```cangjie
// codelint-ignore -start !G.FUN.02 description
func foo(a: Int64, b: Int64, c: Int64, d: Int64) {
    return a + b + c
}
// codelint-ignore -end description
```

**多行屏蔽正确示例 3**，屏蔽 G.FUN.02 告警

```cangjie
/**
 *  codelint-ignore -start !G.FUN.02 description
 */
func foo(a: Int64, b: Int64, c: Int64, d: Int64) {
    return a + b + c
}
// codelint-ignore -end description
```

**单行屏蔽<font color="#dd0000">错误</font>示例 1**，屏蔽 G.FUN.02 告警

```cangjie
func foo(a: Int64, b: Int64, c: Int64, d: Int64) { /*codelint-ignore !G.FUN.02!G.FUN.01*/
    return a + b + c                               // ERROR: 规则间没用空格隔开，屏蔽告警失败
}
```

**单行屏蔽<font color="#dd0000">错误</font>示例 2**，屏蔽 G.FUN.02 告警

```cangjie
func foo(a: Int64, b: Int64, c: Int64, d: Int64) { /*codelint-ignore !G.FUN.02description*/
    return a + b + c                               // ERROR: 规则与描述信息没用空格隔开，屏蔽告警失败
}
```

**多行屏蔽<font color="#dd0000">错误</font>示例 1**，屏蔽 G.FUN.02 告警

```cangjie
/* codelint-ignore -start
 * !G.FUN.02 description */
func foo(a: Int64, b: Int64, c: Int64, d: Int64) {
    return a + b + c
}
/* codelint-ignore -end description */
// ERROR: 屏蔽规则没与 'codelint-ignore' 在同一行，屏蔽告警失败
```

## 文件级告警屏蔽

1. `codelint` 可以通过 `-e` 选项支持文件级别的告警屏蔽。

    通过在 `-e` 后添加屏蔽规则，即可将规则匹配的仓颉文件屏蔽，不会产生关于这些文件的告警。输入的规则为相对 `-f` 源码目录的相对路径（支持正则），输入字符串需要用双引号包含，多条屏蔽规则用空格分隔。例如，下面这条命令屏蔽了 `src/dir1/` 目录内的所有仓颉文件， `src/dir2/a.code` 文件和 `src/` 目录下所有形如 `test*.code` 的仓颉文件。

    ```bash
    codelint -f src/ -e "dir1/ dir2/a.code test*.code"
    ```

2. `codelint` 可以通过后缀为 `.cfg` 的配置文件批量导入屏蔽规则。

    通过 `-e` 选项导入配置文件，与其他屏蔽规则或配置文件用空格分隔。例如，下面这条命令屏蔽了 `src/dir1` 目录和 `src/exclude_config_1.cfg`、`src/dir2/exclude_config_2.cfg` 内配置的所有屏蔽规则对应的文件。

    ```bash
    codelint -f src/ -e "dir1/ exclude_config_1.cfg dir2/exclude_config_2.cfg"
    ```

    `.cfg` 配置文件中可以配置多条屏蔽规则，每行均为一条屏蔽规则，屏蔽规则为相对该配置文件所在目录的相对路径（支持正则），无需双引号包含。例如在 `src/dir2/exclude_config_2.cfg` 中有以下配置，则上述的命令会将 `src/dir2/subdir1/` 目录和 `src/dir2/subdir2/a.code` 文件加入屏蔽。

    ```text
    subdir1/
    subdir2/a.code
    ```

3. `codelint` 可以通过默认配置文件批量导入屏蔽规则。

    `codelint` 屏蔽功能的默认配置文件名为 `codelint_file_exclude.cfg`，位置在 `-f` 源码目录下。例如，当 `src/` 目录下存在 `src/codelint_file_exclude.cfg` 这一配置文件时，`codelint -f src/` 命令会屏蔽 `src/codelint_file_exclude.cfg` 内配置的屏蔽规则对应的文件。如果开发者已经在 `-e` 选项中配置了其他有效的 `.cfg` 配置文件，则 `codelint` 不会检查默认配置文件。

## 支持检查的规范列表（持续新增中）

`codelint` 默认启用的规范列表：

- G.NAM.01 包名采用全小写单词，允许包含数字和下划线。
- G.NAM.02 源文件名采用全小写加下划线风格。
- G.NAM.03 接口，类，struct、枚举类型和枚举成员构造，类型别名，采用大驼峰命名。
- G.NAM.04 函数名称应采用小驼峰命名。
- G.NAM.05 let 的全局变量和 static let 变量的名称采用全大写。
- G.FMT.01 源文件编码格式（包括注释）必须是 UTF-8。
- G.FMT.15 禁止省略浮点数小数点前的 0。
- G.DCL.01 避免遮盖（shadow）。
- G.DCL.02 显式声明 public 变量的类型和 public 函数的返回类型。
- G.FUN.01 函数功能要单一。
- G.FUN.02 禁止函数有未被使用的参数。
- G.FUN.03 避免在无关的函数之间重用名字，构成重载。
- G.CLS.01 override 父类函数时不要增加函数的可访问性。
- G.ITF.02 尽量在类型定义处就实现接口，而不是通过扩展实现接口。
- G.ITF.01 对于需要原地修改对象自身的抽象函数，尽量使用 mut 修饰，以支持 struct 类型实现或扩展该接口。
- G.ITF.03 类型定义时避免同时声明实现父接口和子接口。
- G.ITF.04 尽量通过泛型约束使用接口，而不是直接将接口作为类型使用。
- G.OPR.01 尽量避免违反使用习惯的操作符重载。
- G.OPR.02 尽量避免在枚举类型内定义`()`操作符重载函数。
- G.ENU.01 避免枚举的构造成员与顶层元素同名。
- G.ENU.02 尽量避免不同的 enum 的 constructor 之间不必要的重载。
- G.VAR.01 优先使用不可变变量（CODEVM 后端暂不支持）。
- G.VAR.02 保持变量的作用域尽可能小。
- G.TYP.03 判断变量是否为 NaN 时必须使用 isNaN() 方法。
- G.EXP.01 match 表达式同一层尽量避免不同类别的 pattern 混用。
- G.EXP.02 不要期望浮点运算得到精确的值。
- G.EXP.03 && 、 ||、? 和 ?? 操作符的右侧操作数不要包含副作用。
- G.EXP.04 尽量避免副作用发生依赖于操作符的求值顺序。
- G.EXP.05 用括号明确表达式的操作顺序，避免过分依赖默认优先级。
- G.EXP.06 Bool 类型比较应避免多余的 == 或 !=。
- G.EXP.07 比较两个表达式时，左侧倾向于变化，右侧倾向于不变。
- G.ERR.01 恰当地使用异常或错误处理机制。
- G.ERR.02 防止通过异常抛出的内容泄露敏感信息。
- G.ERR.03 避免对 Option 类型使用 getorthrow 函数。
- G.ERR.04 不要使用 return、break、continue 或抛出异常使 finally 块非正常结束。
- G.PKG.01 避免在 import 声明中使用通配符*。
- G.CON.01 禁止将系统内部使用的锁对象暴露给不可信代码。
- P.01 使用相同的顺序请求锁，避免死锁。
- G.CON.02 在异常可能出现的情况下，保证释放已持有的锁。
- G.CON.03 禁止使用非线程安全的函数来覆写线程安全的函数。
- P.02 避免数据竞争（data race）。
- G.CHK.01 跨信任边界传递的不可信数据使用前必须进行校验。
- G.CHK.02 禁止直接使用外部数据记录日志。
- G.CHK.03 使用外部数据构造的文件路径前必须进行校验，校验前必须对文件路径进行规范化处理。
- G.CHK.04 禁止直接使用不可信数据构造正则表达式。
- G.FIO.01 临时文件使用完毕必须及时删除。
- G.SER.01 禁止序列化未加密的敏感数据。
- G.SER.02 防止反序列化被利用来绕过构造方法中的安全操作。
- G.SER.03 保证序列化和反序列化的变量类型一致。
- G.SEC.01 进行安全检查的方法禁止声明为`open`。
- P.03 对外部对象进行安全检查时需要进行防御性拷贝。
- G.OTH.01 禁止在日志中保存口令、密钥和其他敏感数据。
- G.OTH.02 禁止将敏感信息硬编码在程序中。
- G.OTH.03 禁止代码中包含公网地址。
- G.OTH.04 不要使用 String 存储敏感数据，敏感数据使用结束后应立即清 0。
- FFI.C.7 强制进行指针类型转换时避免出现截断错误。

`codelint` 能够检测，但默认不启用的规范列表（开发者可通过将规范添加至 `codelint_rule_list.json` 以启用这类规则）：

- G.NAM.06 变量的名称采用小驼峰。
- G.VAR.03 避免使用全局变量。
- G.FMT.13 文件头注释应该包含版权许可。

## 规格说明

- G.CON.02 在异常可能出现的情况下，保证释放已持有的锁。

    lock() 函数和 unlock() 函数赋值给变量，赋值后的变量再去加解锁的场景，该规则检查不覆盖。

- G.OTH.03 暂不支持宏检查。
- 只有当宏包在正确的路径下时，`codelint`才能支持宏检查。

    例：a.code 为宏包源码，其正确路径应为 xxx/src/a/a.code。

- `codelint`只有在宏被调用时才能对其进行检查，且无法对宏包中的冗余代码进行检查。

## 支持语法禁用检查

1. `codelint` 可以通过将 G.SYN.01 添加至 `codelint_rule_list.json` 以启用禁用语法的检查。如果使用了禁用的语法元素，`codelint` 将会报错。

2. 当前所支持`codelint`检查的禁用语法如表中所示:

   | 禁用语法     | 关键词          | 说明                                             |
   | ------------ | --------------- | ------------------------------------------------ |
   | 导入包       | Import          | 不允许随意导入包                                 |
   | let 变量     | Let             | 只用 var 变量，不引入不可写变量的概念            |
   | 创建线程     | Spawn           | 不允许创建线程                                   |
   | 线程同步     | Synchronized    | 防止死锁                                         |
   | 主函数       | Main            | 禁止提供入口主函数                               |
   | 定义宏       | MacroQuote      | 禁止定义宏（但允许使用宏）                       |
   | 跨语言       | Foreign         | 禁止跨语言混合编程                               |
   | while 循环   | While           | 防止复杂循环和死循环                             |
   | 扩展         | Extend          | 禁止使用扩展语法                                 |
   | 类型别名     | Type            | 禁止自行定义类型别名                             |
   | 操作符重载   | Operator        | 禁止重载操作符                                   |
   | 全局变量     | GlobalVariable  | 禁止声明和使用全局变量，防止副作用和内存泄漏     |
   | 定义枚举     | Enum            | 禁用 Enum，避免复杂代码                          |
   | 定义类       | Class           | 禁用 Class，避免复杂代码                         |
   | 定义接口     | Interface       | 禁用 Interface，避免复杂代码                     |
   | 定义结构     | Struct          | 禁用 Struct，避免复杂代码                        |
   | 定义泛型     | Generic         | 禁用 Generic，避免复杂代码                       |
   | 条件编译     | When            | 禁止平台相关代码                                 |
   | 模式匹配     | Match           | 函数式编程范式，开发者不易掌握                   |
   | 捕获异常     | TryCatch        | 避免自行处理异常，易导致错误被忽略               |
   | 高阶函数     | HigherOrderFunc | 函数类型的参数或返回值, 避免复杂代码             |
   | 其他基础类型 | PrimitiveType   | 不应使用 Int64、float64、bool 之外的其他基础类型 |
   | 其他容器类型 | ContainerType   | 应使用 List，Map，Set                            |

3. 通过将上述表格中的关键字添加到 `structural_rule_G_SYN_01.json` 中启用对应语法的禁用检查。举例：禁用导入包

```json
{
  "SyntaxKeyword": [
    "Import"
  ]
}
```


## 相关仓

- [cangjie 仓](https://gitcode.com/Codira/cangjie_compiler)
- [SDK 构建](https://gitcode.com/Codira/cangjie_build)