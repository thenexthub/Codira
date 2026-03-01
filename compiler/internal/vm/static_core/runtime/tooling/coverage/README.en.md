# coverage.py Script Usage Instructions:

## Preconditions

To use this script, you need to have syntax tree files (.json), disassembly files (.pa), and ets source code with the same name in the same directory as the script.

Syntax tree files (.json) are placed in the ast directory, disassembly files (.pa) are placed in the pa directory, ets source code is placed in the src directory, and runtime information (.csv) is placed in the runtime_info directory.

The directory structure is as follows:

```
.
├── ast
│   ├── mypackage1
│   │   └── hello.json
│   └── mypackage2
│       └── world.json
├── coverage.py
├── pa
│   ├── mypackage1
│   │   └── hello.pa
│   └── mypackage2
│       └── world.pa
├── runtime_info
│   └── coverageBytecodeInfo.csv
└── src
    ├── mypackage1
    │   └── hello.ets
    └── mypackage2
        └── world.ets
```

## Command Instructions
### 1. Generate AST Files
Command 1: Generate AST files for host side

```
python3 coverage.py gen_ast --src=./src --output=./ --es2panda=../../../out/bin/es2panda --mode=host
```

Command 2: Generate AST files for device side

```
python3 coverage.py gen_ast --src=./src --output=./ --es2panda=../../../out/bin/es2panda --mode=hap
```

Command 3: Generate AST files for host side with multiple files linked

```
python3 coverage.py gen_ast --src=./src --output=./ --es2panda=../../../out/bin/es2panda --ark-link=../../../out/bin/ark_link --mode=host-multi --abc-link-name=linked.abc --ets-arkts-path=../../../plugins/ets/sdk/arkts/ --std-path=../../../plugins/ets/stdlib/std/ --escompat-path=../../../plugins/ets/stdlib/escompat/ --ets-api-path=../../../plugins/ets/sdk/api/
```
### 2. Generate PA Files

Command: Decompile ABC files to generate PA files

```
python3 coverage.py gen_pa --src=./abc --output=./pa --ark_disasm-path=../../../out/bin/ark_disasm
```

### 3. Generate HTML Files

Host Side:
- Generate full coverage HTML file:

```
python coverage.py gen_report --src=./src --mode=host --all
or
python coverage.py gen_report --src=./src --mode=host -a
```

- Generate incremental coverage HTML file:

```
python coverage.py gen_report --src=./src --mode=host --diff
or
python coverage.py gen_report --src=./src --mode=host -d
```

Note: For the device side, replace `--mode=host` with `--mode=hap`. When host source files involve multiple files, replace `--mode=host`  with `--mode=host-multi`. To generate an incremental coverage HTML file, a diff.txt file must be created and placed in the same directory as coverage.py.

Command to generate diff.txt:

```
git diff HEAD^ HEAD > diff.txt
```