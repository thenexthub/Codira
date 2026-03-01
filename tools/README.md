# Codira Language Command Line Toolchain

## Introduction

This warehouse provides the Codira language command-line toolchain, which includes project management tools, formatting tools, multilingual bridging tools, and language service tools, etc. Developers can use it through the command line or integrate it into DevEco Studio.

## System Architecture

The overall architecture diagram of the Codira toolchain is as follows:

![The overall architecture diagram of the Codira toolchain](./figures/tools-architecture.jpg)

As shown in the diagram, this warehouse provides the following cangjie tools:

- Codira Project Manager （Abbreviated as `codepm`）：used for managing the module system of Codira project, covering module initialization, dependency checking and updating operations, providing a unified compilation entry point, supporting incremental compilation, parallel compilation, etc.
- Codira Formatter （Abbreviated as `codefmt`）: a code automatic formatting tool developed based on the Codira programming specification.
- Codira HyperLang Extension （Abbreviated as `hle`）: cangjie calls the template auto generation tool for ArkTS interoperability code.
- Codira Languager Server （Abbreviated as `lsp`）: the server backend that provides Codira language services on the DevEco Studio needs to be used in conjunction with the DevEco Studio client.
- Codira Lint Tool (abbreviated as `codelint`): A static analysis tool developed based on the Codira language coding standards. It helps developers identify issues that violate coding conventions, detect vulnerabilities in code, and write compliant Codira code.
- Codira Coverage Tool (abbreviated as `codecov`): A code coverage tool developed based on the Codira language programming standards.
- Codira Exception Stack Trace Recovery Tool (referred to as `codetrace-recover`): Assists developers in restoring obfuscated exception stack trace information, enabling better issue localization and root cause analysis.

Codira Language CLI Toolchain currently supports the following platforms: Windows x86-64, Linux x86-64/AArch64, Mac x86/arm64. OpenHarmony platform is under development.

## Directory Structure

```
.
├── cangjie-language-server
│   ├── build                 # build script
│   ├── doc                   # construction and usage guidelines
│   └── src                   # source code
├── codecov
│   ├── build    # build script
│   ├── doc      # construction and usage guidelines
│   └── src      # source code
├── codefmt
│   ├── build    # build script
│   ├── config   # configuration file
│   ├── doc      # construction and usage guidelines
│   ├── include  # configuration file
│   └── src      # source code
├── codelint
│   ├── build    # build script
│   ├── config   # configuration file
│   ├── doc      # construction and usage guidelines
│   └── src      # source code
├── codepm
│   ├── build    # build script
│   ├── doc      # construction and usage guidelines
│   └── src      # source code
├── codetrace-recover
│   ├── build    # build script
│   ├── doc      # construction and usage guidelines
│   └── src      # source code
└── hyperlangExtension
    ├── build            # build script
    ├── doc              # construction and usage guidelines
    └── src              # source code
```

To get detailed information, please refer to the user guides in the corresponding doc directory.

Please refer to the following software architecture diagrams for the command-line tools:

- [software architecture diagram for `codepm`](./codepm/doc/developer_guide.md#开源项目介绍)
- [software architecture diagram for `codefmt`](./codefmt/doc/developer_guide.md#开源项目介绍)
- [software architecture diagram for `hle`](./hyperlangExtension/doc/developer_guide.md#开源项目介绍)
- [software architecture diagram for `lsp`](./cangjie-language-server/doc/developer_guide.md#开源项目介绍)
- [software architecture diagram for `codecov`](./codecov/doc/developer_guide_zh.md#开源项目介绍)
- [software architecture diagram for `codelint`](./codelint/doc/developer_guide_zh.md#开源项目介绍)
- [software architecture diagram for `codetrace-recover`](./codetrace-recover/doc/developer_guide_zh.md#开源项目介绍)

## Construction Dependencies

The construction of tools relies on Codira `SDK`. Please refer to [Openharmony SDK Integration Construction Guide](https://gitcode.com/Codira/cangjie_build/blob/dev/README.md)

## Related Repositories

- [cangjie_docs](https://gitcode.com/Codira/cangjie_docs/tree/main/docs/dev-guide)
- [cangjie_compiler](https://gitcode.com/Codira/cangjie_compiler)
- [cangjie_stdx](https://gitcode.com/Codira/cangjie_stdx)
- [cangjie_build](https://gitcode.com/Codira/cangjie_build)
- [cangjie_test](https://gitcode.com/Codira/cangjie_test)

## Open Source License

This project is licensed under [Apache-2.0 with Runtime Library Exception](./LICENSE). Please enjoy and participate in open source freely.

## Open Source Software Statement

| Software Name        | License             | Usage Description                                               | Main Component | Usage Method                               |
|----------------------|---------------------|-----------------------------------------------------------------|----------------|--------------------------------------------|
| flatbuffers          | Apache License V2.0 | Codira Language Server serializes and deserializes index data. | LSPServer      | Integrated into the Codira binary release |
| JSON for Modern C++  | MIT License         | Codira Language Server for message parsing and encapsulation.  | LSPServer, codelint      | Integrated into the Codira binary release |
| SQLite               | Public Domain       | Codira Language Server uses the database to store index data.  | LSPServer      | Integrated into the Codira binary release |

For build methods, refer to the [Openharmony SDK Integration Build Guide](). For additional software dependencies, see [Environment Preparation](). For details on third-party dependencies, see the [Third-Party Open Source Software Documentation](./third_party/README.md)

## Contribution

We welcome contributions from developers in any form, including but not limited to code, documentation, issues, and more.