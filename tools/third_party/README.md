# 使用的开源软件说明

## flatbuffers

### 代码来源说明

FlatBuffers 是一个高效的跨平台、跨语言序列化库，仓颉语言服务工具使用 FlatBuffers 库完成编译器数据到指定格式的序列化反序列化操作。

该仓被仓颉语言服务工具源码依赖，基于开源代码 [flatbuffers OpenHarmony-v6.0-Release](https://gitcode.com/openharmony/third_party_flatbuffers/tags/OpenHarmony-v6.0-Release) 进行使用。

该开源软件被编译器及周边组件以包含头文件的方式使用，并通过链接库（动态库或静态库）的方式依赖。

### 构建说明

本仓库通过 CMake 作为子目标项目引入，编译过程中会自动完成构建并建立依赖关系，具体构建参数请参见 [build.py](./cangjie-language-server/build/build.py) 文件。

开发者也可以手动下载 [flatbuffers](https://gitcode.com/openharmony/third_party_flatbuffers.git) 源码，命令如下：

```shell
mkdir -p cangjie-language-server/third_party/flatbuffers
cd third_party/flatbuffers
git clone -b OpenHarmony-v6.0-Release https://gitcode.com/openharmony/third_party_flatbuffers.git ./
```
构建项目时，则直接使用 third_party/flatbuffers 目录源码进行构建。

## JSON for Modern C++

### 代码来源说明

JSON for Modern C++ 是一个在C++中使用的JSON库，仓颉语言服务工具使用 JSON for Modern C++ 进行JSON的序列化和反序列化。

该仓被仓颉语言服务工具源码依赖，基于开源代码 [JSON for Modern C++ OpenHarmony-v6.0-Release](https://gitcode.com/openharmony/third_party_json/tags/OpenHarmony-v6.0-Release)。

该开源软件被仓颉语言服务工具以包含头文件的方式使用。

### 构建说明

本仓库通过 CMake 直接使用 JSON for Modern C++ 的头文件源码。

开发者也可以手动下载 [JSON for Modern C++](https://gitcode.com/openharmony/third_party_json.git) 源码，命令如下：

```shell
mkdir -p cangjie-language-server/third_party/json-v3.11.3
cd cangjie-language-server/third_party/json-v3.11.3
git clone -b OpenHarmony-v6.0-Release --depth 1 https://gitcode.com/openharmony/third_party_json.git ./
```

构建项目时，则直接使用 cangjie-language-server/third_party/json-v3.11.3 目录源码进行构建。

## SQLite

### 代码来源说明

SQLite 是一种采用C语言编写的SQL数据库引擎，仓颉语言服务工具使用 SQLite 进行索引的存储和读取。

该仓被仓颉语言服务工具源码依赖，开源版本为 [SQLite OpenHarmony-v6.0-Release](https://gitcode.com/openharmony/third_party_sqlite/tags/OpenHarmony-v6.0-Release)。

### 构建说明

本仓库通过 CMake 直接使用 SQLite 的源码。

开发者也可以手动下载 [sqlite](sqlite) 源码，命令如下：

```shell
mkdir -p cangjie-language-server/third_party/sqlite3
cd cangjie-language-server/third_party/sqlite3
git clone -b OpenHarmony-v6.0-Release --depth 1 https://gitcode.com/openharmony/third_party_sqlite.git ./
mkdir -p amalgamation
cp src/shell.c amalgamation/
cp src/sqlite3.c amalgamation/
cp include/sqlite3.h amalgamation/
cp include/sqlite3ext.h amalgamation/
```

构建项目时，则直接使用 cangjie-language-server/third_party/sqlite3 目录源码进行构建。