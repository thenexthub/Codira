# arkcompiler_jsvm

## 介绍
OpenHarmony JSVM-API是基于标准JS引擎提供的一套稳定的API，为开发者提供了较为完整的JS引擎能力，包括创建和销毁引擎，执行JS代码，JS/C++交互等关键能力。

OpenHarmony JSVM-API是C语言接口，遵循C99标准。

通过JSVM-API，开发者可以在应用运行期间直接执行一段动态加载的JS代码。也可以选择将一些对性能、底层系统调用有较高要求的核心功能用C/C++实现并将C++方法注册到JS侧，在JS代码中直接调用，提高应用的执行效率。

## 目录结构
```
/arkcompiler/jsvm
├── interfaces
│ ├── innerkits             # 系统内接口，部件间使用
│ └── kits                  # 应用接口，应用开发者使用
├── src                     # jsvm 代码
│ ├── inspector             # inspector 功能实现
│ └── platform              # 平台相关代码
├── test                    # jsvm 测试套
├── BUILD.gn                # 部件编译脚本
├── jsvm.gni                # jsvm 源文件定义脚本
└── bundle.json             # 部件配置文件
```

## 安装教程

1. 编译命令

```
./build.sh --product-name rk3568 --build-target make_all --target-cpu arm64 --gn-args enable_notice_collection=false --keep-ninja-going
```

## 使用说明

[使用JSVM-API实现JS与C/C++语言交互](https://gitee.com/openharmony/docs/tree/master/zh-cn/application-dev/napi/Readme-CN.md)

## 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request

