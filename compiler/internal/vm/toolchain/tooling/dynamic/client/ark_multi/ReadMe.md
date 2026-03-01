# Ark multithreading command line #

## Function Introduction ##

Can start a specified number of threads, each thread starts the virtual machine and executes the test case simultaneously, used to test the garbage collection mechanism of shared memory.

## Command Help ##

ark_multi [number of threads] [execution file, internal record of abc that needs to be run, one per line] [parameter, same as ark_js_vm]

### Example ###

#### Execute the content of the input.txt file

```

a.abc

a.abc

b.abc

b.abc

c.abc

c.abc

```

#### command

ark_multi 3 input.txt --icu-data-path "third_party/icu/ohos_icu4j/data"



## Execute test262 ##

- #### Using standalone build

1. Local execution of full volume 262

```

python3 ark.py x64.debug test262

```

2. Construct input file test262.txt

Reference [test262 abc files](test262.txt)

3. Execute multi-threaded testing

```
LD_LIBRARY_PATH=out/x64.debug/arkcompiler/ets_runtime/:out/x64.debug/thirdparty/bounds_checking_function/ out/x64.debug/arkcompiler/toolchain/ark_multi 6 arkcompiler/toolchain/tooling/client/ark_multi/test262.txt --icu-data-path "third_party/icu/ohos_icu4j/data" 1>/dev/null

```



## Execute local ts ##

1. Write the local abc path into input.txt

2. Execution

```
LD_LIBRARY_PATH=out/x64.debug/arkcompiler/ets_runtime/:out/x64.debug/thirdparty/bounds_checking_function/ out/x64.debug/arkcompiler/toolchain/ark_multi 6 input.txt--icu-data-path "third_party/icu/ohos_icu4j/data" 
```