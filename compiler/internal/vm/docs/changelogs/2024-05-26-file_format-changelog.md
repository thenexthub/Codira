# 2024-05-26-file_format-changlog
This document describes change log with the following modifications:

* Version
* Method name mangling refactoring

## Version
We update version from 12.0.3.0 to  12.0.4.0

## Method name mangling refactoring
To manually calculate the mapping between the function name in the source code and the function name in the bytecode. The way of distinguishing two functions with the same name in different scopes is changed from obtaining hash for scope to adding scope tags and scope names. To maintain abc's size, some long scope names will be put into a literal array and we add a new Field called "scopeNames" whose value points to the literal array for each file.

For more details about this refactoring, please refer to [arkts-bytecode-function-name.md
](https://gitcode.com/openharmony/docs/blob/dfc65afed16d888d2c816b31e3fcbe17e725c6a2/zh-cn/application-dev/quick-start/arkts-bytecode-function-name.md)