# 方舟多线程运行命令行 #
## 功能介绍 ##
可启动指定数量的线程，每个线程启动虚拟机同时执行用例，用于压测共享内存的垃圾回收机制。

## 命令帮助 ##
ark_multi [线程数] [执行文件，内部记录需要运行的abc，每行一个] [参数，同ark_js_vm]

### 举例 ###
执行文件 input.txt 内容
```
a.abc
a.abc
b.abc
b.abc
c.abc
c.abc
```
命令
ark_multi 3 input.txt --icu-data-path "third_party/icu/ohos_icu4j/data"

## 执行test262 ##
- #### 采用独立编译
1. 本地执行全量262
```
python3 ark.py x64.debug test262
```
2. 构造输入文件 test262.txt
参考 [test262参考文件](test262.txt)
3. 执行多线程测试
```
LD_LIBRARY_PATH=out/x64.debug/arkcompiler/ets_runtime/:out/x64.debug/thirdparty/bounds_checking_function/ out/x64.debug/arkcompiler/toolchain/ark_multi 6 arkcompiler/toolchain/tooling/client/ark_multi/test262.txt --icu-data-path "third_party/icu/ohos_icu4j/data" 1>/dev/null
```

## 执行本地ts ##
1. 将本地abc路径写入 input.txt
2. 执行
```
LD_LIBRARY_PATH=out/x64.debug/arkcompiler/ets_runtime/:out/x64.debug/thirdparty/bounds_checking_function/ out/x64.debug/arkcompiler/toolchain/ark_multi 6 input.txt--icu-data-path "third_party/icu/ohos_icu4j/data" 
```