# How to run ets-func-tests and cts tests locally

## Prerequisites
```bash
$ export ARK_ROOT=/path/to/panda
$ export BUILD=/path/to/panda/build
```
## Run ets-func-tests in INT mode
```bash
$ $ARK_ROOT/tests/tests-u-runner/runner.sh --ets-func-tests --build-dir $BUILD --processes=`nproc`
```

## Run ets-func-tests in JIT mode
```bash
$ $ARK_ROOT/tests/tests-u-runner/runner.sh --ets-func-tests --build-dir $BUILD --jit --ark-args='--compiler-ignore-failures=false' --processes=`nproc`
```

## Run cts tests
```bash
$ $ARK_ROOT/tests/tests-u-runner/runner.sh --ets-cts --build-dir $BUILD --processes=`nproc`
```
