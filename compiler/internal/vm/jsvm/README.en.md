# arkcompiler_jsvm

#### Description
OpenHarmony JSVM-API provides a set of stable APIs based on the standard JavaScript (JS) engine. It provides complete JS engine capabilities, including creating and destroying a JS engine, executing JS code, and implementing interaction between JS and C/C++ modules.

OpenHarmony JSVM-API provides a set of APIs written in C programming language that complies with C99.

JSVM-API allows dynamically loaded JS code segment to be directly run during application runtime. With JSVM-API, you can also use C/C++ to implement core functionalities that demand high performance or closely rely on underlying system invocation, register C++ methods in JS code, and directly call the JS code to improve the execution speed.

#### Directory Structure

```
/arkcompiler/jsvm
├── interfaces
│ ├── innerkits             # interface used in system component
│ └── kits                  # interface provided to app developer
├── src                     # jsvm source code
│ ├── inspector             # source code about inspector
│ └── platform              # source code related to platform
├── test                    # jsvm test suit
├── BUILD.gn                # jsvm compile script
├── jsvm.gni                # jsvm compile script
└── bundle.json             # jsvm config file
```

#### Installation

1.  compile command

```
./build.sh --product-name rk3568 --build-target make_all --target-cpu arm64 --gn-args enable_notice_collection=false --keep-ninja-going
```

#### Instructions

[Using JSVM-API](https://gitee.com/openharmony/docs/tree/master/zh-cn/application-dev/napi/Readme-CN.md)

#### Contribution

1.  Fork the repository
2.  Create Feat_xxx branch
3.  Commit your code
4.  Create Pull Request

