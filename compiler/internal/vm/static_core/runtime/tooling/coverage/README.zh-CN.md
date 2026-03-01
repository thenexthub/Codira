# coverage.py 脚本使用说明：

## 前置条件

使用此脚本同级目录需要有同名的语法树文件(.json)，反汇编文件(.pa)，以及ets源码。

语法树文件(.json)放在ast目录，反汇编文件(.pa)放在pa目录，ets源码放在src目录，运行时信息(.csv)放在runtime_info目录。

目录结构如下：

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

## 指令说明
### 1.生成ast文件
host侧生成ast文件:

```
python3 coverage.py gen_ast --src=./src --output=./ --es2panda=../../../out/bin/es2panda --mode=host
```

host侧多文件链接生成ast文件:

```
python3 coverage.py gen_ast --src=./src --output=./ --es2panda=../../../out/bin/es2panda --ark-link=../../../out/bin/ark_link --mode=host-multi --abc-link-name=linked.abc --ets-arkts-path=../../../plugins/ets/sdk/arkts/ --std-path=../../../plugins/ets/stdlib/std/ --escompat-path=../../../plugins/ets/stdlib/escompat/ --ets-api-path=../../../plugins/ets/sdk/api/
```

真机侧生成ast文件:

```
python3 coverage.py gen_ast --src=./src --output=./ --es2panda=../../../out/bin/es2panda --mode=hap
```

### 2.生成pa文件

abc文件反编译生成pa文件:

```
python3 coverage.py gen_pa --src=./abc --output=./pa --ark_disasm-path=../../../out/bin/ark_disasm
```

### 3.生成html文件

host侧:
- 生成全量覆盖率html文件:

```
python coverage.py gen_report --src=./src --mode=host --all
或
python coverage.py gen_report --src=./src --mode=host -a
```

- 生成增量覆盖率html文件

```
python coverage.py gen_report --src=./src --mode=host --diff
或
python coverage.py gen_report --src=./src --mode=host -d
```

注：真机侧使用需要将`--mode=host`替换为`--mode=hap`；host源文件涉及多个文件时需要将`--mode=host`替换为`--mode=host-multi`；生成增量覆盖率html文件需要生成diff.txt文件并放到coverage.py的同级目录。

生成diff.txt指令：
```
git diff HEAD^ HEAD > diff.txt
```