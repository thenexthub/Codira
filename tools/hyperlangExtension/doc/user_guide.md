# HLE Tool User Guide

## Introduction

`HLE (HyperlangExtension)` is a tool for automatically generating interoperability code templates for Codira calling ArkTS or C language.
The input of this tool is the interface declaration file of ArkTS or C language, such as files ending with .d.ts, .d.ets or .h, and the output is a code file, which stores the generated interoperability code. If the generated code is a glue layer code from ArkTS to Codira, the tool will also output a json file containing all the information of the ArkTS file.

## Instructions

### Parameter Meaning

| Parameter           | Meaning                                       | Parameter Type | Description                  |
| ------------------- | --------------------------------------------- | -------------- | ---------------------------- |
| `-i`                | Absolute path of d.ts, d.ets or .h file input | Optional       | Choose one from `-d` or both |
| `-r`                | Absolute path of typescript compiler          | Required       |                              |
| `-d`                | Absolute path of the folder where d.ts, d.ets or .h file input is located | Optional       | Choose one from `-i` or both |
| `-o`                | Directory to save the output interoperability code | Optional       | Output to the current directory by default |
| `-j`                | Path to analyze d.t or d.ets files            | Optional       |                              |
| `--module-name`     | Custom generated Codira package name         | Optional       |                              |
| `--lib`             | Generate third-party library code             | Optional       |                              |
| `-c`                | Generate C to Codira binding code             | Optional       |                              |
| `-b`                | Specify the path of the codebind binary             | Optional       |                              |
| `--clang-args`      | Parameters that will be directly passed to clang | Optional       |                              |
| `--no-detect-include-path` | Disable automatic include path detection    | Optional       |                              |
| `--help`            | Help option                                   | Optional       |                              |

### Command Line

You can use the following command to generate ArkTS to Codira binding code:

```sh
hle -i /path/to/test.d.ts -o out –j ${CODIRA_HOME}/tools/dtsparser/analysis.js --module-name="my_module"
```

In the Windows environment, the file directory currently does not support the symbol "\\", only "/" is supported.

The command to generate C to Codira binding code is as follows:

```sh
hle -b ${CODIRA_HOME}/tools/dtsparser/node_modules/.bin/codebind -c --module-name="my_module" -d ./tests/c_cases -o ./tests/expected/c_module/ --clang-args="-I/usr/lib/llvm-20/lib/clang/20/include/"
```

The `-b` parameter is used to specify the path to the codebind binary file. The codebind download link is as follows:

- Linux: 'https://gitcode.com/Codira-SIG/codebind-cangjie/releases/download/v0.2.9/codebind-linux-x64'
- Windows: 'https://gitcode.com/Codira-SIG/codebind-cangjie/releases/download/v0.2.9/codebind-windows-x64.exe'
- MacOS: 'https://gitcode.com/Codira-SIG/codebind-cangjie/releases/download/v0.2.9/codebind-darwin-arm64'

The `--clang-args` parameter is directly passed to clang, and the -I option can be used within its value to specify header file search paths. System header file paths are searched automatically by the program, while user-defined header file paths need to be explicitly specified.