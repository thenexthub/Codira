# 实现描述

重要说明：目前 AbcKit 支持 JS、ArkTS 和静态 ArkTS，但**对静态 ArkTS 的支持处于试验阶段**。
编译后的 JS 和 ArkTS 存储在"动态"方舟字节码格式的文件中，静态 ArkTS 存储在"静态"方舟字节码格式的文件中。
AbcKit 根据字节码的格式使用"动态"或"静态"的方式去处理文件。

请先查看 [cookbook](mini_cookbook.md) 以了解用户视角的API。

1. [两种类型的字节码文件](#两种类型的-abc-文件)
2. [C API 和 C++ API](#c-api-和-c-api)
3. [按组件的控制流](#按组件的控制流)
4. [按源文件的控制流](#按源文件的控制流)
5. [动态和静态文件格式的处理](#动态和静态文件格式的处理)
6. [数据结构（上下文）和不透明指针](#数据结构上下文和不透明指针)
7. [数据结构（上下文）实现](#数据结构上下文实现)
8. [头文件命名冲突](#头文件命名冲突)
9. [字节码<-->中间表示](#字节码-中间表示-ir)

## 两种类型的 abc 文件

**AbcKit 支持两种类型的字节码文件**：动态和静态。
根据字节码文件类型可使用以下两种类型的组件：

1. `panda::panda_file` 和 `ark::panda_file`
2. `panda::abc2program` 和 `ark::abc2program`
3. `panda::pandasm` 和 `ark::pandasm`
4. IR 构建器：`libabckit::IrBuilderDynamic` 和 `ark::compiler::IrBuilder`
5. 代码生成：`libabckit::CodeGenDynamic` 和 `libabckit::CodeGenStatic`

注意：`panda::` 是动态运行时命名空间，`ark::` 是静态运行时命名空间。

只有一个静态运行时编译器用于 AbcKit 图表示：`ark::compiler::Graph`。

## C API 和 C++ API

AbcKit 提供两种 API：C API 和 C++ API。

### C API

所有 C API 都存储在 `./include/c` 文件夹中，目录结构如下：

```
include/c/
├── abckit.h                  // 入口点 API
├── api_version.h             // API 版本定义
├── declarations.h            // 基础声明
├── metadata_core.h           // 用于语言无关元数据检查/转换的 API
├── extensions
│   ├── arkts
│   │   └── metadata_arkts.h  // 用于语言特定（ArkTS 和静态 ArkTS）元数据检查/转换的 API
│   └── js
│       └── metadata_js.h     // 用于语言特定（JS）元数据检查/转换的 API
├── ir_core.h                 // 用于语言无关图检查/转换的 API
├── isa
│   ├── isa_dynamic.h         // 用于语言特定（JS 和 ArkTS）图检查/转换的 API
│   └── isa_static.h          // 用于语言特定（静态 ArkTS）图检查/转换的 API（此头文件现在已隐藏）
├── statuses.h                // 错误代码列表
```

### C++ API

C++ API 存储在 `./include/cpp` 文件夹中，目录结构如下：

```
include/cpp/
├── abckit_cpp.h              // C++ API 入口点
├── headers/
│   ├── file.h                // 文件操作
│   ├── graph.h               // 图操作
│   ├── basic_block.h         // 基本块操作
│   ├── instruction.h         // 指令操作
│   ├── literal.h             // 字面量操作
│   ├── value.h               // 值操作
│   ├── type.h                // 类型操作
│   ├── dynamic_isa.h         // 动态 ISA 操作
│   ├── config.h              // 配置
│   ├── utils.h               // 工具函数
│   ├── base_classes.h        // 基础类
│   ├── base_concepts.h       // 手动实现 concepts 特性
│   ├── core/                 // 语言无关 API
│   ├── arkts/                // ArkTS 特定 API
│   └── js/                   // JS 特定 API
```

C API 是纯 C 函数，实现存储在 `./src/` 文件夹中并用 C++ 编写。C++ API 提供了更高级的面向对象接口。

## 按组件的控制流

1. 调用 `openAbc` 时：
   1. 将 `abc` 文件读入 `panda_file`
   2. 使用 `abc2program` 将 `panda_file` 转换为 `pandasm`
2. Abckit 元数据 API 获取/处理 `pandasm` 程序信息
3. 当调用 `createGraphFromFunction` 时：
   1. `pandasm` 程序信息被转换成 `panda_file`（因为当前的 IR 构建器只支持 `panda_file` 输入）
   2. IR 构建器为函数构建控制流图： `ark::compiler::Graph`
4. Abckit Graph API 获取/处理 `ark::compiler::Graph`
5. 当调用 `functionSetGraph` 时，代码生成器使用 `ark::compiler::Graph` 生成 `pandasm` 格式的字节码并替换函数的原始字节码
6. 当调用 `writeAbc` 时，转换后的 `pandasm` 程序被写到 abc 文件中

```
                                                                                ──────────────────────────────────────────────────────────────────────────────────────────────────
                                                                                |                                                                                                /\
                                                                                \/                                                                                               |
x.abc────>(ark/panda)::panda_file───>(ark/panda)::abc2program────>(ark/panda)::pandasm────>(ark/panda)::panda_file───>(ark/panda)::ir_builder────>ark::compiler::Graph──>(ark/panda::)codegen
                                                                    |            /\              |                                                   /\           |
                                                                    |             |              |                                                   |            |
                                                                    |             |              |                                                   |            |
                                                                (abckit metadata API)            |                                                   |            \/
                                                                                                 ──────────>(ark/panda)::RuntimeIface───────────────>(abckit IR API)
```

## 按源文件的控制流

1. 声明 C API
2. 上述 API 的 C++ 实现主要在动态和静态运行时之间进行分发
3. 运行时特定实现：
   1. 动态运行时实现
   2. 静态运行时实现
4. API 接收并返回指向不透明 `AbckitXXX` 结构的指针

```

                                                  |───────────────────────────────────────────|
                                                  |     4. 数据结构（上下文）(./src)            |
                                                  |  metadata_inspect_impl.h                  |
                                                  |  ir_impl.h                                |
                                                  |───────────────────────────────────────────|
                                                                    /\
                                                                    |
                                                                    \/
───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────
                                                                    /\
                                                                    |
                                                                    \/
  |──────────────────────────────────────|        |───────────────────────────────────────────|        |─────────────────────────────────────────────────────────────────|
  |     1. API 声明 (./include/c)         |        |     2. API 实现 (./src/)                  |        |     3.1 动态运行时实现 (./src/adapter_dynamic/)                  |
  |  abckit.h                            |        |  abckit_impl.cpp                          |        |  abckit_dynamic.cpp                                             |
  |  metadata_core.h                     |        |  metadata_(inspect|modify)_impl.cpp       |        |  metadata_(inspect|modify)_dynamic.cpp                          |
  |  ir_core.h                           |        |  ir_impl.cpp                              |        |                                                                 |
  |  extensions/arkts/metadata_arkts.h   |───────>|  metadata_arkts_(inspect|modify)_impl.cpp |───────>|                                                                 |
  |  extensions/js/metadata_js.h         |        |  metadata_js_(inspect|modify)_impl.cpp    |        |                                                                 |
  |  isa/isa_dynamic.h                   |        |  isa_dynamic_impl.cpp                     |        |                                                                 |
  |  isa/isa_static.h                    |        |  isa_static_impl.cpp                      |        |                                                                 |
  |──────────────────────────────────────|        |───────────────────────────────────────────|        |─────────────────────────────────────────────────────────────────|
                                                                    |
                                                                    \/
                                        |───────────────────────────────────────────────────────────────|
                                        |     3.2 静态运行时实现 (./src/adapter_static/)                 |
                                        |  abckit_static.cpp                                            |
                                        |  metadata_(inspect|modify)_static.cpp                         |
                                        |  ir_static.cpp                                                |
                                        |───────────────────────────────────────────────────────────────|

```

## 动态和静态文件格式的处理

许多 API 能够根据 `abc` 文件的源语言处理动态和静态文件格式。
运行时特定实现存储在 `./src/adapter_dynamic/` 和 `./src/adapter_static/` 中

例如， `functionGetName()` API的实现（`./src/metadata_inspect_impl.cpp`）：

```cpp
    switch (function->module->target) {
        case ABCKIT_TARGET_JS:
        case ABCKIT_TARGET_ARK_TS_V1:
            return FunctionGetNameDynamic(function);
        case ABCKIT_TARGET_ARK_TS_V2:
            return FunctionGetNameStatic(function);
    }
```

根据函数的源语言调用两个函数之一：

- `FunctionGetNameDynamic()` 适用于动态字节码，代码位于： `./src/adapter_dynamic/metadata_inspect_dynamic.cpp`
- `FunctionGetNameStatic()` 适用于静态字节码，代码位于： `./src/adapter_static/metadata_inspect_static.cpp`

## 数据结构（上下文）和不透明指针

Abckit C API接收并返回指向不透明 `AbckitXXX` 结构的指针。
用户有这些结构的前向声明类型，实现对用户隐藏并存储在 `./src/metadata_inspect_impl.h` 中。
因此，用户只能从 API 接收指针并将其传递给另一个 API，不能手动修改。

例如，这是来自 `./include/c/metadata_core.h` 的前向类型声明：

```
typedef struct AbckitLiteral AbckitLiteral;
```

这是来自 `./src/metadata_inspect_impl.h` 的实现：

```
struct AbckitLiteral {
    AbckitFile *file;
    libabckit::pandasm_Literal* val;
};
```

## 数据结构（上下文）实现

### 元数据

在 `openAbc` API 调用时，abckit 执行以下步骤：

1. 使用 `panda_file` 打开 `abc` 文件
2. 使用 `abc2program` 将打开的 panda 文件转换为 `pandasm` 程序
3. **贪心遍历**所有 `pandasm` 结构并创建相应的 `AbckitXXX` 结构

所有 `AbckitXXX` 数据结构的实现都存储在 `metadata_inspect_impl.h` 和 `ir_core.h` 中。
顶层数据结构是 `AbckitFile`，用户在 `openAbc` 调用后接收 `AbckitFile*` 指针。

`AbckitXXX` 元数据结构具有"树结构"，与源程序结构匹配，例如：

1. `AbckitFile` 拥有 `unique_ptr<AbckitCoreModule>` 的 vector（每个模块通常对应一个源文件）
2. `AbckitCoreModule` 拥有 `unique_ptr<AbckitCoreNamespace>`（顶层命名空间）、
   `unique_ptr<AbckitCoreClass>`（顶层类）、`unique_ptr<AbckitCoreFunction>`（顶层函数）的 vector
3. `AbckitNamespace` 拥有 `unique_ptr<AbckitCoreNamespace>`（命名空间中嵌套的命名空间）、
   `unique_ptr<AbckitCoreClass>`（命名空间中嵌套的类）、`unique_ptr<AbckitCoreFunction>`（顶层命名空间函数）的 vector
4. `AbckitCoreClass` 拥有 `unique_ptr<AbckitCoreFunction>`（类方法）的 vector
5. `AbckitCoreFunction` 拥有 `unique_ptr<AbckitCoreFunction>`（嵌套在其他函数中的函数）的 vector

### 图

在 `createGraphFromFunction` API 调用时，abckit 执行以下步骤：

1. 从 `pandasm` 函数生成 `panda_file`（这是必需的，因为当前 `ir_builder` 只支持 `panda_file` 输入）
2. 使用 `IrBuilder` 构建 `ark::compiler::Graph`
3. **贪心遍历 `ark::compiler::Graph` 并创建相应的 `AbckitXXX` 结构**

`AbckitXXX` 图结构是：`AbckitGraph`、`AbckitBasicBlock`、`AbckitInst` 和 `AbckitIrInterface`。
对于每个 `ark::compiler::Graph` 基本块和指令，创建 `AbckitBasicBlock` 和 `AbckitInst`。
`AbckitGraph` 包含这样的映射：

```cpp
std::unordered_map<ark::compiler::BasicBlock *, AbckitBasicBlock *> implToBB;
std::unordered_map<ark::compiler::Inst *, AbckitInst *> implToInst;
```

因此我们可以从内部图实现获取相关的 `AbckitXXX` 结构。

## 头文件命名冲突

**重要的实现限制：** libabckit 包含来自动态和静态运行时的头文件，
因此，在构建期间，clang 必须提供两个运行时文件夹的包含路径（`-I`）。
但是两个运行时中有很多具有相同名称的文件和文件夹，这会导致 `#include` 的命名冲突。
这就是 AbcKit 中没有文件同时包含两个运行时头文件的原因：
- 有 `./src/adapter_dynamic/` 文件夹用于包含动态运行时头文件的文件
- 有 `./src/adapter_static/` 文件夹用于包含静态运行时头文件的文件

### 包装器（Wrappers）

但对于某些情况，我们需要在单个文件中处理两个运行时，例如：

- 从 `panda::panda_file::Function` 生成 `ark::compiler::Graph` 时
- 或者从 `ark::compiler::Graph` 生成 `panda::pandasm::Function` 时

对于这种情况，我们使用存储在`./src/wrappers/` 中的**包装器**来处理两个运行时。
在下图中（箭头显示包含方向），您可以看到：

- `metadata_inspect_dynamic.cpp` 包含动态运行时头文件，并且也使用静态图（通过 `graph_wrapper.h`）
- `graph_wrapper.cpp` 包含静态运行时头文件，并且也使用动态 `panda_file`（通过 `abcfile_wrapper.h`）

```
       metadata_inspect_dynamic.cpp ──>graph_wrapper.h<────|
       |                                                   |
       \/                                                  |
DynamicRuntime                                             |<───graph_wrapper.cpp──>StaticRuntime
       /\                                                  |
       |                                                   |
      abcfile_wrapper.cpp────────────>abcfile_wrapper.h<───|
```

按照上图箭头，可以看到没有文件同时包含静态和动态运行时的头文件

<a id="字节码-中间表示-ir"></a>
## 字节码 <──> 中间表示 (IR)

abckit 使用 `ark::compiler::Graph` 作为内部图表示，
因此动态和静态 `pandasm` 字节码都被转换为单个 `IR`

字节码与 IR 的转换方法与字节码优化器相同，
IR 构建器将字节码转换为 IR，代码生成器将 IR 转换为字节码。

- 有两个 IR 构建器：`libabckit::IrBuilderDynamic` 和 `ark::compiler::IrBuilder`
- 两个字节码优化器：`libabckit::CodeGenDynamic` 和 `libabckit::CodeGenStatic`
- 两个运行时接口：`libabckit::AbckitRuntimeAdapterDynamic` 和 `libabckit::AbckitRuntimeAdapterStatic`

运行时接口是静态编译器的一部分，需要抽象出 IR 构建器
IR 构建器的每个用户都应该提供自己的运行时接口（根据编译器的设计）。

IR 接口（`AbckitIrInterface`）源自字节码优化器，需要存储 `panda_file` 实体名称和偏移量之间的关系。
IR 接口的实例：

1. 当函数的字节码转换为 IR 时创建
2. 存储在 `AbckitGraph` 实例中
3. 在 abckit API 实现中使用，以从指令的立即偏移量获取 `panda_file` 实体

### IR 构建器

对于静态字节码，abckit 重用静态运行时的 IrBuilder。

对于动态字节码，abckit 使用动态运行时 IrBuilder 的分支，并进行多种更改。
对于动态字节码，大多数指令仅转换为内部调用（例如 `Intrinsic.callthis0`）。

### 代码生成

对于静态字节码，abckit 基于静态运行时的字节码优化器和代码生成器做了多处修改。
对于动态字节码，abckit 基于动态运行时的字节码优化器和代码生成器做了多处修改。

### 编译器传递

在图创建和字节码生成期间，应用了额外的图清理和优化，
因此如果您这样做：`graph1`->`bytecode`->`graph2` 而不进行任何额外更改，
`graph1` **可能与** `graph2` 存在差异。 
